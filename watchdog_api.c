/*******************************************************************************
--------------------------------------------------------------------------------
*	file name: watchdog_api.c
*	author: Nir Shaulian
*	reviewer: Higher priest of code- Lidor Cohen

--------------------------------------------------------------------------------
*******************************************************************************/
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~includes~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include <stdio.h>
#include <stdlib.h> /*setenv, getenv */
#include <semaphore.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h> /* pid_t */
#include <unistd.h> /* fork */
#include <sys/stat.h> /* For mode constants */
#include <fcntl.h> /* For O_* constants */
#include <sys/wait.h> /* wait */


#include "scheduler.h"
#include "watchdog_api.h"

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~definitions~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef NDEBUG
#define debug_only(code) code
#else
#define debug_only(code)
#endif

#define RED_B "\033[01;31m"
#define GREEN_B "\033[01;32m"
#define YELLOW_B "\033[01;33m"
#define BLUE_B "\033[01;34m"
#define BLACK_B "\033[01;30m"
#define WHITE_B "\033[01;37m"
#define RED "\033[0;31m"
#define GREEN "\033[0;32m"
#define YELLOW "\033[0;33m"
#define RESET "\033[0m"

#define WATCHDOG_PATH "./bin/watchdog.out"
#define DOG_SEM "/wd_dog_semaphore"
#define USER_SEM "/wd_user_semaphore"

#define CHILD 0
#define PATH_SIZE 1000


typedef enum
{
	U_FIRST_RUN = 0,
	U_REBOOTED = 1,
	WDOG_MODE = 2
} dog_mode_t;

typedef enum
{
	SUCCESS = 0,
	FAILURE = 1
} status_t;

struct manager
{
	pid_t pid;
	char **argv;
	sem_t *dog_sem;
	sem_t *user_sem;
	sched_t *sched;
	ilrd_uid_t check_task;
};

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~prototypes~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

static dog_mode_t GetMode(void);
static void WatchDogSide(char *argv[], time_t check_frequency,
														time_t reboot_duration);
static wd_t UserSide(char *argv[], time_t check_frequency,
														time_t reboot_duration);
static void *Comunicator(void *params);
static pid_t Spawn(char *path, char *argv[]);
static void Comunicate(pid_t partner, sched_t *scheduler,
								time_t frequency, ilrd_uid_t *_check_task_uid);
static void Siguser1Handler(int signal);
static int CheckForMessage(void *params);
static int SendMessage(void *params);
static void ComunicatorCleanupHandler(void *params);
static int CancelPoint(void *params);


debug_only(
	static void UserPrintYellow(char *str);
	static void DogPrintGreen(char *str);
	static void PrintColor(char *str);
)


	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~variables~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

static volatile sig_atomic_t sigusr1_counter = 0;
static volatile sig_atomic_t sigusr2_counter = 0;

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~functions~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

wd_t WDStart(char *argv[], time_t check_frequency, time_t reboot_duration)
{
	//check who ran the func (dog or user)
	if (WDOG_MODE == GetMode())
	{
		debug_only(
			UserPrintYellow("Watch Dog Side\n");
		)
		WatchDogSide(argv, check_frequency, reboot_duration);
	}
	else
	{
		return UserSide(argv, check_frequency, reboot_duration);
	}
	return NULL;
}

int WDStop(wd_t watch_dog)
{
	void *res;
	pthread_cancel((pthread_t)watch_dog);
	pthread_join((pthread_t)watch_dog, &res);

	return SUCCESS;
}
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~functions~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

static wd_t UserSide(char *argv[], time_t check_frequency,
														time_t reboot_duration)
{
	pthread_t comunicator = 0;
	sem_t *dog_sem = NULL;
	sem_t *user_sem = NULL;
	sched_t *sched = NULL;
	pid_t dog_pid = 0;
	char frequency_str[PATH_SIZE];
	char reboot_dur_str[PATH_SIZE];
	struct manager *manager;

	manager = (struct manager *)malloc(sizeof(*manager));

	sprintf(frequency_str, "%ld", check_frequency);
	sprintf(reboot_dur_str, "%ld", reboot_duration);
	setenv(FREQENCY_ENV, frequency_str, 0);
	setenv(REBOOT_ENV, reboot_dur_str, 0);
	setenv(WATCHDOG_PATH_ENV, WATCHDOG_PATH, 0);
	setenv(USER_APP_PATH_ENV, argv[0], 0);

	dog_sem = sem_open(DOG_SEM, O_CREAT, 0606, 0);
	user_sem = sem_open(USER_SEM, O_CREAT, 0606, 0);

	if(U_REBOOTED == GetMode())
	{
		debug_only(
			UserPrintYellow("reboot mode\n");
		)
		dog_pid = getppid();
		kill(dog_pid, SIGUSR1);
	}
	else/* if(U_FIRST_RUN == GetMode())*/
	{
		debug_only(
			UserPrintYellow("User side, First run\n");
		)
		argv[0] = WATCHDOG_PATH;
	}
	dog_pid = Spawn(getenv(WATCHDOG_PATH_ENV), argv);

	sched = SchedCreate();
	if(NULL == sched)
	{
		return NULL;
	}
	manager->sched = sched;
	manager->pid = dog_pid;
	manager->argv = argv;
	manager->dog_sem = dog_sem;
	manager->user_sem = user_sem;

	pthread_create(&comunicator, NULL, Comunicator, manager);

	sem_wait(dog_sem);
	sem_wait(user_sem);
	return (wd_t)comunicator;
}

static void *Comunicator(void *params)
{
	sched_t *sched = NULL;
	struct sigaction sigusr1 = {0};
	sem_t *user_sem;
	sem_t *dog_sem;
	time_t frequency = 0;
	pid_t *dog = NULL;
	struct manager *manager = (struct manager *)params;
	char **argv = NULL;
	ilrd_uid_t *check_task = NULL;

	frequency = atol(getenv(FREQENCY_ENV));
	argv = manager->argv;
	dog = &(manager->pid);
	check_task = &(manager->check_task);
	sched = manager->sched;

	pthread_cleanup_push(ComunicatorCleanupHandler, manager);

	sigusr1.sa_handler = Siguser1Handler;
	sigaction(SIGUSR1, &sigusr1, NULL);
	debug_only(
		printf("Comunicator run\n");
	)

	dog_sem = manager->dog_sem;
	user_sem = manager->user_sem;

	sem_post(user_sem); // post for main thread
	printf("sem post by user_sem\n");
	while(1)
	{
		sem_post(user_sem); //one post for dog
		sem_wait(dog_sem);
		Comunicate(*dog, sched, frequency, check_task);
		debug_only(
			PrintColor("before spawn, in Comunicator");
		)
		*dog = Spawn(getenv(WATCHDOG_PATH_ENV), argv);
	}
	pthread_cleanup_pop(0);
	return NULL;
}

static void ComunicatorCleanupHandler(void *params)
{
	sem_t *user_sem;
	sem_t *dog_sem;
	struct manager *manager  = (struct manager *)params;

	SchedRemoveTask(manager->sched, manager->check_task);

	if (SUCCESS != kill(manager->pid, SIGUSR2))
	{
		kill(manager->pid, SIGUSR2);
	}
	wait(NULL);
	SchedDestroy(manager->sched);


	sem_close(manager->dog_sem);
	sem_close(manager->user_sem);

	sem_unlink(DOG_SEM);
	sem_unlink(USER_SEM);

	signal(SIGUSR1, SIG_DFL);

	free(manager);
}

static int CancelPoint(void *params)
{
	pthread_testcancel();
	return SUCCESS;
}

static void Comunicate(pid_t partner, sched_t *scheduler, time_t frequency,
													ilrd_uid_t *_check_task_uid)
{
	SchedAddTask(scheduler, (frequency / 3), (frequency / 3), SendMessage,
															(void *)partner);
	SchedAddTask(scheduler, 0, 1, CancelPoint, NULL);
	*_check_task_uid = SchedAddTask(scheduler, frequency, frequency,
													CheckForMessage, scheduler);
	SchedRun(scheduler);
	SchedFlush(scheduler);
}

static int SendMessage(void *params)
{
	pid_t partner = (pid_t)params;

	//If dog got SIGUSR2, in purpose of stop before sending
	if(0 < sigusr2_counter)
	{
		return 1;
	}
	//send signal to partner
	kill(partner, SIGUSR1);
	debug_only(
		printf("sent signal from %d to %d\n", getpid(), partner);
	)

	return 0;
}

static int CheckForMessage(void *params)
{
	sched_t *scheduler = (sched_t *)params;

	//If dog got SIGUSR2, in purpose of stop before sending
	if(0 < sigusr2_counter)
	{
		SchedStop(scheduler);
	}
	if(0 < sigusr1_counter)
	{
		sigusr1_counter = 0;
		debug_only(
			printf("%d got a signal\n", getpid());
		)
		return 0;
	}
	else
	{
		SchedStop(scheduler);
	}
	return 0;
}

static int CheckForReebotMessage(void *params)
{
	sched_t *scheduler = (sched_t *)params;
	if(0 < sigusr1_counter)
	{
		sigusr1_counter = 0;
		SchedStop(scheduler);
		return SUCCESS;
	}
	else
	{
		return FAILURE;
	}
}

static void Siguser1Handler(int signal)
{
	++sigusr1_counter;
}

static void Siguser2Handler(int signal)
{
	++sigusr2_counter;
}

static pid_t Spawn(char *path, char *argv[])
{
	pid_t pid = fork();
	if (CHILD != pid)
	{
		return pid;
	}
	else
	{
		execvp(path, argv);
		debug_only(
			printf("Spawn failed\n");
		)
		return -1;
	}
}

static dog_mode_t GetMode(void)
{
	char *result = getenv(WD_START_MODE_ENV);
	if (NULL != result)
	{
		return (dog_mode_t)atoi(result);
	}
	return U_FIRST_RUN;
}

static void WatchDogSide(char *argv[], time_t check_frequency, time_t reboot_duration)
{
	sched_t *sched = NULL;
	struct sigaction sigusr1 = {0};
	struct sigaction sigusr2 = {0};
	sem_t *user_sem;
	sem_t *dog_sem;
	pid_t user = 0;
	char wd_start_mode[PATH_SIZE] = "0";
	int status = FAILURE;
	ilrd_uid_t check_task_uid = {0};

	user = getppid();

	sched = SchedCreate();

	sigusr1.sa_handler = Siguser1Handler;
	sigaction(SIGUSR1, &sigusr1, NULL);
	sigusr1.sa_handler = Siguser2Handler;
	sigaction(SIGUSR2, &sigusr2, NULL);

	dog_sem = sem_open(DOG_SEM, O_CREAT, 0606, 0);
	user_sem = sem_open(USER_SEM, O_CREAT, 0606, 0);
	sem_post(dog_sem); // post for main thread
	sem_post(dog_sem); //one post for dog

	status = FAILURE;
	sem_wait(user_sem);
	sem_close(user_sem);
	sem_close(dog_sem);

	Comunicate(user, sched, check_frequency, &check_task_uid);

	//if we got here then user_app is dead or message sent to stop the dog.
	if (0 < sigusr2_counter)
	{
		SchedDestroy(sched);
		return;
	}
	sprintf(wd_start_mode, "%d", U_REBOOTED);
	setenv(WD_START_MODE_ENV, wd_start_mode, 1);
	while(FAILURE == status) //there is need to reboot the user process
	{
		debug_only(
			DogPrintGreen("loop after user crashed");
		)
		user = Spawn(getenv(USER_APP_PATH_ENV), argv);
		SchedAddTask(sched, reboot_duration, 0, CheckForReebotMessage, sched);
		status = SchedRun(sched);
		SchedFlush(sched);
	}

}

debug_only(
	static void UserPrintYellow(char *str)
	{
		printf(YELLOW_B"%s\n"RESET, str);
	}

	static void DogPrintGreen(char *str)
	{
		printf(GREEN_B"%s\n"RESET, str);
	}
	static void PrintColor(char *str)
	{
		printf(WHITE_B"%s\n"RESET, str);
	}
)


pid_t WDStartLite(void)
{
    pid_t pid = getpid();

    if (CHILD == fork())
    {
        return pid;
    }
    wait(NULL);
	debug_only(
		printf("start program again\n");
	)

    return WDStartLite();
}

void WDStopLite(pid_t pid)
{

	kill(pid, SIGTERM);
}

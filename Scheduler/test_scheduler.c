/*******************************************************************************
--------------------------------------------------------------------------------
*	file name: test_scheduler.c
*	author: Nir Shaulian
*	reviewer: Lia Borisover

--------------------------------------------------------------------------------
*******************************************************************************/
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~includes~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include <stdio.h> /*printf */
#include <stdlib.h>
#include <string.h> /*strlen */

#include "scheduler.h"

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~definitions~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#define BOLD printf("\033[1m")
#define BACKGROUND printf("\033[7m")
#define FLICKER printf("\033[5m")
#define UNDER_LINE printf("\033[4m")
#define ITALIC printf("\033[3m")
#define RED printf("\033[0;31m")
#define GREEN printf("\033[0;32m")
#define YELLOW printf("\033[0;33m")
#define BLUE printf("\033[0;34m")
#define RESET printf("\033[0m")
#define RUN_TEST(result, name) {\
            printf("Test %s: ", name);\
            if (result)\
            {\
                GREEN;\
                printf("Success\n");\
                RESET;\
            }\
            else\
            {\
                RED;\
                printf("Failed\n");\
                RESET;\
            }\
        }
#define TIME_TO_PUSH_GIT 7200
enum successful {SUCCEED, FAILED};
enum matching {NO, YES};

struct arr_and_len
{
    ilrd_uid_t arr[100];
    size_t len;
};
typedef struct arr_and_len arr_and_len_t;

struct sched_and_uid
{
    sched_t *sched;
    ilrd_uid_t uid;

};

typedef struct sched_and_uid sched_and_uid_t;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~Declerations~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
int PrintHour(void *no_meaning);
int PrintUid(void *arr_of_uid);
int SchedRemoveTaskWrap(void *sched_uid);
int System(void *str);
int SchedStopWrap(void *sched);
int CheckChar(void *ch);


static void PrintInColors(const char *str);


    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Main~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
int main()
{
    sched_t *my_sched = NULL;
    arr_and_len_t uids;
    time_t now = time(NULL);
    sched_and_uid_t sched_uid;
    char ch = ' ';

    uids.len = 0;

    system("clear");
    PrintInColors("Test starts now!");
    PrintInColors(ctime(&now));

    my_sched = SchedCreate();
    RUN_TEST(NULL != my_sched, "Create 1");
    RUN_TEST(YES == SchedIsEmpty(my_sched), "Create (& IsEmpty)");
    RUN_TEST(0 == SchedSize(my_sched), "Create (& Size)");

    uids.arr[0] = SchedAddTask(my_sched, 1, 600, PrintHour, my_sched );
    ++uids.len;
    uids.arr[1] = SchedAddTask(my_sched, 4, 4, PrintUid, &uids);
    ++uids.len;
    uids.arr[2] = SchedAddTask(my_sched, 4, 0, PrintHour, my_sched );
    ++uids.len;
    uids.arr[3] = SchedAddTask(my_sched, 2, 0, System, "ls");
    ++uids.len;

    uids.arr[4] = SchedAddTask(my_sched, 8, 0, SchedRemoveTaskWrap, &sched_uid);
    ++uids.len;
    sched_uid.sched = my_sched;
    sched_uid.uid = uids.arr[1];
    uids.arr[5] = SchedAddTask(my_sched, 3, 0,
                System, "echo 'what happend?\?\?'");
    ++uids.len;
    uids.arr[6] = SchedAddTask(my_sched, 5, TIME_TO_PUSH_GIT, System, "cd ~/git && git add .");
    ++uids.len;
    /*STOP task*/
    uids.arr[7] = SchedAddTask(my_sched, 9, 5, SchedStopWrap, my_sched);
    ++uids.len;
    uids.arr[8] = SchedAddTask(my_sched, 5, TIME_TO_PUSH_GIT, System,
                                    "cd ~/git && git commit -m \"now\"");
    ++uids.len;
    uids.arr[9] = SchedAddTask(my_sched, 5, TIME_TO_PUSH_GIT, System,
                                    "cd ~/git && git push -u origin master");
    ++uids.len;
    uids.arr[10] = SchedAddTask(my_sched, 6, 6, System, "xeyes");
    ++uids.len;



    RUN_TEST(SUCCEED == SchedRun(my_sched), "Run 1");
    RUN_TEST(0 != SchedSize(my_sched), "size after stop");

    SchedRemoveTask(my_sched, uids.arr[4]);
    uids.arr[11] = SchedAddTask(my_sched, 3, 0, SchedStopWrap, my_sched);
    ++uids.len;

    /*-------------- second RUN -------------*/
    RUN_TEST(SUCCEED == SchedRun(my_sched), "Run 2");

    SchedFlush(my_sched);
    RUN_TEST(0 == SchedSize(my_sched), "size after flush");
    RUN_TEST(SUCCEED == SchedRun(my_sched), "Run (after flush) 3");

    ch = 'a';
    uids.arr[12] = SchedAddTask(my_sched, 1, 2, CheckChar, &ch);
    ++uids.len;
    uids.arr[13] = SchedAddTask(my_sched, 1, 2, PrintHour, &ch);
    ++uids.len;

    printf("Now test 4 will start and countinue when you push \033[1;5;34m'a'\033[0m. \
it will stop if you push something else \n" );
    RUN_TEST(FAILED == SchedRun(my_sched), "Run 4");
    SchedDestroy(my_sched); my_sched = NULL;

    PrintInColors("End of test");


	return 0;
}


    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~functions~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
int PrintHour(void *no_meaning)
{
    time_t now = time(NULL);
    (void)(no_meaning);
    YELLOW;
    printf("PrintHour: %s\n", ctime(&now));
    RESET;

    return 0;
}

int PrintUid(void *arr_of_uid)
{
    size_t i = 0;
    printf("PrintUid starts:\n");
    BACKGROUND;
    printf("counter | pid | time \n");
    RESET;
    for(i = 0; i < ((arr_and_len_t *)(arr_of_uid))->len; ++i)
    {
        printf("  %lu     %d %ld\n",
                            ((arr_and_len_t *)(arr_of_uid))->arr[i].counter,
                            ((arr_and_len_t *)(arr_of_uid))->arr[i].pid,
                            ((arr_and_len_t *)(arr_of_uid))->arr[i].time_stamp);
    }
    putc('\n', stdout);

    return 0;
}

int SchedRemoveTaskWrap(void *sched_uid)
{
    SchedRemoveTask(((sched_and_uid_t *)sched_uid)->sched,
                    ((sched_and_uid_t *)sched_uid)->uid);

    return SUCCEED;
}

int System(void *str)
{
    system((char*)(str));

    return SUCCEED;
}

int SchedStopWrap(void *sched)
{
    return SchedStop((sched_t*)sched);
}

int CheckChar(void *ch)
{
	char key = '\0';
	scanf(" %c", &key);

	return !(*(char *)ch == key);
}
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~static functions~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static void PrintInColors(const char *str)
{
    size_t i = 0;

    for(i = 0; i < strlen(str); ++i)
    {
        switch (i%5) {
            case 0:
                YELLOW;
                printf("%c", str[i]);
                break;
            case 1:
                BLUE;
                printf("%c", str[i]);
                break;
            case 2:
                RESET;
                printf("%c", str[i]);
                break;
            case 3:
                RED;
                printf("%c", str[i]);
                break;
            case 4:
                GREEN;
                printf("%c", str[i]);
                break;
        }
        RESET;
    }
    putc('\n', stdout);

}

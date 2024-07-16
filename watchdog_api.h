#ifndef __WATCHDOG_API_H__
#define __WATCHDOG_API_H__

#include <time.h>/* time_t */

#define WD_START_MODE_ENV "WD_START_MODE_ENV"
#define FREQENCY_ENV "WD_FREQENCY_ENV"
#define REBOOT_ENV "WD_REBOOT_ENV"
#define WATCHDOG_PATH_ENV "WD_WATCHDOG_PATH"
#define USER_APP_PATH_ENV "WD_USER_APP_PATH"

typedef void* wd_t;

/*
Synopsis:
    WDStart keep the caller process running even if it terminates
    abnormaly in the middle of the process.
    WDStop stops the watchdog.
*/
wd_t WDStart(char *argv[], time_t check_frequency, time_t reboot_duration);
int WDStop(wd_t watch_dog);

/*
Synopsis:
    WDStartLite keep the caller process running even if it terminates
    abnormaly in the middle of the process.
    WDStop stops the watchdog.
    lite version: No security for the dog.
*/
pid_t WDStartLite(void);
void WDStopLite(pid_t pid);

#endif   /*__WATCHDOG_API_H__*/
/*
Description:
*            The WDStart function verifies the execution of a segment in the process,
            between the WDstart command and the WDStop command.
*            If the program collapses between start and stop, the function will
            run the program again (from the beginning).
*            The dog is safe too.
*            The environment variables defined as WATCHDOG_PATH_ENV &
            USER_APP_PATH_ENV must be entered using setenv().
*            It is necessary to compile the file 'watchdog.out'.
Parameters:
*           @argv: the 'argv' of the runnung process.
*            @check_frequency: Time interval to confirm a life signal on the
                part of the process and on the part of the dog.
                (The time of sending the messages will be one third of the time
                entered).
*            @reboot_duration: Interval to confirm a life signal from a rebooted
                process. (How long it takes for the process to reach the
                'WDStart' line).
Return:
            A 'wd_t' type value, to pass as argument to WDStop.
            if failed, the func return NULL.
Undefined behavior:
            frequency must be 3 or more.
Error:
            if both (dog and user app) crashing in same time.
*/

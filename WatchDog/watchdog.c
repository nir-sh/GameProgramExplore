/*******************************************************************************
--------------------------------------------------------------------------------
*	file name: watchdog.c
*	author: Nir Shaulian
*	reviewer:

--------------------------------------------------------------------------------
*******************************************************************************/
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~includes~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include <stdio.h>
#include <stdlib.h>

#include "watchdog_api.h"

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~definitions~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef NDEBUG 
#define debug_only(code) code
#else
#define debug_only(code)
#endif

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


	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~functions~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
int main(int argc, char *argv[])
{
	char wd_start_mode[32] = "0";
	time_t check_frequency = atoi(getenv(FREQENCY_ENV));
	time_t reboot_duration = atoi(getenv(REBOOT_ENV));

	debug_only(
		printf("Dog begin\n");
	)
	sprintf(wd_start_mode, "%d", WDOG_MODE);
	setenv(WD_START_MODE_ENV, wd_start_mode, 1);

	WDStart(argv, check_frequency, reboot_duration);

	return 0;
}

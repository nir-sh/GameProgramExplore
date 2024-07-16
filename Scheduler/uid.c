/*******************************************************************************
--------------------------------------------------------------------------------
*	file name: uid.c
*	author: Nir Shaulian
*	reviewer: Master Yoni Ladijensky

--------------------------------------------------------------------------------
*******************************************************************************/
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~includes~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include <assert.h> /* assert */
#include <unistd.h> /* getpid */

#include "uid.h"
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~definitions~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~functions~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

ilrd_uid_t UIDCreate(void)
{
    static size_t counter = 1;
    ilrd_uid_t new = {0};

    new.time_stamp = time(NULL);
    new.pid = getpid();
    new.counter = counter;
    if(-1 == new.time_stamp)
    {
        return UIDGetBadUID();
    }
    ++counter;

    return new;
}

int UIDIsSame(ilrd_uid_t one, ilrd_uid_t other)
{
    return (one.time_stamp == other.time_stamp && one.pid == other.pid &&
            one.counter == other.counter);
}

ilrd_uid_t UIDGetBadUID(void)
{
    ilrd_uid_t bad = {-1, 0, 0};

    return bad;
}

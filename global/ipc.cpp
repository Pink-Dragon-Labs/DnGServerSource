/*
	interprocess communication (ipc) class library
	author: Stephen Nichols
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <poll.h>
#include <limits.h>
#include <sys/time.h>

#include "system.hpp"

int gMaxConnectCount = 2000;

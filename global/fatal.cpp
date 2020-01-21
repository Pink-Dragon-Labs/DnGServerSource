/*
	fatal error handling routines
	author: Stephen Nichols
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>

#include "system.hpp"

void fatal ( const char *format, ... )
{
	char output[100000];
	va_list args;

	va_start ( args, format );
	vsprintf ( sizeof ( output ), output, format, args );
	va_end ( args );

//	int display = sysLogDisplay;
//	sysLogDisplay = 0;

	logDisplay ( "fatal error: %s", output );
	logInfo ( _LOG_ALWAYS, "fatal error: %s", output );

//	sysLogDisplay = display;

	crash();
}

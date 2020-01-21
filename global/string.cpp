#include "system.hpp"

#undef sprintf
#undef vsprintf

#include <stdio.h>
#include <stdlib.h>

int db_sprintf ( int size, char *str, const char *format, ... )
{
	va_list args;
	va_start ( args, format );

	int ret = db_vsprintf ( size, str, format, args );

	va_end ( args );

	return ret;
}

int db_vsprintf ( int size, char *str, const char *format, va_list args )
{
#if 1
	char *dbStrBuffer = (char *)malloc ( size * 2 );

	dbStrBuffer[size] = 0;
	int ret = vsprintf ( dbStrBuffer, format, args );

	if ( dbStrBuffer[size] != 0 )
		fatal ( "string bounds overrun detected (format = '%s')", format );

	strcpy ( str, dbStrBuffer );

	free ( dbStrBuffer );

	return ret;
#else
	return vsprintf ( str, format, args );
#endif
}

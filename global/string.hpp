#ifndef _STRING_HPP_
#define _STRING_HPP_

#include <stdarg.h>

//#ifndef _NO_SPRINTF_OVERRIDE
#undef sprintf
#define sprintf db_sprintf

#undef vsprintf
#define vsprintf db_vsprintf

int db_sprintf ( int size, char *str, const char *format, ... );
int db_vsprintf ( int size, char *str, const char *format, va_list ap );
//#endif

#endif

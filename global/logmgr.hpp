/* 
	log file manager class library
	author: Stephen Nichols
*/

#ifndef _LOGMGR_HPP_
#define _LOGMGR_HPP_

#include "ipc.hpp"
#include "window.hpp"
#include "list.hpp"
#include "msgs.hpp"
#include "hash.hpp"

#include "debug.hpp"
#include "new.hpp"
#include "malloc.hpp"

#define _LOG_ALWAYS 		0xffffffff
#define _LOG_NEVER		0
#define _LOG_DETAILED		1
#define _LOG_FUNCTIONS		2
#define _LOG_GENERAL		4
#define _LOG_INTRICATE		8

extern char *sysLogAppName;
extern int sysLogActive, sysLogDisplay, sysLogAllowed, sysLogLevel;

void logInit ( void );
void logInfo ( int level, const char *format, ... );
void logInfoStr ( char *str );
void logDisplay ( const char *format, ... );

#endif

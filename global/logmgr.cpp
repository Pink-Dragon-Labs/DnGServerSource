/*
	log file manager class library 
	author: Stephen Nichols
*/

#include "system.hpp"
#include <stdarg.h>

class LogData : public ListObject
{
public:
	LogData ( char *name, char *data ) {
		filename = strdup ( name );
		text = strdup ( data );
	};

	virtual ~LogData() {
		free ( filename );
		free ( text );
	};

	char *filename, *text;
};

LinkedList logList;

char *sysLogAppName = "system";
int sysLogActive = FALSE, sysLogDisplay = TRUE, sysLogInitted = FALSE, sysLogAllowed = TRUE, sysLogLevel = _LOG_NEVER;

void logInit ( void ) 
{
	if ( sysLogAllowed == FALSE )
		return;

	int allowed = sysLogAllowed;
	sysLogAllowed = FALSE;

	sysLogActive = TRUE;

	sysLogAllowed = allowed;
	sysLogInitted = TRUE;
}

void logInfo ( int level, const char *format, ... )
{
	if ( level & sysLogLevel ) {
		char output[10240];
		va_list args;

		va_start ( args, format );
		vsprintf ( sizeof ( output ), output, format, args );

		logInfoStr ( output );
	} 
}

void logDisplay ( const char *format, ... )
{
	char output[10240];
	va_list args;

	va_start ( args, format );
	vsprintf ( sizeof ( output ), output, format, args );

	printf ( "%s\n", output );
}

void logInfoStr ( char *str )
{
	static int firstCall = 1;

	if ( sysLogAllowed == FALSE )
		return;

	if ( sysLogInitted == FALSE )
		logInit();

	if ( firstCall ) {
		firstCall = 0;

		int display = sysLogDisplay;
		sysLogDisplay = 0;

		logInfoStr ( "***************** START LOG *****************" );

		sysLogDisplay = display;
	}

	static int busy = 0;

	if ( busy ) 
		return;

	busy++;

	int display = sysLogDisplay, active = sysLogActive;
	char *name = sysLogAppName;

	if ( sysLogDisplay )
		printf ( "%s\n", str );

	if ( active ) {
		char filename[1024], text[10240];
		char *timeAsStr = timeToStr();
		sprintf ( sizeof ( filename ), filename, "../logs/%s.log", name );
		sprintf ( sizeof ( text ), text, "%s: %s\n", timeAsStr, str );

		free ( timeAsStr );

		File *file = new File ( filename );

		if ( file->isOpen() ) {
			file->seek ( file->size() );
			file->printf ( "%s", text );
			file->close();
		}

		delete file;
	}

	busy--;
} 	

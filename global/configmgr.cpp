//
// configmgr
//
// This file contains the ConfigMgr class and supporting structures.
//
// author: Stephen Nichols
//

#include <ctype.h>

#include "logmgr.hpp"
#include "configmgr.hpp"
#include "tools.hpp"
#include "fatal.hpp"
#include "file.hpp"

ConfigMgr::ConfigMgr()
{
	varCount = 0;
}

ConfigMgr::~ConfigMgr()
{
	unload();
}

// this member is called to load a configuration file
int ConfigMgr::load ( const char *format, ... )
{
	char name[1024];

	va_list args;
	va_start ( args, format );
	vsprintf ( name, format, args );
	va_end ( args );

	if ( !exists ( name ) ) 
		return -1;

	File file;
	file.open ( name );

	char *buffer = (char *)malloc ( file.size() ), *bufPtr = buffer, str[1024];
	int bufferSize = file.size();

	// read the buffer
	file.read ( buffer, bufferSize );

	while ( bufferSize > 0 ) {
		// read the line
		bufgets ( str, &bufPtr, &bufferSize );

		// look for a comment
		char *comment = strchr ( str, '#' );

		if ( comment )
			*comment = 0;

		// trim the leading whitespace
		char *start = str;

		while ( *start && isspace ( *start ) )
			start++;

		// get the string length
		int length = strlen ( start );

		if ( length < 1 )
			continue;

		// trim the trailing whitespace
		char *end = &start[length-1];

		while ( *end && isspace ( *end ) ) {
			*end = 0;
			end--;
			length--;
		}

		// skip empty lines
		if ( length < 1 )
			continue;

		// get the separator pointer
		char *space = strchr ( str, ' ' );

		// skip lines with no space
		if ( !space )
			continue;

		// null terminate the space
		*space = 0;

		// point to the variable name and value
		char *namePtr = start;
		char *valuePtr = space + 1;

		// check for too many variables
		if ( varCount == _CONFIGMGR_MAX_ENTRIES ) 
			fatal ( "Too many variables in config file: %s", name );

		variables[varCount] = strdup ( namePtr );
		values[varCount] = strdup ( valuePtr );
		varCount++;
	}

	free ( buffer );
}

// this member is called to unload a configuration file
void ConfigMgr::unload ( void )
{
	for ( int i=0; i<varCount; i++ ) {
		free ( variables[i] );
		free ( values[i] );
	}

	varCount = 0;
}

// this member is called to get a value from the config file
char *ConfigMgr::get ( char *varName, int doFatal )
{
	for ( int i=0; i<varCount; i++ ) {
		if ( !strcmp ( variables[i], varName ) )
			return values[i];
	}

	// fatal out if asked to
	if ( doFatal ) 
		fatal ( "Config file variable '%s' not found.", varName ); 

	return NULL;
}

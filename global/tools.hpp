/*
	generic unix SVR4 tool functions
	(most code is taken straight from Advanced Programming in the UNIX environment)
	
	typed by: Stephen Nichols 
*/

#ifndef _TOOLS_HPP_
#define _TOOLS_HPP_

#include <sys/sysinfo.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <poll.h>

#include "list.hpp"

#include "debug.hpp"
#include "new.hpp"
#include "malloc.hpp"
#include "memorypool.hpp"
#include "sql.hpp"

extern MemoryAllocator _StringObjectAllocator;

#undef new

class StringObject : public ListObject
{
public:
	char *data;
	int index, length;

	StringObject() { data = NULL; length = 0; index = 0; }

	StringObject ( const char* newData ) {
		data = strdup( newData );
		length = strlen ( newData );
		index = 0;
	}

	StringObject ( const char* newData, int size ) {
		data = strdup( newData );
		length = size;
		index = 0;
	}

	virtual ~StringObject() {
		if ( data != NULL ) {
			free ( data );
			data = NULL;
		}
	}

	void* operator new ( size_t size, const char* file, const int nLine ) {
		return _StringObjectAllocator.allocate();
	}

	void operator delete ( void *ptr ) {
		_StringObjectAllocator.deallocate ( (char *)ptr );
	}
};

#define new new( __FILE__, __LINE__ )

/* 
	isastream: function to check if a filedes is a stream or not. 
	(APitUE page 388)
*/
int isastream ( int fd );

/*
	daemonInit: function to initialize the current process as a daemon.
	(APitUE page 418)
*/
int daemonInit ( void );


/*
	return the current time in string format
*/
char* timeToStr ( void );

/*
	use the usleep function to wait some fraction of a second. fsleep ( 10 )   
	would wait 1/10th of a second.
*/
void fsleep ( int amount );


/*
	convert endian types
*/
void convertEndian ( char *buffer, int size );

/* create a socket and bind it to a specific service name on this machine */
int OpenPortTCP ( char *service, char* pHost = NULL );

/* accept a connection from a currently open socket */
int AcceptPortTCP ( int handle, sockaddr_in *address = NULL );

/* create a socket and connect it to a currently open socket on another machine */
int ConnectPortTCP ( char *machine, char *service );

/* set the flags of a filedes */
int SetPortTCP ( int fd, int flags );

/* block this thread */
void block ( void );

/* check to see if a file exists */
int exists ( char *name, ... );

LinkedList *buildTokenList ( char *str, char *separators = " \t\n\r\0x1a" );

int getseconds ( void );

// this routine is not integer compatible with getseconds
int getsecondsfast ( void );

int random ( int start, int end );
double random ( double low, double high );
char *strlower ( char *str );

void sleep_ms ( unsigned int val );

int bufgets ( char *dest, char **buffer, int *size, int maxReadSize = 0 );

inline int bufgetint ( char *str, char **buffer, int *size ) {

// new test to make sure we have a valid string to atoi.

	if ( bufgets ( str, buffer, size, 12 ) ) 
		return atoi ( str );
	else 
		return 0;
};

#define crash(x) _crash ( __FILE__, __LINE__ )
void _crash ( const char *file, const int line ); 

int getDistance ( int x1, int y1, int x2, int y2 );

void encodeString ( char *source, char *dest );
void decodeString ( char *source, char *dest );

int findFile ( char *directory, char *file, char *exclude );

int SCIRandom ( int *seed, int start, int end );
int SCIRandom ( int *seed );

int getMonth ( void );
int getYear ( void );

// this strips the whitespace from the front and back of the given string
void stripWhitespace ( char *string );

// this strips all non-digit characters from the given string 
void stripNonDigits ( char *string );

extern int gCurTime;

// converts an IP address character string to an integer
int csIPtoInt ( char* pIP );

// converts an IP address integer to a character string
char* csIPtoChar ( int nIP );

// checks to see if the database is up
char* checkDBStatus( SQLDatabase* sql );

#endif

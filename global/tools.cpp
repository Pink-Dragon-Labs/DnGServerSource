/*
	generic unix SVR4 tool functions
	(most code is taken straight from Advanced Programming in the UNIX environment)
	
	typed by: Stephen Nichols 
*/

#include "system.hpp"

MemoryAllocator _StringObjectAllocator ( sizeof ( StringObject ), 1024 );

/* 
	isastream: function to check if a filedes is a stream or not. 
	(APitUE page 388)
*/
int isastream ( int fd )
{
	return ( ioctl ( fd, I_CANPUT, 0 ) != -1 );
}

/*
	daemonInit: function to initialize the current process as a daemon.
	(APitUE page 418)
*/
int daemonInit ( void )
{
	/* fork a new child process */
	pid_t pid = fork();

	switch ( pid ) {
		/*
			if the fork fails, return with an error
		*/
		case -1:
			return ( -1 );
			break;
			
		/*
			if I am now the child process, do my thing 
		*/
		case 0:
			/* become session leader */
			setsid();

			/* go to the root */
			chdir ( "/" );

			/* change file mask */
			umask ( 0 );

			break;

		/*
			handle parent side of the fork() 
		*/
		default:
			exit ( 0 );
			break;
	}

	return ( 0 );
}

char* timeToStr ( void )
{
	time_t curTime;

	time ( &curTime );
	char *str = strdup ( ctime ( &curTime ) );

	char *ptr = strchr ( str, '\n' );

	//leak here if there are characters past the \n
	if ( ptr ) *ptr = '\0';

	return str;
}

int getMonth ( void )
{
	struct tm *timeData;
	time_t curTime;

	time ( &curTime );
	timeData = localtime ( &curTime );

	return ( timeData->tm_mon );
}

int getYear ( void )
{
	struct tm *timeData;
	time_t curTime;

	time ( &curTime );
	timeData = localtime ( &curTime );

	return ( timeData->tm_year );
}

void fsleep ( int amount )
{
	int time = 1000000 / amount;
//	usleep ( time );
}

void convertEndian ( char *buffer, int size )
{
	for ( int i=0; i<size; ) {
		char temp = buffer[i];
		buffer[i] = buffer[i+3];
		buffer[i+3] = temp;

		temp = buffer[i+1];
		buffer[i+1] = buffer[i+2];	
		buffer[i+2] = temp;

		i += 4;
	}
}

/* create a socket and bind it to a specific service name on this machine */
int OpenPortTCP ( char *service, char* pHost )
{
	int handle = socket ( AF_INET, SOCK_STREAM, 0 );
	sockaddr_in address;
	char hostname[128];

	if ( handle > -1 ) {
		int temp = 1;

		setsockopt ( handle, SOL_SOCKET, SO_REUSEADDR, (char *)&temp, sizeof ( temp ) );
		setsockopt ( handle, SOL_SOCKET, SO_KEEPALIVE, (char *)&temp, sizeof ( temp ) );

		struct linger ld;
		ld.l_onoff = 0;
		ld.l_linger = 0;

		setsockopt ( handle, SOL_SOCKET, SO_LINGER, (char *)&ld, sizeof ( ld ) );

		struct hostent *host;
		
	 	if ( pHost ) {
			host = gethostbyname ( pHost );
		} else {
			gethostname( hostname, 127 );
			host = gethostbyname ( hostname );
		}
		
		struct servent *serv = getservbyname ( service, "tcp" );

		if ( host && serv ) {
			address.sin_port = serv->s_port;
			address.sin_family = AF_INET;	
			address.sin_addr.s_addr = (int)*((int*)host->h_addr); 

			if ( bind ( handle, (sockaddr *)&address, sizeof ( address ) ) == -1 ) {
				logInfo ( _LOG_ALWAYS, "%s(%d): OpenPortTCP: bind call failed (handle == %d, errno == %d)", __FILE__, __LINE__, handle, errno );

				close ( handle );
				handle = -1;
			}

			if ( listen ( handle, 128 ) == -1 ) {
				logInfo ( _LOG_ALWAYS, "%s(%d): OpenPortTCP: listen call failed (handle == %d, errno == %d)", __FILE__, __LINE__, handle, errno );
				close ( handle );
				handle = -1;
			}
		} else {
			logInfo ( _LOG_ALWAYS, "%s(%d): OpenPortTCP: can't find hostent ( %s ) or servent ( %s ) (hostent == %p, servent == %p)", __FILE__, __LINE__, hostname, service, host, serv );
			close ( handle );
			handle = -1;
		}
	}

	return handle;
}

/* accept a connection from a currently open socket */
int AcceptPortTCP ( int handle, sockaddr_in *addr )
{
	int retHandle = -1;

	sockaddr_in address;
	int len = sizeof ( address );

	retHandle = accept ( handle, (sockaddr *)&address, (socklen_t *) &len );

	if ( addr )
		memcpy ( addr, &address, sizeof ( address ) );

	int temp = 1;

	setsockopt ( retHandle, SOL_SOCKET, SO_REUSEADDR, (char *)&temp, sizeof ( temp ) );
	setsockopt ( retHandle, SOL_SOCKET, SO_KEEPALIVE, (char *)&temp, sizeof ( temp ) );

//	temp = 32 * 1024;
//	setsockopt ( handle, SOL_SOCKET, SO_SNDBUF, (char *)&temp, sizeof ( temp ) );
//	setsockopt ( handle, SOL_SOCKET, SO_RCVBUF, (char *)&temp, sizeof ( temp ) );

	struct linger ld;
	ld.l_onoff = 0;
	ld.l_linger = 0;

	setsockopt ( retHandle, SOL_SOCKET, SO_LINGER, (char *)&ld, sizeof ( ld ) );

	int mode;
	ioctl ( retHandle, I_GWROPT, &mode );
	mode &= ~2;
//	mode &= ~SNDPIPE;
	ioctl ( retHandle, I_SWROPT, mode );

	return retHandle;
}

/* create a socket and connect it to a currently open socket on another machine */
int ConnectPortTCP ( char *machine, char *service )
{
	int handle = socket ( AF_INET, SOCK_STREAM, 0 );
	sockaddr_in address;

	if ( handle > -1 ) {
		int temp = 1;

		setsockopt ( handle, SOL_SOCKET, SO_REUSEADDR, (char *)&temp, sizeof ( temp ) );
		setsockopt ( handle, SOL_SOCKET, SO_KEEPALIVE, (char *)&temp, sizeof ( temp ) );
//		temp = 32 * 1024;
//		setsockopt ( handle, SOL_SOCKET, SO_SNDBUF, (char *)&temp, sizeof ( temp ) );
//		setsockopt ( handle, SOL_SOCKET, SO_RCVBUF, (char *)&temp, sizeof ( temp ) );

		struct linger ld;
		ld.l_onoff = 0;
		ld.l_linger = 0;

		setsockopt ( handle, SOL_SOCKET, SO_LINGER, (char *)&ld, sizeof ( ld ) );

		int mode;
		ioctl ( handle, I_GWROPT, &mode );
//		mode &= ~SNDPIPE;
		mode &= ~2;
		ioctl ( handle, I_SWROPT, mode );

		struct hostent *host = gethostbyname ( machine );
		struct servent *serv = getservbyname ( service, "tcp" );

		if ( host && serv ) {
			address.sin_port = serv->s_port;
			address.sin_family = AF_INET;	
			address.sin_addr.s_addr = (int)*((int*) host->h_addr); 
//			address.sin_addr.s_net = (char)*(host->h_addr); 
//			address.sin_addr.s_host = (char)*(host->h_addr + 1); 
//			address.sin_addr.s_lh = (char)*(host->h_addr + 2); 
//			address.sin_addr.s_impno = (char)*(host->h_addr + 3); 

			if ( connect ( handle, (sockaddr *)&address, sizeof ( address ) ) == -1 ) {
//				logInfo ( _LOG_ALWAYS, "%s(%d): ConnectPortTCP: connect failure (handle == %d)", __FILE__, __LINE__, handle );
				close ( handle );
				handle = -1;	
			}
		} else {
//			logInfo ( _LOG_ALWAYS, "%s(%d): ConnectPortTCP: can't find hostent or servent (hostent == %p, servent == %p)", __FILE__, __LINE__, host, serv );
			close ( handle );
			handle = -1;
		}
	}

	return handle;
}

int SetPortTCP ( int fd, int flags )
{
	return ( fcntl ( fd, F_SETFL, fcntl ( fd, F_GETFL ) | flags ) );
}

void block ( void )
{
}

LinkedList *buildTokenList ( char *str, char *separators )
{
	LinkedList *tokens = new LinkedList;

	// scan for a token
	char *ptr = str, ch, *sepPtr = separators, token[2048], *tokenPtr= token;
	int theSize = strlen ( separators ), index = 0, tokenSize = 0, tokenIdx = 0;

	while ( ch = *ptr ) {
		tokenSize++;
		index++;

		if ( !isalnum ( ch ) ) {
			// this is a quoted string, read until the next string or end of line
			if ( ch == '"' ) {
				ptr++;

				while ( (ch = *ptr) && (ch != '"') ) {
					if ( ch == '\\' ) {
						ptr++;
						ch = *ptr;

						if ( ch == 'n' )
							ch = 13;
					}

					*tokenPtr++ = ch;
					tokenSize++;
					index++;
					ptr++;
				}

				// advance past last quote
				if ( ch == '"' )
					ptr++;

				tokenSize--;
				token[tokenSize] = 0;

				StringObject *string = new StringObject ( token, tokenSize );
				string->index = tokenIdx;

				tokens->add ( string );
				tokenPtr = token;
				tokenSize = 0;
				tokenIdx = index;

				continue;
			}

			// is the current character a separator
			sepPtr = separators;

			for ( int i=0; i<theSize; i++ ) {
				if ( ch == *sepPtr++ ) {
					// we got a separator... see if it's long enough to be a token
					if ( tokenSize > 1 ) {
						tokenSize--;
						token[tokenSize] = 0;

						StringObject *string = new StringObject ( token, tokenSize );
						string->index = tokenIdx;

						tokens->add ( string );
					}
	
					tokenPtr = token;
					tokenSize = 0;
					tokenIdx = index;
	
					break;
				}
			}
		}

		if ( tokenSize )
			*tokenPtr++ = ch;

		ptr++;

		if ( !*ptr ) {
			if ( tokenSize ) {
				token[tokenSize] = 0;

				StringObject *string = new StringObject ( token, tokenSize );
				string->index = tokenIdx;

				tokens->add ( string );
			}
		}
	}

	return tokens;
}

int getseconds ( void )
{
	return (int)time ( NULL );
}

// this function is not integer compatible with getseconds and will return 
// different values...
int getsecondsfast ( void ) {
	return (int) time( NULL );
//	return clock() / CLOCKS_PER_SEC;
}

int random ( int start, int end )
{
	static int initted = 0;

	if ( !initted ) {
		srand ( getseconds() );
		initted = 1;
	}

	int range = (end - start) + 1;

	if ( range )
		return (rand() % range) + start;
	
	return start;
}

double random ( double low, double high )
{
	static int initted = 0;

	if ( !initted ) {
		srand ( getseconds() );
		initted = 1;
	}

	return ((double)rand() / RAND_MAX) * (high-low) + low;
}

int exists ( char *name, ... )
{
	char filename[1024];
	va_list args;

	va_start ( args, name );
	vsprintf ( sizeof ( filename ), filename, name, args );
	va_end ( args );

	int fd = open ( filename, O_RDONLY );

	if ( fd != -1 ) {
		close ( fd );
		return 1;
	}

	return 0;
}

char *strlower ( char *str )
{
	char *ret = str;

	while ( *str ) {
		*str = tolower ( *str );
		str++;
	}

	return ret;
}

void sleep_ms ( unsigned int nmsecs )
{
	struct pollfd dummy;
	poll ( &dummy, 0, nmsecs );
}

int bufgets ( char *dest, char **data, int *theSize, int maxReadSize )
{
	int size = *theSize;

	if ( size < 1 )
		return 0;

	if ( !maxReadSize )
		maxReadSize = size;
	else
		maxReadSize--;

	char *ptr = *data, *oldPtr = ptr, *destPtr = dest; 
	int gotLine = 0, bytesCopied = 0;

	while ( size > 0 ) {
		char ch = *ptr++;

		if ( ch == '\n' ) {
			gotLine = 1;
			size--;
			break;
		}

		if ( ch != 0 ) {	
			if ( bytesCopied < maxReadSize ) {
				*destPtr++ = ch;
				bytesCopied++;
			}
		}

		size--;
	}

	// null terminate the dest data
	*destPtr = 0;

	*data = ptr;
	*theSize = size;

	return gotLine || !size;
}

int getDistance ( int x1, int y1, int x2, int y2 )
{
	double dx = fabs ( (double)(x2-x1) );
	double dy = fabs ( (double)(y2-y1) );

	return (int) sqrt ( (dx*dx) + (dy*dy) );
}

void encodeString ( char *source, char *dest )
{
	char *srcPtr = source, *destPtr = dest;

	while ( *srcPtr ) {
		switch ( *srcPtr ) {
			case '\n': {
				*destPtr++ = '\\';
				*destPtr++ = 'n';
			}

			break;

			case '\t': {
				*destPtr++ = '\\';
				*destPtr++ = 't';
			}

			break;

			case '\r': {
				*destPtr++ = '\\';
				*destPtr++ = 'r';
			}

			break;

			case '\\': {
				*destPtr++ = '\\';
				*destPtr++ = '\\';
			}

			break;

			default: {
				if ( isprint ( *srcPtr ) )
					*destPtr++ = *srcPtr;
			}
		}

		srcPtr++;
	}

	*destPtr = 0;
}

void decodeString ( char *source, char *dest )
{
	char *srcPtr = source, *destPtr = dest;

	while ( *srcPtr ) {
		switch ( *srcPtr ) {
			case '\\': {
				srcPtr++;

				switch ( *srcPtr ) {
					case 'n': {
						*destPtr++ = '\n';
					}

					break;

					case 't': {
						*destPtr++ = '\t';
					}

					break;

					case 'r': {
						*destPtr++ = '\r';
					}

					break;

					case '\\': {
						*destPtr++ = '\\';
					}

					break;
				}

				srcPtr++;
			}

			break;

			default: {
				*destPtr++ = *srcPtr++;
			}
		}
	}

	*destPtr = 0;
}

int SCIRandom ( int *seed, int start, int end )
{
	const unsigned int Generator = 0x00955b29;  // 0x0019660d;		// Old value 0x015a7c4d;

	int theSeed = *seed;

	if ( !theSeed ) 
		theSeed = getseconds();

	unsigned int range = (unsigned int) (end - start) + 1;

//	theSeed = theSeed * Generator + 1;			// Old value
	theSeed = theSeed * Generator + 0x118e6ef;  // 0x3c6ef35f;

	unsigned int tmp = (theSeed & 0x00ffff00L) >> 8;
	tmp *= range;
	tmp >>= 16;
	tmp += start;

	// update seed value
	*seed = theSeed;

	return tmp;
}

int SCIRandom ( int* seed ) {
	const unsigned int Generator = 0x00955b29;  //0x0019660d;

	*seed = *seed * Generator + 0x0118e6ef;  //0x3c6ef35f;

	return *seed;
}

void _crash ( const char *file, const int line )
{
	extern void sigcrash ( int );
	logCrash ( "forced crash at %s:%d\n", file, line );
	sigcrash ( 123 );
	abort();
}

int findFile ( char *directory, char *file, char *exclude )
{
	DIR *dir = opendir ( directory );

	if ( dir ) {
		struct dirent *dirp;

		while ( (dirp = readdir ( dir )) != NULL ) {
			char *ptr = strstr ( dirp->d_name, file );
			char *ePtr = strstr ( dirp->d_name, exclude );

			if ( ptr && !ePtr )
				return 1;
		}

		closedir ( dir );
	}

	return 0;
}

// this strips the whitespace from the given string
void stripWhitespace ( char *string )
{
	// don't process NULL strings
	if ( !string )
		return;

	// get the string length
	int length = strlen ( string );

	// don't process no-length strings
	if ( !length )
		return;

	char* ptr = string;
	char* ptr2 = string;

	while ( ptr && *ptr == ' ' )
		ptr++;

	while ( *ptr ) {
		*ptr2 = *ptr;
		ptr2++;
		ptr++;
	}

	ptr2--;

	while ( ptr2 > string && *ptr2 == ' ' ) {
		ptr2--;
	}

	ptr2[1] = 0;
}

// this strips all non-digits from the given string
void stripNonDigits ( char *str )
{
	if ( !str )
		return;

	char *copy = strdup ( str );
	char *src = copy, *dst = str;

	while ( *src ) {
		if ( isdigit ( *src ) )
			*dst++ = *src;

		src++;
	}

	*dst = 0;

	free ( copy );
}

// converts an IP address character string to an integer (as stored in database)
int csIPtoInt ( char* pIP ) {
  
	  int nResult = 0;
	  char* pLoc = pIP;

	  nResult = atoi ( pLoc );
	  nResult <<= 8;

	  while (*pLoc && *pLoc != '.') {
		  pLoc++;
	  }
	  if (!*pLoc) {
		  return 0;
	  }
	  pLoc++;

	  nResult += atoi ( pLoc );
	  nResult <<= 8;

	  while (*pLoc && *pLoc != '.') {
		  pLoc++;
	  }
	  if (!*pLoc) {
		  return 0;
	  }
	  pLoc++;

	  nResult += atoi ( pLoc );
	  nResult <<= 8;

	  while (*pLoc && *pLoc != '.') {
		  pLoc++;
	  }
	  if (!*pLoc) {
		  return nResult;
	  }
	  pLoc++;

	  nResult += atoi ( pLoc );

	  return nResult;
}

// converts an IP address integer (as stored in database) to a character string (xxx.xxx.xxx.xxx)
char* csIPtoChar ( int nIP ) {
	static char sIP[16];

	sprintf ( 15, sIP, "%d.%d.%d.%d",  ( ( nIP >> 24 ) & 0x000000FF ), ( (nIP >> 16) & 0x000000FF ), ( (nIP >> 8) & 0x000000FF ), ( nIP & 0x000000FF ) );

	return sIP;
}

char* checkDBStatus( SQLDatabase* sql ) {
	char* pResult = NULL;

	SQLResponse* qStatus = sql->query( "select message from messages where type = 'database'" );

	if ( qStatus ) {
		pResult = strdup( qStatus->table( 0, 0 ) );
		delete qStatus;
	}

	return pResult;
}

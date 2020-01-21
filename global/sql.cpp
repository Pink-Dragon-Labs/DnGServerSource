//
// SQL.CPP
//
// Mini-SQL interface class library.
//
// Author: Stephen Nichols
//

#include <errmsg.h>
#include "system.hpp"
#include "sql.hpp"

// 
// SQLResponse: This class represents and provides interface to responses from
// the SQL server.
//

SQLResponse::SQLResponse ( MYSQL *connection ) {
	MYSQL_RES *response = mysql_store_result ( connection );
	rowData = NULL;
	lengths = NULL;
	error = NULL;
	errno = 0;

	if ( response ) {
		rows = (int)mysql_num_rows ( response );
		cols = (int)mysql_num_fields ( response );
		rowData = (char ***)malloc ( sizeof ( char ** ) * rows );
		lengths = (int **)malloc ( sizeof ( int * ) * rows );

		for ( int r=0; r<rows; r++ ) {
			MYSQL_ROW row = mysql_fetch_row ( response );
			rowData[r] = (char **)malloc ( sizeof ( char ** ) * cols );
			lengths[r] = (int *)malloc ( sizeof ( int ) * cols );

			unsigned long *mysqlLengths = mysql_fetch_lengths ( response ); 
			memcpy ( lengths[r], mysqlLengths, sizeof ( unsigned long ) * cols );

			for ( int c=0; c<cols; c++ ) {
				if ( lengths[r][c] ) {
					rowData[r][c] = (char*) malloc( lengths[r][c] );
					memcpy( rowData[r][c], row[c], lengths[r][c] );
				} else
					rowData[r][c] = strdup ( "" );
			}
		}

		mysql_free_result ( response );
	}
}

SQLResponse::~SQLResponse() {
	if ( rowData ) {
		for ( int r=0; r<rows; r++ ) {
			for ( int c=0; c<cols; c++ )
				free ( rowData[r][c] );

			free ( rowData[r] );
			free ( lengths[r] );
		}
		
		free ( rowData );
		free ( lengths );
		rowData = NULL;
	}

	rows = cols = size = 0;
	error = NULL;
	errno = 0;
}

//
// SQLDatabase: This class provides the interface to the Mini-SQL API.
//

SQLDatabase::SQLDatabase ( char *host, char *user, char *password, char *db ) {
	connection = mysql_init ( NULL );
	MYSQL *result = mysql_real_connect ( connection, host, user, password, db, 0, NULL, 0 );

	if ( !result ) {
		error = const_cast<char*>( mysql_error( connection ) );
		errno = mysql_errno ( connection );
	} else {
		error = NULL;
		errno = 0;
	}
}

SQLDatabase::~SQLDatabase() {
	mysql_close ( connection );
	connection = NULL;
	error = NULL;
	errno = 0;
}

// post an SQL query
SQLResponse *SQLDatabase::query ( const char *format, ... ) {
	char *strings[128];
	int lengths[128];

	va_list args;
	va_start ( args, format );
	error = NULL;
	errno = 0;

	// step through the format string and build the query size
	const char *ptr = format;
	int querySize = 0, numStrings = 0;

	while ( *ptr ) {
		// handle the meta-commands...
		if ( *ptr == '%' ) {
			ptr++;

			if ( *ptr == 0 )
				fatal ( "Bad SQL query string '%s'", format );
		
			switch ( *ptr ) {
				case '%': {
					querySize++;	
				}

				break;

				// handle a string
				case 's': {
					char *theStr = va_arg ( args, char * );
					int len = strlen ( theStr );

					char *buf = (char *)malloc ( len * 2 + 1 );
					int theSize = mysql_escape_string ( buf, theStr, len );
					querySize += theSize;

					strings[numStrings] = buf;
					lengths[numStrings] = theSize;
					numStrings++;
				}

				break;

				// handle a set of binary data
				case 'b': {
					int size = va_arg ( args, int );
					char *theBuf = va_arg ( args, char * );

					char *buf = (char *)malloc ( size * 2 + 1 );
					int theSize = mysql_real_escape_string ( connection, buf, theBuf, size );
					querySize += theSize;

					strings[numStrings] = buf;
					lengths[numStrings] = theSize;
					numStrings++;
				} 

				break;

				// handle an int
				case 'd': {
					char temp[25];
					int value = va_arg ( args, int );
					sprintf ( sizeof ( temp ), temp, "%d", value );

					int len = strlen ( temp );
					querySize += len;

					strings[numStrings] = strdup ( temp );
					lengths[numStrings] = len;
					numStrings++;
				}

				break;

		                // handle an int
                		case 'u': {
					char temp[25];
					int value = va_arg ( args, int );
					sprintf ( sizeof ( temp ), temp, "%u", value );

					int len = strlen ( temp );
					querySize += len;

					strings[numStrings] = strdup ( temp );
					lengths[numStrings] = len;
					numStrings++;
				}
				
				break;

				// handle a float
				case 'f': {
					char temp[25];
					double value = va_arg ( args, double );
					sprintf ( sizeof ( temp ), temp, "%f", value );

					int len = strlen ( temp );
					querySize += len;

					strings[numStrings] = strdup ( temp );
					lengths[numStrings] = len;
					numStrings++;
				}

				break;

				default: {
					fatal ( "Bad SQL query string '%s'", format );
				}

				break;
			}
		} else {
			querySize++;
		}

		ptr++;
	}

	// allocate the query
	querySize++;
	char *query = (char *)malloc ( querySize + 1 );
	char *queryPtr = query;

	// step through the format string and build the query
	ptr = format;
	int stringIdx = 0;
	va_start ( args, format );

	while ( *ptr ) {
		// handle the meta-commands...
		if ( *ptr == '%' ) {
			ptr++;

			if ( *ptr == 0 )
				fatal ( "Bad SQL query string '%s'", format );
		
			switch ( *ptr ) {
				case '%': {
					*queryPtr = '%';
					queryPtr++;
				}

				break;

				// handle a string
				case 's': {
					char *theStr = va_arg ( args, char * );

					char *buf = strings[stringIdx];
					int len = lengths[stringIdx];
					stringIdx++;

					memcpy ( queryPtr, buf, len );
					queryPtr += len;

					// toss the string
					free ( buf );
				}

				break;

				// handle a set of binary data
				case 'b': {
					int size = va_arg ( args, int );
					char *theBuf = va_arg ( args, char * );

					char *buf = strings[stringIdx];
					int len = lengths[stringIdx];
					stringIdx++;

					memcpy ( queryPtr, buf, len );
					queryPtr += len;

					// toss the string
					free ( buf );
				} 

				break;

				// handle an int
				case 'd': {
					int value = va_arg ( args, int );

					char *buf = strings[stringIdx];
					int len = lengths[stringIdx];
					stringIdx++;

					memcpy ( queryPtr, buf, len );
					queryPtr += len;

					// toss the string
					free ( buf );
				}

				break;

				 // handle an int
                                case 'u': {
                                        int value = va_arg ( args, int );

					char *buf = strings[stringIdx];
                                        int len = lengths[stringIdx];
                                        stringIdx++;

					memcpy ( queryPtr, buf, len );
                                        queryPtr += len;

					// toss the string
                                        free ( buf );
                                }
					  
                                break;
				
				// handle a float
				case 'f': {
					double value = va_arg ( args, double );

					char *buf = strings[stringIdx];
					int len = lengths[stringIdx];
					stringIdx++;

					memcpy ( queryPtr, buf, len );
					queryPtr += len;

					// toss the string
					free ( buf );
				}

				break;

				default: {
					fatal ( "Bad SQL query string '%s'", format );
				}

				break;
			}
		} else {
			*queryPtr = *ptr;
			queryPtr++;
		}

		ptr++;
	}

	*queryPtr = 0;

queryRetry:
	int state = -1;

	while ( state != 0 ) {
		state = mysql_real_query ( connection, query, querySize );

		// handle reconnecting to the server if the query fails
		if ( state != 0 ) {
			if ( (mysql_errno ( connection ) == CR_SERVER_LOST) || (mysql_errno ( connection ) == CR_SERVER_GONE_ERROR)  )
				mysql_ping ( connection );
			else
				break;
		}
	}

	if ( state != 0 ) {
		error = const_cast<char*>( mysql_error( connection ) );
		errno = mysql_errno ( connection );

		// toss the query data
		free ( query );

		return NULL;
	}

	SQLResponse *response = new SQLResponse ( connection ); 

	if ( response->error ) {
		error = response->error;
		delete response;
		response = NULL;
	} 

	else if ( response->rows == 0 ) {
		delete response;
		response = NULL;
	}
	
	// toss the query data
	free ( query );

	return response;
}

// get the last insert id
int SQLDatabase::lastInsertID ( void ) {
	return (int)mysql_insert_id ( connection );
}

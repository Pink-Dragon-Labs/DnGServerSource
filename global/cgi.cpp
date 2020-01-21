#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cgi.hpp"

CGIParams::CGIParams() {
	int i;

	for ( i=0; i<_MAX_CGI_PARAMS; i++ )
		_names[i] = _values[i] = NULL;

	if ( !getenv ( "CONTENT_LENGTH" ) && !getenv ( "QUERY_STRING" ) )
		return;

	char *lenStr = getenv ( "CONTENT_LENGTH" );
	char *params = NULL;

	if ( lenStr ) {
		int contentLength = atoi ( lenStr ), idx = 0;
		params = (char *)malloc ( contentLength + 2 );

		while ( contentLength-- )
			params[idx++] = fgetc ( stdin );

		params[idx++] = '&';
		params[idx] = 0;
	} else {
		params = strdup ( getenv ( "QUERY_STRING" ) );
	}

	if ( !params )
		return;

	char *ptr, *dup, *token = params;
	i = 0;

	// find each parameter pair
	while ( ptr = strchr ( token, '&' ) ) {
		*ptr = 0;
		_values[i++] = strdup ( token );
		token = ptr + 1;
	}

	free ( params );

	// convert all of the special characters
	for ( i=0; i<_MAX_CGI_PARAMS; i++ ) {
		if ( !_values[i] )
			break;

		dup = strdup ( _values[i] );
		char *src = dup, *dst = _values[i];

		while ( *src ) {
			switch ( *src ) {
				case '+': 
					*dst++ = ' ';
					src++;
					break;

				case '%': {
					char hex[3];
					hex[0] = src[1];
					hex[1] = src[2];
					hex[2] = 0;

					int value;
					sscanf ( hex, "%x", &value );

					*dst++ = (char)value;
					src += 3;
				}

				break;

				default: 
					*dst++ = *src++;
					break;
			}
		}

		*dst = 0;

		free ( dup );
	}			

	for ( i=0; i<_MAX_CGI_PARAMS; i++ ) {
		if ( !_values[i] )
			break;

		ptr = strchr ( _values[i], '=' );

		if ( ptr ) {
			*ptr = 0;
			_names[i] = strdup ( _values[i] );
			strcpy ( _values[i], ptr + 1 );
		}
	}
}

CGIParams::CGIParams( char *queryString, char terminator ) {
	parseQueryString ( queryString, terminator );
}

CGIParams::~CGIParams() {
	for ( int i=0; i<_MAX_CGI_PARAMS; i++ ) {
		if ( _names[i] ) {
			free ( _names[i] );
			_names[i] = NULL;
		}

		if ( _values[i] ) {
			free ( _values[i] );
			_values[i] = NULL;
		}
	}
}

void CGIParams::parseQueryString ( char *queryString, char terminator ) {
	int i;

	for ( i=0; i<_MAX_CGI_PARAMS; i++ )
		_names[i] = _values[i] = NULL;

	if ( !queryString )
		return;

	char *params = NULL;
	params = strdup ( queryString );

	if ( !params )
		return;

	char *ptr, *dup, *token = params;
	i = 0;

	// find each parameter pair
	while ( ptr = strchr ( token, terminator ) ) {
		*ptr = 0;
		_values[i++] = strdup ( token );
		token = ptr + 1;
	}

	if ( strchr( token, '=' ) ) {
		_values[i++] = strdup ( token );
	}

	free ( params );

	// convert all of the special characters
	for ( i=0; i<_MAX_CGI_PARAMS; i++ ) {
		if ( !_values[i] )
			break;

		dup = strdup ( _values[i] );
		char *src = dup, *dst = _values[i];

		while ( *src ) {
			switch ( *src ) {
				case '+': 
					*dst++ = ' ';
					src++;
					break;

				case '%': {
					char hex[3];
					hex[0] = src[1];
					hex[1] = src[2];
					hex[2] = 0;

					int value;
					sscanf ( hex, "%x", &value );

					*dst++ = (char)value;
					src += 3;
				}

				break;

				default: 
					*dst++ = *src++;
					break;
			}
		}

		*dst = 0;

		free ( dup );
	}			

	for ( i=0; i<_MAX_CGI_PARAMS; i++ ) {
		if ( !_values[i] )
			break;

		ptr = strchr ( _values[i], '=' );

		if ( ptr ) {
			*ptr = 0;
			_names[i] = strdup ( _values[i] );
			strcpy ( _values[i], ptr + 1 );
		}
	}
}

char *CGIParams::get ( char *name ) {
	for ( int i=0; i<_MAX_CGI_PARAMS; i++ ) {
		if ( !_names[i] )
			break;

		if ( !strcmp ( _names[i], name ) )
			return ( _values[i] );
	}

	return NULL;
}

int CGIParams::set ( char* name, int nValue ) {
	char sTmp[1024];

	for ( int i=0; i<_MAX_CGI_PARAMS; i++ ) {
		if ( !_names[i] ) {
			_names[i] = strdup( name );
			sprintf( sTmp, "%d", nValue );
			_values[i] = strdup( sTmp );

			return 1;
		}

		if ( !strcmp ( _names[i], name ) ) {
			sprintf( sTmp, "%d", nValue );
			_values[i] = strdup( sTmp );

			return 1;
		}
	}

	return 0;
}

int CGIParams::set ( char* name, char* pValue ) {
	for ( int i=0; i<_MAX_CGI_PARAMS; i++ ) {
		if ( !_names[i] ) {
			_names[i] = strdup( name );
			_values[i] = strdup( pValue );

			return 1;
		}

		if ( !strcmp ( _names[i], name ) ) {
			_values[i] = strdup( pValue );

			return 1;
		}
	}

	return 0;
}

void CGIParams::dump () {
	for ( int i=0; i<_MAX_CGI_PARAMS; i++ ) {
		if ( !_names[i] )
			break;

		printf( "'%s' = '%s'<br>", _names[i], _values[i] );
	}
}


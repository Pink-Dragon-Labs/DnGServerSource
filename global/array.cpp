#include <stdlib.h>
#include "array.hpp"
#include "fatal.hpp"
#include "logmgr.hpp"

Array::Array ( int size )
{
	_data = NULL;
	_size = _actualSize = 0;
	_freeBuffer = 1;

	grow ( size );
} 

Array::Array ( char *ptr, int size ) 
{
	_data = NULL;
	_size = _actualSize = 0;
	_freeBuffer = 1;
}

Array::Array()
{
	_data = NULL;
	_size = _actualSize = 0;
	_freeBuffer = 1;

	grow ( _ARRAY_START_SIZE );
}

Array::~Array()
{
	if ( _data ) {
		if ( _freeBuffer )
			free ( _data );

		_data = NULL;
	}

	_size = _actualSize = 0;
}

void Array::grow ( int size )
{
	if ( !_freeBuffer ) 
		fatal ( "Trying to grow an array with _freeBuffer disabled." );

	int newSize = _actualSize + size;
	unsigned char *newData = (unsigned char *)calloc ( 1, sizeof ( char ) * newSize ); 
	if ( _data ) {
		memcpy ( newData, _data, sizeof ( unsigned char ) * _actualSize );
		free ( _data );
	}

	_data = newData;
	_actualSize = newSize;
}

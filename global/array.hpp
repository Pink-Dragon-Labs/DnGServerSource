/*
	automatic resizing array class
	author: Stephen Nichols
*/

#ifndef _ARRAY_HPP_
#define _ARRAY_HPP_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.hpp"
#include "new.hpp"
#include "malloc.hpp"

#define _ARRAY_START_SIZE	1000
#define _ARRAY_GROW_SIZE	10000	

class Array
{
public:
	unsigned char *_data;
	int _actualSize, _size, _freeBuffer;

	void grow ( int size = _ARRAY_GROW_SIZE );

	inline void checkBounds ( int index ) {
		if ( index >= _actualSize )
			grow ( (index - _actualSize) + _ARRAY_GROW_SIZE );

		if ( index >= _size )
			_size = index + 1;
	};

	Array ( char *ptr, int size );
	Array ( int size );
	Array();
	~Array();

	inline unsigned char *data ( void ) { return _data; };

	inline unsigned char& operator[] ( int index ) {
		checkBounds ( index );
		return _data[index]; 
	};
};

#endif

/*
	packed data class
	author: Stephen Nichols
*/

#ifndef _PACKDATA_HPP_
#define _PACKDATA_HPP_

#include <stdarg.h>

#include "array.hpp"

#include "new.hpp"

class PackedData : public Array
{
	int _getIndex, _putIndex;

public:
	PackedData();
	PackedData ( char *ptr, int size );

	virtual ~PackedData();

	inline int size ( void ) {
		if ( !this )
			return 0;

		return _size;
	};

	inline int getDataLeft ( void ) {
		if ( !this || _getIndex >= _size )
			return 0;

		return _size - _getIndex;
	};

	void init ( char *ptr, int size ); 

	inline int putIndex ( void ) { 
		if ( !this )
			return 0;

		return _putIndex;
	};

	inline int getIndex ( void ) { 
		if ( !this )
			return 0;

		return _getIndex;
	};

	inline char* getData ( void ) { 
		if ( !this )
			return 0;

		return (char*) data() + _getIndex;
	};

	inline void setGetIndex ( int val ) {
		if ( !this )
			return;

		_getIndex = val;
	};

	/* byte manipulation functions */
	inline unsigned char byteAt ( int index ) { 
		if ( !this )
			return 0;

		return _data[index];
	};

	inline unsigned char getByte ( int count = 1 ) {
		if ( !this )
			return 0;

		while ( count > 1 ) {
			_getIndex++;
			count--;
		}

		return byteAt ( _getIndex++ );
	};

	/* byte manipulation functions */
	inline void setByte ( int index, unsigned char value ) {
		if ( !this )
			return;

		_data[index] = value;
	};

	inline void putByte ( unsigned char value ) {
		if ( !this )
			return;

		checkBounds ( _putIndex + 1 );
		_data[_putIndex++] = value;
	};

	/* word manipulation functions */
	inline short wordAt ( int index ) {
		if ( !this )
			return 0;

		return (short) ( byteAt ( index ) | (byteAt ( index + 1 ) << 8) );
	};

	inline short getWord ( int count = 1 ) {
		if ( !this )
			return 0;

		while ( count > 1 ) {
			getByte ( 2 );
			count--;
		}

		return (short) ( getByte() | (getByte() << 8) );
	};

	/* word manipulation functions */
	inline void setWord ( int index, short value ) {
		if ( !this )
			return;

		setByte ( index, (value & 0x00FF) );
		setByte ( index + 1, (value & 0xFF00) >> 8 );
	};

	inline void putWord ( short value ) {
		if ( !this )
			return;

		putByte ( (value & 0x00FF) );
		putByte ( (value & 0xFF00) >> 8 );
	};

	/* long manipulation functions */
	inline int longAt ( int index ) {
		if ( !this )
			return 0;

		return (int) ( byteAt ( index ) | (byteAt ( index + 1 ) << 8) | (byteAt ( index + 2 ) << 16) | (byteAt ( index + 3 ) << 24) );
	};

	/* long manipulation functions */
	inline void setLong ( int index, int value ) {
		if ( !this )
			return;

		setByte ( index, (value & 0x000000FF));	
		setByte ( index + 1, (value & 0x0000FF00) >> 8);
		setByte ( index + 2, (value & 0x00FF0000) >> 16);
		setByte ( index + 3, (value & 0xFF000000) >> 24);
	};

	inline int getLong ( int count = 1 ) {
		if ( !this )
			return 0;

		while ( count > 1 ) {
			getByte ( 4 );
			count--;
		}
			
		return (int) ( getByte() | (getByte() << 8) | (getByte() << 16) | (getByte() << 24) );
	};

	inline void putLong ( int value ) {
		if ( !this )
			return;

		putByte ( (value & 0x000000FF) );	
		putByte ( (value & 0x0000FF00) >> 8 );
		putByte ( (value & 0x00FF0000) >> 16 );
		putByte ( (value & 0xFF000000) >> 24 );
	};
//	---------------------------------------------------------------------------------------------

	/* string manipulation functions */
	inline char *stringAt ( int index, int length = 0 ) {
		if ( !this )
			return NULL;

		if ( !length ) {
			length = longAt ( index );
			index += 4;
		}

		char *str = (char *)malloc ( length );
		memcpy ( str, data() + index, length );

		return str;
	};

	inline void getString ( char *ptr ) {
		if ( !this )
			return;

		int length = getWord();

		memcpy ( ptr, data() + _getIndex, length );
		_getIndex += length;

		ptr[length] = 0;
	};

	inline char *getString ( int length = 0 ) {
		if ( !this )
			return NULL;

		if ( !length ) {
			length = getWord();
		}

		else if ( length == -1 ) {
			length = _actualSize - _getIndex;
		}

		// overflow test
		if ( length < 1 || length > 10240 )
			return NULL;

		char *str = (char *)malloc ( length + 1 );

		memcpy ( str, data() + _getIndex, length );
		_getIndex += length;

		str[length] = 0;

		return str;
	};

	inline void putString ( const char *str, int length = 0 ) {
		if ( !this )
			return;

		if ( !length ) {
			length = strlen ( str ); 
			putWord ( length );
		}

		checkBounds ( _putIndex + length + 1 );
		memcpy ( data() + _putIndex, str, length );
		_putIndex += length;
	};

	inline void putEncryptedString ( unsigned char* str, int length ) {
		if ( !this )
			return;

		if ( !length ) {
			length = strlen ( (char*) str ); 
			putWord ( length );
		}

		unsigned char nFirst = rand() >> 8;
		unsigned char nSecond = rand() >> 16;

		putByte ( nFirst );
		putByte ( nSecond );

		checkBounds ( _putIndex + length + 1 );

		unsigned char* pCopy = data() + _putIndex;
		_putIndex += length;

		for (int nLoop = 0;nLoop < length;nLoop += 4) {
			pCopy[ ( nLoop + 0 ) ] = str[ ( nLoop + 0 ) ] + nFirst - 0x39;
			pCopy[ ( nLoop + 1 ) ] = str[ ( nLoop + 1 ) ] - nSecond + 0x46;
			pCopy[ ( nLoop + 2 ) ] = str[ ( nLoop + 2 ) ] - nFirst + 0x0c;
			pCopy[ ( nLoop + 3 ) ] = str[ ( nLoop + 3 ) ] + nSecond - 0x30;
		}
	};

	inline void putArray ( void *ptr, int size ) {
		if ( !this )
			return;

		checkBounds ( _putIndex + size );
		memcpy ( data() + _putIndex, ptr, size );

		_putIndex += size;
	};

	inline void getArray ( void *ptr, int size ) {
		if ( !this )
			return;

		memcpy ( ptr, data() + _getIndex, size );
		_getIndex += size;
	}

	inline void printf ( char *str, ... ) {
		if ( !this )
			return;

		char output[10240];
		va_list args;

		va_start ( args, str );
		vsprintf ( output, str, args );
		va_end ( args );

		int length = strlen ( output );

		checkBounds ( _putIndex + length );
		memcpy ( data() + _putIndex, output, length );
		_putIndex += length;
	};

	// --------------------------------------------------------------------------------------------------------
	//	BLE functions	Big Little Endian
	// --------------------------------------------------------------------------------------------------------

	/* word manipulation functions */
	inline short BLE_wordAt ( int index ) {
		if ( !this )
			return 0;

		return (short) ( byteAt ( index + 1) | (byteAt ( index ) << 8) );
	};

	inline short getBLE_Word ( int count = 1 ) {
		if ( !this )
			return 0;

		while ( count > 1 ) {
			getByte ( 2 );
			count--;
		}

		return (short) ( ( getByte() << 8 ) | getByte() );
	};

	/* word manipulation functions */
	inline void setBLE_Word ( int index, short value ) {
		if ( !this )
			return;

		setByte ( index + 1, (value & 0xFF00) >> 8 );
		setByte ( index, (value & 0x00FF) );
	};

	inline void putBLE_Word ( short value ) {
		if ( !this )
			return;

		putByte ( (value & 0xFF00) >> 8 );
		putByte ( (value & 0x00FF) );
	};

	/* long manipulation functions */
	inline int BLE_longAt ( int index ) {
		if ( !this )
			return 0;

		return (int) ( byteAt ( index + 3 ) | (byteAt ( index + 2 ) << 8) | (byteAt ( index + 1 ) << 16) | (byteAt ( index + 0 ) << 24) );
	};

	/* long manipulation functions */
	inline void setBLE_Long ( int index, int value ) {
		if ( !this )
			return;

		setByte ( index + 3, (value & 0x000000FF));	
		setByte ( index + 2, (value & 0x0000FF00) >> 8);
		setByte ( index + 1, (value & 0x00FF0000) >> 16);
		setByte ( index + 0, (value & 0xFF000000) >> 24);
	};

	inline int getBLE_Long ( int count = 1 ) {
		if ( !this )
			return 0;

		while ( count > 1 ) {
			getByte ( 4 );
			count--;
		}
			
		return (int) ( ( getByte() << 24 ) | (getByte() << 16) | (getByte() << 8) | getByte() );
	};

	inline void putBLE_Long ( int value ) {
		if ( !this )
			return;

		putByte ( (value & 0xFF000000) >> 24 );
		putByte ( (value & 0x00FF0000) >> 16 );
		putByte ( (value & 0x0000FF00) >> 8 );
		putByte ( (value & 0x000000FF) );	
	};

	/* string manipulation functions */
	inline char *BLE_stringAt ( int index, int length = 0 ) {
		if ( !this )
			return NULL;

		if ( !length ) {
			length = BLE_longAt ( index );
			index += 4;
		}

		char *str = (char *)malloc ( length );
		memcpy ( str, data() + index, length );

		return str;
	};

	inline void getBLE_String ( char *ptr ) {
		if ( !this )
			return;

		int length = getBLE_Word();

		memcpy ( ptr, data() + _getIndex, length );
		_getIndex += length;

		ptr[length] = 0;
	};

	inline char *getBLE_String ( int length = 0 ) {
		if ( !this )
			return NULL;

		if ( !length ) {
			length = getBLE_Word();
		}

		else if ( length == -1 ) {
			length = _actualSize - _getIndex;
		}

		// overflow test
		if ( length < 1 || length > 10240 )
			return NULL;

		char *str = (char *)malloc ( length + 1 );

		memcpy ( str, data() + _getIndex, length );
		_getIndex += length;

		str[length] = 0;

		return str;
	};

	inline void putBLE_String ( char *str, int length = 0 ) {
		if ( !this )
			return;

		if ( !length ) {
			length = strlen ( str ); 
			putBLE_Word ( length );
		}

		checkBounds ( _putIndex + length + 1 );
		memcpy ( data() + _putIndex, str, length );
		_putIndex += length;
	};

	inline void putEncryptedBLE_String ( unsigned char* str, int length ) {
		if ( !this )
			return;

		if ( !length ) {
			length = strlen ( (char*) str ); 
			putBLE_Word ( length );
		}

		unsigned char nFirst = rand() >> 8;
		unsigned char nSecond = rand() >> 16;

		putByte ( nFirst );
		putByte ( nSecond );

		checkBounds ( _putIndex + length + 1 );

		unsigned char* pCopy = data() + _putIndex;
		_putIndex += length;

		for (int nLoop = 0;nLoop < length;nLoop += 4) {
			pCopy[ ( nLoop + 0 ) ] = str[ ( nLoop + 0 ) ] + nFirst - 0x39;
			pCopy[ ( nLoop + 1 ) ] = str[ ( nLoop + 1 ) ] - nSecond + 0x46;
			pCopy[ ( nLoop + 2 ) ] = str[ ( nLoop + 2 ) ] - nFirst + 0x0c;
			pCopy[ ( nLoop + 3 ) ] = str[ ( nLoop + 3 ) ] + nSecond - 0x30;
		}
	};
};

#endif

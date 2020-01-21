/*
	FILE.HPP
	general file I/O class library
*/

#ifndef _FILE_HPP_
#define _FILE_HPP_

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "types.hpp"
#include "counter.hpp"

#include "debug.hpp"
#include "new.hpp"
#include "malloc.hpp"

class File
{
private:
	char *_name;
	int _size, _index, _seekCount, _writeCount, _readCount, _handle;

	void init ( void );

	inline void validateIndex ( void ) {
		if ( isOpen() ) {
			if ( index() != lseek ( handle(), 0, SEEK_CUR ) ) {
				printf ( "index is invalid. %d != %d\n", index(), lseek ( handle(), 0, SEEK_CUR ) );
			}
		}
	};

public:
	File();
	File ( char *name );
	virtual ~File();

	// member access functions
	inline char *&name ( void ) { return ( _name ); };
	inline int &size ( void ) { return ( _size ); };
	inline int &index ( void ) { return ( _index ); };
	inline int &seekCount ( void ) { return ( _seekCount ); };
	inline int &writeCount ( void ) { return ( _writeCount ); };
	inline int &readCount ( void ) { return ( _readCount ); };
	inline int &handle ( void ) { return ( _handle ); };

	// check to see if this file is open
	inline int isOpen ( void ) { return ( (handle() == -1)? FALSE : TRUE); };

	virtual int open ( char *name, ... );
	virtual int close ( void );

	virtual int read ( void *buffer, int len );
	virtual int write ( void *buffer, int len );
	virtual int seek ( int index );
	virtual int printf ( const char *format, ... );
	virtual int gets ( char *buffer, int size );
	virtual int truncate ( int newSize = 0 );
	virtual int append ();

	void tell ( void );

	int lock ( int index, int size );
	int unlock ( int index, int size );
};

extern int gFileBytesRead;
extern int gFileSeeks;
extern Counter gFileHandles;

#endif

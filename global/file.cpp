/*
	FILE.HPP
	RealmServer Database Engine: general file management
*/

#include "system.hpp"

LinkedList gOpenFileList;


int gFileBytesRead = 0, gFileSeeks = 0;

Counter gFileHandles;

//#define _FILE_DEBUG

File::File()
{
#ifdef _FILE_DEBUG
	printf ( "File::File() called\n" );
#endif
	init();
}

File::File ( char *name ) 
{
	init();
	open ( name );
}

File::~File()
{
	close();
}

void File::tell ( void )
{
#ifdef _FILE_DEBUG
	printf ( "File::tell() called\n" );
#endif
	if ( isOpen() ) {
		index() = lseek ( handle(), 0, SEEK_CUR );
		size() = lseek ( handle(), 0, SEEK_END );
		lseek ( handle(), index(), SEEK_SET );

#ifdef _FILE_DEBUG
		printf ( "\ttell() has file\n" );
		printf ( "\ttell() generated index() of %d\n", index() );
		printf ( "\ttell() generated size() of %d\n", size() );
#endif
	}
}

void File::init ( void )
{
#ifdef _FILE_DEBUG
	printf ( "File::init() called\n" );
#endif
	name() = NULL;
	size() = 0;
	index() = 0;
	handle() = -1;
}

void logFileOpen ( char *name )
{
	int handle = ::open ( "../logs/files.log", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP );

	if ( handle != -1 ) {
		char output[1024], *timeAsStr = timeToStr();
		sprintf ( sizeof ( output ), output, "%s: '%s' opened\n", timeAsStr, name );
		free ( timeAsStr );

		lseek ( handle, 0, SEEK_END );
		::write ( handle, output, strlen ( output ) );

		::close ( handle );
	}
}

void logFileClose ( char *name )
{
	int handle = ::open ( "../logs/files.log", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR );

	if ( handle != -1 ) {
		char* timeAsStr = timeToStr();
		char output[1024];
		sprintf ( sizeof ( output ), output, "%s: '%s' closed\n", timeAsStr, name );
		free ( timeAsStr );

		lseek ( handle, 0, SEEK_END );
		::write ( handle, output, strlen ( output ) );

		::close ( handle );
	}
}

int File::open ( char *name, ... )
{
	char filename[1024];

	va_list args;
	va_start ( args, name );
	vsprintf ( sizeof ( filename ), filename, name, args );
	va_end ( args );

	name = filename;

	int retVal = FALSE;

	close();

	this->name() = strdup ( name );
	handle() = ::open ( this->name(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP );

	if ( handle() == -1 )
		handle() = ::open ( this->name(), O_RDONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP );

	tell();

//	logFileOpen ( this->name() );

	if ( isOpen() ) {
//		gOpenFileList.add ( (ListObject *)(void *)this );
		gFileHandles.increment();
		retVal = TRUE;
	} else {
		close();
	}

	seekCount() = 0;
	readCount() = 0;
	writeCount() = 0;

#ifdef _FILE_DEBUG
	printf ( "File::open ( %s ) called. %s\n", name, isOpen()? "succesful" : "failed" );
#endif

	return ( retVal );
}

int File::close ( void )
{
	int retVal = FALSE;

	if ( name() ) {
//		logFileClose ( this->name() );

		free ( name() );
		name() = NULL;
		retVal = TRUE;
	}

	if ( isOpen() ) {
		::close ( handle() );
		gFileHandles.decrement();
//		gOpenFileList.del ( (ListObject *)(void *)this );
		handle() = -1;
		retVal = TRUE;
	}

	return ( retVal );
}

int File::read ( void *buffer, int len )
{
	int retVal = FALSE;

#ifdef _FILE_DEBUG
	printf ( "File::read ( %p, %d ) called\n", buffer, len );
#endif

#ifdef _FILE_DEBUG
	validateIndex();
#endif

	if ( isOpen() ) {
		int theIndex = index();
		lock ( theIndex, len );

		if ( ::read ( handle(), buffer, len ) == len ) {
			gFileBytesRead += len;
			index() += len;
			retVal = TRUE;

			readCount()++;

#ifdef _FILE_DEBUG
			printf ( "\tfread() was succesful\n\tindex() is now %d\n", index() );
#endif			
		} else {
#ifdef _FILE_DEBUG
			printf ( "\tfread() failed\n\tindex() is now %d\n", index() );
#endif			
		}

		unlock ( theIndex, len );
	}

#ifdef _FILE_DEBUG
	validateIndex();
#endif

	return ( retVal );
}

int File::write ( void *buffer, int len )
{
	int retVal = FALSE;

#ifdef _FILE_DEBUG
	validateIndex();
#endif

#ifdef _FILE_DEBUG
	printf ( "File::write ( %p, %d ) called\n", buffer, len );
#endif

	if ( isOpen() ) {
		int theIndex = index();
		lock ( theIndex, len );

		if ( ::write ( handle(), buffer, len ) == len ) {
			// update file information! (like tell)
			theIndex += len;

			if ( theIndex > size() ) 
				size() = theIndex;

			index() = theIndex;

			retVal = TRUE;
			writeCount()++;

#ifdef _FILE_DEBUG
			printf ( "\tfwrite() was succesful\n\tindex() is now %d\n", index() );
#endif			
		} else {
#ifdef _FILE_DEBUG
			printf ( "\tfwrite() failed\n\tindex() is now %d\n", index() );
#endif			
		}

		unlock ( theIndex, len );
	}

#ifdef _FILE_DEBUG
	validateIndex();
#endif

	return ( retVal );
}

int File::seek ( int where )
{
	int retVal = FALSE;

#ifdef _FILE_DEBUG
	validateIndex();
#endif

#ifdef _FILE_DEBUG
	printf ( "File::seek ( %d ) called\n", where );
#endif

	if ( isOpen() ) {
		if ( where != index() ) {
			if ( lseek ( handle(), where, SEEK_SET ) == where ) {
				gFileSeeks++;
				seekCount()++;
		 		index() = where;

#ifdef _FILE_DEBUG
				printf ( "\tseek was succesful\n\tindex() is now %d\n", index() );
#endif
				retVal = TRUE;
			} else {
#ifdef _FILE_DEBUG
				printf ( "\tseek failed\n\tindex() == %d\n", index() );
#endif
			}
		} else {
#ifdef _FILE_DEBUG
			printf ( "\tseek is redundant: %d == %d\n", where, index() );
#endif
			retVal = TRUE;
		}
	}

#ifdef _FILE_DEBUG
	validateIndex();
#endif

	return ( retVal );
}

int File::append () {
        if ( isOpen() ) {
		return lseek ( handle(), 0, SEEK_END ); 
	}

	return 0;
}


int File::printf ( const char *format, ... )
{
	va_list args;
	char output[40960];

	va_start ( args, format );
	vsprintf ( sizeof ( output ), output, format, args );
	va_end ( args );

	return write ( output, strlen ( output ) );
}

int File::gets ( char *buffer, int size )
{
	if ( !isOpen() )
		return FALSE;

	char ch = 0; 

	while ( read ( &ch, 1 ) && ch != '\n' && size ) {
		*buffer++ = ch;
		size--;
	}

	*buffer = 0;

	return ch == '\n' || !size;
}

int File::lock ( int offset, int len )
{
	return 1;

	int index = this->index();

	seek ( offset );
	int retVal = lockf ( handle(), F_LOCK, len );
	seek ( index );

	return retVal;
}

int File::unlock ( int offset, int len )
{
	return 1;

	int index = this->index();

	seek ( offset );
	int retVal = lockf ( handle(), F_ULOCK, len );
	seek ( index );

	return retVal;
}

int File::truncate ( int newSize )
{
	if ( isOpen() ) {
		::ftruncate ( handle(), newSize );
		tell();

		return 1;
	}

	return 0;
}

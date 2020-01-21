#include "system.hpp"

#undef malloc
#undef free
#undef calloc
#undef realloc

int gAppStarted = 0;

int SmartHeap_malloc = 0;

int gAllocCount = 0, gAllocSize = 0;
char *_minPtr = NULL, *_maxPtr = NULL;

int gLargestAllocSize = 0, gLargestLine = 0;
char *gLargestFile = "";

AllocStruct gAllocations[5000];
int g_nAllocations = 0;

malloc_t *gAllocList = NULL, *gAllocListTail = NULL;

void *db_malloc ( int size, const char *file, int line ) {
	int nFound = 0;

	int newSize = size + sizeof ( malloc_t ) + 4;
	newSize &= 0xfffffffc;

	if ( g_nAllocations ) {
		int nAdd = 1;
		int nLoc = -1;

		for (nFound = 0;nFound < g_nAllocations;nFound++) {
			if ( file == gAllocations[nFound].file && line == gAllocations[nFound].line && newSize == gAllocations[nFound].nSize ) {
				nAdd = 0;
				break;
			} else if ( nLoc == -1 && !( gAllocations[nFound].nAlloc - gAllocations[nFound].nFree ) ) {
				nLoc = nFound;
			}
		}

		if ( nAdd ) {
			if ( nLoc == -1 ) {
				if ( g_nAllocations < 4999 ) {
					gAllocations[ nFound ].file = file;
					gAllocations[ nFound ].line = line;
					gAllocations[ nFound ].nSize = newSize;
					gAllocations[ nFound ].nAlloc = 0;
					gAllocations[ nFound ].nFree = 0;
	
					g_nAllocations++;
				} else {
					gAllocations[nFound].file = "Too many different sizes and places";
				}
			} else {
				gAllocations[ nLoc ].file = file;
				gAllocations[ nLoc  ].line = line;
				gAllocations[ nLoc  ].nSize = newSize;
				gAllocations[ nLoc  ].nAlloc = 0;
				gAllocations[ nLoc  ].nFree = 0;

				nFound = nLoc;
			}
		}
	} else {
		gAllocations[ 0 ].file = file;
		gAllocations[ 0 ].line = line;
		gAllocations[ 0 ].nSize = newSize;
		gAllocations[ 0 ].nAlloc = 0;
		gAllocations[ 0 ].nFree = 0;

		g_nAllocations++;
	}

	void *ptr = NULL;

	ptr = calloc ( 1, newSize );

	if ( ptr == NULL ) {
		logCrash ( "got a NULL from allocating memory of %d", newSize );
		crash();
	}

	gAllocations[ nFound ].nAlloc++;

	malloc_t *header = (malloc_t *)ptr;

	if ( !_minPtr )
		_minPtr = (char *)ptr;

	if ( !_maxPtr )
		_maxPtr = (char *)ptr;

	char *str = (char *)ptr + sizeof ( malloc_t );

	if ( ptr < _minPtr )
		_minPtr = (char *)ptr;

	if ( str > _maxPtr )
		_maxPtr = (char *)str;

	header->magicHeader = _MAGIC_HEADER;
	header->nAlloc = nFound;

	// update the links
	header->next = NULL;
	header->prev = gAllocListTail;

	if ( gAllocListTail )
		gAllocListTail->next = header;

	gAllocListTail = header;

	if ( !gAllocList )
		gAllocList = header;

	gAllocSize += ( header->size = newSize );

	if ( size > gLargestAllocSize ) {
		gLargestAllocSize = size;
	}

	header->type = _MEM_ARRAY;

	gAllocCount++;

	return str; 
}

void *db_realloc ( void *ptr, int size, const char *file, int line )
{
	if ( !isValidPtr ( ptr ) ) {
		logCrash ( "invalid pointer in call to db_realloc:\n" );
		crash();
	}

	malloc_t *header = (malloc_t *)(((char *)ptr) - sizeof ( malloc_t ));

	void *newPtr = db_malloc ( size, file, line );
	memcpy ( newPtr, ptr, header->size - sizeof ( malloc_t ) );

	db_free ( ptr, file, line );

	return newPtr;
}

void db_free ( void *ptr, const char *file, int line )
{
	if ( !isValidPtr ( ptr ) ) {
		logCrash ( "invalid pointer in call to db_free: %d of %s\n", line, file );
		crash();
		crash();
	}

	malloc_t *header = (malloc_t *)(((char *)ptr) - sizeof ( malloc_t ));

	if ( header->type == _MEM_FORBIDDEN_FREE ) {
		logCrash ( "freeing protected pointer: %d of %s\n", line, file );

		return;
	}

	if ( header->type == _MEM_FREE ) {
		logCrash ( "freeing pointer twice: %d of %s\n", line, file );

		return;
	}

	if ( !( gAllocations[ header->nAlloc ].nAlloc - gAllocations[ header->nAlloc ].nFree ) ) {
		logCrash ( "freeing Allocated memory pointers more than once for %s at line %d of size %d - %d of %s\n", 
				gAllocations[ header->nAlloc ].file,
				gAllocations[ header->nAlloc ].line,
				gAllocations[ header->nAlloc ].nSize, line, file );
		return;
	}

	header->type = _MEM_FREE;

	gAllocations[ header->nAlloc ].nFree++;

	if ( header->next )
		header->next->prev = header->prev;

	if ( header->prev )
		header->prev->next = header->next;

	if ( header == gAllocListTail )
		gAllocListTail = header->prev;

	if ( header == gAllocList )
		gAllocList = header->next;

	gAllocSize -= header->size;
	gAllocCount--;

	free ( header );
}

char *db_strdup ( const char *str, const char *file, int line )
{
	char *newStr = (char *)db_malloc ( strlen ( str ) + 1, file, line );
	strcpy ( newStr, str );
	return newStr;
}

int isValidPtr ( void *ptr )
{
	if (!ptr)
		return 0;

	int ptrVal = (int)ptr;

	if ( ptrVal % 4 )
		return 0;

	if ( ptr < _minPtr || (ptr == (void *)-1) )
		return 0;

	malloc_t *header = (malloc_t *)(((char *)ptr) - sizeof ( malloc_t ));

	int valid = (int)(header->magicHeader == _MAGIC_HEADER);

	return valid;
}

void setMemType ( void *ptr, int type )
{
	if ( !isValidPtr ( ptr ) ) {
		logCrash ( "invalid pointer in call to setMemType: \n");
		crash();
	}

	malloc_t *header = (malloc_t *)(((char *)ptr) - sizeof ( malloc_t ));
	header->type = type;
}

int getMemType ( void *ptr )
{
	if ( !isValidPtr ( ptr ) ) {
		logCrash ( "invalid pointer in call to getMemType:\n" );
		crash();
	}

	malloc_t *header = (malloc_t *)(((char *)ptr) - sizeof ( malloc_t ));
	return header->type;
}

void logCrash ( const char* format, ... )
{
	char output[1024];
	va_list args;

	va_start ( args, format );
	vsprintf ( sizeof ( output ), output, format, args );

	File *file = new File ( "crash.log" );

	if ( file->isOpen() ) {
		file->seek ( file->size() );
		file->printf ( "%s", output );
		file->close();
	}

	delete file;
}

void sysfree ( void *ptr )
{
	free ( ptr );
}

void *sysmalloc ( int size )
{
	return malloc ( size );
}

void *syscalloc ( int elsize, int size )
{
	return calloc ( elsize, size );
}

void *sysrealloc ( void *ptr, int size )
{
	return realloc ( ptr, size );
}

void memoryDump ( void )
{
	File file ( "memorydump.txt" );
	file.truncate ();

	malloc_t *ptr = gAllocList;

	while ( ptr ) {
		const char *name = strrchr ( gAllocations[ptr->nAlloc].file, '/' );
		if ( name )
			name++;
		else
			name = gAllocations[ptr->nAlloc].file;

		file.printf ( "%d %s(%d) %d\n", gAllocations[ptr->nAlloc].nSize, name, gAllocations[ptr->nAlloc].line, ptr->type );
		ptr = ptr->next;
	}

	file.close ();
}


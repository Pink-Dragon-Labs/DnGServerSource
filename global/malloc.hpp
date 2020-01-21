#ifndef _MALLOC_HPP_
#define _MALLOC_HPP_

#define _MAGIC_HEADER 0xDEAFDEAD
#define _MAGIC_FOOTER 0xFEEDABCD

enum {
	_MEM_STRING,
	_MEM_ARRAY,
	_MEM_STRUCT,
	_MEM_FREE,
	_MEM_CLIENT,
	_MEM_FORBIDDEN_FREE,
};

struct AllocStruct {
	const char*	file;
	int		line;
	int		nSize;
	int		nAlloc;
	int		nFree;
};

typedef struct malloc_st
{
	int magicHeader;
	int type;
	int size;
	int nAlloc;
	malloc_st *next, *prev;
} malloc_t;

extern malloc_t *gAllocList;

#undef malloc
#define malloc(size) db_malloc ( size, __FILE__, __LINE__ )

#undef realloc
#define realloc(ptr, size) db_realloc ( ptr, size, __FILE__, __LINE__ )

#undef calloc
#define calloc(elsize,size) db_malloc ( (elsize) * (size), __FILE__, __LINE__ )

#undef free
#define free(ptr) db_free ( ptr, __FILE__, __LINE__ )

#undef strdup
#define strdup(ptr) db_strdup ( ptr, __FILE__, __LINE__ )

#define getPtrType(ptr) getMemType ( ptr )
#define setPtrType(ptr,type) setMemType ( ptr, type )

void *db_malloc ( int size, const char* file, int line );
void *db_realloc ( void *ptr, int size, const char* file, int line );

void *sysmalloc ( int size );
void *syscalloc ( int elsize, int size );
void *sysrealloc ( void *ptr, int size );

void db_free ( void *ptr, const char* file, int line );
char *db_strdup ( const char *ptr, const char *file, int line );
int getMemType ( void *ptr );
void setMemType ( void *ptr, int type );
void logCrash ( const char *format, ... );
int isValidPtr ( void *ptr );

void logPointerHistory ( void *ptr );

void sysfree ( void *ptr );

void memoryDump ( void );

extern int gAllocCount;
extern int gAllocSize;
extern int gLargestAllocSize;
extern char *gLargestFile;
extern int gLargestLine;

extern int gAllocMap[160];
extern int gAllocOther;

extern int gAppStarted;

extern AllocStruct gAllocations[5000];
extern int g_nAllocations;

#endif

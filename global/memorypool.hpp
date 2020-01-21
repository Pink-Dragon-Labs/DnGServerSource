#ifndef _MEMORYPOOL_HPP_
#define _MEMORYPOOL_HPP_

#include <stdio.h>
#include <stdlib.h>

#include "malloc.hpp"

typedef struct 
{
	char *owner;
	int index, nextFree;
} MemoryBlock;

class MemoryPool
{
public:
	int size, maxSize, firstFree, numBlocks;
	MemoryBlock* pLastFree;
	int nFree, nAlloc;

	char **data;

	MemoryPool ( int blockSize ); 
	~MemoryPool();

	char *allocate ( void );
	void deallocate ( MemoryBlock *block );
	int isValidPtr ( char *ptr );
};

class MemoryAllocator
{
public:
	MemoryPool *pool;
	int size, maxAllocations;

	MemoryAllocator ( int theSize, int maxAlloc );

	~MemoryAllocator();

	char *allocate ( void );
	void deallocate ( char *ptr );
	int isValidPtr ( char *ptr );
};

#endif

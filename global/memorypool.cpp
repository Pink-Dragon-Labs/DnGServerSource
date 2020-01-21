#include "system.hpp"

MemoryPool::MemoryPool ( int blockSize ) {
	nAlloc = 0;
	nFree = 0;
	
	size = blockSize + sizeof ( MemoryBlock );
	size += 4 - (size % 4);

	maxSize = 1024;
	numBlocks = 1;

	data = (char **)sysmalloc ( sizeof ( char * ) );
	data[0] = (char *) sysmalloc ( maxSize * size );
	memset( data[0], 0, maxSize * size );

	char *ptr = data[0];
	MemoryBlock *block = NULL;

	firstFree = 0;

	for ( int i=0; i<maxSize; i++ ) {
		block = (MemoryBlock *)ptr; 

		block->owner = (char *)this;
		block->index = i;
		block->nextFree = i + 1;

		ptr += size;
	}

	block->nextFree = -1;
	pLastFree = block;
}

MemoryPool::~MemoryPool() 
{
	if ( data ) {
		sysfree ( data );
		data = NULL;
	}

	size = 0;
	maxSize = 0;
}

char *MemoryPool::allocate ( void ) 
{
	MemoryBlock *block = NULL;

	// make new data block 
	if ( firstFree == -1 ) {
		numBlocks++;
		data = (char **)sysrealloc ( data, sizeof ( char * ) * numBlocks );
		data[numBlocks - 1] = (char *)sysmalloc ( maxSize * size );

		char *ptr = data[numBlocks - 1];

		int start = maxSize * (numBlocks - 1);
		int end = start + maxSize;

		for ( int i=start; i<end; i++ ) {
			block = (MemoryBlock *)ptr;

			block->owner = (char *)this;
			block->index = i;
			block->nextFree = i + 1;

			ptr += size;
		}

		block->nextFree = -1;
		pLastFree = block;

		// link previous one
		ptr = data[numBlocks - 2] + ((maxSize - 1) * size);
		block = (MemoryBlock *)ptr;

		block->nextFree = start;
		firstFree = start;
	}

	nAlloc++;

	int coarseIndex = firstFree >> 10;
	int index = firstFree - (coarseIndex << 10);

	char *ptr = data[coarseIndex] + (index * size); 
	block = (MemoryBlock *)ptr;

	if ( ( firstFree = block->nextFree ) == -1 ) {
		pLastFree = NULL;
	}

	return ptr + sizeof ( MemoryBlock );
}

void MemoryPool::deallocate ( MemoryBlock *block ) 
{
	if ( data ) {
//		block->nextFree = firstFree;
//		firstFree = block->index;
		if ( firstFree == -1 )
			firstFree = block->index;

		if ( pLastFree )
			pLastFree->nextFree = block->index;

		block->nextFree = -1;
		pLastFree = block;

	        nFree++;
	}
}

int MemoryPool::isValidPtr ( char *ptr ) 
{
	ptr -= sizeof ( MemoryBlock );

	for ( int i=0; i<numBlocks; i++ ) {
		char *dataPtr = data[i];

		// is the pointer inside of one of the blocks of data?
		if ( (dataPtr <= ptr) && ((dataPtr + (maxSize * size)) > ptr) ) {
			// check the offset of the pointer... is should be evenly divisible
			int offset = (int) (ptr - dataPtr);

			if ( !(offset % size) )
				return 1;

			return 0;
		}
	}

	return 0;
}

MemoryAllocator::MemoryAllocator ( int theSize, int maxAlloc )
{
	size = theSize;
	maxAllocations = maxAlloc;

	pool = new MemoryPool ( theSize );
}

MemoryAllocator::~MemoryAllocator() 
{
	delete pool;
}

char *MemoryAllocator::allocate ( void ) 
{
	char *ptr = pool->allocate();

	return ptr;
}

void MemoryAllocator::deallocate ( char *ptr ) 
{
	MemoryBlock *block = (MemoryBlock *)(ptr - sizeof ( MemoryBlock ));
	MemoryPool *thePool = (MemoryPool *)block->owner;

	if ( thePool == pool )
		thePool->deallocate ( block );
}

int MemoryAllocator::isValidPtr ( char *ptr ) 
{
	int ret = pool->isValidPtr ( ptr );
	return ret;
}

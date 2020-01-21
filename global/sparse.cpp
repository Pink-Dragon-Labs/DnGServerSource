//
// sparse.cpp
//
// sparse array class
//
// author: Stephen Nichols
//

#include "sparse.hpp"

SparseArray::SparseArray ( unsigned long theTblSize, unsigned long theChunkSize )
{
	theTblSize /= theChunkSize;

	tableSize = theTblSize;
	chunkSize = theChunkSize;

	// extra padding for the table
	theTblSize++;

	table = (void ***)calloc ( theTblSize, sizeof ( void ** ) );
	//memset ( table, 0, sizeof ( void ** ) * theTblSize );
}

SparseArray::~SparseArray()
{
	for ( long i=0; i<tableSize; i++ ) {
		if ( table[i] ) {
			void **ptr = table[i];

			for ( long j=0; j<chunkSize; j++ ) {
				if ( ptr[j] ) {
					free ( ptr[j] );
					ptr[j] = NULL;
				}
			}

			free ( ptr );
			table[i] = NULL;
		}
	}
}

void SparseArray::add ( void *obj, long number )
{
	if ( number < 0 )
		return;

	long chunkIndex = number / chunkSize;
	number -= chunkIndex * chunkSize;

	if ( chunkIndex > tableSize )
		return;

	if ( !table[chunkIndex] )
		table[chunkIndex] = (void **)malloc ( sizeof ( void * ) * chunkSize );

	void **ptr = table[chunkIndex];
	ptr[number] = obj;
}

void SparseArray::del ( long number )
{
	if ( number < 0 )
		return;

	long chunkIndex = number / chunkSize;
	number -= chunkIndex * chunkSize;

	if ( chunkIndex > tableSize )
		return;

	if ( !table[chunkIndex] ) {
		return;
	}

	void **ptr = table[chunkIndex];
	ptr[number] = NULL;

	int isEmpty = 1;

	for ( long i=0; i<chunkSize; i++ ) {
		if ( ptr[i] ) {
			isEmpty = 0;
			break;
		}
	}

	if ( isEmpty ) {
		free ( ptr );
		table[chunkIndex] = NULL;
	}
}

void *SparseArray::lookup ( long number )
{
	if ( number < 0 )
		return NULL;

	long chunkIndex = number / chunkSize;
	number -= chunkIndex * chunkSize;

	if ( chunkIndex > tableSize )
		return NULL;

	if ( !table[chunkIndex] ) {
		return NULL;
	}

	void **ptr = table[chunkIndex];
	void *thePtr = ptr[number];

	return thePtr;
}

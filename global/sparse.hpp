//
// sparse.hpp
//
// sparse array class
//
// author: Stephen Nichols
//

#ifndef _SPARSE_HPP_
#define _SPARSE_HPP_

#include "list.hpp"

class SparseArray : public ListObject
{
public:
	SparseArray ( unsigned long theTableSize, unsigned long theChunkSize );
	virtual ~SparseArray();

	void add ( void *ptr, long number );
	void del ( long number );
	void *lookup ( long number );

	unsigned long tableSize, chunkSize;
	void ***table;
};

#endif

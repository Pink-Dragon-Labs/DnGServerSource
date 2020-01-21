//
//	generic hash table class library
//	author: Stephen Nichols
//

#ifndef _HASH_HPP_
#define _HASH_HPP_

#include "list.hpp"

#include "debug.hpp"
#include "new.hpp"
#include "malloc.hpp"

// hash a string, and return the hash value
inline int hashString ( const char* str ) {
	int val = 1;

	while ( *str ) 
		val += *str++ * val;

	return val; 
}

//
// HashableObject: This is the class to derive from if you want your object to
// be hashable with the HashTable class.
//
class HashableObject : public ListObject
{
protected:
	unsigned int _intHashValue, _charHashValue;

public:
	HashableObject();
	virtual ~HashableObject();

	// calculate the hash value for this object
	virtual void setIntHashValue ( void ) { _intHashValue = 0; };
	virtual void setCharHashValue ( void ) { _charHashValue = 0; };
	void setHashValues ( void ) { setIntHashValue(); setCharHashValue(); };

	// return the hash value for this object
	inline unsigned int intHashValue ( void ) { return _intHashValue; };
	inline unsigned int charHashValue ( void ) { return _charHashValue; };

	// return whether or not the hash object matches the passed criteria
	virtual int hashMatch ( int val ) { return 0; };
	virtual int hashMatch ( char *ptr ) { return 0; };
};

class HashTable
{
protected:
	LinkedList **_intHashTable, **_charHashTable;
	int _hashTableSize;

	inline void init ( void ) {
		_intHashTable = NULL;
		_charHashTable = NULL;
		_hashTableSize = 0;
	};

public:

	HashTable ( int size );
	virtual ~HashTable();

	// add an object to this hash table
	void add ( HashableObject *obj );

	// delete an object from this hash table
	void del ( HashableObject *obj );

	// release all of the elements of this hash table
	void release ( void );

	// find a HashableObject by an integer
	HashableObject *findByInt ( int val );

	// find a HashableObject by a string
	HashableObject *findByStr ( char *str );
};

#endif

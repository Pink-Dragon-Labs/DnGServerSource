//
//	generic hash table class library
//	author: Stephen Nichols
//

#include "hash.hpp"
#include "system.hpp"

HashableObject::HashableObject()
{
}

HashableObject::~HashableObject()
{
}

HashTable::HashTable ( int size )
{
	init();

	_intHashTable = (LinkedList **)malloc ( sizeof ( LinkedList ) * size );
	_charHashTable = (LinkedList **)malloc ( sizeof ( LinkedList ) * size );

	for ( int i=0; i<size; i++ ) {
		_intHashTable[i] = new LinkedList;
		_charHashTable[i] = new LinkedList;
	}

	_hashTableSize = size;
}

HashTable::~HashTable()
{
	for ( int i=0; i<_hashTableSize; i++ ) {
		delete _intHashTable[i];
		delete _charHashTable[i];
	}

	free ( _intHashTable );
	free ( _charHashTable );

	init();
}

void HashTable::add ( HashableObject *obj )
{
	obj->setHashValues();
	_intHashTable[obj->intHashValue() % _hashTableSize]->add ( obj );
	_charHashTable[obj->charHashValue() % _hashTableSize]->add ( obj );
}

void HashTable::del ( HashableObject *obj )
{
	LinkedList *list = NULL;

	list = _intHashTable[obj->intHashValue() % _hashTableSize];

	if ( list )
		list->del ( obj );

	list = _charHashTable[obj->charHashValue() % _hashTableSize];

	if ( list )
		list->del ( obj );
}

void HashTable::release ( void )
{
	for ( int i=0; i<_hashTableSize; i++ ) {
		_intHashTable[i]->release();
		_charHashTable[i]->release();
	}
}

HashableObject *HashTable::findByInt ( int val )
{
	LinkedList *list = _intHashTable[abs ( val ) % _hashTableSize];

	if ( !list )
		return NULL;

	LinkedElement *element = list->head();

	while ( element ) {
		HashableObject *obj = (HashableObject *)element->ptr();

		if ( obj->hashMatch ( val ) ) {
			return obj;
		}

		element = element->next();
	}

	return NULL;
}

HashableObject *HashTable::findByStr ( char *str )
{
	int val = hashString ( str ) % _hashTableSize;
	LinkedList *list = _charHashTable[val];

	if ( !list )
		return NULL;

	LinkedElement *element = list->head();

	while ( element ) {
		HashableObject *obj = (HashableObject *)element->ptr();

		if ( obj->hashMatch ( str ) ) {
			return obj;
		}

		element = element->next();
	}

	return NULL;
}

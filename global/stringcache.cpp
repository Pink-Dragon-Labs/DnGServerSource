//
// String Cache
//
// manages a pool of strings to eliminate duplicate storage
//
// Author: Michael Nicolella
//

#include"stringcache.hpp"
#include"logmgr.hpp"
#include"fatal.hpp"

const char* StringCache::Buffer::allocateString( const char* str )
{
	unsigned int strLength = strlen( str ) + 1; //add one for terminating \0

	if( strLength > BufferSize ) {
		fatal("%s:%d - Fatal error: a string of size %d cannot fit into the string cache. Maximum length is %d (%d bytes needed)\n\nString: %s", __FILE__, __LINE__, strLength, BufferSize, strLength - BufferSize, str );
		return 0;
	}

	//allocate the string into this buffer and load it into the tree
	if( strLength <= sizeFree() ) {
		//char* strPtr = new ( _buffer + _size ) char[ strLength * sizeof char ]; //mike-future

		const char* strPtr = strcpy( &_buffer[ _size ] , str );
		_size += strLength;

		_buffer[ _size - 1 ] = '\0';

//		if( IsThisATestServer() && strlen( strPtr ) != strlen( str ) )
//			fatal("StringCache::Buffer::allocateString() - size of new string doesn't match original. Something terrible has happened.\n\nStored string: %s\n\n---\nSubmitted string: %s\n", strPtr, str );

		return strPtr;
	}
	else {
		logInfo( _LOG_ALWAYS, "Buffer::allocateString() - not enough size in buffer. %d requested, %d free", strLength, sizeFree() );
		return 0;
	}

}

StringCache::Buffer* StringCache::Manager::allocateBuffer()
{
	Buffer** newBufferTable = new Buffer*[ numBuffers + 1 ];

	//copy the old table into the new
	for( unsigned int x = 0; x < numBuffers; ++x )
		newBufferTable[x] = bufferTable[x];

	//allocate a new StringTableBuffer and stick it onto the end
	newBufferTable[ numBuffers ] = new Buffer;

	//delete the old table
	delete [] bufferTable;

	//load the new table
	bufferTable = newBufferTable;
	
	//increase the number of buffers we have
	//allocated and return the newly created one.
	return newBufferTable[ numBuffers++ ];
}


StringCache::Manager::Manager() : numBuffers(0), bufferTable(0)
{
}

StringCache::Manager::~Manager()
{
	for( int x = 0; x < numBuffers; ++x )
	{
		delete bufferTable[ x ];
	}

	delete [] bufferTable;
}

unsigned long StringCache::Manager::generateHash( const char *str )
{
	//"djb2" string hash algorithm
    unsigned long hash = 5381;
    int c;
    while (c = *str++)
		hash = ((hash << 5) + hash) + c;
    return hash;
}

const char* StringCache::Manager::submitString( const char* str )
{
	if( str == 0 ) return 0;

    //first we need to figure out if this string has already been submitted
	//and resides in one of our buffers.

	//generate a hash for this string
	unsigned long hashValue = generateHash( str );
	unsigned long strLength = strlen( str ) + 1;

	//take the hashValue and limit it by the number of lists we are managing
	unsigned long hashIndex = hashValue % Manager::NumLists;

	if( hashIndex >= Manager::NumLists )
		logInfo( _LOG_ALWAYS, "StringCache::Manager::submitString() - list index out of bounds.\n\nSubmitted string: %s\n", str );

	//this is the LinkedList that must hold information about this string
	//(either it does already, or we need to add it to it)
	LinkedList* entryList = &listArray[ hashIndex ];

	//go through the entry list until we find an entry that has the same
	//hash value and stringcompare's to the same string.

	LinkedElement* entryElement = entryList->head();

	while( entryElement ) {
		StringEntry* entry = static_cast< StringEntry*>( entryElement->ptr() );
		entryElement = entryElement->next();

		if( entry && entry->hashValue == hashValue ) {
			//the hashes match - this is quite possibly the same string.
			//strcmp to verify

			if( !strcmp( entry->ptr, str ) ) {
				//same string. get this pointer and return it, we're done.
//				if( IsThisATestServer() && generateHash( entry->ptr ) != entry->hashValue ) {
//					fatal("StringCache::Manager::submitString() - hash of string doesn't match stored value. Something terrible has happened.\n\nStored string: %s\n\n---\nSubmitted string: %s\n", entry->ptr, str );
//				}

				return entry->ptr;
			}
		}
	}

	//we did not find the string stored here. we need to allocate it somewhere.
	StringEntry* newEntry = new StringEntry();
	newEntry->hashValue = hashValue;

	//loop through each buffer and ask
	for( unsigned short buf = 0; buf < numBuffers; ++buf )
	{
		Buffer* buffer = bufferTable[buf];

		if( strLength <= buffer->sizeFree() ) {
			//we can stick it here
			newEntry->ptr = buffer->allocateString( str );
			
			//add the entry to the list
			entryList->add( newEntry );
			
//			if( IsThisATestServer() && generateHash( newEntry->ptr ) != generateHash( str ) )
//				fatal("StringCache::Manager::submitString() - hash of new string doesn't match original. Something terrible has happened.\n\nStored string: %s\n\n---\nSubmitted string: %s\n", newEntry->ptr, str);
			
			return newEntry->ptr;
		}
	}

	//we don't have a buffer that has enough room for this string. need a new one
	Buffer* newBuffer = allocateBuffer();
	if( newBuffer ) {
		newEntry->ptr = newBuffer->allocateString( str );

		if( newEntry->ptr )
			entryList->add( newEntry );
		else {
			fatal( "%s:%d - StringCache::Manager::submitString() - string allocation failure.\n\nSubmitted string: %s", __FILE__, __LINE__, str );
			delete newEntry;
			return 0;
		}

//		if( IsThisATestServer() && generateHash( newEntry->ptr ) != generateHash( str ) )
//			fatal("StringCache::Manager::submitString() - hash of new string doesn't match original. Something terrible has happened.\n\nStored string: %s\n\n---\nSubmitted string: %s\n", newEntry->ptr, str );

		return newEntry->ptr;

	}
	else {
		fatal( "%s:%d - StringCache::Manager::submitString() - buffer allocation failure.\n\nSubmitted string: %s", __FILE__, __LINE__, str );
		delete newEntry;
		return 0;
	}

	fatal( "%s:%d - StringCache::Manager::submitString() - error.\n\nSubmitted string: %s", __FILE__, __LINE__, str );
	return 0;
}

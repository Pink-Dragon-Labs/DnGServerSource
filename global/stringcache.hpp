//
// String Cache
//
// manages a pool of strings to eliminate duplicate storage
//
// Author: Michael Nicolella
//

#ifndef _STRINGCACHE_H
#define _STRINGCACHE_H

#include "list.hpp"


namespace StringCache
{
	class Buffer
	{
		//256k buffer
		static const unsigned int BufferSize = 262144;

		//flat buffer to store our strings
		char* _buffer; //void* _buffer;

		//current amount allocated
		unsigned int _size;

	public:
		inline unsigned int sizeFree() const {
			if( _size < BufferSize ) return BufferSize - _size;
			else return 0;
		}

		Buffer() : _size( 0 ), _buffer( new char[ BufferSize ] )
		{
			//_buffer = operator new( buffersize * sizeof char );
		}
		
		~Buffer() {
			//operator delete( _buffer );
			delete [] _buffer;
		}

		const char* allocateString( const char* str );

	private:
		Buffer( const Buffer& );
		Buffer& operator=( const Buffer& );
	};

	//this table manages all active StringBuffers and is
	//the interface to the user
	class Manager
	{
		struct StringEntry : public ListObject
		{
			unsigned long hashValue;
			const char* ptr;
		};

		static const unsigned int NumLists = 256;

		//LinkedList of StringEntry*
		LinkedList listArray[ NumLists ];

		//this is a pointer to an array of pointers that point to each buffer
		Buffer** bufferTable;
		unsigned short numBuffers; //number of allocated buffers
		Buffer* allocateBuffer();

		unsigned long generateHash( const char *str );

	public:
		Manager();
		~Manager();

		//call this function to submit a string to the table. If it exists, it will return a pointer
		//to the existing string, otherwise it will allocate it and return a pointer to that
		const char* submitString ( const char* str );

	private:
		Manager( const Manager& );
		Manager& operator=( const Manager& );
	};
};

#endif

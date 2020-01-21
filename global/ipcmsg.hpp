//
// ipcmsg.hpp
//
// This file contains the IPCMessage class and supporting structures.
//
// author: Stephen Nichols
//

#ifndef _IPCMSG_HPP_
#define _IPCMSG_HPP_

#include "list.hpp"
#include "memorypool.hpp"

// declare the global IPCMessage allocator
extern MemoryAllocator gIPCMessageAllocator;

// enumerate the valid message states
enum {
	_IPCMSG_RCV_HEADER,
	_IPCMSG_RCV_DATA,
	_IPCMSG_SND_HEADER,
	_IPCMSG_SND_DATA
};

// enumerate the valid message allocation types
enum {
	_IPCMSG_ALLOC_MALLOC,
	_IPCMSG_ALLOC_POOL
};

//
// IPCMsgHeader: This is the header that is placed on every IPCMessage.
//
typedef struct {
	int size, type;
} IPCMessageHeader;

//
// IPCMessage: This class represents a single message and is used to receive and send messages across a socket.

#undef new

class IPCMessage : public ListObject
{
protected:
	int _type, _size, _index, _bufferSize, _state, _marker, _msgType;
	char *_data, *_buffer;
	void *_from, *_to;

	// this is the header for this message
	IPCMessageHeader _header;

public:
	IPCMessage ( int type, void *data, int size, void *from, void *to = NULL );
	IPCMessage ( IPCMessage *msg );
	IPCMessage();
	virtual ~IPCMessage();

	// The following members are for accessing private members.
	inline int &type ( void ) { return ( _type ); };
	inline char *data ( void ) { return ( _data ); };
	inline void *&from ( void ) { return ( _from ); };
	inline void *&to ( void ) { return ( _to ); };
	inline int size ( void ) { return ( _size ); };
	inline int &marker ( void ) { return _marker; };

	// This member is called to read this message from the provided socket.
	int read ( int handle, int maxMsgSize );

	// This member is called to write this message to the provided socket.
	virtual int write ( int handle );

	// This member decodes a message using the supplied seed, and returning the new one.
	int decode( int* nSeed );
	void encode( int* nSeed );
	void Dump( char* pMsg );

	// All IPCMessages are allocated via the MemoryPool.
	void* operator new ( size_t size, char* file, int nLine ) {
		return gIPCMessageAllocator.allocate();
	};

	// All IPCMessages are deleted via the MemoryPool.
	void operator delete ( void *ptr ) {
		gIPCMessageAllocator.deallocate ( (char *)ptr );
	};
};

#define new new( __FILE__, __LINE__ )

class IPCBLE_Message : public IPCMessage
{
public:
	IPCBLE_Message ( int type, void *data, int size, void *from, void *to = NULL );
	IPCBLE_Message ( IPCMessage *msg );
	IPCBLE_Message();

	// This member is called to write this message to the provided socket.
	virtual int write ( int handle );
};

#endif

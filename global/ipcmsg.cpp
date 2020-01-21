//
// ipcmsg.cpp
//
// This file contains the IPCMessage class implementation.
//
// author: Stephen Nichols
//

#include "system.hpp"

// define the memory allocator for IPCMessages
MemoryAllocator gIPCMessageAllocator ( sizeof ( IPCMessage ), 10000 );

// define the size of the preallocated messages
#define _IPCMSG_MAX_PREALLOC_SIZE 	1024

// define the MemoryAllocator that represents the messages that fall below _IPCMSG_MAX_PREALLOC_SIZE
MemoryAllocator gIPCMsgBuffers ( _IPCMSG_MAX_PREALLOC_SIZE, 1000 );

//
// This constructor initializes this message via the provided data.
//

IPCMessage::IPCMessage ( int type, void *data, int size, void *from, void *to )
{
	int nSize = size;

	// set the type
	_type = type;

	// determine the message allocation method based on size (default to malloc)
	if ( data && nSize ) {
		nSize += 7;				//	Make it an even size.
		nSize &= 0xfffffffc;

		// use the pool if the size is within limits
		if ( nSize <= _IPCMSG_MAX_PREALLOC_SIZE ) {
			_msgType = _IPCMSG_ALLOC_POOL;
			_data = (char *)gIPCMsgBuffers.allocate();
		} else {
			// otherwise use malloc
			_msgType = _IPCMSG_ALLOC_MALLOC;
			_data = (char *)malloc ( nSize );
		}

		// copy the provided data to the message buffer
		memcpy ( _data, data, size );
		_size = nSize;
	} else {
		_data = NULL;
		_size = 0;
	}

	// finish initting the message
	_from = from;
	_to = to;
	_index = 0;
	_bufferSize = 0;
	_state = _IPCMSG_RCV_HEADER;
	_marker = 0;
}

//
// This constructor copies another IPCMessage.
//

IPCMessage::IPCMessage ( IPCMessage *msg )
{
	_type = msg->type();
	int size = msg->size();
	char *data = msg->data();

	// determine the message allocation method based on size (default to malloc)
	_msgType = _IPCMSG_ALLOC_MALLOC;

	if ( data && size ) {
		// use the pool if the size is within limits
		if ( size <= _IPCMSG_MAX_PREALLOC_SIZE ) {
			_msgType = _IPCMSG_ALLOC_POOL;
			_data = (char *)gIPCMsgBuffers.allocate();
		} else {
			// otherwise use malloc
			_data = (char *)malloc ( size );
		}

		// copy the provided data to the message buffer
		memcpy ( _data, data, size );
	} else {
		_data = NULL;
		_size = 0;
	}

	// finish initting the message
	_size = size;
	_from = msg->from();
	_to = msg->to();
	_index = 0;
	_bufferSize = 0;
	_state = _IPCMSG_RCV_HEADER;
	_marker = msg->marker();
}

//
// This is the default constructor.
//

IPCMessage::IPCMessage()
{
	_type = 0;
	_size = 0;
	_data = NULL;
	_from = NULL;
	_to = NULL;
	_index = 0;
	_bufferSize = 0;
	_state = _IPCMSG_RCV_HEADER;
	_marker = 0;
	_msgType = _IPCMSG_ALLOC_MALLOC;
}

// 
// This is the destructor.
//

IPCMessage::~IPCMessage()
{
	_type = 0;
	_size = 0;
	
	// free the _data via the proper deallocation method
	if ( _data ) {
		switch ( _msgType ) {
			case _IPCMSG_ALLOC_MALLOC: 
				free ( _data );
				break;

			case _IPCMSG_ALLOC_POOL:
				gIPCMsgBuffers.deallocate ( _data );
				break;
		}

		_data = NULL;
	}

	_from = NULL;
}

//
// This member writes as much of the remaining message data as possible to the provided socket handle.
//

int IPCMessage::write ( int handle )
{
	// validate the state
	if ( _state < _IPCMSG_SND_HEADER )
		_state = _IPCMSG_SND_HEADER;

	for ( ;; ) {
	  	// if no data has been sent, setup for header send
	  	if ( _state == _IPCMSG_SND_HEADER && !_index && !_bufferSize ) {

	  		// fill in the header
	  		_header.size = _size;
	  		_header.type = _type;

	  		// setup the buffer
	  		_buffer = (char *)&_header;
	  		_bufferSize = sizeof ( _header );
			_index = 0;
	  	}

		// setup for sending data
		else if ( _state == _IPCMSG_SND_DATA && !_index && !_bufferSize ) {
			_buffer = (char *)_data;
			_bufferSize = _size;
		}

		// write as much of the buffer as possible
		int result = ::write ( handle, _buffer + _index, _bufferSize );

		// handle any errors...
		if ( result < 0 ) {
			if ( errno == EAGAIN || errno == EINTR )
				return -1;
	
			return -2;
		} else {
			// update the buffer info...
			_bufferSize -= result;
			_index += result;
		}

		// return if there is any buffer left
		if ( _bufferSize )
			return -1;

		// handle changing states and continue sending
		if ( _state == _IPCMSG_SND_HEADER ) {
			_state = _IPCMSG_SND_DATA;
			_index = 0;
			_bufferSize = 0;
		}

		// if we just finished sending the data, return success
		else if ( _state == _IPCMSG_SND_DATA ) {
			return 0;
		}
	} 

	return 0;
}

int IPCMessage::read ( int handle, int maxMsgSize )
{
	for ( ;; ) {
	  	// if no data has been read, setup for header read
	  	if ( _state == _IPCMSG_RCV_HEADER && !_index && !_bufferSize ) {
  			// setup the buffer
  			_buffer = (char *)&_header;
  			_bufferSize = sizeof ( _header );
			_index = 0;
	  	}

		// setup for reding data
		else if ( _state == _IPCMSG_RCV_DATA && !_index && !_bufferSize ) {
			// setup the message from the header
			_size = _header.size;
			_type = _header.type;

			// validate the header data
			if ( _size < 0 || _size > maxMsgSize ) {
				if ( ( maxMsgSize != 127 ) || ( _size & 0x80ffffff ) ) {
					logInfo (  _LOG_ALWAYS, "ILLEGAL: got message %d of %d on handle %d", _type, _size, handle );
					return -3;
				} else {
					// reverse the size because of Big/Little Endian
					_size >>= 24;
				}
			}

			// allocate the data for this message based on size
			if ( _size <= _IPCMSG_MAX_PREALLOC_SIZE ) {
				_msgType = _IPCMSG_ALLOC_POOL;
				_data = (char *)gIPCMsgBuffers.allocate();
			} else {
				_msgType = _IPCMSG_ALLOC_MALLOC;
				_data = (char *)malloc ( _size );
			}
			
			// setup the buffer info
			_buffer = (char *)_data;
			_bufferSize = _size;
			_size -= 4;
			_index = 0;

			// there is no data to read, we're done
			if ( !_bufferSize )
				return 0;
		}

		// read as much of the buffer as possible
		int result = ::read ( handle, _buffer + _index, _bufferSize );

		// handle any errors...
		if ( result <= 0 ) {
			if ( errno == EAGAIN || errno == EINTR )
				return -1;
	
			return -2;
		} else {
			// update the buffer info...
			_bufferSize -= result;
			_index += result;
		}

		// return if there is any buffer left
		if ( _bufferSize )
			return -1;

		// handle changing states and continue reading
		if ( _state == _IPCMSG_RCV_HEADER ) {
			_state = _IPCMSG_RCV_DATA;
			_index = 0;
			_bufferSize = 0;
		}

		// if we just finished reading the data, return success
		else if ( _state == _IPCMSG_RCV_DATA ) {
			return 0;
		}
	} 

	return 0;
}

int IPCMessage::decode( int* nSeed ) {
	int nLen = ( _size >> 2 ) - 1;

	_type ^= SCIRandom ( nSeed );

	int* pBuf = (int*) _buffer;
	int nMask = _type;
	int nEncrypt;

	for (nEncrypt = 0;nEncrypt < nLen;nEncrypt++) {
		pBuf[ nEncrypt ] ^= SCIRandom( nSeed );

		nMask ^= pBuf[ nEncrypt ];
	}

	int nRet = 1;

	if ( _size > 4 && nMask != pBuf[ nEncrypt ] ) {
		nRet = 0;
	}

	return nRet;
}

void IPCMessage::encode( int* nSeed ) {
	int* pBuf = (int*) _data;

	_type ^= SCIRandom ( nSeed );

	int nMask = _type;
	int nEncode;

	nMask ^= pBuf[ 0 ];
	pBuf[ 0 ] ^= SCIRandom( nSeed );

	nMask ^= pBuf[ 1 ];
	pBuf[ 1 ] ^= SCIRandom( nSeed );

	if ( _size > 8 ) {
		nMask ^= pBuf[ 2 ];
		pBuf[ 2 ] ^= SCIRandom( nSeed );

		if ( _size > 12 ) {
			int nSize = ( _size >> 2 ) - 1;

			for (nEncode = 3;nEncode < nSize;nEncode++) {
				nMask ^= pBuf[ nEncode ];
			}

			pBuf[ nEncode ] = nMask;
		}
	}
}

void IPCMessage::Dump( char* pMsg ) {
	logDisplay ( "%s [% 4d] %08X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n", pMsg, _size, _type, (unsigned char) _data[0], (unsigned char) _data[1], (unsigned char) _data[2], (unsigned char) _data[3], (unsigned char) _data[4], (unsigned char) _data[5], (unsigned char) _data[6], (unsigned char) _data[7], (unsigned char) _data[8], (unsigned char) _data[9], (unsigned char) _data[10], (unsigned char) _data[11], (unsigned char) _data[12], (unsigned char) _data[13], (unsigned char) _data[14], (unsigned char) _data[15], (unsigned char) _data[16], (unsigned char) _data[17], (unsigned char) _data[18], (unsigned char) _data[19], (unsigned char) _data[20], (unsigned char) _data[21], (unsigned char) _data[22], (unsigned char) _data[23], (unsigned char) _data[24], (unsigned char) _data[25], (unsigned char) _data[26], (unsigned char) _data[27], (unsigned char) _data[28], (unsigned char) _data[29], (unsigned char) _data[30], (unsigned char) _data[31] );
}

//
// This constructor initializes this message via the provided data.
//
IPCBLE_Message::IPCBLE_Message ( int type, void *data, int size, void *from, void *to ) : IPCMessage ( type, data, size, from, to ) {}

//
// This constructor copies another IPCMessage.
//
IPCBLE_Message::IPCBLE_Message( IPCMessage * msg ) : IPCMessage ( msg ) {}

//
// This is the default constructor.
//
IPCBLE_Message::IPCBLE_Message() : IPCMessage() {}

//
// This member writes as much of the remaining message data as possible to the provided socket handle.
//
int IPCBLE_Message::write ( int handle )
{
	// validate the state
	if ( _state < _IPCMSG_SND_HEADER )
		_state = _IPCMSG_SND_HEADER;

	for ( ;; ) {
	  	// if no data has been sent, setup for header send
	  	if ( _state == _IPCMSG_SND_HEADER && !_index && !_bufferSize ) {

	  		// fill in the header
	  		_header.size = htonl ( _size );
	  		_header.type = htonl ( _type );

	  		// setup the buffer
	  		_buffer = (char *)&_header;
	  		_bufferSize = sizeof ( _header );
			_index = 0;
	  	}

		// setup for sending data
		else if ( _state == _IPCMSG_SND_DATA && !_index && !_bufferSize ) {
			_buffer = (char *)_data;
			_bufferSize = _size;
		}

		// write as much of the buffer as possible
		int result = ::write ( handle, _buffer + _index, _bufferSize );

		// handle any errors...
		if ( result < 0 ) {
			if ( errno == EAGAIN || errno == EINTR )
				return -1;
	
			return -2;
		} else {
			// update the buffer info...
			_bufferSize -= result;
			_index += result;
		}

		// return if there is any buffer left
		if ( _bufferSize )
			return -1;

		// handle changing states and continue sending
		if ( _state == _IPCMSG_SND_HEADER ) {
			_state = _IPCMSG_SND_DATA;
			_index = 0;
			_bufferSize = 0;
		}

		// if we just finished sending the data, return success
		else if ( _state == _IPCMSG_SND_DATA ) {
			return 0;
		}
	} 

	return 0;
}


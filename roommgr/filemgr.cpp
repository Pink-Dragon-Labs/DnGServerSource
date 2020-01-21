//
// filemgr
//
// This file contains the FileMgr
//
// author: Bryan Waters
//

#include "filemgr.hpp"
#include "roommgr.hpp"
#include "system.hpp"
#include "globals.hpp"

//
// FileMgr
//
// This class handles interfacing between a remote FILEMGR process and
// this server.
//

FileMgr::FileMgr() {
	maxMsgSize = 10000000;
}

FileMgr::~FileMgr() {
}

// this member is called to write a file
void FileMgr::writeFile ( char *name, void *data, int size, cbDefines callback, int context ) {
	PackedData msg;

	msg.putLong ( context );
	msg.putByte ( callback );

	msg.putString ( name );
	msg.putLong ( size );
	msg.putArray ( data, size );

	send ( _IPC_FILEMGR_PUT, msg.data(), msg.size() );
}

// this member is called to write a file via PackedData
void FileMgr::writeFile ( char *name, PackedData *data, cbDefines callback, int context ) {
	PackedData msg;
	int nLen = data->size();

	msg.putLong ( context );
	msg.putByte ( callback );

	msg.putString ( name );
	msg.putLong ( nLen );
	msg.putArray ( data->data(), nLen );

	send ( _IPC_FILEMGR_PUT, msg.data(), msg.size() );
}

// this member tries to create a file if it does not already exist
void FileMgr::exclusiveCreate ( char *name, cbDefines callback, int context ) {
	PackedData msg;

	msg.putLong ( context );
	msg.putByte ( callback );

	msg.putString ( name );

	send ( _IPC_FILEMGR_EXCLUSIVE_CREATE, msg.data(), msg.size() );
}

// this member is called to read a file
void FileMgr::readFile ( char *name, cbDefines callback, int context ) {
	PackedData msg;

	msg.putLong ( context );
	msg.putByte ( callback );

	msg.putString ( name );

	send ( _IPC_FILEMGR_GET, msg.data(), msg.size() );
}

// this member is called to erase a file
void FileMgr::eraseFile ( char *name, cbDefines callback, int context ) {
	PackedData msg;

	msg.putLong ( context );
	msg.putByte ( callback );

	msg.putString ( name );

	send ( _IPC_FILEMGR_ERASE, msg.data(), msg.size() );
}

// this member is called to append a string to a file (usually for logs)
void FileMgr::appendToFile ( char *name, char *str, cbDefines callback, int context ) {
	PackedData msg;
	int nLen = strlen( str );

	msg.putLong ( context );
	msg.putByte ( callback );

	msg.putString ( name );
	msg.putLong ( nLen );
	msg.putArray ( str, nLen );

	send ( _IPC_FILEMGR_APPEND, msg.data(), msg.size() );
}

// this member is called to append a packdata to a file 
void FileMgr::appendToFile ( char *name, PackedData *packet, cbDefines callback, int context ) {
	PackedData msg;
	int nLen = packet->size();

	msg.putLong ( context );
	msg.putByte ( callback );

	msg.putString ( name );
	msg.putLong ( nLen );
	msg.putArray ( packet->data(), nLen );

	send ( _IPC_FILEMGR_APPEND, msg.data(), msg.size() );
}


// this member is called to verify a file exists
void FileMgr::exists ( char *name, cbDefines callback, int context ) {
	PackedData msg;

	msg.putLong ( context );
	msg.putByte ( callback );

	msg.putString ( name );

	send ( _IPC_FILEMGR_EXISTS, msg.data(), msg.size() );
}

// This function handles incoming messages.
void FileMgr::handleMessage ( IPCMessage *message ) {
	clock_t nStartTime = clock();

	switch ( message->type() ) {
		case _IPC_FILEMGR_PUT:
		case _IPC_FILEMGR_APPEND:
		case _IPC_FILEMGR_ERASE:
		case _IPC_FILEMGR_EXISTS:
		case _IPC_FILEMGR_EXCLUSIVE_CREATE:
		case _IPC_FILEMGR_GET: {
			PackedData packet ( message->data(), message->size() );

			switch ( packet.getByte() ) {	//	Switch on the callback
				case cbReadWhatsNew:
					ReadWhatsNew( &packet );
					break;

				case cbReadMailFile:
					ReadMailFile ( &packet );
					break;
			}
			}

			break;

		default:
			IPCClient::handleMessage ( message );
			break;
	}

  if ( ( message->type() > -1 ) && ( message->type() < _IPC_MAX_MESSAGES ) ) {
		clock_t nEndTime = clock() - nStartTime;

	  IPCStats::Msgs[ message->type() ].addExecTime( nEndTime, message );
  }
}

// this function is called to erase a file without a callback
void eraseFile ( const char *format, ... )
{
	char filename[1024];

	va_list args;
	va_start ( args, format );
	vsprintf ( sizeof ( filename ), filename, format, args );
	va_end ( args );

	gFileMgr->eraseFile ( filename, cbNone );
}

void ReadWhatsNew ( PackedData* packet ) {
	int nContext = packet->getLong();
	
	if (  packet->getLong() == _FM_RESULT_OK ) {
		WorldObject *obj = roomMgr->findObject ( nContext );

		if ( !obj || !obj->player )
			return;

		int nSize = packet->getLong();

		if ( nSize ) {
			PackedMsg response;
			response.grow ( 10240 );

			char *ptr = packet->getData();
			int bufferSize = nSize;
			char str[2048];

			while ( bufgets ( str, &ptr, &bufferSize, sizeof ( str ) ) ) {
				response.putByte ( 1 );
				response.putString ( str );
			}

			response.putByte ( 0 );

			roomMgr->sendTo ( _IPC_WHATS_NEW, response.data(), response.size(), obj->player );
		}
	}
}


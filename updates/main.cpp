//
// MAIN.CPP
//
// Main module for automatic update server.
//

//#include "../global/system.hpp"
//#include <sys/resource.h>
//#include <algorithm>
#include <sys/resource.h>
#include <algorithm>
#include "../global/system.hpp"
//#include "ipc.hpp"
//#include "new.hpp"
//#include "file.hpp"
//#include "packdata.hpp"
//#include "packmsg.hpp"
//#include "fatal.hpp"
//#include "logmgr.hpp"
//#include "configmgr.hpp"
#define _BLOCK_SIZE 3072

#define _PKG_BASIC 		1
#define _PKG_COMBAT		2

LinkedList patches;
int currentVersion = 0;

class PatchInfo : public ListObject
{
public:
	PatchInfo ( char *str ) {
		name = strdup ( str );
		data = NULL;

		if ( exists ( str ) ) {
			File file ( str );
			size = file.size();

			data = (char *)malloc ( size );
			file.read ( data, size );
		}
	};

	virtual ~PatchInfo() {
		if ( name ) {
			free ( name );
			name = NULL;
		}

		if ( data ) {
			free ( data );
			data = NULL;
		}
	};

	char *name, *data;
	int size;
};

class UpdateServer : public IPCServer 
{
public:
	void handleMessage ( IPCMessage *message );
};

void UpdateServer::handleMessage ( IPCMessage *message )
{
	switch ( message->type() ) {
		case _IPC_PATCH_INFO: {
			PackedData packet ( message->data(), message->size() );

			// get the version number to give info on
			int version = packet.getLong();

			// new 
			int badPacket = 0;

			// new
			if ( version < 0 || version > 65000 )
				badPacket = 1;
				
			int package = 0;

			LinkedList *list = (LinkedList *)patches.at ( package );
			PatchInfo *patch;

			if ( !badPacket ) 
				patch = (PatchInfo *)list->at ( version );
			else 
				patch = NULL;

			if ( patch ) {
				int patchSize = patch->size / _BLOCK_SIZE;

				if ( patch->size % _BLOCK_SIZE )
					patchSize++;

				PackedMsg response;
				response.putACKInfo ( message->type(), patchSize );

				send (
					_IPC_PLAYER_ACK,
					(char *)response.data(),
					response.size(),
					(IPCClient *)message->from()
				);
			} else {
				PackedMsg response;
				response.putACKInfo ( message->type() );
	
				send ( 
					_IPC_PLAYER_NAK,
					(char *)response.data(),
					response.size(),
					(IPCClient *)message->from()
				);
			}
		}

		break;

		case _IPC_PATCH_REQUEST: {
			// send the most current version number to the client
			PackedMsg response;
			response.putACKInfo ( message->type(), currentVersion );

			send ( 
				_IPC_PLAYER_ACK, 
				(char *)response.data(), 
				response.size(), 
				(IPCClient *)message->from() 
			); 
		}

		break;

		case _IPC_PATCH_BLOCK: {
			PackedData packet ( message->data(), message->size() );

			// get the version number
			int version = packet.getLong();

			// new 
			int badPacket = 0;

			// new
			if ( version > 65000 ) {
				printf ( "bad version == %d\n", version );
				badPacket = 1;
			}
				
			int package = 0;
			LinkedList *list = (LinkedList *)patches.at ( package );

			// get the block number
			int block = packet.getLong();

			if ( block < 0 ) 
				badPacket = 1;

			PatchInfo *patch;

			if ( !badPacket ) 
				patch = (PatchInfo *)list->at ( version );
			else 
				patch = NULL;

			if ( patch ) {
				// calculate byte index of the requested block
				int index = block * _BLOCK_SIZE;
				int size = std::min(patch->size, _BLOCK_SIZE);//_BLOCK_SIZE <? patch->size - index;

				PackedMsg response;
				response.putACKInfo ( message->type(), size );
				response.putArray ( patch->data + index, size );

				send (
					_IPC_PLAYER_ACK,
					(char *)response.data(),
					response.size(),
					(IPCClient *)message->from()
				);
			} else {
				PackedMsg response;
				response.putACKInfo ( message->type() );

				send ( 
					_IPC_PLAYER_NAK,
					(char *)response.data(),
					response.size(),
					(IPCClient *)message->from()
				);
			}
		}

		break;

		case _IPC_OLD_PATCH_INFO: {
			PackedData packet ( message->data(), message->size() );

			// get the version number to give info on
			int version = packet.getLong();

			// new 
			int badPacket = 0;

			// new
			if ( version < 0 || version > 65000 )
				badPacket = 1;
				
			int package = 0;

			LinkedList *list = (LinkedList *)patches.at ( package );
			PatchInfo *patch;

			if ( !badPacket ) 
				patch = (PatchInfo *)list->at ( version );
			else 
				patch = NULL;

			if ( patch ) {
				int patchSize = patch->size / _BLOCK_SIZE;

				if ( patch->size % _BLOCK_SIZE )
					patchSize++;

				PackedMsg response;
				response.putACKInfo ( message->type(), patchSize );

				send (
					_IPC_PLAYER_ACK,
					(char *)response.data(),
					response.size(),
					(IPCClient *)message->from()
				);
			} else {
				PackedMsg response;
				response.putACKInfo ( message->type() );
	
				send ( 
					_IPC_PLAYER_NAK,
					(char *)response.data(),
					response.size(),
					(IPCClient *)message->from()
				);
			}
		}

		break;

		case _IPC_OLD_PATCH_REQUEST: {
			// send the most current version number to the client
			PackedMsg response;
			response.putACKInfo ( message->type(), currentVersion );

			send ( 
				_IPC_PLAYER_ACK, 
				(char *)response.data(), 
				response.size(), 
				(IPCClient *)message->from() 
			); 
		}

		break;

		case _IPC_OLD_PATCH_BLOCK: {
			PackedData packet ( message->data(), message->size() );

			// get the version number
			int version = packet.getLong();

			// new 
			int badPacket = 0;

			// new
			if ( version > 65000 ) {
				printf ( "bad version == %d\n", version );
				badPacket = 1;
			}
				
			int package = 0;
			LinkedList *list = (LinkedList *)patches.at ( package );

			// get the block number
			int block = packet.getLong();

			if ( block < 0 ) 
				badPacket = 1;

			PatchInfo *patch;

			if ( !badPacket ) 
				patch = (PatchInfo *)list->at ( version );
			else 
				patch = NULL;

			if ( patch ) {
				// calculate byte index of the requested block
				int index = block * _BLOCK_SIZE;
				int size = std::min(patch->size, _BLOCK_SIZE);//_BLOCK_SIZE <? patch->size - index;

				PackedMsg response;
				response.putACKInfo ( message->type(), size );
				response.putArray ( patch->data + index, size );

				send (
					_IPC_PLAYER_ACK,
					(char *)response.data(),
					response.size(),
					(IPCClient *)message->from()
				);
			} else {
				PackedMsg response;
				response.putACKInfo ( message->type() );

				send ( 
					_IPC_PLAYER_NAK,
					(char *)response.data(),
					response.size(),
					(IPCClient *)message->from()
				);
			}
		}

		break;

		case ( _IPC_OLD_PATCH_REQUEST << 24 ): {
			// send the most current version number to the client
			PackedMsg response;
			response.putBLE_ACKInfo ( _IPC_OLD_PATCH_REQUEST, currentVersion );

			BLE_send ( 
				_IPC_PLAYER_ACK, 
				(char *)response.data(), 
				response.size(), 
				(IPCClient *)message->from() 
			); 
		}

		break;
		
		case ( _IPC_OLD_PATCH_INFO << 24 ): {
				PackedData packet ( message->data(), message->size() );

				// get the version number to give info on
				int version = packet.getBLE_Long();

				// new 
				int badPacket = 0;

				// new
				if ( version < 0 || version > 65000 )
					badPacket = 1;
					
				int package = 0;

				LinkedList *list = (LinkedList *)patches.at ( package );
				PatchInfo *patch;

				if ( !badPacket ) 
					patch = (PatchInfo *)list->at ( version );
				else 
					patch = NULL;

				if ( patch ) {
					int patchSize = patch->size / _BLOCK_SIZE;

					if ( patch->size % _BLOCK_SIZE )
						patchSize++;

					PackedMsg response;
					response.putBLE_ACKInfo ( _IPC_OLD_PATCH_INFO, patchSize );

					BLE_send (
						_IPC_PLAYER_ACK,
						(char *)response.data(),
						response.size(),
						(IPCClient *)message->from()
					);
				} else {
					PackedMsg response;
					response.putBLE_ACKInfo ( _IPC_OLD_PATCH_INFO );
		
					BLE_send ( 
						_IPC_PLAYER_NAK,
						(char *)response.data(),
						response.size(),
						(IPCClient *)message->from()
					);
				}
			}

			break;

		case ( _IPC_OLD_PATCH_BLOCK << 24 ): {
				PackedData packet ( message->data(), message->size() );

				// get the version number
				int version = packet.getBLE_Long();

				// new 
				int badPacket = 0;

				// new
				if ( version > 65000 ) {
					printf ( "bad version == %d\n", version );
					badPacket = 1;
				}
					
				int package = 0;
				LinkedList *list = (LinkedList *)patches.at ( package );

				// get the block number
				int block = packet.getBLE_Long();

				if ( block < 0 ) 
					badPacket = 1;

				PatchInfo *patch;

				if ( !badPacket ) 
					patch = (PatchInfo *)list->at ( version );
				else 
					patch = NULL;

				if ( patch ) {
					// calculate byte index of the requested block
					int index = block * _BLOCK_SIZE;
					int size = std::min(patch->size, _BLOCK_SIZE);//_BLOCK_SIZE <? patch->size - index;

					PackedMsg response;
					response.putBLE_ACKInfo ( _IPC_OLD_PATCH_BLOCK, size );
					response.putArray ( patch->data + index, size );

					BLE_send (
						_IPC_PLAYER_ACK,
						(char *)response.data(),
						response.size(),
						(IPCClient *)message->from()
					);
				} else {
					PackedMsg response;
					response.putBLE_ACKInfo ( _IPC_OLD_PATCH_BLOCK );

					BLE_send ( 
						_IPC_PLAYER_NAK,
						(char *)response.data(),
						response.size(),
						(IPCClient *)message->from()
					);
				}
			}

			break;

		case _IPC_CLIENT_CONNECTED: {
			IPCServer::handleMessage ( message );

			((IPCClient*) message->from() )->maxMsgSize = 127;
			}

			break;

		case _IPC_CLIENT_HUNG_UP:
			IPCServer::handleMessage ( message );

		break;
	}
}

void parsePatchDirectory ( char *name )
{
	FILE *file = fopen ( name, "rt" );

	if ( file ) {
		char str[1024];
		LinkedList *list = new LinkedList;
		patches.add ( list );

		while ( !feof ( file ) && fgets ( str, 1024, file ) ) {
			char *ptr = strchr ( str, '\n' );

			if ( ptr )
				*ptr = 0;

			if ( strlen ( str ) && exists ( str ) ) {
				list->add ( new PatchInfo ( str ) );
			} else {
				fatal ( "invalid file name in '%s'", name );
			}
		}

		fclose ( file );
	}
}

int main ( int argc, char **argv )
{
	if ( argc != 2 ) {
		logDisplay ( "usage: %s [config file]", argv[0] );
		return 1;
	}

	// load the config options
	ConfigMgr config;
	config.load ( argv[1] );

	char *port = config.get ( "port" );
	char *patchFile = config.get ( "patchFile" );
	
	struct rlimit limit;

	getrlimit ( RLIMIT_NOFILE, &limit );
	limit.rlim_cur = limit.rlim_max;
	setrlimit ( RLIMIT_NOFILE, &limit );

	sysLogLevel = _LOG_ALWAYS;
	sysLogDisplay = 1;

	parsePatchDirectory ( patchFile );

	currentVersion = ((LinkedList *)patches.at ( 0 ))->size();

	printf ( "current version is %d\n", currentVersion );

	UpdateServer *server = new UpdateServer;
	server->init ( port );

	logDisplay ( "\nupdate server running..." );

	for ( ;; ) {
		gIPCPollMgr.doit();
		server->doit();
	}


	return -1;
}

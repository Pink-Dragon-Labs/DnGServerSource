//
// MAIN.CPP
//
// Main module for automatic update server.
//

#include "../global/system.hpp"
#include "../global/sql.hpp"
#include <sys/resource.h>

SQLDatabase *gSQL = NULL;
char *serverList;

//
// class RouteInfo: Simple data holding class for representing the IP and port
// of a server.
//
class RouteInfo : public ListObject
{
public:
	char *ip, *port;

	RouteInfo ( char *theIP, char *thePort ) { 
		ip = strdup ( theIP ); 
		port = strdup ( thePort ); 
	};

	virtual ~RouteInfo() {};
};
RouteInfo *info = NULL;

// 
// Definition of the RoutingServer.
//

class RoutingServer : public IPCServer 
{
public:
	void handleMessage ( IPCMessage *message );
};

void RoutingServer::handleMessage ( IPCMessage *message )
{
	switch ( message->type() ) {
		case _IPC_ROUTE_INFO: {
			PackedData packet ( message->data(), message->size() );
			unsigned char cmd = packet.getByte();

			switch ( cmd ) {
				case 0: {
					PackedMsg response;
					response.putACKInfo ( message->type() );
					response.putString ( info->ip );
					response.putString ( info->port );

					send (
						_IPC_PLAYER_ACK,
						(char *)response.data(),
						response.size(),
						(IPCClient *)message->from()
					);
				}

				break;

				case 1: {
					// ask the SQL database for the most recent list of game servers
					SQLResponse *servers = gSQL->query ( "select name, isUp, userCount, ip, port from %s where active='yes'", serverList );

					if ( servers ) {
						PackedMsg response;
						response.putACKInfo ( message->type()  );
						response.putByte ( servers->rows );

						for ( int r=0; r<servers->rows; r++ ) {
							for ( int i=0; i<5; i++ )
								response.putString ( servers->table ( r, i ) );
						}

						send (
							_IPC_PLAYER_ACK,
							(char *)response.data(),
							response.size(),
							(IPCClient *)message->from()
						);

						delete servers;
					}
				}
		
				break;
			}
		}

		break;

		case _IPC_OLD_ROUTE_INFO: {
			PackedData packet ( message->data(), message->size() );
			char *cmd = packet.getString();

			logDisplay( "Old client has connected...");

			if ( strstr ( cmd, "updates " ) ) {
				PackedMsg response;
				response.putACKInfo ( message->type() );
				response.putString ( info->ip );
				response.putString ( info->port );

				send (
					_IPC_PLAYER_ACK,
					(char *)response.data(),
					response.size(),
					(IPCClient *)message->from()
				);
			}

			else if ( strstr ( cmd, "gamelist " ) ) {
				// ask the SQL database for the most recent list of game servers
				SQLResponse *servers = gSQL->query ( "select name, isUp, userCount, ip, port from %s where active='yes'", serverList );

				if ( servers ) {
					PackedMsg response;
					response.putACKInfo ( message->type()  );
					response.putByte ( servers->rows );

					for ( int r=0; r<servers->rows; r++ ) {
						for ( int i=0; i<5; i++ )
							response.putString ( servers->table ( r, i ) );
					}

					send (
						_IPC_PLAYER_ACK,
						(char *)response.data(),
						response.size(),
						(IPCClient *)message->from()
					);

					delete servers;
				}
			}

			// old router handling...
			else if ( strstr ( cmd, "roommgr " ) ) {
				// ask the SQL database for the most recent list of game servers
				SQLResponse *servers = gSQL->query ( "select ip, port from %S where id=0", serverList );

				if ( servers ) {
					PackedMsg response;
					response.putACKInfo ( message->type()  );
					response.putString ( servers->table ( 0, 0 ) );
					response.putString ( servers->table ( 0, 1 ) );
	
					send (
						_IPC_PLAYER_ACK,
						(char *)response.data(),
						response.size(),
						(IPCClient *)message->from()
					);

					delete servers;
				}
			}

			if ( cmd )
				free ( cmd );
		}

		break;

		case _IPC_CLIENT_CONNECTED: {
			IPCServer::handleMessage ( message );

			IPCClient *client = (IPCClient *)message->from();
			client->maxMsgSize = 127;
			}

			break;

		case _IPC_CLIENT_HUNG_UP:
			IPCServer::handleMessage ( message );
			break;
	}
}

int main ( int argc, char **argv )
{
	logDisplay("Starting routing server...\n");
	if ( argc != 2 ) {
		logDisplay ( "usage: %s [config file]", argv[0] );
		return 1;
	}

	// load all of the config information
	ConfigMgr config;
	config.load ( argv[1] );

	// get SQL database info
	char *sqlServer = config.get ( "sqlServer" );
	char *sqlDB = config.get ( "sqlDB" );
	char *sqlUser = config.get ( "sqlUser" );
	char *sqlPW = config.get ( "sqlPW" );
    serverList = config.get ( "serverList" );

	gSQL = new SQLDatabase ( sqlServer, sqlUser, sqlPW, sqlDB );

	// get the port name of listen on
	char *port = config.get ( "port" );

	char *ip = config.get ( "updateIP", 0 );

	if ( ip ) {

		char *port = config.get ( "updatePort", 0 );

		if ( port ) {

			logDisplay ( "Routing to update server at %s:%s", ip, port );
			info = new RouteInfo ( ip, port );
		}
		else{
			logDisplay( "Please specify an updatePort in your config file...");
		}
	}
	else{
		logDisplay( "Please specify an updateIP in your config file...");
	}
		
	
	struct rlimit limit;

	getrlimit ( RLIMIT_NOFILE, &limit );
	limit.rlim_cur = limit.rlim_max;
	setrlimit ( RLIMIT_NOFILE, &limit );

	sysLogLevel = _LOG_ALWAYS;
	sysLogDisplay = 1;

	RoutingServer *server = new RoutingServer;
	server->init ( port );

	logDisplay ( "\nRouting server listening for messages..." );

	for ( ;; ) {
		gIPCPollMgr.doit();
		server->doit();
	}

	return -1;
}

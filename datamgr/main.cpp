//
// main
//
// This module contains the startup code for the DataMgr.
//
// author: Stephen Nichols
//

#include "../global/system.hpp"
#include "datamgr.hpp"

int main ( int argc, char **argv ) {
	// validate the argument list
	if ( argc != 2 )
		fatal ( "usage %s config-file-path", argv[0] );

	// load the config file
	ConfigMgr config;

	if ( config.load ( argv[1] ) == -1 )
		fatal ( "could not load config file '%s'", argv[1] );

	// get the config information 
	char *sqlServer = config.get ( "sqlServer" );
	char *sqlDB = config.get ( "sqlDB" );
	char *sqlUser = config.get ( "sqlUser" );
	char *listenHost = config.get ( "listenHost" );
	char *listenPort = config.get ( "listenPort" );
	char *sqlPW = config.get ( "sqlPW" );

	// init the data manager to listen on the listenPort
	gDataMgr.init ( listenPort, listenHost );

	// setup the sql database
	gDataMgr.initSQL ( sqlServer, sqlUser, sqlPW, sqlDB );

	// no servers can be up right now, update the database
	gDataMgr.sql->query ( "update serverList set isUp='down', userCount=0" );

	// display welcome message
	logDisplay ( "DataMgr running..." );

	// loop forever
	for ( ;; ) {
		gIPCPollMgr.doit();
		gDataMgr.doit();
	}

	return 0;
}

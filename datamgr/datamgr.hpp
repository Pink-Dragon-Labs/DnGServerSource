//
// datamgr
//
// This file contains the DataMgr class and supporting structures.
//
// author: Stephen Nichols
//

#ifndef _DATAMGR_HPP_
#define _DATAMGR_HPP_

#include "../global/ipc.hpp"
#include "../global/sql.hpp"
#include "../global/datamgrdefs.hpp"

// this is the maximum number of servers allowed to be registered
#define _MAX_SERVER 256

// Any changes to this struct need to be made also in the roommgr/rmplayer.hpp file!!!!
// Any changes to this struct need to be made also in the roommgr/rmplayer.hpp file!!!!
struct CrimeData {
        int criminal, loadTime;
        unsigned murders;
        unsigned pickedPockets;
        unsigned tourneyKills, tourneyDeaths;
        unsigned arenaKills, arenaDeaths;
        unsigned criminalsKilled;
        unsigned bountyCollected;
        unsigned bountyOnHead;
        int timeLeft;
};
// Any changes to the above struct need to be made also in the roommgr/rmplayer.hpp file!!!!
// Any changes to the above struct need to be made also in the roommgr/rmplayer.hpp file!!!!

//
// DataMgr: This class is based on the IPCServer class and handles incoming
// _DATAMGR_ messages.
//

class DataMgr : public IPCServer
{
public:
	DataMgr();
	virtual ~DataMgr();

	// this member is called to init the SQL database
	void initSQL ( char *server, char *user, char *pass, char *db );

	// this member is called to handle incoming messages
	virtual void handleMessage ( IPCMessage *msg );

	// this is the main SQL database connection
	SQLDatabase *sql;
};

// this is the global datamgr
extern DataMgr gDataMgr;

// this is the global table of character tables
extern char *gCharacterTables[_MAX_SERVER];

// this is the global table of house tables
extern char *gHouseTables[_MAX_SERVER];

#endif

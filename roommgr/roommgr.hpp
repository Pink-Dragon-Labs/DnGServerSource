/*
	room management process
	author: Stephen Nichols
*/

#ifndef _ROOMMGR_HPP_ 
#define _ROOMMGR_HPP_ 

#define _COMBAT_TURN_BASED

#define _DUH_ 1

#include "../global/system.hpp"
#include "magic.hpp"
#include "dice.hpp"
#include "wobject.hpp"
#include "rmplayer.hpp"
#include "npc.hpp"
#include "npcs.hpp"
#include "quest.hpp"
#include "rmroom.hpp"
#include "rmparser.hpp"
#include "worldlib.hpp"
#include "tables.hpp"
#include "talktree.hpp"
#include "timesys.hpp"
#include "zone.hpp"
#include "characters.hpp"
#include "combat.hpp"
#include "../global/packmsg.hpp"
#include "special.hpp"
#include "squirmy.hpp"
#include "channel.hpp"
#include "recipe.hpp"
#include "skills.hpp"
#include "rmtools.hpp"
#include "bountylist.hpp"
#include "tracking.hpp"
#include "house.hpp"
#include "login.hpp"
#include "datamgrclient.hpp"

#define _RM_CACHE_MAX	10240
#define _ROOM_LIMIT 17

#define _MAX_SERVID (unsigned int)0xFFFFFFFF
#define _OBJECT_CHUNK_SIZE 32768
#define _SERVID_TBL_SIZE 65536
#define _SERVID_MAX (_SERVID_TBL_SIZE * 32)

#define _MAX_MESSAGES 1000
extern char *gMsgTable[_MAX_MESSAGES];

class RMServer : public PlayerServer 
{
public:
	LinkedList _zones, _objects, _classes, _characters, _rooms, _gms, _guides, _hosts;
	BinaryTree classTree;
	ObjectInfoParser objectInfoParser;

	int initted;

	RMServer() {
		initted = 0;
	}

	/* handle incoming messages */
	virtual void handleMessage ( IPCMessage *message );
	void assignPlayerToThread ( RMPlayer *player );

	virtual int send ( int type, char *buffer, int size, IPCClient *client );
	virtual int send ( char *buffer, int size, IPCClient *client ) { return IPCServer::send ( buffer, size, client ); };

	RMRoom *sendRoomInfo ( int number, RMPlayer *player );
	RMRoom *sendRoomInfo ( int number, long servID );

	void sendToRoom ( int command, void *msg, int size, RMRoom *room, RMPlayer *exclusion );

	void sendToRoom ( int command, void *msg, int size, RMRoom *room );
	void sendToRoom ( int command, void *msg, int size, int number );

	void sendToList ( int command, void *msg, int size, LinkedList *list, RMPlayer *exclusion = NULL );

	void sendSystemMsg ( const char *format, ... );
	void sendSystemMsg ( const char *title, RMPlayer *player, const char *format, ... );

	void sendRoomText ( RMRoom *room, const char *format, ... );
	void sendPlayerText ( RMPlayer *player, const char *format, ... );
	void sendPlayerChat ( RMPlayer *source, RMPlayer *player, const char *format, ... );
	void sendListText ( LinkedList *list, const char *format, ... );
	void sendListChat ( RMPlayer *source, LinkedList *list, const char *format, ... );
	void sendPlayersText ( RMPlayer *player, const char *format, ... );

	void sendRoomInfo ( RMRoom *room, const char *format, ... );
	void sendPlayerInfo ( RMPlayer *player, const char *format, ... );
	void sendListInfo ( LinkedList *list, const char *format, ... );
	void sendPlayersInfo ( RMPlayer *player, const char *format, ... );

	RMRoom *findRoom ( int number );

	void addZone ( Zone *zone );
	void delZone ( Zone *zone );

	void addObject ( WorldObject *object );
	void deleteObject ( WorldObject *object );
	
	WorldObject *findClass ( const char *name );
	void addClass ( WorldObject *object );

	WorldObject *findObject ( long servID );
	WorldObject *findObjectLast ( long servID );
	WorldObject *findObject ( const char *name );
	WorldObject *findObjectByClass ( const char *name );
	WorldObject *getObject ( long servID );

	WorldObject *findComponentObject ( int skill, char *text );

	void destroyObj ( WorldObject *object, int notify, char *file, int line );
	void destroyObj ( long servID, int notify, char *file, int line );

	virtual void deletePlayer ( Player *player );
	virtual void addPlayer ( Player *player );

	void forceLogout ( void );

	// process all of the library files
	int processLibrary ( char *directory );

	// parse a zone file
	int parseZoneFile ( char *name );

	// parse an object file
	int parseObjectFile ( char *name );
};

void logChatData ( const char *format, ... );
void logHack ( const char *format, ... );

extern RMServer *roomMgr;

extern int sysStatsFlag;
extern int gMaxConnections;
extern LinkedList gRequestQueue;
extern LinkedList gLoginQueue;
extern LinkedList gDeadPlayers;
extern LinkedList gDeadRooms;
extern LinkedList gWantedList;
//extern LinkedList gSerialNumbers;
extern LinkedList gMailFiles;
extern LinkedList gComponentObjects[];

extern int gSparseArray, gHighConnections;
extern RMPlayer *gActivePlayer;
extern char gCrashMessage[1024];
extern char	gShutdownMessage[1024];

extern SparseArray *gRoomArray;

int allocateServID ( void );
void freeServID ( long servID );

extern int gCurServID, gParsingClasses, gFatalCount;

WorldObject *loadCharacterData ( RMPlayer *pPlayer, char *buffer, int size, char *ownerName, char *characterFileName );
WorldObject *loadOldCharacterData ( char *name );
WorldObject *convertObject ( char *classID );

extern BinaryTree gObjectTree;
extern SparseArray *gObjectArray;

//int findSerialNumber ( int num );
//void writeSerialNumbers ( void );
int opposedRoll ( int a, int b );

extern int gHouseExits[];
extern int gSpawnCount;
extern int gSpawnTotal;

extern int gStartTime;

extern double gExpBoost;
extern int gLootBoost;

extern WorldObject *gTreasureTbls[50][1024];

extern LinkedList gServers;

MsgProcessor* getNextServer ( void );
MsgProcessor* getNextDungeonServer ( void );

extern int bEventHappening;
extern int bEventReadOnly;
extern char* gEventTitle;
extern char* gEventInformation;

extern LinkedList gEventPlayers;
extern LinkedList gBadEventPlayers;

bool IsThisATestServer();

#endif

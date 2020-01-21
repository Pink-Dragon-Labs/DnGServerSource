#ifndef _RMROOM_HPP_
#define _RMROOM_HPP_

#include "filemgr.hpp"
#include "../global/system.hpp"
#include "datamgrclient.hpp"

#define _ROOM_OBJ_LIMIT 72

extern int gATPCount;

extern MemoryAllocator gATPInfoAllocator;

// enumerate the types of rooms
enum {
	_ROOM_SWAMP,
	_ROOM_DESERT,
	_ROOM_FOREST,
	_ROOM_MOUNTAIN,
	_ROOM_TOWN_GATE,
	_ROOM_TOWN_WALL,
	_ROOM_TOWN_NOWALL
};

// enumerate the towns
enum {
	_TOWN_LEINSTER,		// dead town!
	_TOWN_LEINSTER_EAST,	// 1
	_TOWN_LEINSTER_WEST,	// 2
	_TOWN_KURZ,				// 3
	_TOWN_USK,				// 4
	_TOWN_ASGARD,			// 5
	_TOWN_MURIAS,			// 6
	_TOWN_DRUNE,			// 7
	_TOWN_SILVERBROOK,	// 8
	_TOWN_WEN,				// 9
	_TOWN_CAER_FAWDRY,	// 10
	_TOWN_MONMOUTH,		// 11
	_TOWN_ARIMATHOR,		// 12
	_TOWN_MAX,
};

extern char gTownNames [ _TOWN_MAX ][ 15 ];

// define the direction exit bits
#define _ROOM_NORTH_BIT		1
#define _ROOM_SOUTH_BIT		2
#define _ROOM_EAST_BIT		4
#define _ROOM_WEST_BIT		8
#define _ROOM_UP_BIT			16
#define _ROOM_DOWN_BIT		32

#define _RM_FLAG_GROUP		(1 << 3)
#define _RM_FLAG_BLOCK_N	(1 << 4)
#define _RM_FLAG_BLOCK_S	(1 << 5)
#define _RM_FLAG_BLOCK_E	(1 << 6)
#define _RM_FLAG_BLOCK_W	(1 << 7)
#define _RM_FLAG_NO_MAGIC	(1 << 8)
#define _RM_FLAG_NO_DROP	(1 << 9)
#define _RM_FLAG_NO_USE		(1 << 10)

// max rooms in a building
#define _DBB_MAX_ROOMS 32

class RMRoom;
class WorldObject;
class RMPlayer;

typedef struct {
	// number of rooms in this building
	int numRooms;

	// room numbers
	int rooms[_DBB_MAX_ROOMS];
} DBBuilding;

// define the building class
class Building : public ListObject
{
public:
	Building();
	virtual ~Building();

	// add a room to this building
	void addRoom ( RMRoom *room );

	// delete a room from this building
	void delRoom ( RMRoom *room );

	// object list management
	void addObject ( WorldObject *object );
	void delObject ( WorldObject *object );

	// player list management
	void addPlayer ( RMPlayer *player );
	void delPlayer ( RMPlayer *player );

	// find a room in this building
	RMRoom *findRoom ( int number );

	// utility methods
	inline int playerCount ( void ) { return players.size(); }

	void writeHouseData ( void );
	int loadHouseData ( char *buffer, int size );
	//void loadOldHouseData ( WorldObject *character );

	void setOwnerName ( char *str );

	// properties section
	LinkedList rooms, players, objects, doors;
	int servID, destructing, disposeDelay, homeTown, isDead, resetHouse, changed, sqlID, accountID;
	char *_owner;
};

#undef new

class ATPInfo : public ListObject 
{
public:
	ATPInfo() {
		type = -1;
		x = -1;
		y = -1;
		z = -1;

		gATPCount++;
	};

	ATPInfo ( int theType, int theX, int theY, int theZ ) {
		type = theType;
		x = theX;
		y = theY;
		z = theZ;

		gATPCount++;
	};

	~ATPInfo() {
		gATPCount--;
	};

	int type, x, y, z;

	void* operator new ( size_t size, char* file, int nLine ) {
		return gATPInfoAllocator.allocate();
	};

	void operator delete ( void *ptr ) {
		gATPInfoAllocator.deallocate ( (char *)ptr );
	};
};

#define new new( __FILE__, __LINE__ )

class RMPlayer;
class WorldObject;
class Zone;

#define _DBR_THRESHOLD 149999999
#define _DBR_OFFSET (_DBR_THRESHOLD + 1)

#define _DBR_MAX_ATPS 64
#define _DBR_MAX_OBJECTS 72
#define _DBR_MAX_LINKS 10

typedef struct
{
	// visual information
	int picture;

	// owning building servID
	int building;

	// exit information
	int north, south, east, west, up, down;

	// room name
	char name[64];

	// ATP information
	int numATPs;
	ATPInfo atpList[_DBR_MAX_ATPS];

	// WorldObject information
	int numObjects;
	int objectList[_DBR_MAX_OBJECTS];

	// exit coordinates
	int exitCoords[6][2];
} DBRoom;

#undef new

class RMRoom : public LinkedList 
{
public:
	int north, south, east, west, up, down, midiFile, instanceNum, isDungeonEntrance, bAllowAmbush;
	int exitCoords[6][2];
	int bRubberWalled;
	unsigned char exits;	
	LinkedList objects, atpList, riddledObjects;
	int type, number, picture, flags, servID, isDead, tempID, groupChance, numPlayers;
	Zone *zone;
	char *title;
	Building *building;

	RMRoom();
	virtual ~RMRoom();
 
	RMRoom *clone ( void );
	void copyOther ( RMRoom* pRoom );

	// set this room to be a dungeon entrance
	void SetDungeonEntrance();

	void write ( void );
	void addPlayer ( RMPlayer *, PackedData *thePacket, int updateZone = 1 );
	void delPlayer ( RMPlayer *, int updateZone = 1 );

	int playerCount ( void );

	// handle adding and deleting objects from this room
	void addObject ( WorldObject *object, int notify = 1 );
	WorldObject *addObject ( char *name, int x, int y, int loop = 0, int notify = 1 );

	void delObject ( WorldObject *object, int notify = 1 );

	// handle adding a room link
	void addRoomLink ( RMRoom *room );
	void delRoomLink ( RMRoom *room );

	// build a packet that represents this room
	void buildPacket ( PackedData *packet );
	
	// set my properties based on a packet
	void fromPacket ( PackedData *packet );

	// see if this room has a particular set of exits
	inline int hasExit ( unsigned char exit ) { 	
		int retVal = exits & exit;
		return retVal;
	};

	// map a particular exit to it's record number
	int mapExitToRoom ( unsigned char exit );

	// call to check if a particular exit direction is blocked
	int isExitBlocked ( unsigned char exit );

	// find a player in this room with a specific name
	RMPlayer *findPlayerByName ( char *name );

	// add a new atp to this room
	void addATP ( int type, int x, int y, int z );

	// object maintenance
	void sendObjInfo ( WorldObject *object, RMPlayer *exclusion = NULL, PackedData *packet = NULL );
	void sendObjDestroy ( WorldObject *object, RMPlayer *exclusion = NULL );

	// anti-magic junk
	void setAntiMagic ( int percent, int duration );

	// process junk every game tick
	virtual void doit ( void );

	// set the title
	void setTitle ( char *str );

	void *operator new ( size_t size, char* file, int nLine ) { return db_malloc ( size, file, nLine ); }

	void operator delete ( void *ptr ) { free ( ptr ); }
};

#define new new( __FILE__, __LINE__ )

extern LinkedList gBuildings;
extern LinkedList gEmptyBuildings;

Building *findHouse ( char *name );

#define _MAX_ROOM_ID 65536
#define _ROOM_ID_TBL_SIZE (_MAX_ROOM_ID / 32)

extern int gRoomIDTbl[_ROOM_ID_TBL_SIZE];

// this function is called to create a house from a loaded buffer
Building *processHouseData ( char *name, char *buffer, int size, int houseID, int accountID );

// this function is called to start the house loading process
void loadHouse ( char *name, hlcallback_t callback, int context = 0 );

#endif

/*
	ZONE.HPP
	Classes for managing zones

	author: Stephen Nichols
	significant contributor! Michael Nicolella (spawning groups automagically)
*/

#ifndef _ZONES_HPP_
#define _ZONES_HPP_

#include "timesys.hpp"
#include "../global/system.hpp"
#include "scriptmgr.hpp"

class RMRoom;
class RMPlayer;
class WorldObject;
class NPC;
class CAmbushGroup;

#define _ALLOW_NO_COMBAT			0
#define _ALLOW_PLAYER_COMBAT		1
#define _ALLOW_NPC_COMBAT			2
#define _COMBAT_TOURNEY				4
#define _COMBAT_ARENA				8
#define _COMBAT_NO_ALIGNMENT		16
#define _COMBAT_NO_TREASURE		32

#define _MAX_ENTRANCE 50

#define _MAX_INSTANCE 1000

enum {
	_RESET_NORMAL,
	_RESET_OPEN_ENTRANCE,
	_RESET_CLOSE_ENTRANCE,
	_RESET_DESTROY,
	_RESET_MAX
};

class Zone;

class ResetAction : public ListObject
{
public:
	ResetAction ( LinkedList *list, int type, int duration );
	virtual ~ResetAction();

	int type, duration;
};

class MonsterType : public ListObject
{
public:
	MonsterType ( char *name, int population );
	virtual ~MonsterType();

	NPC *makeMonster ( void );

	char *name;
	int population;

	//linked list of ACTUAL living monsters
	LinkedList monsters;
};

//mike-groupspawn
class MonsterGroup : public ListObject
{
public:
	MonsterGroup();
	virtual ~MonsterGroup();

	//add a monster type by class name and population to be included when
	//this kind of group is created
	void addMonsterType( char* name, int population );

	//call to instanciate an definition list of NPC's that should be in this group
	//this returns a LinkedList* of <NPC*>'s
	LinkedList* makeMonsterGroup( void );

	//- linked list of <MonsterType*>
	//- this dictates each monster type, as well as how many of each monster
	//should be placed in this group.
	LinkedList monsterTypes;
	
	int population; //how many of these groups should exist in the zone?

	//- linked list of ACTUAL living groups
	//- each element here is a LinkedList* of RMPlayer* 's
	//- the RMPlayer's that are in a group should know where this list exists
	//so when their group is disbanded, they can remove it from here, and
	//the zone will know that a group has dissolved
	LinkedList groups;
};

//
// CZoneScript: This is a derivative of CManagedScript that represents
// a zone-specific script.
//

class CZoneScript : public CManagedScript
{
public:
	CZoneScript() {};
	virtual ~CZoneScript() {};

	// get the pointer to our zone...
	Zone *GetZone ( void ) { return (Zone *)m_pClient; };
};

//
// class Zone
//
// A zone is a list of rooms that have shared properties.  For instance,
// if a player is in a special area he may not be able to cast magic
// spells.  If the implementor wants that ability, he should make
// a zone that contains all of the rooms that he wants to affect
// and set the properties.  Also, a zone is useful in that each one
// has it's own reset time and object age allowance.  When the zone
// is reset, all of the rooms are reloaded from the world library and
// each dynamic object's age is checked against it's allowance.  If 
// it is older than allowed, it is destroyed.  One important note is
// that a zone WILL NOT reset if players are present within it.  However,
// once the last player leaves the zone, it will automatically reset.
//

class Zone : public ListObject
{
protected:
	// pointer to our active zone script...
	CZoneScript *m_pScript;

	// pointer to our active script name...
	char *m_pScriptName;

public:
	// call to set the zone script to a named zone script...
	CZoneScript *SetScript ( char *pScriptName );

public:
	int _resetInterval, _maximumObjectAge, _allowCombat, _allowEntry, didClose, worldSize;
	char _name[64], *worldData;

	short dungeonShutdownTimer;

	Zone();
	virtual ~Zone();

	// property access members
	inline int &resetInterval ( void ) { return _resetInterval; };
	inline int &maximumObjectAge ( void ) { return _maximumObjectAge; };
	inline int &allowCombat ( void ) { return _allowCombat; };

	// public properties
	char *title;
	int pkgInfo, midiFile, groupChance, numMonsters, numGroups;
	
	LinkedList worldFiles, players, rooms, npcs, objects, externalNPCs, ambushGroups;
	LinkedList monsterTypes;

	//mike-groupspawn
	//this is basically a definition list for what groups should be in the zone
	//data type of an element is <MonsterGroup*>
	LinkedList monsterGroups;

	// add an ambush group to the zone
	void addAmbushGroup ( CAmbushGroup *pGroup );

	// Dungeon entrance Q stuff
	LinkedList *dungeonEntranceArray[_MAX_ENTRANCE];

	inline int allowEntry ( void ) { 
		int retVal = _allowEntry;
		return retVal;
	};

	inline void allowEntry ( int value ) { 
		_allowEntry = value;
	};

	inline char *name ( void ) { return _name; };

	// find and return the specified room (if it is in the zone)
	RMRoom *findRoom ( int number );
	WorldObject *findObject ( char *name );

	// add a player to this zone
	void addPlayer ( RMPlayer *player );

	// delete a player from this zone
	void delPlayer ( RMPlayer *player );

	// handle adding an NPC
	void addNPC ( WorldObject *npc );
	void addExternalNPC ( WorldObject *npc );

	// handle deleting an NPC
	void delNPC ( WorldObject *npc );

	// add an object to this zone
	void addObject ( WorldObject *object );

	// delete an object from this zone
	void delObject ( WorldObject *object );

	// add a world file to this zone
	void addWorldFile ( char *name );

	// load and parse the world files
	int parseWorldFiles ( void );

	// load and parse one world file
	int parseWorldFile ( char *name );

	// load from internal buffer
	int loadFromBuffer ( void );

	// load all of the dynamic objects from this zone's object database
	void loadObjects ( void );

	// purge all of the objects that need to be purged
	void purgeObjects ( void );

	// add a new monster type
	void addMonsterType ( char *name, int population );

	//mike-groupspawn
	void addMonsterGroup( MonsterGroup* newMonsterGroup );

	// reset this zone
	int reset ( void );

	// toss the rooms in this zone
	void tossRooms ( void );

	// init the zone database
	void initDatabase ( void );

	// set the title
	void setTitle ( char *str );

	// generate new monsters...
	void generateMonsters ( void );

	// ambush players...
	void ambushPlayers ( void );

	Zone *spawn ( void );

	int instance, resetPending, homeTown, isDungeon, resetAction, id, isRandom, ambushInterval, lastAmbushTime;
	WorldObject *entrance;
	char *entranceName;

	int addPlayerToQueue ( RMPlayer *player );
	void delPlayerFromQueue ( RMPlayer *player );
	int enterFromQueue( void );
	int queueStatus ( RMPlayer *player );
};

#endif

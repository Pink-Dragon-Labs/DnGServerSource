//
// mlaction.hpp
//
// File that contains basic monster logic 'action class' code.
//
// Author: Janus Anderson
//

#ifndef _MLACTION_HPP_
#define _MLACTION_HPP_

class RMPlayer;
class NPC;
class WorldObject;
class RMRoom;

#include "../global/system.hpp"

// current logic based on 5 second updates
#define _UPDATE_SECONDS 5

#undef new 

class MonsterActions : public LinkedList
{
	public:
	MonsterActions();
	virtual ~MonsterActions();
	int chooseAction ( void );

	NPC *npc;
	RMRoom *room;

	void *operator new ( size_t size, char* file, int nLine ) { return db_malloc ( size, file, nLine ); }

	void operator delete ( void *ptr ) { free ( ptr ); }
};

#define new new( __FILE__, __LINE__ )

class MonsterAction : public ListObject
{
public:
	bool dungeonOverride;

	protected:
	NPC *npc;
	RMPlayer *actionTarget;
	WorldObject *objectTarget;
	MonsterActions *myGroup;

	public:
	int weightMod;
	int lastActionTime;
	int delayTime;

	virtual ~MonsterAction();
	MonsterAction() {}
	MonsterAction ( int mod, NPC *me, bool dungeonOverride = false );

	virtual int evaluate( void ){ return 0; }
	virtual int execute( void ){ return _UPDATE_SECONDS; }
};

class MWActionChill : public MonsterAction
{
	public:
	MWActionChill ( int mod, NPC *me, bool dungeonOverride = false );
	virtual int evaluate ( void );
	virtual int execute ( void );
};

class MWActionWander : public MonsterAction
{
	public:
	MWActionWander ( int mod, NPC *me, bool dungeonOverride = false );
	virtual int evaluate ( void );
	virtual int execute ( void );
};

class MWActionChangeRooms : public MonsterAction
{
	public:
	MWActionChangeRooms ( int mod, NPC *me, bool dungeonOverride = false );
	virtual int evaluate ( void );
	virtual int execute ( void );
};

class MWActionChangeRoomsBoss : public MonsterAction
{
	public:
	MWActionChangeRoomsBoss ( int mod, NPC *me, bool dungeonOverride = false );
	virtual int evaluate ( void );
	virtual int execute ( void );
};

class MWActionJump : public MonsterAction
{
	public:
	MWActionJump ( int mod, NPC *me, bool dungeonOverride = false );
	virtual int evaluate ( void );
	virtual int execute ( void );
};

class MWActionHeal : public MonsterAction
{
	public:
	MWActionHeal ( int mod, NPC *me, bool dungeonOverride = false );
	virtual int evaluate ( void );
	virtual int execute ( void );
};

class MWActionCure : public MonsterAction
{
	public:
	MWActionCure ( int mod, NPC *me, bool dungeonOverride = false );
	virtual int evaluate ( void );
	virtual int execute ( void );
};

class MWActionGroup : public MonsterAction
{
	public:
	MWActionGroup ( int mod, NPC *me, bool dungeonOverride = false );
	virtual int evaluate ( void );
	virtual int execute ( void );
};

class MWActionSteal : public MonsterAction
{
	public:
	MWActionSteal ( int mod, NPC *me, bool dungeonOverride = false );
	virtual int evaluate ( void );
	virtual int execute ( void );
};

class MWActionTake : public MonsterAction
{
	bool destroyObject;

	public:
	MWActionTake ( int mod, NPC *me, bool dungeonOverride = false, bool destroyObject = false );
	virtual int evaluate ( void );
	virtual int execute ( void );
};

class MWActionFlee : public MonsterAction
{
	protected:
	int fleeDir;
	int fleeCount;

	public:
	MWActionFlee ( int mod, NPC *me, bool dungeonOverride = false );
	virtual int evaluate ( void );
	virtual int execute ( void );
};

class MWActionRandomEmote : public MonsterAction
{
	protected:
	LinkedList stringList;

	public:
	MWActionRandomEmote ( int mod, NPC *me, bool dungeonOverride = false );
	void add ( char *emote );

	virtual int evaluate ( void );
	virtual int execute ( void );
};

class MWActionBless : public MonsterAction
{
	protected:
	int spellID;
	int effectID;
	int effectType;

	public:
	MWActionBless ( int mod, NPC *me, int sID, int eID, int eType, bool dungeonOverride = false );

	virtual int evaluate ( void );
	virtual int execute ( void );
};

class MWActionCurse : public MonsterAction
{
	protected:
	int spellID;
	int effectID;
	int effectType;

	public:
	MWActionCurse ( int mod, NPC *me, int sID, int eID, int eType, bool dungeonOverride = false );

	virtual int evaluate ( void );
	virtual int execute ( void );
};

#endif
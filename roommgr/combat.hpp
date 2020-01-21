/*
	COMBAT.HPP
	Combat related classes.

	Author: Stephen Nichols
*/

#ifndef _COMBAT_HPP_
#define _COMBAT_HPP_

#include "../global/system.hpp"

class MsgProcessor;

#define _COMBAT_GRID_WIDTH	24
#define _COMBAT_GRID_HEIGHT	18

#define _MAX_COMBATANT 32

// define the different sides to combat
enum {
	_COMBAT_ATTACKER,
	_COMBAT_DEFENDER
};

// define the action types
enum {
	_CA_MOVE,
	_CA_ATTACK,
	_CA_GUARD,
	_CA_CHASE,
	_CA_FLEE,
	_CA_CAST_SPELL,
	_CA_EAT,
	_CA_CHARGE,
	_CA_EQUIP,
	_CA_EXIT,
	_CA_BERSERK,
	_CA_SAY,
	_CA_MAX
};

extern char* gCombatActionText[_CA_MAX];
extern class CombatActionLogger gCombatActionLogger;

class CombatAction : public ListObject
{
public:
	CombatAction() { client = NULL; };
	CombatAction ( WorldObject *obj );
	CombatAction( const CombatAction& action );
	virtual ~CombatAction();

	// do whatever this combat action needs to do
	virtual int doit ( PackedData *movie );

	// do whatever pre-action setup that needs to be done
	virtual void setup ( void );

	// can this action do anything?
	virtual int canDoit ( void );

	// set the next action
	inline void setNext ( CombatAction *action ) { nextAction = action; };

	// object this action is acting on
	WorldObject *client;
	int type, done, allowRetaliate, initiative, roundDelay;

	// next action to perform
	CombatAction *nextAction;
};

class CombatActionLogger
{
	LinkedList actionList; //list of CombatAction*

public:
	CombatActionLogger();
	~CombatActionLogger();

	template<class _ACTIONTYPE> void logAction( _ACTIONTYPE* action )
	{
		if( !action ) return;

		//copy the combataction object and throw it in the list.
		actionList.add( new _ACTIONTYPE( *action ) );
	}

	LinkedList* getActionList() { return &actionList; }
	LinkedList* findActionTypes( int type );
	bool existsActionType( int type );
	void reset( void );
};

//
// CombatMove: handle moving client to a destination point
//

class CombatMove : public CombatAction
{
public:
	CombatMove ( WorldObject *obj, int destX, int destY, int dist );
	CombatMove( const CombatMove& action );
	virtual ~CombatMove();

	// move my client toward the destination
	virtual int doit ( PackedData *movie );

	// can my client move at all?
	virtual int canDoit ( void );

	// destination
	int x, y, distance, movePerRound;
};

// 
// CombatChase: handle chasing a player
//

class CombatChase : public CombatMove
{
public:
	CombatChase ( WorldObject *obj, WorldObject *whoToChase, int dist ); 
	CombatChase ( const CombatChase& action );
	virtual ~CombatChase();

	// chase my target
	virtual int doit ( PackedData *movie );
	virtual int canDoit ( void );

	// object this action is chasing
	WorldObject *whoToChase;
	int targetServID;
};

// 
// CombatCharge: handle charging a player
//

class CombatCharge : public CombatMove
{
public: 
	CombatCharge ( WorldObject *obj, WorldObject *whoToCharge, int dist );
	CombatCharge ( const CombatCharge& action );
	virtual ~CombatCharge();

	// charge my target
	virtual int doit ( PackedData *movie );
	virtual int canDoit ( void );
	virtual void setup ( void );

	// object to chase
	WorldObject *whoToChase;
	int targetServID;
};

//
// CombatAttack: handle attacking a particular opponent
//

class CombatAttack : public CombatAction
{
public:
	CombatAttack ( WorldObject *obj, WorldObject *target );
	CombatAttack ( const CombatAttack& action );
	virtual ~CombatAttack();

	// handle attacking the opponent
	virtual int doit ( PackedData *movie );

	// setup our stuff
	virtual void setup ( void );

	// can my client attack anything?
	virtual int canDoit ( void );

	// Calculate random chance based off value
    virtual int calcChance(int baseStat); // <-- add this

	// who to attack
	WorldObject *whoToAttack;
	int targetServID;
};

// 
// CombatEquip: handle equipping an item in combat
//

class CombatEquip : public CombatAction
{
public:
	CombatEquip ( WorldObject *obj, WorldObject *target );
	CombatEquip( const CombatEquip& action );
	virtual ~CombatEquip();

	// handle equipping the item
	virtual int doit ( PackedData *movie );

	// setup our stuff
	virtual void setup ( void );

	// can my client equip that item?
	virtual int canDoit ( void );

	// what to equip
	WorldObject *item;
	int targetServID;
};

// 
// CombatBerserk: handle berserking
//

class CombatBerserk : public CombatAttack
{
public:
	CombatBerserk ( WorldObject *obj );
	CombatBerserk ( const CombatBerserk& action );
	virtual ~CombatBerserk();

	// can my client berserk
	virtual int canDoit ( void );

	CombatAction *lastAction;
};

//
// CombatSay: handle saying something
//

class CombatSay : public CombatAction
{
public:
	CombatSay ( WorldObject *obj, const char *format, ... );
	CombatSay ( const CombatSay& action );
	virtual ~CombatSay();

	// handle saying what's on our mind
	virtual int doit ( PackedData *movie );

	// can we say anything?
	virtual int canDoit ( void );

	// what to say
	char *text;
};

//
// CombatGuard: handle guarding against a particular player or all players
//

class CombatGuard : public CombatAction
{
public:
	CombatGuard ( WorldObject *obj, WorldObject *target = NULL );
	CombatGuard ( const CombatGuard& action );
	virtual ~CombatGuard();

	// handle guarding
	virtual int doit ( PackedData *movie );

	// setup our stuff
	virtual void setup ( void );

	// can my client find anyone to hit while guarding
	virtual int canDoit ( void );

	// who to guard (NULL is all enemies)
	WorldObject *whoToGuard;
	int targetServID;
};

//
// CombatFlee: handle leaving combat! 
//

class CombatFlee : public CombatAction
{
public:
	CombatFlee ( WorldObject *obj );
	CombatFlee ( const CombatFlee& action );
	virtual ~CombatFlee();

	// handle fleeing
	virtual int doit ( PackedData *movie );

	// can I flee?
	virtual int canDoit ( void );
};

//
// CombatExit: handle exiting normally from combat
//

class CombatExit : public CombatAction
{
public:
	CombatExit ( WorldObject *obj );
	CombatExit ( const CombatExit& action );
	virtual ~CombatExit();

	// handle exiting
	virtual int doit ( PackedData *movie );

	// can I exit?
	virtual int canDoit ( void );

	// did I already exit?
	int didExit;
};

//
// CombatCastSpell: handle casting a spell
//

class CombatCastSpell : public CombatAction
{
public:
	CombatCastSpell ( WorldObject *obj, int spellID, int casterServID, int servID, int combatX, int combatY );
	CombatCastSpell ( WorldObject *obj, int spellID, int servID, int combatX, int combatY );
	CombatCastSpell ( const CombatCastSpell& action );
	virtual ~CombatCastSpell();

	// handle casting the spell
	virtual int doit ( PackedData *movie );

	// can I cast the spell
	virtual int canDoit ( void );

	// properties
	int spell;
	int targetServID, casterServID;
	int targetX, targetY;
	int didCast;
};

//
// CombatProcSpell: handle an object casting a spell
//

class CombatProcSpell : public CombatAction
{
public:
    CombatProcSpell ( WorldObject *obj, int spellID, int casterServID, int servID, int combatX, int combatY );
    CombatProcSpell ( WorldObject *obj, int spellID, int servID, int combatX, int combatY );
    CombatProcSpell ( const CombatCastSpell& action );
    virtual ~CombatProcSpell();

    // handle casting the spell
    virtual int doit ( PackedData *movie );

    // can I cast the spell
    virtual int canDoit ( void );

    // properties
    int spell;
    int targetServID, casterServID;
    int targetX, targetY;
    int didCast;
};

//
// CombatEat: handle eating an object 
//

class CombatEat : public CombatAction
{
public:
	CombatEat ( WorldObject *obj, WorldObject *target );
	CombatEat ( const CombatEat& action );
	virtual ~CombatEat();

	// handle eating the object
	virtual int doit ( PackedData *movie );

	// can I eat the object 
	virtual int canDoit ( void );

	// properties
	int targetServID;
	int didEat;
};

// this class represents a group of individuals engaged in combat
class CombatGroup : public ListObject
{
public:
	CombatGroup();
	virtual ~CombatGroup();

	// return the servID of the CombatCloud
	int servID ( void );

	// add a character to the combatants list
	void addCharacter ( WorldObject *obj, int side );

	// delete a character from the combatants list
	void deleteCharacter ( WorldObject *obj );

	// Reward the other side for you plugging
	void rewardCombatants( WorldObject* obj );

	// add an object to the objects list
	void addObject ( WorldObject *obj );

	// delete an object from the objects list
	void deleteObject ( WorldObject *obj );

	// position a character, if we can
	int positionCharacter ( WorldObject *obj, int x, int y );

	// is a square occupied? 
	int isOccupied ( int x, int y );

	// assign CombatActions to all NPCs and computer-controlled players
	void doNPCLogic ( PackedData *packet = NULL );

	// process all of the actions that are pending 
	void processActions ( void );

	// start the next round
	void nextRound ( PackedData *movie );

	// turn login
	void makeTurnReady ( WorldObject *obj );
	void makeCombatReady ( WorldObject *obj );

	// combat cloud object
	WorldObject *cloud;

	// list of attacking combatants
	LinkedList attackers;

	// list of defending combatants
	LinkedList defenders;

	// list of all combatants
	LinkedList combatants;

	// list of all player objects
	LinkedList players;

	// list of all objects
	LinkedList objects;

	// list of pending actions
	LinkedList actions;

	// list of players who are ready to process their turn
	LinkedList turnReadyList;

	// list of players who are not ready to process their turn
	LinkedList turnPendingList;

	// list of players who are in the combat but are not ready to begin yet
	LinkedList combatPendingList;

	// list of players who are in the combat and are ready to begin
	LinkedList combatReadyList;

	int pvp, attackerLevel, defenderLevel, obstacleSeed, gainedEP, round, attackerDamage, defenderDamage, attackerHealth, defenderHealth, freeFlee, ambushAttack;

	// this is the array of object servIDs in this combat group 
	int grid[_COMBAT_GRID_WIDTH][_COMBAT_GRID_HEIGHT];
	void findClosestPoint ( int x1, int y1, int x2, int y2, int *pointX, int *pointY  );
	void displayGrid();

	char *combatNames[_MAX_COMBATANT];
};

int calcStealChance ( WorldObject *attacker, WorldObject *defender = NULL );

#endif

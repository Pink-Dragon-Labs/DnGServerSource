//
// NPC.HPP
//
// Non-player character code.
//
// author: Stephen Nichols
//

#ifndef _NPC_HPP_
#define _NPC_HPP_

#include "rmplayer.hpp"
#include "mlaction.hpp"

enum {
	_COMBAT_START,
	_COMBAT_ASSESS,
	_COMBAT_APPROACH,
	_COMBAT_ATTACK,
	_COMBAT_RETREAT,
	_COMBAT_FLEE,
	_COMBAT_WAIT,
	_COMBAT_EXIT,
};

#define _ENEMY_SUMMONED_MONSTERS 0
#define _FRIENDLY_SUMMONED_MONSTERS 1
#define _MAX_SUMMONED_MONSTERS 5

class NPC : public RMPlayer
{
public:
	NPC();
	virtual ~NPC();

	virtual void handleMsg ( IPCMessage *msg );
	virtual int doit ( void );
	virtual void say ( const char *format, ... );
	virtual void tell ( RMPlayer *player, const char *format, ... );
	virtual void emote ( const char *format, ... );
	virtual void teleport ( int num, int bShowAnim = 1 );
	virtual int castOOCSpell ( int spellID, WorldObject *pWObject );
	virtual int gotoXY ( int x, int y, PackedData *packet = NULL );
	virtual int posn ( int x, int y );
	virtual int hide ( void );
	virtual int show ( void );
	virtual int changeRoom ( int dir );
	virtual int changeRoomBoss ( int dir );
	virtual int combatPosn ( int x, int y, PackedData *packet = NULL );
	virtual int partyHeal ( void );
	virtual void init ( void ) {}
	int getSummonedCount ( int type );

	// logic functions
	virtual int normalLogic ( void );
	virtual int combatLogic ( void );
	virtual void setCombatTarget( WorldObject* target );
	virtual void killedTarget ( void );
	virtual void acquiredTarget ( void );
	virtual void hitTarget ( PackedData *packet );
	virtual int spellAttack ( void );

	int activeRoom ( void );  

	void rob ( WorldObject *target );
	void take ( WorldObject *target );

	// properties
	int combatState, turnCount;
	WorldObject *combatTarget;
	int attackHigh;

	// information for flee-mode.
	int fleeMode;
	int isFleeing ( void ) { return fleeMode; }

	// special logic and info routines
	virtual int iWantToJump ( WorldObject *thisCharacter );
	virtual int isFriend ( WorldObject *thisCharacter );
	virtual int isEnemy ( WorldObject *thisCharacter );
	int canTarget ( RMPlayer *thePlayer );
	virtual int doSpecialLogic ( void );

	// actionlist information
	MonsterActions actionList;

	LinkedElement *npcElement, *npcQueueElement;
	int aiReady;
};

extern void *npcThread ( void *arg );

NPC *makeNPC ( WorldObject *obj );

#endif
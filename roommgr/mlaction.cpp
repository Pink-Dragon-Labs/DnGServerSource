//
// mlaction.cpp
//
// Control logic for basic monster logic actions
//
// Author: Janus Anderson
//

#include "roommgr.hpp"
#include "globals.hpp"

MonsterAction::MonsterAction ( int mod, NPC *me, bool dungeonOverride )
{
	weightMod = mod;
	npc = me;
	delayTime = 15;
	lastActionTime = getseconds();
	actionTarget = NULL;
	objectTarget = NULL;

	this->dungeonOverride = dungeonOverride;

	// find my parent
	myGroup = &npc->actionList;

	npc->actionList.add ( this );
}

MonsterAction::~MonsterAction ()
{
	npc->actionList.del ( this );
}


MonsterActions::MonsterActions()
{
	npc = NULL;
}

MonsterActions::~MonsterActions()
{
}

int MonsterActions::chooseAction(void)
{
	/* function will iterate through all actions in self list and */
	/* execute the action with the highest return value */

	/* first, iterate through the objects in the room and separate */
	/* into multiple lists for action execution */

	int nextUpdate=_UPDATE_SECONDS;
	LinkedElement *element=NULL;
	MonsterAction *bestAction=NULL;
	int bestWeight = 0;
	int curTime = getseconds();

	int allowCombat = npc->zone->allowCombat();
	room = npc->room;

	//if it's a dungeon, and there's no players, return 10...
	//if ( (room->picture == 3071) && !room->numPlayers )
	//	return 10;

	element = head();

	while ( element )
	{
		MonsterAction *thisAction = (MonsterAction *)element->ptr();
		int thisWeight = thisAction->evaluate() + thisAction->weightMod;

		if ( thisWeight > bestWeight )
		{
			if ( curTime > thisAction->lastActionTime + thisAction->delayTime )
			{
				//if it's an empty dungeon screen, return 10...
				//if we are overriding for dungeons, and it's a dungeon screen, don't return 10
				if ( ( room->picture == 3071 && !thisAction->dungeonOverride ) && !room->numPlayers )
					return 10;

				bestWeight = thisWeight;
				bestAction = thisAction;
			}
		}

		element = element->next();
	}

	if ( bestAction )
	{
		bestAction->lastActionTime = curTime;

		nextUpdate=bestAction->execute();
	}

	return ( nextUpdate );
}


MWActionChill::MWActionChill ( int mod, NPC *me, bool ) : MonsterAction ( mod, me )
{
}

int MWActionChill::evaluate(void)
{
	return ( rand100() );
}

int MWActionChill::execute(void)
{
	return ( _UPDATE_SECONDS );
}

MWActionWander::MWActionWander ( int mod, NPC *me, bool ) : MonsterAction ( mod, me )
{
	delayTime = 30;
}

int MWActionWander::evaluate(void)
{
	return rand100();
}

int MWActionWander::execute(void)
{
	// going to wander to a spot on the screen.

	if ( (npc->room->picture != 3071) )
	{
		npc->gotoXY ( random ( 50, 600 ), random ( 150, 300 ) );
	}

	return ( _UPDATE_SECONDS );
}

MWActionChangeRooms::MWActionChangeRooms ( int mod, NPC *me, bool ) : MonsterAction ( mod, me )
{
	delayTime = 60;
}

int MWActionChangeRooms::evaluate(void)
{
	return rand100() / 2;
}

int MWActionChangeRooms::execute(void)
{
	if ( (npc->room->picture != 3071) )
	{
		// pick a random direction
		npc->changeRoom ( random ( 0, 3 ) );
	}
	else
	{
		// i'm in a dungeon... twich about
	}

	return ( _UPDATE_SECONDS*2 );
}

MWActionChangeRoomsBoss::MWActionChangeRoomsBoss ( int mod, NPC *me, bool ) : MonsterAction ( mod, me )
{
	delayTime = 60;
}

int MWActionChangeRoomsBoss::evaluate(void)
{
	return rand100() / 2;
}

int MWActionChangeRoomsBoss::execute(void)
{
	if ( (npc->room->picture != 3071) )
	{
		// pick a random direction
		npc->changeRoomBoss ( random ( 0, 3 ) );
	}
	else
	{
		// i'm in a dungeon... twich about
	}

	return ( _UPDATE_SECONDS*2 );
}

MWActionJump::MWActionJump ( int mod, NPC *me, bool ) : MonsterAction ( mod, me )
{
}

int MWActionJump::evaluate(void)
{
	// examine all targets in this room - if there is someone to jump
	// then weight a jump.

	LinkedElement *element;
	actionTarget = NULL;
	int weight = 0;

	if ( !random ( 0, 2 ) )
		return 0;

	// build a list of peple to jump
	LinkedList jumpList;
	element = myGroup->room->head();

	while ( element )
	{
		RMPlayer *thePlayer = (RMPlayer *)element->ptr();
		WorldObject *theChar = thePlayer->character; 

		if ( npc->iWantToJump ( thePlayer->character ) )
			jumpList.add ( thePlayer );

		element = element->next();
	}

	if ( jumpList.size() )
	{
		int whoToJump = random ( 0, ( jumpList.size()-1 ) );

		element = jumpList.head();

		while ( element )
		{
			if ( whoToJump-- == 0 )
			{
				// this is the one to jump...
				actionTarget = (RMPlayer*)element->ptr();
				weight = random ( 0, 100 );

				//mike - dont jump the same player if they have been
				//		 jumped within 20 seconds of now
				if( (getseconds() - actionTarget->lastJumpTime) < 20 )
					weight = 0;

				break;
			}

			element = element->next();
		}

		jumpList.release();
	}

	return weight;
}

int MWActionJump::execute(void)
{
	if ( actionTarget )
	{
		// jump this target
		PackedMsg response;
		response.putLong ( npc->character->servID );
		response.putLong ( npc->character->room->number );
		npc->engage ( actionTarget->character, &response );
		response.putByte ( _MOVIE_END );
		roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, response.data(), response.size(), npc->character->room );
		actionTarget->lastJumpTime = getseconds(); //mike
	}

	actionTarget = NULL;
	return ( _UPDATE_SECONDS );
}

MWActionHeal::MWActionHeal ( int mod, NPC *me, bool ) : MonsterAction ( mod, me )
{
}

int MWActionHeal::evaluate(void)
{
	int weight = 0;
	actionTarget = NULL;

	// scan through friendlist and build weight
	LinkedElement *element = myGroup->room->head();

	while ( element )
	{
		RMPlayer *thePlayer = (RMPlayer *)element->ptr();
		WorldObject *theChar = thePlayer->character;

		if ( npc->isFriend ( theChar ) && npc->canTarget ( thePlayer ) && (theChar->health < theChar->healthMax) )
		{
			// consider this player...
			int ratio = (theChar->health * 100) / (theChar->healthMax);
			if ( ( 100 - ratio ) > weight )
			{
				weight = ( 100 - ratio );
				actionTarget = thePlayer;
			}
		}

		element = element->next();
	}

	return ( weight );
}

int MWActionHeal::execute(void)
{
	if ( actionTarget && npc->character->getSkill ( _SKILL_THAUMATURGY ) )
	{
		if ( npc->character->getSkill ( _SKILL_THAUMATURGY ) > 2 )
		{
			// use greater heal
			npc->castOOCSpell ( _SPELL_GREATER_HEAL, actionTarget->character );
		}
		else
		{
			// regular heal
			npc->castOOCSpell ( _SPELL_HEAL, actionTarget->character );
		}
	}
	return ( _UPDATE_SECONDS );
}


MWActionCure::MWActionCure ( int mod, NPC *me, bool ) : MonsterAction ( mod, me )
{
}

int MWActionCure::evaluate(void)
{
	int weight = 0;
	actionTarget = NULL;

	// scan through friendlist and build weight
	LinkedElement *element = myGroup->room->head();

	while ( element )
	{
		RMPlayer *thePlayer = (RMPlayer *)element->ptr();
		WorldObject *theChar = thePlayer->character;

		if ( npc->isFriend ( theChar ) && npc->canTarget ( thePlayer ) ) {
			affect_t *affect = theChar->hasAffect ( _AFF_POISONED );

			if ( affect ) {
				int value = affect->value * 2;

				// consider this player...
				if ( weight < value )
				{
					weight = value;
					actionTarget = thePlayer;
				}
			}
		}

		element = element->next();
	}

	return ( weight );
}


int MWActionCure::execute(void)
{
	if ( actionTarget && npc->character->getSkill ( _SKILL_THAUMATURGY ) )
	{
		// cure poison this character
		if ( npc->character->getSkill ( _SKILL_THAUMATURGY ) > 2 )
		{
			// use cure poison
			npc->castOOCSpell ( _SPELL_CURE_POISON, actionTarget->character );
		}
		else
		{
			// use purify
			npc->castOOCSpell ( _SPELL_PURIFY, actionTarget->character );
		}
	}

	return ( _UPDATE_SECONDS );
}

MWActionGroup::MWActionGroup ( int mod, NPC *me, bool ) : MonsterAction ( mod, me )
{
	delayTime = 300;
}

int MWActionGroup::evaluate(void)
{
	if ( npc->groupLeader == NULL )
	{
		if ( npc->room->zone->numGroups < npc->room->groupChance ) {
			return rand100();
		}
	}

	return 0;
}

int MWActionGroup::execute(void)
{
	LinkedElement *element = npc->room->head();

	while ( element )
	{
		NPC* thePlayer = (NPC *)element->ptr();
		WorldObject *theChar = thePlayer->character;

		element = element->next();

		// SNTODO: remove the ethereal hack here...

		if ( npc->groupLeader == NULL &&
			 thePlayer->isNPC &&
			 thePlayer != npc &&
			 ( npc->isFriend ( thePlayer->character ) ) &&
			 ( thePlayer->isFriend ( npc->character ) ) &&
			 !theChar->combatGroup &&
			 (abs ( npc->character->level - theChar->level ) < 4) &&
			 !theChar->hasAffect ( _AFF_DAMAGE_ETHEREAL ) )
		{
			npc->joinGroup ( thePlayer->groupLeader? thePlayer->groupLeader : thePlayer );
			return ( _UPDATE_SECONDS );
		}
	}

	return ( _UPDATE_SECONDS );
}


MWActionTake::MWActionTake ( int mod, NPC *me, bool, bool ) : MonsterAction ( mod, me )
{
	delayTime = 5;
}

int MWActionTake::evaluate( void )
{
	int weight = 0;

	// if this is a no-treasure-drop zone, don't pick stuff up...
	if ( npc->room && npc->room->zone && (npc->room->zone->allowCombat() & _COMBAT_NO_TREASURE) ) 
		return 0;

	if ( rand100() < 50 )
		return 0;

	// are there objects in this room that I could take?
	if ( npc->room->objects.size() )
	{
		// am I too encumbered to pick up anything else?
		if ( npc->character->encumberance() < 70 )
		{
			// scan through the list of objects in this room
			LinkedElement *element = npc->room->objects.head();
			while ( element )
			{
				WorldObject *target = (WorldObject *)element->ptr();

				// is this object in a combat cloud
				// can I carry this object?
				if ( ( target->getBase ( _BCARRY ) ) &&
					 ( ! target->combatGroup ) )
				{
					int thisWeight = target->value * 2;
					if ( thisWeight > weight )
					{
						weight = thisWeight;
						objectTarget = target;
					}
				}

				element = element->next();
			}
		}
	}

	return weight;
}

int MWActionTake::execute ( void )
{
	if ( objectTarget && isValidPtr ( objectTarget ) && (getMemType ( objectTarget ) == _MEM_WOBJECT) ) {
		npc->take ( objectTarget );
		objectTarget->physicalState = _STATE_MUST_DROP;

		// destroy the object that we just took if the zone is not a dungeon
//		if ( !npc->zone->isDungeon )
//			roomMgr->destroyObj ( objectTarget, 0, __FILE__, __LINE__ );

		objectTarget = NULL;
	}

	return _UPDATE_SECONDS;
}


MWActionSteal::MWActionSteal ( int mod, NPC *me, bool ) : MonsterAction ( mod, me )
{
	delayTime = 120;
}

int MWActionSteal::evaluate(void)
{
	int weight = 0;
	actionTarget = NULL;

	// scan the area and see if there's someone's pocket to pick...

	LinkedElement *element = npc->room->head();
	while ( element )
	{
		RMPlayer *thePlayer = (RMPlayer *)element->ptr();

		if ( npc->canTarget ( thePlayer ) )
		{
			// figure out how likely of a target this player is
			weight = thePlayer->character->value + ( thePlayer->character->manaValue * 5 ) ;
			weight -= thePlayer->character->level * 2;
			weight += npc->character->level;
			actionTarget = thePlayer;
			break;
		}

		element = element->next();
	}

	return weight;
}

int MWActionSteal::execute(void)
{
	// steal from actionTarget !

	if ( actionTarget )
	{
		// pick this guys pocket
		npc->gotoXY ( actionTarget->character->x, actionTarget->character->y );
		npc->rob ( actionTarget->character );

		// run away - add flee action
		new MWActionFlee ( 0, npc );
	}

	actionTarget = NULL;
	// update quickly so I can get away !!
	return ( 1 );
}


MWActionRandomEmote::MWActionRandomEmote ( int mod, NPC *me, bool ) : MonsterAction ( mod, me )
{
	delayTime = 120;
}

void MWActionRandomEmote::add ( char *emote )
{
	// copy this string and add to our list of emotes
	stringList.add ( new StringObject ( emote ) );
}

int MWActionRandomEmote::evaluate ( void )
{
	if ( !npc->room->numPlayers )
		return 0;

	return rand100();
}

int MWActionRandomEmote::execute ( void )
{
	// pick a random element from 1 to size of list and display it.
	int whichString = random ( 0, ( stringList.size() - 1 ) );

	// iterate and find the string to display
	LinkedElement *element = stringList.head();

	while ( element )
	{
		if ( whichString-- == 0 )
		{
			// display this element
			npc->emote ( (char *) ((StringObject *)element->ptr())->data );
			break;
		}

		element = element->next();
	}

	return ( _UPDATE_SECONDS );
}


MWActionFlee::MWActionFlee ( int mod, NPC *me, bool ) : MonsterAction ( mod, me )
{
	fleeDir = 0;
	fleeCount = 0;
	delayTime = 0;
}

int MWActionFlee::evaluate(void)
{
	// montors flee-mode.
	return 100;
}

int MWActionFlee::execute(void)
{
	// change rooms as fast as possible

	if ( (npc->room->picture != 3071) )
	{
		npc->changeRoom ( fleeDir );

		// switch flee direction
		if ( random ( 0, 1 ) ) fleeDir ++;
		else fleeDir --;

		if ( fleeDir < 0 ) fleeDir = 3;
		if ( fleeDir > 3 ) fleeDir = 0;

		fleeCount++;
		// stop running away after awhile
		if ( fleeCount > 3 && ( !random ( 0, 2 ) ) )
		{
			// remove myself out of the list of processed actions
			delete ( this );
		}
	}

	return (_UPDATE_SECONDS * 2);
}


MWActionBless::MWActionBless ( int mod, NPC *me, int sID, int eID, int eType, bool ) : MonsterAction ( mod, me )
{
	// this function will cast spell (spellID) on friends
	spellID = sID;
	effectID = eID;
	effectType = eType;
	delayTime = 60;
}

int MWActionBless::evaluate ( void )
{
	if ( !random ( 0, 2 ) )
		return 0;

	return rand100();
}

int MWActionBless::execute ( void )
{
	// look through our friendlist and bless one of them
	LinkedElement *element = myGroup->room->head();

	while ( element ) {
		RMPlayer *thePlayer = (RMPlayer *)element->ptr();
		WorldObject *theChar = thePlayer->character;

		if ( npc->isFriend ( theChar ) && npc->canTarget ( thePlayer ) ) {
			if ( !theChar->hasAffect ( effectID, effectType ) ) {
				npc->castOOCSpell ( spellID, thePlayer->character );
				return _UPDATE_SECONDS;
			}
		}

		element = element->next();
	}

	return _UPDATE_SECONDS;
}

MWActionCurse::MWActionCurse ( int mod, NPC *me, int sID, int eID, int eType, bool ) : MonsterAction ( mod, me )
{
	// this function will cast spell (spellID) on an enemy
	spellID = sID;
	effectID = eID;
	effectType = eType;
	delayTime = 60;
}

int MWActionCurse::evaluate ( void )
{
	if ( !random ( 0, 2 ) )
		return 0;

	return rand100();
}

int MWActionCurse::execute ( void )
{
	// look through our enemylist and curse one of them
	LinkedElement *element = myGroup->room->head();

	while ( element ) {
		RMPlayer *thePlayer = (RMPlayer *)element->ptr();
		WorldObject *theChar = thePlayer->character;

		if ( npc->isEnemy ( theChar ) && npc->canTarget ( thePlayer ) ) {
			if ( !theChar->hasAffect ( effectID, effectType ) ) {
				npc->castOOCSpell ( spellID, thePlayer->character );
				return _UPDATE_SECONDS;
			}
		}

		element = element->next();
	}

	return _UPDATE_SECONDS;
}
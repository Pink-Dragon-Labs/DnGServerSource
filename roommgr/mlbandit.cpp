//
// mlbandit.cpp
//
// Control logic for Bandits.
//
// Author: Zachary Kaiser
//
#include <algorithm>
#include "roommgr.hpp"
#include "globals.hpp"

Bandit::Bandit()
{
}

Bandit::~Bandit()
{
}

void Bandit::init ( void )
{
	// set up action list
	new MWActionChill ( 0, this );
	new MWActionJump ( 0, this );
	new MWActionWander ( 0, this );
	new MWActionGroup ( 0, this );
	new MWActionChangeRooms ( 0, this );
	new MWActionTake ( 0, this );
	new MWActionSteal ( 0, this );
	MWActionRandomEmote* emote = new MWActionRandomEmote ( 0, this );
	emote->add ( "eyes your armor." );
	emote->add ( "fiddles with his blade." );
	emote->add ( "mumbles a foreign language." );
	emote->add ( "sizes you up." );
    emote->add ( "coughs." );
    emote->add ( "spits towards you." );
    emote->add ( "swears under his breath at you." );
}

int Bandit::spellAttack ( void )
{
	int retVal = 0;
	int rate = 0;

	int	distance = getDistance ( combatTarget->combatX, combatTarget->combatY, character->combatX, character->combatY );

	if ( (distance < gTouchDistance) && isEnemy( combatTarget ) && random ( 0, 1 ) ) {

		switch ( random ( 0, 2 ) ) {

			case 0:
			return 0;

			// spell blast
			case 1: {
	   			setAction ( new CombatCastSpell ( character, _SPELL_SPELL_BLAST, combatTarget->servID, 0, 0 ) );
				retVal = 1;
			}
	   		break;
			// AMA
			case 2: {
	   			setAction ( new CombatCastSpell ( character, _SPELL_ANTI_MAGIC_AURA, combatTarget->servID, 0, 0 ) );
				retVal = 1;
			}
	   		break;
		}

	} else {

		switch ( random ( 0, 3 ) ) 
		{
			// melee attack
			case 0:
			return retVal;
	
			// cast berserk
			case 1: {
				if ( character->opposition->size() > 1 ) {
	
					LinkedElement *element = character->opposition->head();
	
					while ( element ) {
	
						WorldObject *targetObj = (WorldObject *)element->ptr();
						element = element->next();
	
						if ( !targetObj->hasAffect ( _AFF_BERSERK ) ) {
							setAction ( new CombatCastSpell ( character, _SPELL_BERSERK, targetObj->servID, 0, 0 ) );
							retVal = 1;
							break;
						}
					}
				}
			}
			break;
			// cast iron chains
			case 2: {
	
				LinkedElement *element = character->opposition->head();
	
				while ( element ) {
	
					WorldObject *targetObj = (WorldObject *)element->ptr();
					element = element->next();
	
					if ( !targetObj->hasAffect ( _AFF_STUN ) ) {
						setAction ( new CombatCastSpell ( character, _SPELL_IRON_CHAINS, targetObj->servID, 0, 0 ) );
						retVal = 1;
						break;
					}
				}
			}
			break;
			// cast killstar
			case 3: {
	
				LinkedElement *element = character->opposition->head();
	
				while ( element ) {
	
					WorldObject *targetObj = (WorldObject *)element->ptr();
					element = element->next();
	
					if ( !targetObj->hasAffect ( _AFF_CONFUSION ) ) {
						setAction ( new CombatCastSpell ( character, _SPELL_KILL_STAR, targetObj->servID, 0, 0 ) );
						retVal = 1;
						break;
					}
				}
			}
			break;
		}
	}
	// if retVal is 0, just attack normal - spell cast, return.
   	return retVal;
}

int Bandit::combatLogic ( void )
{
	if ( !room || character->health <= 0 ) 
		return -1;

	LinkedList *opposition = character->opposition;
	CombatGroup *group = character->combatGroup;

	if ( character->hasAffect ( _AFF_LOYALTY_SHIFT ) ) {
		WorldObject *obj = (WorldObject *)opposition->head()->ptr();

		if ( obj )
			opposition = obj->opposition;
	}

	if ( character->summoned ) {
		WorldObject *caster = roomMgr->findObject ( character->summoned );

		if ( !caster || !group->combatants.contains ( caster ) ) {
			setAction ( new CombatExit ( character ) );
			return 2;
		}
	}

	WorldObject *target = NULL;
	int targetCount = 0;

	// acquire a new target
	if ( opposition->size() ) {
		LinkedElement *element = character->opposition->head();
		int bestWeight = -1000000;

		while ( element ) {
			WorldObject *obj = (WorldObject *)element->ptr();
			element = element->next();

			int dist = getDistance ( obj->combatX, obj->combatY, character->combatX, character->combatY );

			// attack more threatening players first
			int weight = (50 - dist);

			// adjust based on summoned status...
			if ( obj->summoned ) {
				if ( character->summoned ) { 
					weight += 5;
				} else {
					weight -= 5;
				}
			}

#if 0
			// attack higher level people (big threat)
			// or possibly attack low level people depending on attackHigh
			// which is set on construction.
			if ( attackHigh ) weight += obj->level;
			else ( weight -= obj->level );

			// tend to stay on target with last opponent
			if ( obj == combatTarget )
				weight += 30;

			// try and spread out
			LinkedElement *elementA = obj->opposition->head();

			while ( elementA ) {
				WorldObject *object = (WorldObject *)elementA->ptr();

				if ( object->player->isNPC && ((NPC *)object->player)->combatTarget == obj ) {
					if ( groupLeader == this )
						weight += 20;
					else
						weight -= 20;
				}

				elementA = elementA->next();
			}
#endif

			// ignore invisible folks...
			//if ( !character->CanSee ( obj ) ) {
			//	weight = 0;
			//}

			// ignore dead folk
			if ( obj->health < 1 ) {
				weight -= 10000;
			} else {
				targetCount++;
			}

			if ( weight > bestWeight ) {
				bestWeight = weight;
				target = obj;
			}

		}
	}

	combatTarget = target;

	if ( combatTarget && combatTarget->health < 1 )
		combatTarget = NULL;

	// attack my target if he is close enough, or move instead
	if ( combatTarget ) {

		if ( !spellAttack() ) {

			// equip here
			if ( ( character->view == 100 || character->view == 200 ) && 
				( !character->curWeapon || !character->curWeapon->getBase ( _BWEAPON ) ) ) {

				BContainer *container = (BContainer *)character->getBase ( _BCONTAIN );

				if ( container ) {

					LinkedElement *element = container->contents.head();

					while ( element ) {
						WorldObject *obj = (WorldObject *)element->ptr();
						
						BWeapon *bweapon = (BWeapon *)obj->getBase ( _BWEAPON );

						if ( bweapon && !obj->destructing ) {
							setAction ( new CombatEquip ( character, roomMgr->findObject ( obj->servID ) ) );
							return 2;
						}
						element = element->next();
					}
				}
			}

			int dist = getDistance ( combatTarget->combatX, combatTarget->combatY, character->combatX, character->combatY );
	
			dist -= combatTarget->calcNumMoves();
	
			if ( dist < 0 )
				dist = 0;
	
			int rate = std::max(1, character->calcNumMoves());
//1 >? character->calcNumMoves();
	
			if ( character->curWeapon ) {
				BWeapon *bweapon = (BWeapon *)character->curWeapon->getBase ( _BWEAPON );
				if ( bweapon->isMissile ) 
					rate = 100000;
			}
	
			if ( dist > rate ) 
				setAction ( new CombatChase ( character, combatTarget, 1 ) );
			else 
				setAction ( new CombatAttack ( character, combatTarget ) );

			return 2;
		}

	} else {
		if ( character->health < ( character->healthMax * 0.75 ) ) {
			character->changeHealth( character->healthMax - character->health, NULL );
		}
		
		if ( character->hasAffect ( _AFF_POISONED ) ) {
			character->clearAffect ( _AFF_POISONED, _AFF_TYPE_NORMAL );
		}

		setAction ( new CombatExit ( character ) );	
	}

	return 2;
}


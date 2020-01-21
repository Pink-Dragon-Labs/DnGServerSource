//
// mlcastleboss.cpp
//
// Control logic for Castle Bosses.
//
// Author: Zachary Kaiser
//

#include <algorithm>
#include "roommgr.hpp"
#include "globals.hpp"

CastleBoss::CastleBoss()
{
}

CastleBoss::~CastleBoss()
{
}

void CastleBoss::init ( void )
{
	// set up action list
   	new MWActionTake ( 0, this );

	MWActionRandomEmote* emote = new MWActionRandomEmote ( 0, this );
	emote->add ( "sings a harmonius tune." );
    emote->add ( "examines their armor for imperfections." );
    emote->add ( "checks for your name in their tome." );
    emote->add ( "downs a chalice of wine." );
    emote->add ( "eats a forbidden fruit." );
}

int CastleBoss::normalLogic ( void )
{
	int retVal = 10;

	if ( character->summoned || !room || character->health <= 0 )
		return -1;

	if ( groupLeader && groupLeader != this )
		return 10;

	return ( actionList.chooseAction() );
}


int CastleBoss::spellAttack ( void ) {
    int retVal = 0;

	// do I wanna cast a spell or attack?
	switch ( random ( 0, 9 ) ) {
		// melee attack
		case 0:
		return retVal;
        // cast lightning bolt
		case 1:
			setAction ( new CombatCastSpell ( character, _SPELL_LIGHTNING_BOLT, combatTarget->servID, 0, 0 ) );
			retVal = 1;
        break;

        // cast gust of wind
		case 2:
			setAction ( new CombatCastSpell ( character, _SPELL_GUST_OF_WIND, combatTarget->servID, 0, 0 ) );
			retVal = 1;
        break;

        // cast earthquake
		case 3:
			setAction ( new CombatCastSpell ( character, _SPELL_EARTHQUAKE, combatTarget->servID, 0, 0 ) );
			retVal = 1;
        break;

        // summon dragon
        case 4:
			setAction ( new CombatCastSpell ( character, _SPELL_65, combatTarget->servID, combatTarget->combatX, combatTarget->combatY ) );
			retVal = 1;
        break;

        // cast fireball
		case 5:
			setAction ( new CombatCastSpell ( character, _SPELL_FIREBALL, combatTarget->servID, combatTarget->combatX, combatTarget->combatY ) );
			retVal = 1;
        break;

        // cast death wish if target is good
		case 6:
			if ( combatTarget->alignment > 171 ) {
				setAction ( new CombatCastSpell ( character, _SPELL_DEATH_WISH, combatTarget->servID, 0, 0 ) );
				retVal = 1;
			}
        break;

        // cast mass drain
		case 7:
			setAction ( new CombatCastSpell ( character, _SPELL_MASS_DRAIN, combatTarget->servID, 0, 0 ) );
			retVal = 1;
        break;

        // cast wrath of the gods if target is evil
		case 8:
			if ( combatTarget->alignment < 171 ) {
				setAction ( new CombatCastSpell ( character, _SPELL_WRATH_OF_THE_GODS, combatTarget->servID, 0, 0 ) );
				retVal = 1;
			}
        break;

        // cast light dart if target is evil
        case 9:
			if ( combatTarget->alignment < 171 ) {
				setAction ( new CombatCastSpell ( character, _SPELL_LIGHT_DART, combatTarget->servID, 0, 0 ) );
				retVal = 1;
			}
        break;
	}
	// if retVal is 0, just attack normal - spell cast, return.
   	return retVal;
}


int CastleBoss::combatLogic ( void )
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
//
// mlgonanpet.cpp
//
// Control logic for Volkor's Pets 
//
// Author: Zachary Kaiser
//

#include <algorithm>
#include "roommgr.hpp"
#include "globals.hpp"

Gonanpet::Gonanpet()
{
}

Gonanpet::~Gonanpet()
{
}

void Gonanpet::init ( void )
{
	// set up action list
	new MWActionChangeRoomsBoss ( 0, this );
	new MWActionWander ( 0, this );
	new MWActionChill ( 0, this );
	

	MWActionRandomEmote* emote = new MWActionRandomEmote ( 0, this );
	emote->add ( "nibbles her master's boot." );
    emote->add ( "sharpens her fangs." );
    emote->add ( "hisses at you." );
    emote->add ( "eats a morsel from Volkor." );
    emote->add ( "digs her claws into the ground." );
    emote->add ( "stretches her tail." );

}

int Gonanpet::normalLogic ( void )
{
	if ( character->summoned || !room || character->health <= 0 ) return -1;
	if ( groupLeader && groupLeader != this ) return 10;

	char chance = random( 0, 99 );

	//3% chance of looking for a player or NPC to leech health
	if( chance < 97 ) return ( actionList.chooseAction() );

	//find a player or NPC in the room.
	LinkedList SmiteList;
	LinkedElement* playerElement = room->head();

	while( playerElement ) {
		RMPlayer* player = static_cast<RMPlayer*>( playerElement->ptr() );
		playerElement = playerElement->next();

		if( !player || !player->character ) continue;
		
		if( player->character ) {
			SmiteList.add( player );
		}
	}

	if( SmiteList.size() ) {
		RMPlayer* player = static_cast<RMPlayer*>(SmiteList.at( random( 0, SmiteList.size() - 1 ) ));
		SmiteList.release();

        // leech player or NPC's health
		if( player && player->character ) {
			castOOCSpell( _SPELL_STEAL_LIFE, player->character );
			return 10;
		}
	}

	return ( actionList.chooseAction() );
}


int Gonanpet::spellAttack ( void ) 
{
	// Gonan's pet should cast assist spells -

	int sorcerySkill = character->getSkill (_SKILL_SORCERY );
	int elementalSkill = character->getSkill ( _SKILL_ELEMENTALISM );

	// do I wanna cast a spell?
	if ( random ( 0, 1 ) )
	{
		if ( elementalSkill )
		{
			switch ( random ( 0, 2 ) )
			{
				case 0:

				case 1:
				// cast earthquake
				setAction ( new CombatCastSpell ( character, _SPELL_EARTHQUAKE, combatTarget->servID, 0, 0 ) );
				return 1;

				case 2:
				// cast elphames justice
				setAction ( new CombatCastSpell ( character, _SPELL_ELPHAMES_JUSTICE, character->servID, 0, 0 ) );
				return 1;
			}
		}

	}

	// normal melee attack
	return 0;
}


int Gonanpet::combatLogic ( void )
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
			if ( !character->CanSee ( obj ) ) {
				weight = 0;
			}

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
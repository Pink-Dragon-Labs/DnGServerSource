//
// mlgodsewer.cpp
//
// Control logic for Sewer Gods.
//
// Author: Zachary Kaiser
//


#include <algorithm>
#include "roommgr.hpp"
#include "globals.hpp"

Godsewer::Godsewer()
{
}

Godsewer::~Godsewer()
{
}

void Godsewer::init ( void )
{
	// set up action list
	new MWActionChill ( 0, this );
	MWActionRandomEmote* emote = new MWActionRandomEmote ( 0, this );
	emote->add ( "scoffs at the believer of the new Gods." );
	emote->add ( "turns their eyes upon you." );
	emote->add ( "contemplates a sacrifice." );
	emote->add ( "snickers sardonically." );
	emote->add ( "stares at you with disdain." );
	emote->add ( "fears no mortal." );
}

int Godsewer::spellAttack ( void ) 
{
	// Gods should cast elemental + necromancer spells -

	int thaumaturgySkill = character->getSkill ( _SKILL_THAUMATURGY );
	int sorcerySkill = character->getSkill (_SKILL_SORCERY );
	int elementalSkill = character->getSkill ( _SKILL_ELEMENTALISM );
	int necroSkill = character->getSkill ( _SKILL_NECROMANCY );

	// do I wanna cast a spell?
	if ( random ( 0, 1 ) )
	{
		if ( elementalSkill )
		{
			switch ( random ( 0, 4 ) )
			{
				case 0:
				// cast electric fury
				setAction ( new CombatCastSpell ( character, _SPELL_ELECTRIC_FURY, combatTarget->servID, 0, 0 ) );
				return 1;

				case 1:
				// cast gust of wind
				setAction ( new CombatCastSpell ( character, _SPELL_GUST_OF_WIND, combatTarget->servID, 0, 0 ) );
				return 1;

				case 2:
				// cast earthquake
				setAction ( new CombatCastSpell ( character, _SPELL_EARTHQUAKE, combatTarget->servID, 0, 0 ) );
				return 1;

				case 3:
				// crawling charge
				setAction ( new CombatCastSpell ( character, _SPELL_118, combatTarget->servID, 0, 0 ) );
				return 1;
			}
		}

		if ( necroSkill )
		{
			switch ( random ( 0, 7 ) )
			{
				case 0:
				// summon dragon
				setAction ( new CombatCastSpell ( character, _SPELL_ICE_ORB, combatTarget->servID, 0, 0 ) );
				return 1;

				case 1:
				// cast fireball
				setAction ( new CombatCastSpell ( character, _SPELL_FIREBALL, combatTarget->servID, 0, 0 ) );
				return 1;

				case 2:
				// cast death wish
				setAction ( new CombatCastSpell ( character, _SPELL_DEATH_WISH, combatTarget->servID, 0, 0 ) );
				return 1;

				case 3:
				// cast fear
				setAction ( new CombatCastSpell ( character, _SPELL_FEAR, combatTarget->servID, 0, 0 ) );
				return 1;

				case 4:
				// cast DV
				setAction ( new CombatCastSpell ( character, _SPELL_DUACHS_VENGEANCE, combatTarget->servID, 0, 0 ) );
				return 1;

				case 5:
				// cast DV
				setAction ( new CombatCastSpell ( character, _SPELL_FLAME_ORB, combatTarget->servID, 0, 0 ) );
				return 1;
			}
		}

		if ( thaumaturgySkill )
		{
			// cast a light dart
			setAction ( new CombatCastSpell ( character, _SPELL_LIGHT_DART, combatTarget->servID, 0, 0 ) );
			return 1;
		}
	}

	// if I'm a sorcerer combat teleport if out of range
	if ( sorcerySkill )
	{
		int dist = getDistance ( combatTarget->combatX, combatTarget->combatY, character->combatX, character->combatY );

		int rate = std::max(1, character->calcNumMoves());
//1 >? character->calcNumMoves();

		if ( dist > rate ) {
			/* combat teleport */
			setAction ( new CombatCastSpell ( character, _SPELL_COMBAT_TELEPORT, character->servID, combatTarget->combatX, combatTarget->combatY ) );
			return 1;
		}
	}

	// normal melee attack
	return 0;
}


int Godsewer::combatLogic ( void )
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
//
// mlgonan.cpp
//
// Control logic for Volkor the Destroyer. (Raid Boss)
//
// Author: Zachary Kaiser
//

#include <algorithm>
#include "roommgr.hpp"
#include "globals.hpp"

Gonan::Gonan()
{
}

Gonan::~Gonan()
{
}

void Gonan::init ( void )
{
	// set up action list
   	new MWActionGroup ( 0, this );
	new MWActionTake ( 0, this );
	new MWActionChangeRoomsBoss ( 0, this );
	new MWActionWander ( 0, this );
	new MWActionChill ( 0, this );
	

	MWActionRandomEmote* emote = new MWActionRandomEmote ( 0, this );
	emote->add ( "radiates with power." );
    emote->add ( "flexes." );
    emote->add ( "laughs, and the room shakes." );
    emote->add ( "sharpens his claw weapon." );
    emote->add ( "mumbles a demonic incantation." );
    emote->add ( "laughs at your stature." );
	emote->add ( "throws his pet a morsel." );
	emote->add ( "thumps you on the head." );
	emote->add ( "cracks his neck." );
	emote->add ( "ponders who to smite next." );
	emote->add ( "mumbles 'Tohra ahmi ahmwa surak' under his breath threateningly." );
}

int Gonan::normalLogic ( void )
{
	if ( character->summoned || !room || character->health <= 0 ) return -1;
	if ( groupLeader && groupLeader != this ) return 10;

	char chance = random( 0, 99 );

	//3% chance of looking for a player or NPC to zap
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

		// zap the player or NPC
		if( player && player->character ) {
			castOOCSpell( _SPELL_LIGHTNING_BOLT, player->character );
			return 10;
		}
	}

	return ( actionList.chooseAction() );
}


int Gonan::spellAttack ( void ) 
{
	// Gonan should cast unforgiving spells -

	int thaumaturgySkill = character->getSkill ( _SKILL_THAUMATURGY );
	int sorcerySkill = character->getSkill (_SKILL_SORCERY );
	int elementalSkill = character->getSkill ( _SKILL_ELEMENTALISM );
	int necroSkill = character->getSkill ( _SKILL_NECROMANCY );
    int mystSkill = character->getSkill ( _SKILL_MYSTICISM );

	// do I wanna cast a spell?
	if ( random ( 0, 1 ) )
	{
		if ( elementalSkill )
		{
			switch ( random ( 0, 13 ) )
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
				// summon myst dragon
				setAction ( new CombatCastSpell ( character, _SPELL_SUMMON_DRAGON, combatTarget->servID, combatTarget->combatX, combatTarget->combatY ) );
				return 1;

                case 4:
				// cast light dart
				setAction ( new CombatCastSpell ( character, _SPELL_LIGHT_DART, combatTarget->servID, 0, 0 ) );
				return 1;

				case 5:
				// cast wrath of the gods
				setAction ( new CombatCastSpell ( character, _SPELL_WRATH_OF_THE_GODS, combatTarget->servID, 0, 0 ) );
				return 1;

                case 6:
				// cast mass zerk
				setAction ( new CombatCastSpell ( character, _SPELL_MASS_BERSERK, combatTarget->servID, 0, 0 ) );
				return 1;

				case 7:
				// cast mass fumble
				setAction ( new CombatCastSpell ( character, _SPELL_MASS_FUMBLE, combatTarget->servID, 0, 0 ) );
				return 1;

				case 8:
				// cast fear
				setAction ( new CombatCastSpell ( character, _SPELL_FEAR, combatTarget->servID, 0, 0 ) );
				return 1;

                case 9:
				// summon random god
				setAction ( new CombatCastSpell ( character, _SPELL_107, combatTarget->servID, combatTarget->combatX, combatTarget->combatY ) );
				return 1;

				case 10:
				// cast mass drain
				setAction ( new CombatCastSpell ( character, _SPELL_MASS_DRAIN, combatTarget->servID, 0, 0 ) );
				return 1;

				case 11:
				// cast death wish
				setAction ( new CombatCastSpell ( character, _SPELL_DEATH_WISH, combatTarget->servID, 0, 0 ) );
				return 1;

				case 12 :
				// cast DV
				setAction ( new CombatCastSpell ( character, _SPELL_DUACHS_VENGEANCE, combatTarget->servID, 0, 0 ) );
				return 1;

				case 13:
				// cast elphames justice
				setAction ( new CombatCastSpell ( character, _SPELL_ELPHAMES_JUSTICE, character->servID, 0, 0 ) );
				return 1;
			}
		}

	}

	// normal melee attack
	return 0;
}


int Gonan::combatLogic ( void )
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
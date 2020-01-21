//
// Control logic for minotaur guards.
//
// Author: Michael Nicolella
//

#include "roommgr.hpp"

MinotaurGuard::MinotaurGuard()
{
}

MinotaurGuard::~MinotaurGuard()
{
}

void MinotaurGuard::init( void )
{
	new MWActionChill ( 0, this );
	new MWActionWander ( 0, this );
	new MWActionChangeRooms ( 0, this );

	MWActionRandomEmote* emote = new MWActionRandomEmote ( 0, this );
	emote->add ( "patrols the area." );
	emote->add ( "looks over at you and raises an eyebrow." );
	emote->add ( "straightens his cap." );
	emote->add ( "mumbles a spell under his breath." );
	emote->add ( "mumbles a prayer under his breath." );
	emote->add ( "searches over the ground." );
}

int MinotaurGuard::normalLogic ( void )
{
	if ( character->summoned || !room || character->health <= 0 ) return -1;
	if ( groupLeader && groupLeader != this ) return 10;

	char chance = random( 0, 99 );

	//3% chance of looking for a minotaur to zap
	if( chance < 97 ) return ( actionList.chooseAction() );

	//find a minotaur in the room.
	LinkedList MinotaurList;
	LinkedElement* playerElement = room->head();

	while( playerElement ) {
		RMPlayer* player = static_cast<RMPlayer*>( playerElement->ptr() );
		playerElement = playerElement->next();

		if( !player || !player->character ) continue;
		
		if( !strcmp( player->character->basicName, "minotaur" ) ) {
			MinotaurList.add( player );
		}
	}

	if( MinotaurList.size() ) {
		RMPlayer* minotaur = static_cast<RMPlayer*>(MinotaurList.at( random( 0, MinotaurList.size() - 1 ) ));
		MinotaurList.release();

		//see if this room borders another zone. if so, blast the minotaur
		bool isZoneBorder = false;

		RMRoom* tRoom = roomMgr->findRoom( room->north );
		if( tRoom && tRoom->zone != room->zone ) isZoneBorder = true;
		tRoom = roomMgr->findRoom( room->south );
		if( !isZoneBorder && tRoom && tRoom->zone != room->zone ) isZoneBorder = true;	
		tRoom = roomMgr->findRoom( room->east );
		if( !isZoneBorder && tRoom && tRoom->zone != room->zone ) isZoneBorder = true;
		tRoom = roomMgr->findRoom( room->west );
		if( !isZoneBorder && tRoom && tRoom->zone != room->zone ) isZoneBorder = true;

		if( minotaur && minotaur->character && isZoneBorder ) {
			castOOCSpell( _SPELL_LIGHTNING_BOLT, minotaur->character );
			return 10;
		}
	}

	return ( actionList.chooseAction() );
}

int MinotaurGuard::spellAttack( void )
{
	if( !character ) return 0;

	if( character->health < (character->healthMax * .75 ) ) {
		switch( random(0,3) ) {
			case 1:
				setAction ( new CombatCastSpell ( character, _SPELL_HEAL, character->servID, 0, 0 ) );
				return 1;
				break;
			case 2:
				setAction ( new CombatCastSpell ( character, _SPELL_GREATER_HEAL, character->servID, 0, 0 ) );
				return 1;
				break;
		}
	}

	if ( random ( 0, 3 ) == 0 ) {
		switch ( random ( 0, 3 ) ) {
			case 0:
				setAction ( new CombatCastSpell ( character, _SPELL_FEAR, character->servID, 0, 0 ) );
				break;
			case 1:
				setAction ( new CombatCastSpell ( character, _SPELL_CONFUSION, combatTarget->servID, 0, 0 ) );
				break;
			case 2:
			case 3:
				setAction ( new CombatCastSpell ( character, _SPELL_MASS_BERSERK, character->servID, 0, 0 ) );
				break;
		}
	} else {
		switch( random(0, 3) ) {
			case 0:
			case 1:
				setAction ( new CombatCastSpell ( character, random(0,1)?_SPELL_ELECTRIC_FURY:_SPELL_EARTHQUAKE, character->servID, 0, 0 ) );
				break;
			case 2:
				setAction ( new CombatCastSpell ( character, _SPELL_REPEL, character->servID, character->combatX, character->combatY ) );
				break;
			case 3:
				setAction ( new CombatCastSpell ( character, _SPELL_SUMMON_ELEMENTAL, character->servID, character->combatX, character->combatY ) );
				break;
		}
	}

	return 1;
}

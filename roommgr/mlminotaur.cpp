//
// mlminotaur.cpp
//
// Control logic for Minotaurs.
//
// Author: Michael Nicolella
//

#include "roommgr.hpp"

//
// base Minotaur
//

Minotaur::Minotaur()
{
}

Minotaur::~Minotaur()
{
}

int Minotaur::isFriend ( WorldObject *object )
{	//make minotaurs only friendly with other minotaurs.
	return (!strcmp( object->basicName, "minotaur" )) ? TRUE : FALSE;
}

void Minotaur::init ( void )
{
	// set up action list
	new MWActionChill ( 0, this );
	new MWActionWander ( 0, this );
	new MWActionChangeRooms ( 0, this );
	new MWActionTake ( 0, this );

	MWActionRandomEmote* emote = new MWActionRandomEmote ( 0, this );
	emote->add ( "growls, urgently looking around." );
	emote->add ( "sniffs at the air." );
	emote->add ( "huffs, pawing the ground." );
	emote->add ( "stomps his hoof." );
	emote->add ( "snarls in your direction." );
	emote->add ( "fixes his eyes on you and snorts." );

	emote->add ( "mumbles a prayer." );
	emote->add ( "grips a strange looking object and mumbles." );
}

void Minotaur::killedTarget ( void )
{
	//some creative emotes when we kill someone
	unsigned char chance = random(0,100);
	
	if( combatTarget->character ) {
		if( chance < 25 )
			emote("%s in victory over %s!", random(0,1)? "growls" : "roars", &combatTarget->character->properName);
		else if( chance < 50 )
			emote("stands over %s and %s!", &combatTarget->character->properName, random(0,1)? "bellows" : "roars");
	}
}

void Minotaur::acquiredTarget ( void )
{
	//some creative emotes when we target someone
	unsigned char chance = random(0,100);

	if( combatTarget && combatTarget->character ) {
		if( chance < 15 ) //10% chance to let our target know we're onto them!
			emote("glistens at the mouth, locking his eyes %son %s", random(0,1) ? "intently " : "",  &combatTarget->character->properName );
		else if( chance < 25 )
			emote("fixes his eyes on %s", &combatTarget->character->properName );
	}
}


// ---------------------------------------------------
// minotaur warrior
// ---------------------------------------------------

MinotaurWarrior::MinotaurWarrior()
{
}

MinotaurWarrior::~MinotaurWarrior()
{
}

void MinotaurWarrior::init ( void )
{
	Minotaur::init();
	MWActionRandomEmote* emote = new MWActionRandomEmote ( 0, this );
	emote->add ( "brandishes his axe." );
}

int MinotaurWarrior::combatLogic( void )
{
	if( !character || !character->opposition ) {
		setAction( new CombatGuard( character ) );
		return 2;
	}

	if( combatTarget && random(0, 9) < 6 ) {
		//if we are already targetting someone, there is a 60% chance to stay on target
		//else pick a random opponent to target (which may be the same target)
		setAction( new CombatAttack( character, combatTarget ) );
	} else {
		//attack random opponent
		if( character->opposition ) {
			unsigned short oppositionSize = character->opposition->size();
			
			int randIndex = random(0, oppositionSize - 1);
			
			if( randIndex >= 0 && randIndex < oppositionSize ) {
				WorldObject* opponent = (WorldObject*) character->opposition->at( randIndex );
				if( opponent ) {
					setCombatTarget( opponent );
					setAction( new CombatAttack( character, combatTarget ) );
				} else setAction( new CombatGuard( character ) );
			} else setAction( new CombatGuard( character ) );
		} else setAction( new CombatGuard( character ) );
	}

	return 2;
}

// ---------------------------------------------------
// minotaur mage
// ---------------------------------------------------

MinotaurMage::MinotaurMage()
{
}

MinotaurMage::~MinotaurMage()
{
}

void MinotaurMage::init ( void )
{
	Minotaur::init();

	MWActionRandomEmote* emote = new MWActionRandomEmote ( 0, this );
	emote->add ( "gazes at you." );
	emote->add ( "grips his staff." );
}

int MinotaurMage::combatLogic( void )
{
	if( !character || !character->opposition ) {
		setAction( new CombatGuard( character ) );
		return 2;
	}

	//if i was melee'ed the round before, melee back!
	LinkedList* attackList = gCombatActionLogger.findActionTypes( _CA_ATTACK );

	LinkedElement* attackActionElement = attackList->head();

	while ( attackActionElement ) {
		CombatAttack* attackAction = (CombatAttack*) attackActionElement->ptr();
		attackActionElement = attackActionElement->next();

		if( character->opposition->contains( attackAction->client ) )
			if( attackAction->targetServID == character->servID ) {
				setCombatTarget( attackAction->client );
				setAction( new CombatAttack( character, attackAction->client ) );
				return 2;
				break;
			}
	}

	//find the square that would affect the most players with an area spell.
	int bestX = character->combatX;
	int bestY = character->combatY;
	int bestCount = 0;

	for( char combatX = character->combatX - 4; combatX <= character->combatX + 4; ++combatX ) {
		if( combatX < 0 ) continue;
		else if( combatX >= _COMBAT_GRID_WIDTH ) continue;

		for( char combatY = character->combatY - 4; combatY <= character->combatY + 4; ++combatY ) {
			if (combatY < 0) continue;
			else if( combatY >= _COMBAT_GRID_HEIGHT ) continue;

			int numOpponents = 0;

			//count the number of players in this area...
			//get the servID of the object in this square
			for( char testCombatX = character->combatX - 4; testCombatX <= character->combatX + 4; ++testCombatX ) {
				if( testCombatX < 0 ) continue;
				else if( testCombatX >= _COMBAT_GRID_WIDTH ) continue;

				for( char testCombatY = character->combatY - 4; testCombatY <= character->combatY + 4; ++testCombatY ) {
					if (testCombatY < 0) continue;
					else if( testCombatY >= _COMBAT_GRID_HEIGHT ) continue;

					int servID = character->combatGroup->grid[testCombatX][testCombatY];

					if( servID <= 1 ) continue;

					//if theres an opposition, increment
					if( character->opposition->contains( roomMgr->findObject( servID ) ) )
						numOpponents++;
				}
			}

			if( numOpponents > bestCount ) {
				//got a better square
				bestCount = numOpponents;
				bestX = combatX;
				bestY = combatY;
			}
		}
	}

	attackList->release();
	delete attackList;

	//we have the best square to target to damage the most players.
	//blast them if there are any
	if( bestCount ) {
		unsigned char chance = random(0, 99);

		if( chance < 10 )	//10% chance of repelling
			setAction(new CombatCastSpell( character, _SPELL_REPEL, servID, character->combatX, character->combatY ) );
		else if ( chance < 60 ) //50% chance of burning mist
			setAction(new CombatCastSpell( character, _SPELL_BURNING_MISTCLOUD, servID, bestX, bestY ) );
		else if ( chance < 85 ) //25% chance of nauseating mist
			setAction(new CombatCastSpell( character, _SPELL_NAUSEATING_MISTCLOUD, servID, bestX, bestY ) );
		else				//15% chance of deadly mist
			setAction(new CombatCastSpell( character, _SPELL_DEADLY_MISTCLOUD, servID, bestX, bestY ) );

	} else {

		//cast a targetted spell on a random opponent
		unsigned short spellPool[5] = { _SPELL_FLAME_ORB,
										_SPELL_ICE_ORB,
										_SPELL_POISON_BOLT,
										_SPELL_LIGHTNING_BOLT,
										_SPELL_FREEZE };
		char spellToCast = random(0,4);

		//target a random opponent with it
		if( character->opposition ) {
			unsigned short oppositionSize = character->opposition->size();
			
			int randIndex = random(0, oppositionSize - 1);
			
			if( randIndex >= 0 && randIndex < oppositionSize ) {
				WorldObject* opponent = (WorldObject*) character->opposition->at( randIndex );
				if( opponent ) {
					
					setCombatTarget( opponent );
					
					//10% chance to melee instead
					if( random( 0, 9 ) == 5 )
						setAction( new CombatAttack( character, opponent ) );
					else 
						setAction( new CombatCastSpell( character, spellPool[spellToCast], opponent->servID, 0, 0 ) );

				} else setAction( new CombatGuard( character ) );
			} else setAction( new CombatGuard( character ) );
		} else setAction( new CombatGuard( character ) );
	}
	// END MAGE STRATEGY
	// ---------------------------------------------

	return 2;
}

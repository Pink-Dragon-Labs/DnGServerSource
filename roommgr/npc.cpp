//
// NPC.CPP
//
// Non-player character code.
//
// author: Stephen Nichols
//
#include <algorithm>
#include "roommgr.hpp"
#include "npc.hpp"
#include "globals.hpp"

//MIKE-todo, make this a nice templated factory...
NPC *makeNPC ( WorldObject *obj )
{
	BNPC *npc = (BNPC *)obj->getBase ( _BNPC );

	if ( npc ) {
		NPC *player = NULL;
		char *npcCode = npc->code;

		if ( npcCode ) {
			if ( !strcmp ( npcCode, "tulor" ) )
				player = new Tulor;

			else if ( !strcmp ( npcCode, "evilFaery" ) )
				player = new EvilFaery;

			else if ( !strcmp ( npcCode, "goodFaery" ) )
				player = new GoodFaery;

			else if ( !strcmp ( npcCode, "shopkeeper" ) )
				player = new NPC;

			else if ( !strcmp ( npcCode, "magistrate" ) )
				player = new Magistrate;

			else if ( !strcmp ( npcCode, "necromancer" ) )
				player = new Necro;

			else if ( !strcmp ( npcCode, "marvin" ) )
				player = new Marvin;

			else if ( !strcmp ( npcCode, "ogreMage" ) )
				player = new OgreMage;

			else if ( !strcmp ( npcCode, "trollSpellcaster" ) )
				player = new TrollSpellcaster;

			else if ( !strcmp ( npcCode, "daemon" ) )
				player = new Daemon;

			else if ( !strcmp ( npcCode, "earthElemental" ) )
				player = new EarthElemental;

			else if ( !strcmp ( npcCode, "waterElemental" ) )
				player = new WaterElemental;

			else if ( !strcmp ( npcCode, "airElemental" ) )
				player = new AirElemental;

			else if ( !strcmp ( npcCode, "fireElemental" ) )
				player = new FireElemental;

			else if ( !strcmp ( npcCode, "lich" ) )
				player = new Lich;

			else if ( !strcmp ( npcCode, "seraph" ) )
				player = new Seraph;

			else if ( !strcmp ( npcCode, "imp" ) )
				player = new Imp;

			else if ( !strcmp ( npcCode, "guardian" ) )
				player = new Guardian;

			else if ( !strcmp ( npcCode, "thief" ) )
				player = new Thief;

			else if ( !strcmp ( npcCode, "hunter" ) )
				player = new Hunter;

			else if ( !strcmp ( npcCode, "mouse" ) )
				player = new Mouse;

			else if ( !strcmp ( npcCode, "stormBat" ) )
				player = new StormBat;

			else if ( !strcmp ( npcCode, "fury" ) )
				player = new Fury;

			else if ( !strcmp ( npcCode, "paladin" ) )
				player = new Paladin;

			else if ( !strcmp ( npcCode, "cleric" ) )
				player = new Cleric;

			else if ( !strcmp ( npcCode, "wizLight" ) )
            	player = new LightWiz;

			else if ( !strcmp ( npcCode, "powerWiz" ) )
            	player = new PowerWiz;

			else if ( !strcmp ( npcCode, "watcher" ) )
				player = new Watcher;

			else if ( !strcmp ( npcCode, "hellsoul" ) )
				player = new HellSoul;

//	---------------------------------------------------------------------
//	Zach's monster logic

			else if ( !strcmp ( npcCode, "bandit" ) )
				player = new Bandit;

			else if ( !strcmp ( npcCode, "bmystic" ) )
				player = new Bmystic;

			else if ( !strcmp ( npcCode, "swashbuckler" ) )
				player = new Swashbuckler;

//	---------------------------------------------------------------------
//	Sewer Logic

			else if ( !strcmp ( npcCode, "godprot" ) )
				player = new Godprot;

			else if ( !strcmp ( npcCode, "godsewer" ) )
				player = new Godsewer;

//	---------------------------------------------------------------------
//	Xerxes Logic

			else if ( !strcmp ( npcCode, "xerxes" ) )
				player = new Xerxes;

//	---------------------------------------------------------------------
//	House Pet Logic

			else if ( !strcmp ( npcCode, "housepet" ) )
				player = new HousePet;

//	---------------------------------------------------------------------
//	Dragon Logic

			else if ( !strcmp ( npcCode, "Dragon" ) )
				player = new Dragon;

//	---------------------------------------------------------------------
//	Traveller Logic

			else if ( !strcmp ( npcCode, "traveller" ) )
				player = new Traveller;

//	---------------------------------------------------------------------
//	Gonan Logic

			else if ( !strcmp ( npcCode, "gonan" ) )
				player = new Gonan;

//	---------------------------------------------------------------------
//	Gonan's pet Logic

			else if ( !strcmp ( npcCode, "gonanpet" ) )
				player = new Gonanpet;

//	---------------------------------------------------------------------
//	Castle Boss Logic

			else if ( !strcmp ( npcCode, "castleboss" ) )
				player = new CastleBoss;

//	---------------------------------------------------------------------
//	Castle Thief Logic

			else if ( !strcmp ( npcCode, "castlethief" ) )
				player = new RogueThief;

//	---------------------------------------------------------------------
//	Castle Wizard Logic

			else if ( !strcmp ( npcCode, "goodCastleWizard" ) )
				player = new GoodCastleWizard;

			else if ( !strcmp ( npcCode, "neutralCastleWizard" ) )
				player = new NeutralCastleWizard;

			else if ( !strcmp ( npcCode, "evilCastleWizard" ) )
				player = new EvilCastleWizard;

//	---------------------------------------------------------------------
//	Castle Guard Logic

			else if ( !strcmp ( npcCode, "castleguard" ) )
				player = new CastleGuard;

//	---------------------------------------------------------------------
//	Hyjinks monster logic
			else if ( !strcmp ( npcCode, "berserker" ) )
				player = new Berserker;

			else if ( !strcmp ( npcCode, "goodWizard" ) )
				player = new GoodWizard;

			else if ( !strcmp ( npcCode, "neutralWizard" ) )
				player = new NeutralWizard;

			else if ( !strcmp ( npcCode, "evilWizard" ) )
				player = new EvilWizard;

//	---------------------------------------------------------------------

//	---------------------------------------------------------------------
//	- Minotaurs
			else if ( !strcmp ( npcCode, "minotaur" ) )
				player = new Minotaur;
			else if ( !strcmp ( npcCode, "minotaurWarrior" ) )
				player = new MinotaurWarrior;
			else if ( !strcmp ( npcCode, "minotaurMage" ) )
				player = new MinotaurMage;
//
//	---------------------------------------------------------------------
//	- Drear Valley Aegiscians
			else if ( !strcmp ( npcCode, "minotaurGuard" ) )
				player = new MinotaurGuard;
//	---------------------------------------------------------------------
//	- Pumpkin Patch NPC's - placeholders that spawn pumpkins
			else if ( !strcmp ( npcCode, "pumpkin-dropper" ) )
				player = new PumpkinDropper;
//	---------------------------------------------------------------------
//	- The Mines NPC's - placeholders that spawn bars
//			else if ( !strcmp ( npcCode, "mines-dropper0" ) )
//				player = new MinesDropper(0);
//			else if ( !strcmp ( npcCode, "mines-dropper1" ) )
//				player = new MinesDropper(1);
//			else if ( !strcmp ( npcCode, "mines-dropper2" ) )
//				player = new MinesDropper(2);
//			else if ( !strcmp ( npcCode, "mines-dropper3" ) )
//				player = new MinesDropper(3);
//			else if ( !strcmp ( npcCode, "miner" ) )
//				player = new Miner();
//	---------------------------------------------------------------------
			else
				player = new SmartMonster;

		} else {
			player = new NPC;
		}

		player->servID = obj->servID;
		player->owner = NULL;
		player->character = obj;
		player->handlesMsgs = 1;
		player->isNPC = 1;

		obj->isVisible = 1;
		obj->player = player;
		obj->isZoneObject = 1;
		obj->value = 0;
		obj->manaValue = 0;

		if ( obj->minLevel )
			obj->level = random ( obj->minLevel, obj->maxLevel );

		obj->calcHealth();
		obj->health = obj->healthMax;

		obj->generateTreasure();

		// let the wobject be born...
		obj->processActions ( vBeBorn, NULL );

		player->init();

		return player;
	}

	return NULL;
}

NPC::NPC()
{
	aiReady = 0;

	combatState = _COMBAT_START;
	turnCount = 0;
	turnCount = 0;
	combatTarget = NULL;
	delay = 1;

	npcElement = gNpcList.add ( this );

	attackHigh = random ( 0, 1 );
	actionList.npc = this;
}

NPC::~NPC()
{
	if ( npcElement ) {
		gNpcList.delElement ( npcElement );
		npcElement = NULL;
	}
}

void NPC::handleMsg ( IPCMessage *msg )
{
	delete msg;
}

void NPC::say ( const char *format, ... )
{
	if ( character ) {
		char output[1024];
		va_list args;

		va_start ( args, format );
		vsprintf ( sizeof ( output ), output, format, args );
		va_end ( args );

		sendRoomText ( "%s: %s\n", character->getName(), output );
	}
}

void NPC::tell ( RMPlayer *player, const char *format, ... )
{
	if ( character ) {
		char output[1024];
		va_list args;

		va_start ( args, format );
		vsprintf ( sizeof ( output ), output, format, args );
		va_end ( args );

		roomMgr->sendPlayerText ( player, "|c67|%s tells you '%s'\n", character->getName(), output );
	}
}

void NPC::emote ( const char *format, ... )
{
	if ( character ) {
		char output[1024];
		va_list args;

		va_start ( args, format );
		vsprintf ( sizeof ( output ), output, format, args );
		va_end ( args );

		if ( character->combatGroup )
			roomMgr->sendListText ( &character->combatGroup->players, "%s %s\n", character->getName(), output );
		else
			sendRoomText ( "%s %s\n", character->getName(), output );
	}
}

int NPC::castOOCSpell ( int spellID, WorldObject *pWObject )
{
	PackedMsg *packet = NULL;

	if ( activeRoom() )
		packet = new PackedMsg;

	spell_info *spell = &gSpellTable[spellID];

	if ( packet ) {
		/* set up packet */
		packet->putLong ( character->servID );
		packet->putLong ( character->room->number );
		packet->putByte ( _MOVIE_CAST_BEGIN );
		packet->putLong ( character->servID );
	}

	castSpell ( spell, character, pWObject->servID, -1, -1, packet );

	if ( packet ) {
		packet->putByte ( _MOVIE_CAST_END );
		packet->putLong ( character->servID );

		packet->putByte ( _MOVIE_END );

		roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, packet->data(), packet->size(), character->room );
	}

	if ( packet )
		delete packet;

	return TRUE;
}

void NPC::rob ( WorldObject *target )
{
	if ( character ) {
		PackedMsg *response = NULL;

		if ( activeRoom() ) {
			response = new PackedMsg;

			response->putLong ( character->servID );
			response->putLong ( character->room->number );
		}

		RMPlayer::rob ( target, response );

		if ( response ) {
			response->putByte ( _MOVIE_END );

			roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, response->data(), response->size(), character->room );

			delete response;
		}
	}
}

void NPC::take ( WorldObject *target )
{
	if ( character )
	{
		gotoXY ( target->x, target->y );

		// build a packet
		PackedMsg *response = NULL;

		if ( activeRoom() ) {
			response = new PackedMsg;

			response->putLong ( character->servID );
			response->putLong ( character->room->number );
		}

		logDisplay( "NPC of %s takes object of %s", character->classID, target->classID );
		character->take ( target, response );

		// set the treasure bit if an NPC takes an item...
		target->physicalState |= _STATE_TREASURE;

		if ( response ) {
			response->putByte ( _MOVIE_END );

			roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, response->data(), response->size(), character->room );

			emote ("picks up the %s.", target->getName() );

			delete response;
		}
	}
}

int NPC::iWantToJump ( WorldObject *thisCharacter )
{
	// function will return TRUE if this NPC wants to jump thisCharacter.

	if ( thisCharacter->player->checkAccess ( _ACCESS_AMBUSH ) )
		return FALSE;

	// make sure all group members are not immortal
	if ( thisCharacter->player->groupLeader ) {
		LinkedElement *pElement = thisCharacter->player->groupLeader->group.head();

		while ( pElement ) {
			RMPlayer *pThePlayer = (RMPlayer *)pElement->ptr();
			pElement = pElement->next();

			if ( pThePlayer->checkAccess( _ACCESS_AMBUSH ) ) {
				return FALSE;
			}
		}
	}

	if ( thisCharacter->combatGroup )
		return FALSE;

	if ( thisCharacter->player->isNPC )
		return FALSE;

	if ( room->zone && !room->zone->allowCombat() )
		return FALSE;

	if ( thisCharacter->health < 1 )
		return FALSE;

	if ( isEnemy ( thisCharacter ) )
		if ( ( character->health * 2 ) > ( thisCharacter->health * 3 ) )
			return TRUE;

	return FALSE;
}

int NPC::isFriend ( WorldObject *thisCharacter )
{
	int alignment = character->coarseAlignment();
	int charAlign = thisCharacter->coarseAlignment();

	// neutral creatures are friendly to everyone
	if ( ( alignment == _ALIGN_NEUTRAL ) ||
		// good creatures are friendly to non-evil cretures
		( (alignment == _ALIGN_GOOD) && (charAlign != _ALIGN_EVIL) ) ||
		// evil creatures are friendly to evil creatures
		( (alignment == _ALIGN_EVIL) && (charAlign == _ALIGN_EVIL) ) ) {
			return TRUE;
	}

	return FALSE;
}

int NPC::isEnemy ( WorldObject *thisCharacter )
{
	return !isFriend ( thisCharacter );
}

int NPC::canTarget ( RMPlayer *thePlayer )
{
	if ( ( thePlayer->character != character ) &&
		 ( zone->allowCombat() ) &&
		 ( !thePlayer->character->hidden ) &&
		 ( !thePlayer->character->combatGroup ) &&
		 ( thePlayer->character->health > 0 ) &&
		 ( !thePlayer->isNPC ) )
	{
		return TRUE;
	}

	return FALSE;
}

void NPC::setCombatTarget( WorldObject* target )
{
	if( target && combatTarget != target ) {
		combatTarget = target;
		acquiredTarget();
	} else combatTarget = target;
}

int NPC::doSpecialLogic ( void )
{
	// virtual function provides some 'colorful' wandering logic for npcs - such
	// as stealing, casting specific non-combat spells, etc.

	return 0;
}

int NPC::gotoXY ( int x, int y, PackedData *thePacket )
{
	PackedMsg response;
	PackedData *packet = NULL;

	int distance = getDistance ( character->x, character->y, x, y );
	int retVal = std::max( (distance / 100), 5);
//(distance / 100) >? 5;

	if ( thePacket ) {
		packet = thePacket;
	} else {
		packet = &response;
		packet->putLong ( character->servID );
		packet->putLong ( character->room->number );
	}

	if ( character ) {
		if ( activeRoom() ) {
			packet->putByte ( _MOVIE_MOVETO );
			packet->putWord ( x );
			packet->putWord ( y );

			if ( thePacket == NULL ) {
				packet->putByte ( _MOVIE_END );

				roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, packet->data(), packet->size(), character->room );
			}
		}

		character->x = x;
		character->y = y;

		if ( groupLeader == this ) {
			LinkedElement *element = group.head();

			while ( element ) {
				NPC *npc = (NPC *)element->ptr();

				if ( npc != this && (npc->character->combatGroup == character->combatGroup) )
					npc->gotoXY ( x + random ( 0, 50 ) - 25, y + random ( 0, 50 ) - 25 );

				element = element->next();
			}
		}
	}

	return retVal;
}

void NPC::teleport ( int num, int bShowAnim )
{
	if ( character ) {
		PackedMsg *response = NULL;

		if ( activeRoom() ) {
			response = new PackedMsg;

			response->putLong ( character->servID );
			response->putLong ( character->room->number );
		}

		character->teleport ( num, response, bShowAnim );

		if ( response ) {
			response->putByte ( _MOVIE_END );

			if ( bShowAnim ) {
				roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, response->data(), response->size(), character->room );
			}

			delete response;
		}

		newRoom ( roomMgr->findRoom ( num ) );
	}
}

int NPC::posn ( int x, int y )
{
	if ( character ) {
		PackedMsg *response = NULL;

		if ( activeRoom() ) {
			response = new PackedMsg;

			response->putLong ( character->servID );
			response->putLong ( character->room->number );
			response->putByte ( _MOVIE_POSN );
			response->putWord ( x );
			response->putWord ( y );
			response->putByte ( character->loop );
			response->putByte ( _MOVIE_END );

			roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, response->data(), response->size(), character->room );

			delete response;
		}

		character->x = x;
		character->y = y;

		if ( groupLeader == this ) {
			LinkedElement *element = group.head();

			while ( element ) {
				NPC *npc = (NPC *)element->ptr();
				if ( npc != this && (npc->character->combatGroup == character->combatGroup) )
					npc->posn ( x, y );
				element = element->next();
			}
		}
	}

	return 0;
}

int NPC::hide ( void ) 
{
	if ( character ) {
		PackedMsg *response = NULL;

		if ( activeRoom() ) {
			response = new PackedMsg;

			response->putLong ( character->servID );
			response->putLong ( character->room->number );
			response->putByte ( _MOVIE_HIDE );
			response->putByte ( _MOVIE_END );

			roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, response->data(), response->size(), character->room );

			delete response;
		}

		character->hidden++;

		if ( groupLeader == this ) {
			LinkedElement *element = group.head();

			while ( element ) {
				NPC *npc = (NPC *)element->ptr();
				if ( npc != this && (npc->character->combatGroup == character->combatGroup) )
					npc->hide();
				element = element->next();
			}
		}
	}

	return 0;
}

int NPC::show ( void ) 
{
	if ( character ) {
		PackedMsg *response = NULL;

		if ( activeRoom() ) {
			response = new PackedMsg;

			response->putLong ( character->servID );
			response->putLong ( character->room->number );
			response->putByte ( _MOVIE_SHOW );
			response->putByte ( _MOVIE_END );

			roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, response->data(), response->size(), character->room );

			delete response;
		}

		character->hidden--;

		if ( groupLeader == this ) {
			LinkedElement *element = group.head();

			while ( element ) {
				NPC *npc = (NPC *)element->ptr();
				if ( npc != this && (npc->character->combatGroup == character->combatGroup) )
					npc->show();
				element = element->next();
			}
		}
	}

	return 0;
}

int NPC::combatPosn ( int theX, int theY, PackedData *thePacket )
{
	PackedMsg response;
	PackedData *packet = &response;

	if ( thePacket ) {
		packet = thePacket;
	} else {
		packet->putLong ( character->servID );
		packet->putLong ( character->room->number );
	}

	if ( character->combatGroup->positionCharacter ( character, theX, theY ) == -1 )
		return 1;

	packet->putByte ( _MOVIE_COMBAT_MOVE );
	packet->putByte ( character->servID );
	packet->putByte ( character->combatX );
	packet->putByte ( character->combatY );

	if ( !thePacket ) {
		packet->putByte ( _MOVIE_END );
		roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, packet->data(), packet->size(), character->room );
	}

	return 3;
}

int NPC::getSummonedCount ( int type )
{
	int count = 0;
	LinkedList *summonedList;

	// set up list to iterate through
	LinkedList *opposition = character->opposition;

	if ( !opposition )
		return 0;

	summonedList = opposition;

	if ( type == _FRIENDLY_SUMMONED_MONSTERS ) {
		WorldObject *obj = (WorldObject *)opposition->head()->ptr();
		summonedList = obj->opposition;
	}

	// iterate through this list and check for summoned NPC's
	if ( summonedList->size() ) {
		LinkedElement *element = summonedList->head();

		while ( element )
		{
			WorldObject *p_obj = (WorldObject *)element->ptr();
			element = element->next();

			/* check to see if this element is a summoned NPC and increment count */
			if ( p_obj->summoned && p_obj->health > 0 ) count++;
		}
	}

	return count;
}

void NPC::hitTarget ( PackedData *packet )
{
}

int NPC::spellAttack ( void )
{
	return 0;
}

int NPC::combatLogic ( void )
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

	if(combatTarget != target) {
        setCombatTarget( target );
	}

	if ( combatTarget && combatTarget->health < 1 ) {
		//mike - enabled this call to killedTarget
		killedTarget();
		setCombatTarget( NULL );
	}

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
	
			int rate = std::max(1 , character->calcNumMoves());
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
		setAction ( new CombatExit ( character ) );	
	}

	return 2;
}

int NPC::normalLogic ( void )
{
	return 5;
}

int NPC::changeRoom ( int dir ) 
{
	// pick a random direction
	int newX = character->x, newY = character->y;
	int enterX = character->x, enterY = character->y;
	int exitX = character->x, exitY = character->y;
	RMRoom *nextRoom = NULL;

	switch ( dir ) {
		// north
		case 0: 
			exitY = 130;	
			enterY = 390;
			newY = 315;
			nextRoom = roomMgr->findRoom ( room->north );
			break;

		// south
		case 1:
			exitY = 390;
			enterY = 130;
			newY = 145;
			nextRoom = roomMgr->findRoom ( room->south );
			break;

		// east
		case 2:
			exitX = 660;
			enterX = -60;
			newX = 20;
			nextRoom = roomMgr->findRoom ( room->east );
			break;

		// west
		case 3:
			exitX = -60;
			enterX = 660;
			newX = 620;
			nextRoom = roomMgr->findRoom ( room->west );
			break;
	}

	if ( nextRoom && nextRoom->size() < _ROOM_LIMIT && nextRoom->zone == room->zone && !nextRoom->isDungeonEntrance ) {
		PackedMsg *response = NULL;

		if ( activeRoom() ) {
			response = new PackedMsg;

			response->putLong ( character->servID );
			response->putLong ( character->room->number );
			response->putByte ( _MOVIE_CHANGE_ROOM );
			response->putByte ( 1 << dir );
			response->putByte ( _MOVIE_END );

			roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, response->data(), response->size(), room );

			delete response;
		}

		gotoXY ( exitX, exitY );
		hide();

		newRoom ( nextRoom );

		posn ( enterX, enterY );
		show();
		gotoXY ( newX, newY );
	}

	return 1;
}

int NPC::changeRoomBoss ( int dir ) 
{
	// pick a random direction
	int newX = character->x, newY = character->y;
	int enterX = character->x, enterY = character->y;
	int exitX = character->x, exitY = character->y;
	RMRoom *nextRoom = NULL;

	switch ( dir ) {
		// north
		case 0: 
			exitY = 130;	
			enterY = 390;
			newY = 315;
			nextRoom = roomMgr->findRoom ( room->north );
			break;

		// south
		case 1:
			exitY = 390;
			enterY = 130;
			newY = 145;
			nextRoom = roomMgr->findRoom ( room->south );
			break;

		// east
		case 2:
			exitX = 660;
			enterX = -60;
			newX = 20;
			nextRoom = roomMgr->findRoom ( room->east );
			break;

		// west
		case 3:
			exitX = -60;
			enterX = 660;
			newX = 620;
			nextRoom = roomMgr->findRoom ( room->west );
			break;
	}

	if ( nextRoom && nextRoom->size() < _ROOM_LIMIT && nextRoom->zone->allowCombat() != 0 && !nextRoom->isDungeonEntrance ) {
		PackedMsg *response = NULL;

		if ( activeRoom() ) {
			response = new PackedMsg;

			response->putLong ( character->servID );
			response->putLong ( character->room->number );
			response->putByte ( _MOVIE_CHANGE_ROOM );
			response->putByte ( 1 << dir );
			response->putByte ( _MOVIE_END );

			roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, response->data(), response->size(), room );

			delete response;
		}

		gotoXY ( exitX, exitY );
		hide();

		newRoom ( nextRoom );

		posn ( enterX, enterY );
		show();
		gotoXY ( newX, newY );
	}

	return 1;
}

int NPC::doit ( void )
{
	int retVal = 10;

	if ( !character->combatGroup ) {
		retVal = normalLogic();
	}

	return retVal;
}

void NPC::killedTarget ( void )
{
}

void NPC::acquiredTarget ( void )
{
}

int NPC::partyHeal ( void )
{
	int didHeal=FALSE;

	if ( random ( 0, 1 ) )
		return FALSE;

	if ( character->health < ( character->healthMax / 3 ) )
	{
		/* if I'm a master thaum. cast greater heal, else cast normal heal */
		int skill = character->getSkill ( _SKILL_THAUMATURGY );

		if ( skill > 2 && skill < 4 )
		{
			setAction ( new CombatCastSpell ( character, _SPELL_HEAL, character->servID, 0, 0 ) );
			didHeal=TRUE;
		}
		else if ( skill >= 4 )
		{
			setAction ( new CombatCastSpell ( character, _SPELL_GREATER_HEAL, character->servID, 0, 0 ) );
			didHeal=TRUE;
		}
	}
	else
	{
		LinkedList *myList = NULL;

		/* check to see if any of my party members are hurt */
		if ( character->combatGroup->attackers.contains ( character ) )
			/* use attackers list and search for people to heal */
			myList = &character->combatGroup->attackers;
		else
			myList = &character->combatGroup->defenders;

		/* iterate through list.. */
		if ( myList->size() > 1 )
		{
			LinkedElement *element = myList->head();

			while ( element )
			{
				WorldObject *p_obj = (WorldObject *)element->ptr();
				element = element->next();

				/* make sure not self and see if hurt */
				if ( p_obj != character && ( p_obj->health < ( p_obj->healthMax / 2 ) ) && random ( 0, 1 ) )
				{
					/* heal our party friend up */
					/* if I'm a master thaum. cast greater heal, else cast normal heal */
					if (! random (0, 2))
					{
						setAction ( new CombatCastSpell ( character, _SPELL_HEAL, p_obj->servID, 0, 0 ) );
						didHeal=TRUE;
						/* all done -- return */
						return 1;
					}
				}
			}
		}
	}
	return (didHeal);
}

int NPC::activeRoom ( void )
{
	if ( character && character->room && character->room->numPlayers > 0 )
		return 1;

	return 0;
}

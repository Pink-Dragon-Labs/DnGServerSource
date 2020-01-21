/*
	ACTIONS.CPP
	Global action functions

	Author: Stephen Nichols
*/

#include "roommgr.hpp"
#include "globals.hpp"

#define ACTION_SETUP \
PackedData *thePacket = packet;\
PackedMsg msg;

#define PREPARE_ACTION_MOVIE \
RMRoom *targetRoom = target->getRoom();\
if ( !packet || (targetRoom->number != packet->longAt ( 12 )) ) {\
	target->makeVisible ( 1 );\
	msg.putLong ( target->servID );\
	msg.putLong ( targetRoom->number );\
	thePacket = &msg;\
}

#define SEND_ACTION_MOVIE \
if ( thePacket == &msg ) {\
	thePacket->putByte ( _MOVIE_END );\
	roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, thePacket->data(), thePacket->size(), targetRoom );\
}

//
// return a pointer to the object named "name"
// 
// note: this handles special names like 'dobj' and 'iobj'
//

WorldObject *findActionObj ( WorldObject *object, char *name )
{
	if ( !object || !object->isWorldObject() )
		return NULL;

	if ( !strcmp ( name, "dobj" ) )
		return object->directObject;

	if ( !strcmp ( name, "iobj" ) )
		return object->indirectObject;

	if ( !strcmp ( name, "self" ) )
		return object;

	RMRoom *room = object->getRoom();
	TreeNode *node = NULL;

	if ( room->instanceNum > -1 ) {
		char theName[512];
		sprintf ( sizeof ( theName ), theName, "%s%03d", name, room->instanceNum );
		node = gObjectTree.find ( theName );
	} else {
		node = gObjectTree.find ( name );
	}

	if ( node ) 
		return (WorldObject *)node->data;

	return NULL;			
}

// teleport someone somewhere
ACTION_FN ( actTeleport ) 
{
	ACTION_SETUP

	if ( argc >= 2 ) {
		// get the room number to go to
		int room = atoi ( argv[1] );

		// find the target of the teleport
		WorldObject *target = findActionObj ( object, argv[0] );

		if ( !target || !target->player ) 
			return _WO_ACTION_PROHIBITED;

		if ( target->room->instanceNum > -1 ) {
			room = (room * 1000) + target->room->instanceNum;
		}

		if ( argc >= 3 && target->player->groupLeader && atoi ( argv[2] ) ) {
			LinkedElement *element = target->player->groupLeader->group.head();

			while ( element ) {
				RMPlayer *player = (RMPlayer *)element->ptr();
				element = element->next();

				if ( player->room == target->player->room ) 
					player->character->teleport ( room, thePacket );
			}
		} else {
			target->teleport ( room, thePacket );
		}
	}
	return _WO_ACTION_HANDLED;
}

// damage some object
ACTION_FN ( actDamage )
{
	ACTION_SETUP

	if ( argc == 4 ) {
		// get who we are supposed to damage
		WorldObject *target = findActionObj ( object, argv[0] );

		if ( target ) {
			// special preparation macro
			PREPARE_ACTION_MOVIE

			char output[1024] = "";
			int damage = random ( atoi ( argv[2] ), atoi ( argv[3] ) );
			target->takeDamage ( atoi ( argv[1] ), object, damage, output, thePacket );

			putMovieText ( object, thePacket, "%s", output );

			SEND_ACTION_MOVIE
		}
	}

	return _WO_ACTION_HANDLED;
}

// heal some object
ACTION_FN ( actHeal )
{
	ACTION_SETUP

	if ( argc == 3 ) {
		// get who we are supposed to heal
		WorldObject *target = findActionObj ( object, argv[0] );

		if ( target ) {
			// special preparation macro
			PREPARE_ACTION_MOVIE

			char output[1024] = "";
			int damage = random ( atoi ( argv[1] ), atoi ( argv[2] ) );
			target->takeDamage ( WorldObject::_DAMAGE_HEAL, object, -damage, output, thePacket );

			putMovieText ( object, thePacket, "%s", output );

			SEND_ACTION_MOVIE
		}
	}

	return _WO_ACTION_HANDLED;
}

// open some object
ACTION_FN ( actOpen )
{
	ACTION_SETUP

	if ( argc == 1 ) {
		// the the object to open
		WorldObject *target = findActionObj ( object, argv[0] );

		if ( !target || !target->getBase ( _BOPEN ) )
			return _WO_ACTION_PROHIBITED;

		// special preparation macro
		PREPARE_ACTION_MOVIE

		object->open ( target, thePacket );

		WorldObject *linkObj = object->linkTo;

		if ( linkObj ) {
			if ( linkObj->beOpened ( linkObj ) == _WO_ACTION_HANDLED ) {
				PackedMsg msg;

				msg.putLong ( linkObj->servID );
				msg.putLong ( linkObj->getRoom()->number );

				msg.putByte ( _MOVIE_OPEN );
				msg.putLong ( linkObj->servID );
				msg.putLong ( linkObj->servID );

				msg.putByte ( _MOVIE_END );

				roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, msg.data(), msg.size(), linkObj->getRoom() );
			}
			// send the movie
			SEND_ACTION_MOVIE
		}
	}

	return _WO_ACTION_HANDLED;
}

// close some object
ACTION_FN ( actClose )
{
	ACTION_SETUP

	if ( argc == 1 ) {
		// the the object to open
		WorldObject *target = findActionObj ( object, argv[0] );

		if ( !target || !target->getBase ( _BOPEN ) )
			return _WO_ACTION_PROHIBITED;
		
		// special preparation macro
		PREPARE_ACTION_MOVIE

		object->close ( target, thePacket );

		// send the movie
		SEND_ACTION_MOVIE
	}
	return _WO_ACTION_HANDLED;
}

// lock some object
ACTION_FN ( actLock )
{
	ACTION_SETUP

	if ( argc == 1 ) {
		// get the object to lock
		WorldObject *target = findActionObj ( object, argv[0] );

		if ( !target || !target->getBase( _BLOCK ) )
			return _WO_ACTION_PROHIBITED;

		PREPARE_ACTION_MOVIE

		int retVal = target->lock ( target, NULL, thePacket );

		SEND_ACTION_MOVIE
	}
	return _WO_ACTION_HANDLED;
}

// unlock some object
ACTION_FN ( actUnlock )
{
	ACTION_SETUP

	if ( argc == 1 ) {
		// get the object to unlock
		WorldObject *target = findActionObj ( object, argv[0] );

		if ( !target || !target->getBase ( _BLOCK ) )
			return _WO_ACTION_PROHIBITED;

		PREPARE_ACTION_MOVIE

		int retVal = target->unlock ( target, NULL, thePacket );

		SEND_ACTION_MOVIE
	}
	return _WO_ACTION_HANDLED;
}

// activate a switch
ACTION_FN ( actActivate )
{
	ACTION_SETUP

	if ( argc == 1 ) {
		// the the object to open
		WorldObject *target = findActionObj ( object, argv[0] );

		if ( target && target->getBase ( _BSWITCH ) ) {

			PREPARE_ACTION_MOVIE

			object->open ( target, thePacket );

			SEND_ACTION_MOVIE
		}
	}

	return _WO_ACTION_HANDLED;
}

// deactivate a switch 
ACTION_FN ( actDeactivate )
{
	ACTION_SETUP

	if ( argc == 1 ) {
		// the the object to open
		WorldObject *target = findActionObj ( object, argv[0] );

		if ( target && target->getBase ( _BSWITCH ) ) {

			PREPARE_ACTION_MOVIE

			object->close ( target, thePacket );

			SEND_ACTION_MOVIE
		}
	}

	return _WO_ACTION_HANDLED;
}

// enable a switch
ACTION_FN ( actEnable )
{
	PackedData *thePacket = packet;

	if ( argc == 1 ) {
		// the the object to enable
		WorldObject *target = findActionObj ( object, argv[0] );

		if ( target ) {
			BSwitch *bswitch = (BSwitch *)target->getBase ( _BSWITCH );

			if ( bswitch )
				bswitch->enabled = 1;
		}
	}

	return _WO_ACTION_HANDLED;
}

// disable a switch
ACTION_FN ( actDisable )
{
	PackedData *thePacket = packet;

	if ( argc == 1 ) {
		// the the object to enable
		WorldObject *target = findActionObj ( object, argv[0] );

		if ( target ) {
			BSwitch *bswitch = (BSwitch *)target->getBase ( _BSWITCH );

			if ( bswitch )
				bswitch->enabled = 0;
		}
	}

	return _WO_ACTION_HANDLED;
}

// create an object somewhere
ACTION_FN ( actCreateObj )
{
	PackedData *thePacket = packet;

	if ( argc == 5 ) {
		RMRoom *room = roomMgr->findRoom ( atoi ( argv[1] ) );

		if ( room && roomMgr->findClass ( argv[0] ) ) {
			WorldObject *obj = room->addObject ( argv[0], atoi ( argv[2] ), atoi ( argv[3] ), atoi ( argv[4] ), 0 );

			obj->makeVisible ( 1, thePacket );
		}
	}

	return _WO_ACTION_HANDLED;
}

// summon a createure into a room
ACTION_FN ( actSummon )
{
	PackedData *thePacket = packet;

	if ( argc < 7 )
		return _WO_ACTION_HANDLED;

	int roomNum = atoi ( argv[0] );

	if ( object->room && (object->room->instanceNum > -1) ) {
		roomNum = (roomNum * 1000) + object->room->instanceNum;
	}

	RMRoom *room = roomMgr->findRoom ( roomNum );
	int engage = atoi ( argv[1] );
	int count = atoi ( argv[2] );

	int index = 3;

	NPC *firstNPC = NULL;

	for ( int i=0; i<count; i++ ) {
		WorldObject *super = roomMgr->findClass ( argv[index] );

		if ( room && super ) {
			WorldObject *obj = room->addObject ( argv[index], atoi ( argv[index + 1] ), atoi ( argv[index + 2] ), atoi ( argv[index + 3] ), 0 );

			index += 3;

			NPC *npc = makeNPC ( obj );
			npc->newRoom ( room );

			if ( firstNPC ) {
				npc->joinGroup ( firstNPC );
			} else {
				firstNPC = npc;
			}

			// let the ai go!
			npc->aiReady = 1;
		}
	}

	if ( engage && firstNPC ) {
		WorldObject *target = object->directObject;

		if ( !target->combatGroup && (target->health > 0) )
			firstNPC->engage ( object->directObject, thePacket, object->directObject->x, object->directObject->y );
	}

	return _WO_ACTION_HANDLED;
}

// place floating text over an object
ACTION_FN ( actFloatingText )
{
	return _WO_ACTION_HANDLED;
}

// generate a zone "chat" message
ACTION_FN ( actZoneMsg )
{
	return _WO_ACTION_HANDLED;
}

// generate a room "chat" message
ACTION_FN ( actRoomMsg )
{
	return _WO_ACTION_HANDLED;
}

// play a sound effect
ACTION_FN ( actPlaySound )
{
	return _WO_ACTION_HANDLED;
}

// play a midi file
ACTION_FN ( actPlayMusic )
{
	return _WO_ACTION_HANDLED;
}

// cast a spell on an object
ACTION_FN ( actCastSpell )
{
	ACTION_SETUP

	if ( argc == 2 ) {
		// the object to 
		WorldObject *target = findActionObj ( object, argv[1] );
		int spellID = atoi ( argv[0] );

		if ( target ) {
			PREPARE_ACTION_MOVIE

			spell_info *spell = &gSpellTable[spellID];
			WorldObject *owner = object->getBaseOwner();

			if ( owner->player ) {
				owner->player->castSpell ( spell, object, target->servID, -1, -1, thePacket );
			} else {
				char output[1024] = "";
				spell->function ( 0, object, target->servID, -1, -1, output, thePacket, NULL );
				putMovieText ( object, thePacket, "%s", output );
			}
			

			SEND_ACTION_MOVIE
		}
	}

	return _WO_ACTION_HANDLED;
}

// cast a spell on all players in a room
ACTION_FN ( actCastSpellRoom )
{
	return _WO_ACTION_HANDLED;
}

// destroy an object
ACTION_FN ( actDestroyObj )
{
	return _WO_ACTION_HANDLED;
}

// open an object for a time and close it again
ACTION_FN ( actTimedOpen )
{
	return _WO_ACTION_HANDLED;
}

// clear actions from a particular object
ACTION_FN ( actClearActions )
{
	// find object to clear actions of
	WorldObject *target = findActionObj ( object, argv[0] );

	if ( target && target->actions ) {
		// convert the verb
		int verb = textToVerb ( argv[1] );

		LinkedElement *element = target->actions->head();

		while ( element ) {
			ActionInfo *info = (ActionInfo *)element->ptr();
			element = element->next();

			if ( info->verb == verb ) {
				target->actions->del ( info );
				delete info;
			}
		}
	}
	
	return _WO_ACTION_HANDLED;
}

// add a new action to an object
ACTION_FN ( actAddAction )
{
	// find object to add action to
	WorldObject *target = findActionObj ( object, argv[0] );

	if ( target ) {
		int newArgc = argc - 3;
		char **newArgv = (char **)malloc ( sizeof ( char * ) * newArgc );

		int verb = textToVerb ( argv[1] );
		action_t action = getActionFunction ( argv[2] );

		int i;

		for ( i=3; i<argc; i++ ) 
			newArgv[i-3] = strdup ( argv[i] );

		target->addAction ( verb, action, newArgc, newArgv );

		// toss the action parameters
		for ( i=0; i<newArgc; i++ )
			free ( newArgv[i] );

		free ( newArgv );
	}

	return _WO_ACTION_HANDLED;
}

// set an affect on an object
ACTION_FN ( actSetAffect )
{
	if ( argc == 5 ) {
		WorldObject *target = findActionObj ( object, argv[0] );

		if ( target ) {
			target->addAffect ( atoi ( argv[1] ), atoi ( argv[2] ), atoi ( argv[3] ), atoi ( argv[4] ), 0, packet );
		}
	}
	// mike - added option to specify value of affect.
	else if ( argc == 6 ) {
		WorldObject *target = findActionObj ( object, argv[0] );

		if ( target ) {
			target->addAffect ( atoi ( argv[1] ), atoi ( argv[2] ), atoi ( argv[3] ), atoi ( argv[4] ), atoi ( argv[5] ), packet );
		}
	}

	return _WO_ACTION_HANDLED;
}

// clear an affect on an object
ACTION_FN ( actClearAffect )
{
	if ( argc == 4 ) {
		WorldObject *target = findActionObj ( object, argv[0] );

		if ( target ) {
			target->delAffect ( atoi ( argv[1] ), atoi ( argv[2] ), atoi ( argv[3] ), packet );
		}
	}

	return _WO_ACTION_HANDLED;
}

// kill an object
ACTION_FN ( actKill )
{
	WorldObject *target = findActionObj ( object, argv[0] );

	if ( target ) 
		target->changeHealth ( -target->health, target, 1, 1, packet );

	return _WO_ACTION_HANDLED;
}

// force owner to drop object
ACTION_FN ( actForceDrop )
{
	WorldObject *target = findActionObj ( object, argv[0] );

	if ( !target || !target->getBase ( _BCARRY ) || target->getBase ( _BHEAD ) || target->hasAffect( _AFF_STAFF ) )
		return _WO_ACTION_PROHIBITED;

	WorldObject *owner = target->getBaseOwner();

	if ( owner == target )
		return _WO_ACTION_PROHIBITED;

	int oldHealth = owner->health;
	owner->health = 1;

	target->forceOff();
	target->forceOut();

	packet->putByte ( _MOVIE_TAKE_OFF );
	packet->putLong ( owner->servID );
	packet->putLong ( target->servID );

	packet->putByte ( _MOVIE_DROP );
	packet->putLong ( owner->servID );
	packet->putLong ( target->servID );

	owner->health = oldHealth;

	return _WO_ACTION_HANDLED;
}

// damn an object
ACTION_FN ( actDamn )
{
	return _WO_ACTION_HANDLED;
}

// inspire an object
ACTION_FN ( actInspire )
{
	return _WO_ACTION_HANDLED;
}

// change experience
ACTION_FN ( actChangeExperience )
{
	ACTION_SETUP
	//changeExperience obj amount

	if ( argc == 2 ) {
		// the the object to mark
		WorldObject *target = findActionObj ( object, argv[0] );

		if ( !target || !target->getBase ( _BCHARACTER ) )
			return _WO_ACTION_PROHIBITED;

		int amount = atoi( argv[1] );

		BCharacter* bchar = static_cast<BCharacter*>( target->getBase( _BCHARACTER ) );

		PREPARE_ACTION_MOVIE

		bchar->gainExperience( amount, packet );

		SEND_ACTION_MOVIE
	}

	return _WO_ACTION_HANDLED;
}

// give the mark (or curse) of Enid to someone...
ACTION_FN ( actEnidMark )
{
	ACTION_SETUP

	if ( argc == 1 ) {
		// the the object to mark
		WorldObject *target = findActionObj ( object, argv[0] );

		if ( !target || !target->getBase ( _BCHARACTER ) )
			return _WO_ACTION_PROHIBITED;

		// special preparation macro
		PREPARE_ACTION_MOVIE

		// get the alignment of our target...
		int nAlignment = target->coarseAlignment();

		// if the alignment is good, give the mark...
		if ( nAlignment == _ALIGN_GOOD ) {
			// put the bless special effect...
			packet->putByte ( _MOVIE_SPECIAL_EFFECT );
			packet->putLong ( target->servID );
			packet->putByte ( _SE_ENIDS_BLESSING );
			packet->putByte ( 1 );
			packet->putByte ( 1 );
			packet->putLong ( target->servID );

			//when you are blessed by a god, remove all other blessings and
			//curses.
			target->delAffect ( _AFF_CURSE_ENID, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, packet );
			
			target->delAffect ( _AFF_MARK_DUACH, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, packet );
			target->delAffect ( _AFF_CURSE_DUACH, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, packet );

			//despothes' mark
			target->delAffect ( _AFF_MARK_DESPOTHES, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, packet );
			target->delAffect ( _AFF_IMMOLATION_LIGHTNING, _AFF_TYPE_NORMAL, _AFF_SOURCE_GIFT, packet );
			target->delAffect ( _AFF_DAMAGE_COLD, _AFF_TYPE_WEAKNESS, _AFF_SOURCE_GIFT, packet );
			//despothes' curse
			target->delAffect ( _AFF_CURSE_DESPOTHES, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, packet );
			target->delAffect ( _AFF_DAMAGE_LIGHTNING, _AFF_TYPE_WEAKNESS, _AFF_SOURCE_GIFT, packet );

			// see if the target already has the mark or not...
			affect_t *pAffect = target->hasAffect ( _AFF_MARK_ENID, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL );

			// if target has the affect, just update the duration, otherwise add it...
			if ( pAffect ) pAffect->duration = 360;
			else target->addAffect ( _AFF_MARK_ENID, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, 360, 0, packet );

			putMovieText ( target, packet, "Enid is pleased with the prayer of %s. %s has been blessed with immunity to death magic for 6 hours!\n", target->getName(), target->getName() );
		}

		//MIKE - changed this activation from == _ALIGN_EVIL
		// to include neutral alignments to be cursed.

		// if the alignment is *NOT* good, give the curse...
		else
		{
			// put the curse special effect...
			packet->putByte ( _MOVIE_SPECIAL_EFFECT );
			packet->putLong ( target->servID );
			packet->putByte ( _SE_DEATH_WISH );
			packet->putByte ( 1 );
			packet->putLong ( target->servID );

			// handle removing existing marks and curses 
			target->delAffect ( _AFF_MARK_ENID, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, packet );

			// see if the target already has the curse or not...
			affect_t *pAffect = target->hasAffect ( _AFF_CURSE_ENID, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL );

			// if target has the affect, update the duration, else add it...
			if ( pAffect ) pAffect->duration = 360;
			else target->addAffect ( _AFF_CURSE_ENID, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, 360, 0, packet );

			putMovieText ( target, packet, "Enid is angered by the prayer of %s. %s has been cursed with healing magic immunity for 6 hours!\n", target->getName(), target->getName() );
		}

		// update health display...
		target->calcHealth();
		packet->putByte ( _MOVIE_HEALTH_MAX );
		packet->putLong ( target->servID );
		packet->putLong ( target->servID );
		packet->putLong ( target->healthMax );

		// send the movie
		SEND_ACTION_MOVIE
	}

	return _WO_ACTION_HANDLED;
}

// give the mark (or curse) of Duach to someone...
ACTION_FN ( actDuachMark )
{
	ACTION_SETUP

	if ( argc == 1 ) {
		// the the object to mark
		WorldObject *target = findActionObj ( object, argv[0] );

		if ( !target || !target->getBase ( _BCHARACTER ) )
			return _WO_ACTION_PROHIBITED;

		// special preparation macro
		PREPARE_ACTION_MOVIE

		// get the alignment of our target...
		int nAlignment = target->coarseAlignment();

		// if the alignment is evil, give the mark...
		if ( nAlignment == _ALIGN_EVIL ) {
			// put the bless special effect...
			packet->putByte ( _MOVIE_SPECIAL_EFFECT );
			packet->putLong ( target->servID );
			packet->putByte ( _SE_ENIDS_BLESSING );
			packet->putByte ( 1 );
			packet->putByte ( 1 );
			packet->putLong ( target->servID );

			//when you are blessed by a god, remove all other blessings and
			//curses.
			target->delAffect ( _AFF_MARK_ENID, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, packet );
			target->delAffect ( _AFF_CURSE_ENID, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, packet );

			target->delAffect ( _AFF_CURSE_DUACH, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, packet );

			//despothes' mark
			target->delAffect ( _AFF_MARK_DESPOTHES, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, packet );
			target->delAffect ( _AFF_IMMOLATION_LIGHTNING, _AFF_TYPE_NORMAL, _AFF_SOURCE_GIFT, packet );
			target->delAffect ( _AFF_DAMAGE_COLD, _AFF_TYPE_WEAKNESS, _AFF_SOURCE_GIFT, packet );
			//despothes' curse
			target->delAffect ( _AFF_CURSE_DESPOTHES, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, packet );
			target->delAffect ( _AFF_DAMAGE_LIGHTNING, _AFF_TYPE_WEAKNESS, _AFF_SOURCE_GIFT, packet );

			// see if the target already has the mark or not...
			affect_t *pAffect = target->hasAffect ( _AFF_MARK_DUACH, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL );

			// if target has the affect, update the duration, otherwise add it...
			if ( pAffect ) pAffect->duration = 360;
			else target->addAffect ( _AFF_MARK_DUACH, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, 360, 0, packet );

			putMovieText ( target, packet, "Duach is pleased with the prayer of %s. %s has been blessed with 90%% poison resistance for 6 hours!\n", target->getName(), target->getName() );
		}

		//MIKE - changed the activation from good, to not evil. will include
		// neutral alignments into the curse now.

		// if the alignment is NOT EVIL, give the curse...
		else{ //if ( nAlignment != _ALIGN_EVIL ) {
			// put the curse special effect...
			packet->putByte ( _MOVIE_SPECIAL_EFFECT );
			packet->putLong ( target->servID );
			packet->putByte ( _SE_DEATH_WISH );
			packet->putByte ( 1 );
			packet->putLong ( target->servID );

			// handle removing existing marks and curses 
			target->delAffect ( _AFF_MARK_DUACH, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, packet );

			// see if the target already has the curse or not...
			affect_t *pAffect = target->hasAffect ( _AFF_CURSE_DUACH, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL );

			// if target has the affect, update the duration, else add it...
			if ( pAffect ) pAffect->duration = 360;
			else target->addAffect ( _AFF_CURSE_DUACH, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, 360, 0, packet );

			putMovieText ( target, packet, "Duach is angered by the prayer of %s. %s has been cursed with immunity to resistance spells and constant damage weakness for 6 hours!\n", target->getName(), target->getName() );
		}

		// update health display...
		target->calcHealth();
		packet->putByte ( _MOVIE_HEALTH_MAX );
		packet->putLong ( target->servID );
		packet->putLong ( target->servID );
		packet->putLong ( target->healthMax );

		// send the movie
		SEND_ACTION_MOVIE
	}

	return _WO_ACTION_HANDLED;
}

// MIKE - give the mark (or curse) of Despothes to someone...
ACTION_FN ( actDespothesMark )
{
	ACTION_SETUP

	if ( argc == 1 ) {
		// the the object to mark
		WorldObject *target = findActionObj ( object, argv[0] );

		if ( !target || !target->getBase ( _BCHARACTER ) )
			return _WO_ACTION_PROHIBITED;

		// special preparation macro
		PREPARE_ACTION_MOVIE

		// get the alignment of our target...
		int nAlignment = target->coarseAlignment();

		// if the alignment is neutral, give the mark...
		if ( nAlignment == _ALIGN_NEUTRAL )
		{
			packet->putByte ( _MOVIE_SPECIAL_EFFECT );
			packet->putLong ( target->servID );
			packet->putByte ( _SE_BANG );
			packet->putByte ( 1 );
			packet->putByte ( 53 ); //color, 53 = blue
			packet->putLong ( target->servID ); //target
			//packet->putByte ( _SE_BANG_BLUE );
			//packet->putByte ( 1 );
			//packet->putLong ( target->servID ); //target

			// handle removing existing marks and curses 
			target->delAffect ( _AFF_MARK_ENID, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, packet );
			target->delAffect ( _AFF_CURSE_ENID, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, packet );
			target->delAffect ( _AFF_MARK_DUACH, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, packet );
			target->delAffect ( _AFF_CURSE_DUACH, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, packet );
			
			//despothes' curse:
			target->delAffect ( _AFF_CURSE_DESPOTHES, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, packet );
			target->delAffect ( _AFF_DAMAGE_LIGHTNING, _AFF_TYPE_WEAKNESS, _AFF_SOURCE_GIFT, packet );
			
			// see if the target already has the mark. if so, update the duration, else add the affect
			affect_t *pAffect;
			
			//despothes' mark (hair color change)
			pAffect = target->hasAffect ( _AFF_MARK_DESPOTHES, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL );
			if (pAffect) pAffect->duration = 360;
			else target->addAffect ( _AFF_MARK_DESPOTHES, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, 360, 0, packet );

			//despothes' mark (lightning immolation)
			pAffect = target->hasAffect ( _AFF_IMMOLATION_LIGHTNING, _AFF_TYPE_NORMAL, _AFF_SOURCE_GIFT );
			if(pAffect) pAffect->duration = 360;
			else target->addAffect ( _AFF_IMMOLATION_LIGHTNING, _AFF_TYPE_NORMAL, _AFF_SOURCE_GIFT, 360, 50, packet );

			//despothes' mark (cold weakness)
			pAffect = target->hasAffect ( _AFF_DAMAGE_COLD, _AFF_TYPE_WEAKNESS, _AFF_SOURCE_GIFT );
			if(pAffect) pAffect->duration = 360;
			else target->addAffect ( _AFF_DAMAGE_COLD, _AFF_TYPE_WEAKNESS, _AFF_SOURCE_GIFT, 360, 50, packet );

			putMovieText ( target, packet, "Despothes is pleased with the prayer of %s. %s has been blessed with lightning immolation and a weakness to cold damage for 6 hours!\n", target->getName(), target->getName() );
		}

		// if the alignment is NOT neutral, give the curse...
		else
		{
			packet->putByte ( _MOVIE_SPECIAL_EFFECT );
			packet->putLong ( target->servID );
			packet->putByte ( _SE_ELECTRIC_FURY );
			packet->putByte ( 1 ); //async
			packet->putByte ( 1 ); //number of targets
			packet->putLong ( target->servID ); //target1

			// handle removing existing marks and curses 
			//despothes' mark
			target->delAffect ( _AFF_MARK_DESPOTHES, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, packet );
			target->delAffect ( _AFF_IMMOLATION_LIGHTNING, _AFF_TYPE_NORMAL, _AFF_SOURCE_GIFT, packet );
			target->delAffect ( _AFF_DAMAGE_COLD, _AFF_TYPE_WEAKNESS, _AFF_SOURCE_GIFT, packet );

			// see if the target already has the curse or not...
			affect_t *pAffect;
			
			//if we have the affect, update the duration
			//if we don't have the affect, add it.
			pAffect = target->hasAffect ( _AFF_CURSE_DESPOTHES, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL );
			if (pAffect) pAffect->duration = 360;
			else target->addAffect ( _AFF_CURSE_DESPOTHES, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, 360, 0, packet );

			pAffect = target->hasAffect ( _AFF_DAMAGE_LIGHTNING, _AFF_TYPE_WEAKNESS, _AFF_SOURCE_GIFT );
			if(pAffect) pAffect->duration = 360;
			else target->addAffect ( _AFF_DAMAGE_LIGHTNING, _AFF_TYPE_WEAKNESS, _AFF_SOURCE_GIFT, 360, 50, packet );

			putMovieText ( target, packet, "Despothes is angered by the prayer of %s. %s has been cursed with a weakness to lightning for 6 hours!\n", target->getName(), target->getName() );
		}

		// update health display...
		target->calcHealth();
		packet->putByte ( _MOVIE_HEALTH_MAX );
		packet->putLong ( target->servID );
		packet->putLong ( target->servID );
		packet->putLong ( target->healthMax );

		// send the movie
		SEND_ACTION_MOVIE
	}

	return _WO_ACTION_HANDLED;
}


// player state management... 
ACTION_FN ( actChangeManaDrain )
{
	ACTION_SETUP

	if ( argc == 2 ) {
		// the the object to modify...
		WorldObject *target = findActionObj ( object, argv[0] );

		if ( !target || !target->player )
			return _WO_ACTION_PROHIBITED;

		CPlayerState *pPlayer = target->character;
		double dAmount = atof ( argv[1] );

		pPlayer->ChangeManaDrain ( dAmount );
	}

	return _WO_ACTION_HANDLED;
}

ACTION_FN ( actChangeMeleePhase )
{
	ACTION_SETUP

	if ( argc == 2 ) {
		// the the object to modify...
		WorldObject *target = findActionObj ( object, argv[0] );

		if ( !target || !target->player )
			return _WO_ACTION_PROHIBITED;

		CPlayerState *pPlayer = target->character;
		int dAmount = atoi ( argv[1] );

		pPlayer->ChangeMeleePhasing ( dAmount );
	}

	return _WO_ACTION_HANDLED;
}

ACTION_FN ( actChangeEvilMDMMod )
{
	ACTION_SETUP

	if ( argc == 2 ) {
		// the the object to modify...
		WorldObject *target = findActionObj ( object, argv[0] );

		if ( !target || !target->player )
			return _WO_ACTION_PROHIBITED;

		CPlayerState *pPlayer = target->character;
		int dAmount = atoi ( argv[1] );

		pPlayer->ChangeEvilMDMMod ( dAmount );
	}

	return _WO_ACTION_HANDLED;
}

ACTION_FN ( actChangeGoodMDMMod )
{
	ACTION_SETUP

	if ( argc == 2 ) {
		// the the object to modify...
		WorldObject *target = findActionObj ( object, argv[0] );

		if ( !target || !target->player )
			return _WO_ACTION_PROHIBITED;

		CPlayerState *pPlayer = target->character;
		int dAmount = atoi ( argv[1] );

		pPlayer->ChangeGoodMDMMod ( dAmount );
	}

	return _WO_ACTION_HANDLED;
}

ACTION_FN ( actChangeMeleeArmorPiercing )
{
	ACTION_SETUP

	if ( argc == 2 ) {
		// the the object to modify...
		WorldObject *target = findActionObj ( object, argv[0] );

		if ( !target || !target->player )
			return _WO_ACTION_PROHIBITED;

		CPlayerState *pPlayer = target->character;
		int dAmount = atoi ( argv[1] );

		pPlayer->ChangeMeleeArmorPierce ( dAmount );
	}

	return _WO_ACTION_HANDLED;
}

ACTION_FN ( actChangeMystImmunityCount )
{
	ACTION_SETUP

	if ( argc == 2 ) {
		// the the object to modify...
		WorldObject *target = findActionObj ( object, argv[0] );

		if ( !target || !target->player )
			return _WO_ACTION_PROHIBITED;

		CPlayerState *pPlayer = target->character;
		int dAmount = atoi ( argv[1] );

		int nImmunityCount = pPlayer->GetMystImmunityCount();
		nImmunityCount -= dAmount;

		if ( nImmunityCount < 0 )
			pPlayer->ChangeMystImmunityCount ( -nImmunityCount );
	}

	return _WO_ACTION_HANDLED;
}

ACTION_FN ( actChangeCastResistance )
{
	ACTION_SETUP

	if ( argc > 2 ) {
		// the the object to modify...
		WorldObject *target = findActionObj ( object, argv[0] );

		if ( !target || !target->player )
			return _WO_ACTION_PROHIBITED;

		CPlayerState *pPlayer = target->character;
		int nValue = atoi ( argv[1] );

		for ( int i=2; i<argc; i++ ) {
			pPlayer->ChangeCastResistance ( atoi ( argv[i] ), nValue );	
		}
	}

	return _WO_ACTION_HANDLED;
}

ACTION_FN ( actChangeSpellResistance )
{
	ACTION_SETUP

	if ( argc > 2 ) {
		// the the object to modify...
		WorldObject *target = findActionObj ( object, argv[0] );

		if ( !target || !target->player )
			return _WO_ACTION_PROHIBITED;

		CPlayerState *pPlayer = target->character;
		int nValue = atoi ( argv[1] );

		for ( int i=2; i<argc; i++ ) {
			pPlayer->ChangeSpellResistance ( atoi ( argv[i] ), nValue );	
		}
	}

	return _WO_ACTION_HANDLED;
}

ACTION_FN ( actChangeSDM )
{
	ACTION_SETUP

	if ( argc > 2 ) {
		// the the object to modify...
		WorldObject *target = findActionObj ( object, argv[0] );

		if ( !target || !target->player )
			return _WO_ACTION_PROHIBITED;

		CPlayerState *pPlayer = target->character;
		int nValue = atoi ( argv[1] );

		for ( int i=2; i<argc; i++ ) {
			pPlayer->ChangeSDM ( atoi ( argv[i] ), nValue );	
		}
	}

	return _WO_ACTION_HANDLED;
}

ACTION_FN ( actMinotaurSoulConsume )
{
	if ( argc == 2 ) {	
		ACTION_SETUP

		WorldObject *target = findActionObj ( object, argv[0] );

		if ( !target || !target->player )
			return _WO_ACTION_PROHIBITED;
		
		unsigned char chance = random(0,100);

		PREPARE_ACTION_MOVIE
					
		char output[1024] = "";

		if( chance < 40 ) { //40% chance of invul
			//give improved invulnerability

			thePacket->putByte ( _MOVIE_SPECIAL_EFFECT );
			thePacket->putLong ( target->servID );
			thePacket->putByte ( _SE_BANG_GREEN );
			thePacket->putByte ( 1 );
			thePacket->putLong ( target->servID ); //target

			affect_t *affect = target->hasAffect ( _AFF_INVULNERABLE );

			//this value is a bit higher than the standard value (70) gotten by the spell
			if ( affect ) {
				//if theyre already invulnerable, lets upgrade it
				affect->value = 85;
				affect->duration = 30; // lasts for 30 minutes
			} else {
				target->addAffect ( _AFF_INVULNERABLE, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, 30, 85, thePacket );
			}

			// |c23| = green
			putMovieText ( target, thePacket, "|c23|The soul of the minotaur is in harmony with %s's own. %s is surrounded by an improved invulnerability aura.", &target->character->properName, &target->character->properName );
		}
		else if ( chance < 60 ) {
			//next 20% = poison.

			thePacket->putByte ( _MOVIE_SPECIAL_EFFECT );
			thePacket->putLong ( target->servID );
			thePacket->putByte ( _SE_STEAL_LIFE );
			thePacket->putByte ( 1 );
			thePacket->putLong ( target->servID ); //target

			unsigned char soulLevel = 50 + (100 * atoi( argv[1] ));
			unsigned short poisonDamage = (unsigned short)random( (soulLevel * .75), (soulLevel * 1.75) );
			target->takeDamage( _AFF_DAMAGE_POISON, NULL, poisonDamage, NULL, thePacket );
				
			putMovieText ( target, thePacket, "|c60|The soul of the minotaur breaks into a rage within %s. A poison is released into %s body.", &target->character->properName, target->getPronoun( 1 ) );
			//------------------------------
		} else {
			putMovieText ( target, thePacket, "|c20|The soul of the minotaur seems to dissolve within %s.", &target->character->properName );
		}
		
		SEND_ACTION_MOVIE
	}

	return _WO_ACTION_HANDLED;
}

ACTION_FN ( actAddForwardProc )
{
	ACTION_SETUP

	if ( argc > 2 ) {
		// the the object to modify...

		// we can only handle self with this type of scroll
		WorldObject *ftarget = findActionObj ( object, argv[0] );

        RMPlayer *target;

        if(ftarget)
            target = ftarget->getBaseOwner()->player;

       if ( !target || !target->player )
		return _WO_ACTION_PROHIBITED;

        int procTarget = -1;

		if ( !strcmp ( argv[1], "head" ) )
            procTarget = 0;
        else if ( !strcmp ( argv[1], "chest" ) )
            procTarget = 1;
        else if ( !strcmp ( argv[1], "bands" ) )
            procTarget = 4;
        else if ( !strcmp ( argv[1], "legs" ) )
            procTarget = 6;
        else if ( !strcmp ( argv[1], "feet" ) )
            procTarget = 7;
        else if ( !strcmp ( argv[1], "shield" ) )
            procTarget = 17;
        else if ( !strcmp ( argv[1], "weapon" ) )
            procTarget = 18;

        if(procTarget == -1)return _WO_ACTION_PROHIBITED;

		int proc = atoi ( argv[2] );

        if(!isValidForwardProcID(proc))return _WO_ACTION_PROHIBITED;

        BWearable *bwear;
        BWeapon *bweapon;


        if(procTarget == 17)
            bwear = (BWearable *)target->character->curShield->getBase(_BWEAR);
        else if (procTarget == 18)
            bweapon = (BWeapon *)target->character->curWeapon->getBase(_BWEAPON);
        else bwear = (BWearable *)target->character->getWornOn(procTarget)->getBase(_BWEAR);


        if(procTarget != 18)
        {
            if(bwear)
            {
                bwear->spellProcID = proc;
                roomMgr->sendPlayerText(target, "|c2|Your %s glows for a moment and your %s accepts the enchantment.", object->getName(), bwear->self->getName());
            }
        }
        if(procTarget == 18)
        {
            if(bweapon)
            {
                bweapon->spellProcID = proc;
                roomMgr->sendPlayerText(target, "|c2|Your %s glows for a moment and your %s accepts the enchantment.", object->getName(), bweapon->self->getName());
            }
        }

	}

	return _WO_ACTION_HANDLED;
}

ACTION_FN ( actAddReverseProc )
{
	ACTION_SETUP

	if ( argc > 2 ) {
		// the the object to modify...

		// we can only handle self with this type of scroll
		WorldObject *ftarget = findActionObj ( object, argv[0] );

        RMPlayer *target;

        if(ftarget)
            target = ftarget->getBaseOwner()->player;

       if ( !target || !target->player )
		return _WO_ACTION_PROHIBITED;

        int procTarget = -1;

		if ( !strcmp ( argv[1], "head" ) )
            procTarget = 0;
        else if ( !strcmp ( argv[1], "chest" ) )
            procTarget = 1;
        else if ( !strcmp ( argv[1], "bands" ) )
            procTarget = 4;
        else if ( !strcmp ( argv[1], "legs" ) )
            procTarget = 6;
        else if ( !strcmp ( argv[1], "feet" ) )
            procTarget = 7;
        else if ( !strcmp ( argv[1], "shield" ) )
            procTarget = 17;
        else if ( !strcmp ( argv[1], "weapon" ) )
            procTarget = 18;

        if(procTarget == -1)return _WO_ACTION_PROHIBITED;

		int proc = atoi ( argv[2] );

        if(!isValidReverseProcID(proc))return _WO_ACTION_PROHIBITED;

        BWearable *bwear;
        BWeapon *bweapon;


        if(procTarget == 17)
            bwear = (BWearable *)target->character->curShield->getBase(_BWEAR);
        else if (procTarget == 18)
            bweapon = (BWeapon *)target->character->curWeapon->getBase(_BWEAPON);
        else bwear = (BWearable *)target->character->getWornOn(procTarget)->getBase(_BWEAR);


        if(procTarget != 18)
        {
            if(bwear)
            {
                bwear->reverseProcID = proc;
                roomMgr->sendPlayerText(target, "|c2|Your %s glows for a moment and your %s accepts the enchantment.", object->getName(), bwear->self->getName());
            }
        }
        if(procTarget == 18)
        {
            if(bweapon)
            {
                bweapon->reverseProcID = proc;
                roomMgr->sendPlayerText(target, "|c2|Your %s glows for a moment and your %s accepts the enchantment.", object->getName(), bweapon->self->getName());
            }
        }

	}

	return _WO_ACTION_HANDLED;
}

ACTION_FN ( actAddReverseProcChance )
{
	ACTION_SETUP

	if ( argc > 2 ) {
		// the the object to modify...

		// we can only handle self with this type of scroll
		WorldObject *ftarget = findActionObj ( object, argv[0] );

        RMPlayer *target;

        if(ftarget)
            target = ftarget->getBaseOwner()->player;

       if ( !target || !target->player )
		return _WO_ACTION_PROHIBITED;

        int procTarget = -1;

		if ( !strcmp ( argv[1], "head" ) )
            procTarget = 0;
        else if ( !strcmp ( argv[1], "chest" ) )
            procTarget = 1;
        else if ( !strcmp ( argv[1], "bands" ) )
            procTarget = 4;
        else if ( !strcmp ( argv[1], "legs" ) )
            procTarget = 6;
        else if ( !strcmp ( argv[1], "feet" ) )
            procTarget = 7;
        else if ( !strcmp ( argv[1], "shield" ) )
            procTarget = 17;
        else if ( !strcmp ( argv[1], "weapon" ) )
            procTarget = 18;

        if(procTarget == -1)return _WO_ACTION_PROHIBITED;

		int chance = atoi ( argv[2] );

       // if(!isValidReverseProcID(proc))return _WO_ACTION_PROHIBITED;

        BWearable *bwear;
        BWeapon *bweapon;


        if(procTarget == 17)
            bwear = (BWearable *)target->character->curShield->getBase(_BWEAR);
        else if (procTarget == 18)
            bweapon = (BWeapon *)target->character->curWeapon->getBase(_BWEAPON);
        else bwear = (BWearable *)target->character->getWornOn(procTarget)->getBase(_BWEAR);


        if(procTarget != 18)
        {
            if(bwear)
            {
                bwear->reverseProcChance = chance;
                //roomMgr->sendPlayerText(target, "|c2|Your %s crumbles while your %s glows for a momment.", object->getName(), bwear->self->getName());
            }
        }
        if(procTarget == 18)
        {
            if(bweapon)
            {
                bweapon->reverseProcChance = chance;
               // roomMgr->sendPlayerText(target, "|c2|Your %s crumbles while your %s glows for a momment.", object->getName(), bweapon->self->getName());
            }
        }

	}

	return _WO_ACTION_HANDLED;
}

ACTION_FN ( actAddForwardProcChance )
{
	ACTION_SETUP

	if ( argc > 2 ) {
		// the the object to modify...

		// we can only handle self with this type of scroll
		WorldObject *ftarget = findActionObj ( object, argv[0] );

        RMPlayer *target;

        if(ftarget)
            target = ftarget->getBaseOwner()->player;

       if ( !target || !target->player )
		return _WO_ACTION_PROHIBITED;

        int procTarget = -1;

		if ( !strcmp ( argv[1], "head" ) )
            procTarget = 0;
        else if ( !strcmp ( argv[1], "chest" ) )
            procTarget = 1;
        else if ( !strcmp ( argv[1], "bands" ) )
            procTarget = 4;
        else if ( !strcmp ( argv[1], "legs" ) )
            procTarget = 6;
        else if ( !strcmp ( argv[1], "feet" ) )
            procTarget = 7;
        else if ( !strcmp ( argv[1], "shield" ) )
            procTarget = 17;
        else if ( !strcmp ( argv[1], "weapon" ) )
            procTarget = 18;

        if(procTarget == -1)return _WO_ACTION_PROHIBITED;

		int chance = atoi ( argv[2] );

       // if(!isValidReverseProcID(proc))return _WO_ACTION_PROHIBITED;

        BWearable *bwear;
        BWeapon *bweapon;


        if(procTarget == 17)
            bwear = (BWearable *)target->character->curShield->getBase(_BWEAR);
        else if (procTarget == 18)
            bweapon = (BWeapon *)target->character->curWeapon->getBase(_BWEAPON);
        else bwear = (BWearable *)target->character->getWornOn(procTarget)->getBase(_BWEAR);


        if(procTarget != 18)
        {
            if(bwear)
            {
                bwear->forwardProcChance = chance;
                //roomMgr->sendPlayerText(target, "|c2|Your %s crumbles while your %s glows for a momment.", object->getName(), bwear->self->getName());
            }
        }
        if(procTarget == 18)
        {
            if(bweapon)
            {
                bweapon->forwardProcChance = chance;
               // roomMgr->sendPlayerText(target, "|c2|Your %s crumbles while your %s glows for a momment.", object->getName(), bweapon->self->getName());
            }
        }

	}

	return _WO_ACTION_HANDLED;
}

//GemGameChest - vClose trigger
ACTION_FN ( actGemChest_vClose )
{
	//on closing this chest we need to check if there is anything in it.
	//If so, we need to play the 'poof' special effect

	if( !object ) return _WO_ACTION_HANDLED;

	BContainer* bcontain = static_cast<BContainer*>( object->getBase( _BCONTAIN ) );
	if( !bcontain ) return _WO_ACTION_HANDLED;

	ACTION_SETUP
	WorldObject *target = object;
	PREPARE_ACTION_MOVIE

	if( !bcontain->contents.size() ) {
		//the chest is empty, how dare they!
		thePacket->putByte ( _MOVIE_SPECIAL_EFFECT );
		thePacket->putLong ( object->servID );
		thePacket->putByte ( _SE_POOF );
		thePacket->putByte ( 1 );
		thePacket->putLong ( object->servID );
	}

	//bcontain->contents.head()

	return _WO_ACTION_HANDLED;
}

void GemGameTreasure( int lowNumItems, int highNumItems, const char* treasureClass, WorldObject* chestObj )
{
	if( !treasureClass || !chestObj ) return;

	int numItems = random( lowNumItems, highNumItems );

	WorldObject *super = roomMgr->findClass ( const_cast<char*>( treasureClass ) );
	BTreasure *btreasure = (BTreasure *)super->getBase ( _BTREASURE );

	if( super && btreasure ) {
		while( numItems-- ) {
			WorldObject* o = btreasure->makeObj();
			o->addToDatabase();
			o->forceIn( chestObj, 1 );
		}
	}
	else logDisplay ( "%s:%d no treasure base", __FILE__, __LINE__ );

}

ACTION_FN ( actGemChest_vLock )
{
	//get class numbers of gems if they exist
	static WorldObject* gemI    = roomMgr->findClass( "MistGemI" );
	static WorldObject* gemII   = roomMgr->findClass( "MistGemII" );
	static WorldObject* gemIII  = roomMgr->findClass( "MistGemIII" );
	static WorldObject* gemIV   = roomMgr->findClass( "MistGemIV" );
	static WorldObject* gemV    = roomMgr->findClass( "MistGemV" );
	static WorldObject* gemVI   = roomMgr->findClass( "MistGemVI" );
	static WorldObject* gemVII  = roomMgr->findClass( "MistGemVII" );
	static WorldObject* gemVIII = roomMgr->findClass( "MistGemVIII" );
	static WorldObject* gemIX   = roomMgr->findClass( "MistGemIX" );

	if ( !gemI || !gemII || !gemIII || !gemIV ||
		!gemV || !gemVI || !gemVII || !gemVIII || !gemIX )
		return _WO_ACTION_HANDLED;

	//someone just locked the chest...

	if( !object ) return _WO_ACTION_HANDLED;


	BContainer* bcontain = static_cast<BContainer*>( object->getBase( _BCONTAIN ) );
	if( !bcontain ) return _WO_ACTION_HANDLED;

	if ( argc != 1 || strcmp( argv[0], "dobj" ) )
		return _WO_ACTION_HANDLED;

	WorldObject *target = findActionObj ( object, argv[0] );

	ACTION_SETUP
	PREPARE_ACTION_MOVIE
	char output[1024] = "";

	if( !bcontain->contents.size() ) {
		//the chest is empty, we zap the person that locked it (shame on them)
		thePacket->putByte ( _MOVIE_SPECIAL_EFFECT );
		thePacket->putLong ( object->servID );
		thePacket->putByte ( _SE_LIGHTNING );
		thePacket->putByte ( 1 );
		thePacket->putLong ( target->servID );

		target->takeDamage ( _AFF_DAMAGE_LIGHTNING, object, random( 20, 300 ), output, thePacket, 1 );
	} else {
		
		//there's something in the chest

		char chance = random(0, 99);

		
		//40% chance of gobbling it all up
		
		//if( chance < 40 ) {
		if( chance < 0 ) {

			thePacket->putByte ( _MOVIE_SPECIAL_EFFECT );
			thePacket->putLong ( object->servID );
			thePacket->putByte ( _SE_POOF );
			thePacket->putByte ( 1 );
			thePacket->putLong ( object->servID );

			//destroy everything in there!
			LinkedElement* element = bcontain->contents.head();
			while( element ) {
				WorldObject* obj = static_cast< WorldObject* >( element->ptr() );
				element = element->next();
				if( !obj ) continue;

				thePacket->putByte ( _MOVIE_TOSS_OBJECT );
				thePacket->putLong ( object->servID );
				thePacket->putLong ( obj->servID );
				roomMgr->destroyObj ( obj, 0, __FILE__, __LINE__ );
			}

			//unlock and open up
			object->unlock( object, 0, thePacket );
			object->open( object, thePacket );

		}
		//35% chance of teleporting away with the objects
		//else if ( chance < 75 ) {
		else if ( chance < 0 ) {
			RMRoom* newRoom = roomMgr->findRoom( 5029 );
			//RMRoom* newRoom = (RMRoom*)(object->room->zone->rooms.at ( random ( 0, object->room->zone->rooms.size() - 1 )))
			int randomRoom = newRoom->number;
			roomMgr->sendRoomText( object->room, "Heading to room %d! Come on!", randomRoom );
			
			//poof
			thePacket->putByte ( _MOVIE_SPECIAL_EFFECT );
			thePacket->putLong ( object->servID );
			thePacket->putByte ( _SE_POOF );
			thePacket->putByte ( 1 );
			thePacket->putLong ( object->servID );

			//destroy the chest
			object->room->delObject( object, 0 );

			//have to send this toss movie - can't notify with delObject() else
			//the client crashes trying to execute the lock movie that
			//gets inserted afterwards.
			thePacket->putByte ( _MOVIE_TOSS_OBJECT );
			thePacket->putLong ( object->servID );
			thePacket->putLong ( object->servID );

			newRoom->addObject( object, 1 );
			
			PackedMsg newRoomMovie;
			
			newRoomMovie.putLong( -1 );
			newRoomMovie.putLong ( newRoom->number );
			newRoomMovie.putByte ( _MOVIE_SPECIAL_EFFECT );
			newRoomMovie.putLong ( object->servID );
			newRoomMovie.putByte ( _SE_POOF );
			newRoomMovie.putByte ( 1 );
			newRoomMovie.putLong ( object->servID );
			newRoomMovie.putByte ( _MOVIE_END );
			roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, newRoomMovie.data(), newRoomMovie.size(), newRoom );
		
		//25% chance of treasure
		} else {

			//we didn't gobble things up, and we didn't change rooms. Let's check the
			//contents and see what we've got.

			unsigned int goldValue = 0;
			short classification = 0;

			LinkedElement* objElement = bcontain->contents.head();
			while( objElement )
			{
				WorldObject* objContained = static_cast< WorldObject* >( objElement->ptr() );
				objElement = objElement->next();

				if( !objContained ) continue;

				if( objContained->isMoney() ) {
					goldValue += objContained->value;
				}
				//shift the classification - the chest can hold up to 4
				else if( !strcmp( objContained->basicName, "talisman" ) ) {
					//we have a talisman, lets see which one
					int n = objContained->classNumber;

					if( n == gemI->classNumber ||
						n == gemII->classNumber ||
						n == gemIII->classNumber ) classification += 1;
					else if( n == gemIV->classNumber ||
						n == gemV->classNumber ||
						n == gemVI->classNumber ) classification += 4;
					else if( n == gemVII->classNumber ||
						n == gemVIII->classNumber ||
						n == gemIX->classNumber ) classification += 16;

				} else classification -= 25; //we dont like other items

				//remove this object from the chest
				thePacket->putByte ( _MOVIE_TOSS_OBJECT );
				thePacket->putLong ( object->servID );
				thePacket->putLong ( objContained->servID );
				roomMgr->destroyObj ( objContained, 0, __FILE__, __LINE__ );

			}

			//the talisman classification is the main determiner of the treasure
			//then finely adjusted by the amount of gold

			//gobble it up
			LinkedElement* element = bcontain->contents.head();
			while( element ) {
				WorldObject* obj = static_cast< WorldObject* >( element->ptr() );
				element = element->next();
				if( !obj ) continue;

				thePacket->putByte ( _MOVIE_TOSS_OBJECT );
				thePacket->putLong ( object->servID );
				thePacket->putLong ( obj->servID );
				roomMgr->destroyObj ( obj, 0, __FILE__, __LINE__ );
			}

			if( classification < 1 ) {
				// CLASSIFICATION 0 -------------

				//we really don't care for the offering.
				thePacket->putByte ( _MOVIE_SPECIAL_EFFECT );
				thePacket->putLong ( object->servID );
				thePacket->putByte ( _SE_BANG );
				thePacket->putByte ( 1 );
				thePacket->putByte ( 58 ); //color (58 = red?)
				thePacket->putLong ( object->servID );

				//zap the player that locked it due to the worthless investment!
				thePacket->putByte ( _MOVIE_SPECIAL_EFFECT );
				thePacket->putLong ( object->servID );
				thePacket->putByte ( _SE_LIGHTNING );
				thePacket->putByte ( 1 );
				thePacket->putLong ( target->servID );

				target->takeDamage ( _AFF_DAMAGE_LIGHTNING, object, random( 20, 300 ), output, thePacket, 1 );

				// CLASSIFICATION 0 -------------

			}
			else if ( classification < 6 ) {

				// CLASSIFICATION A -------------

				//they could have done better, but at least they gave us something decent.
				thePacket->putByte ( _MOVIE_SPECIAL_EFFECT );
				thePacket->putLong ( object->servID );
				thePacket->putByte ( _SE_BANG_ORANGE );
				thePacket->putByte ( 1 );
				thePacket->putLong ( object->servID );

				//lets see how deep their pockets were
				if( goldValue == 0 ) {
					//zero?? zap them
				}
				else if( goldValue < 5000 ) {
					//TREASURE 0
					GemGameTreasure( 3, 6, "GemGameTreasure0", object );
				}
				else if ( goldValue < 20000 ) {
					//TREASURE 1
					GemGameTreasure( 3, 6, "GemGameTreasure1", object );
				}
				else if ( goldValue < 50000 ) {
					//TREASURE 3
					GemGameTreasure( 2, 5, "GemGameTreasure3", object );
				}
				else {
					//TREASURE 4
					GemGameTreasure( 2, 4, "GemGameTreasure4", object );
				}
	
				// CLASSIFICATION A -------------

			}
			else if ( classification < 45 ) {

				// CLASSIFICATION B -------------

				//good mid-range classification
				thePacket->putByte ( _MOVIE_SPECIAL_EFFECT );
				thePacket->putLong ( object->servID );
				thePacket->putByte ( _SE_BANG_YELLOW );
				thePacket->putByte ( 1 );
				thePacket->putLong ( object->servID );

				//lets see how deep their pockets were
				if( goldValue == 0 ) {
					//zero?? zap them
				}

				else if( goldValue < 5000 ) {
					//an insult! Kill them!
					//TREASURE 1
					GemGameTreasure( 3, 6, "GemGameTreasure1", object );
				}
				else if ( goldValue < 20000 ) {
					//we like this
					//TREASURE 2
					GemGameTreasure( 2, 5, "GemGameTreasure2", object );

				}
				else if ( goldValue < 50000 ) {
					//TREASURE 4
					GemGameTreasure( 2, 4, "GemGameTreasure4", object );

				}
				else {
					//TREASURE 6
					GemGameTreasure( 1, 2, "GemGameTreasure6", object );

				}

				// CLASSIFICATION B -------------

			}
			else {

				// CLASSIFICATION C -------------

				//good stuff! rock and roll!
				thePacket->putByte ( _MOVIE_SPECIAL_EFFECT );
				thePacket->putLong ( object->servID );
				thePacket->putByte ( _SE_EARTHQUAKE );
				thePacket->putByte ( 0 );
				thePacket->putLong ( object->servID );

				//everyone in the room needs damaging (10% - 25% of their health)
				LinkedElement* playerElement = object->room->head();

				while( playerElement ) {
					RMPlayer* rmp = static_cast<RMPlayer*>( playerElement->ptr() );
					playerElement = playerElement->next();

					if( rmp && rmp->character ) {
						int damage = static_cast<int>( random( rmp->character->healthMax * .1, rmp->character->healthMax * .25 ) );
						rmp->character->takeDamage( WorldObject::_DAMAGE_EARTHQUAKE, object, damage, output, thePacket, 1 );
					}
				}


				if( goldValue == 0 ) {

					//take away 5% - 15% of their max health
					int damage = static_cast<int>( random( target->healthMax * .05, target->healthMax * .15 ) );
					target->takeDamage( WorldObject::_DAMAGE_EARTHQUAKE, object, damage, output, thePacket, 1 );

					//TREASURE 0
					GemGameTreasure( 3, 6, "GemGameTreasure0", object );
				}
				else if( goldValue < 5000 ) {
					//TREASURE 4
					GemGameTreasure( 2, 4, "GemGameTreasure4", object );
				}
				else if ( goldValue < 20000 ) {
					//TREASURE 5
					GemGameTreasure( 2, 3, "GemGameTreasure5", object );
				}
				else if ( goldValue < 50000 ) {
					//TREASURE 7
					GemGameTreasure( 1, 3, "GemGameTreasure7", object );
				}
				else {
					//TREASURE 8
					GemGameTreasure( 1, 3, "GemGameTreasure8", object );
				}

				// CLASSIFICATION C -------------
			}

			//unlock and open up to reveal what's inside!
			object->unlock( object, 0, thePacket );
			object->open( object, thePacket );
		}
	}

	putMovieText ( object, thePacket, "%s", output );

	SEND_ACTION_MOVIE

	return _WO_ACTION_HANDLED;
}

/*
	COMBAT.CPP
	Combat related classes.

	Author: Stephen Nichols
*/
#include <algorithm>
#include "roommgr.hpp"
#include "combat.hpp"
#include "globals.hpp"

typedef struct {
	int x, y;
} posn_t;

posn_t gAttackerPosn[_MAX_NPC_GROUP_SIZE] = {
	{1, 6},
	{0, 5},
	{0, 7},
	{0, 6},
	{0, 4},
	{0, 8},
	{0, 3},
	{0, 9},
	{0, 2},
	{0, 10},
	{0, 1},
	{0, 11},
};

posn_t gDefenderPosn[_MAX_NPC_GROUP_SIZE] = {
	{23, 6},
	{22, 5},
	{22, 7},
	{22, 6},
	{22, 4},
	{22, 8},
	{22, 3},
	{22, 9},
	{22, 2},
	{22, 10},
	{22, 1},
	{22, 11},
};

char* gCombatActionText[_CA_MAX] = {
	"move",
	"attack",
	"guard",
	"chase",
	"flee",
	"cast spell",
	"eat",
	"charge"
	"equip",
	"exit",
	"berserk",
	"say"
};

CombatActionLogger gCombatActionLogger;

//
// mike - these 2 obstacleRandom functions act just like SCIRandom(), however
// the generator and incrementor here must always match the one in
// the client's math.cpp
//
int obstacleRandom ( int* seed ) {
	*seed = *seed * 0x0019660d + 0x3c6ef35f;  
	return *seed;
}
int obstacleRandom ( int* seed, int start, int end )
{
	unsigned int range = (unsigned int) (end - start) + 1;
	unsigned int tmp = (obstacleRandom(seed) & 0x00ffff00L) >> 8;
	tmp = ((tmp * range) >> 16 ) + start;
	return tmp;
}

CombatGroup::CombatGroup()
{
	setPtrType ( this, _MEM_COMBAT_GROUP );

	// init the grid
	memset ( grid, 0, sizeof ( grid ) );
	cloud = NULL;
	pvp = 0;
	attackerLevel = defenderLevel = 0;
	gainedEP = 0;
	round = 0;
	attackerDamage = defenderDamage = 0;
	attackerHealth = defenderHealth = 0;
	freeFlee = 1;
	ambushAttack = 0;

	int i;

	obstacleSeed = random ( 0x0001, 0xFFFF );

	int tSeed = obstacleSeed;
	int obstacleCount = obstacleRandom ( &tSeed, 5, 15 );

	for ( i=0; i<obstacleCount; i++ ) {
		int theX = obstacleRandom ( &tSeed, 0, 23 );
		int theY = obstacleRandom ( &tSeed, 0, 17 );
		obstacleRandom ( &tSeed, 1, 10 );
		obstacleRandom ( &tSeed, 1, 10 );

		grid[theX][theY] = 1;
	}

	for ( i=0; i<_MAX_COMBATANT; i++ ) {
	 	combatNames[i] = NULL;
	}
}

CombatGroup::~CombatGroup()
{
	// step through the combatants list and clear the combatGroup pointers
	LinkedElement *element = combatants.head();

	while ( element ) {
		WorldObject *obj = (WorldObject *)element->ptr();
		
		if ( obj )
			obj->combatGroup = NULL;

		element = element->next();
	}

	attackers.release();
	defenders.release();
	combatants.release();
	players.release();
	turnReadyList.release();
	turnPendingList.release();
	combatReadyList.release();
	combatPendingList.release();

	PackedMsg response;

	if ( !cloud || ( cloud && cloud->destructing ) ) 
		goto skipCloudCleanup;

	if ( !isValidPtr ( cloud ) ) {
		logInfo ( _LOG_ALWAYS, "Combat cloud is invalid ptr on cleanup" );
		goto skipCloudCleanup;
	}

	if (  getPtrType ( cloud ) != _MEM_WOBJECT ) {
		logInfo ( _LOG_ALWAYS, "Combat cloud is invalid MEM_WOBJECT on cleanup" );
		goto skipCloudCleanup;
	}

	if ( !cloud->combatGroup ) {
		goto skipCloudCleanup;
	}

	if ( cloud->combatGroup != this ) {
		logInfo ( _LOG_ALWAYS, "Combat cloud has invalid group on cleanup" );
		goto skipCloudCleanup;
	}

	// step through all of the objects and destroy treasure objects
	if ( cloud->room ) {
		PackedMsg tossMsg;
		tossMsg.putLong ( cloud->servID );
		tossMsg.putLong ( cloud->room->number );

		element = objects.head();

		while ( element ) {
			WorldObject *pObject = (WorldObject *)element->ptr();
			element = element->next();
	
			if ( pObject->physicalState & _STATE_TREASURE ) {
				tossMsg.putByte ( _MOVIE_TOSS_OBJECT );
				tossMsg.putLong ( cloud->servID );
				tossMsg.putLong ( pObject->servID );

				roomMgr->destroyObj ( pObject, 0, __FILE__, __LINE__ );
			}
		}

		tossMsg.putByte ( _MOVIE_END );

		roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, tossMsg.data(), tossMsg.size(), cloud->room );
	}

	// make all of the objects in the combat cloud visible 
	element = objects.head();

	// put the number of objects
	response.putByte ( objects.size() );
	response.putWord ( cloud->x );
	response.putWord ( cloud->y );
	response.putLong ( cloud->room? cloud->room->number : -1 );

	while ( element ) {
		LinkedElement *next = element->next();

		WorldObject *obj = (WorldObject *)element->ptr();

		if ( obj ) {

			response.putLong ( obj->servID );
			
			obj->x = cloud->x;
			obj->y = cloud->y;
			
			deleteObject ( obj );
		}

		element = next;
	}

	if ( cloud->room )
		roomMgr->sendToRoom ( _IPC_COMBAT_EXIT, response.data(), response.size(), cloud->room );

	cloud->combatGroup = NULL;
	roomMgr->destroyObj ( cloud, TRUE, __FILE__, __LINE__ );

skipCloudCleanup:

	for ( int i=0; i<_MAX_COMBATANT; i++ ) 
		if ( combatNames[i] )
			free ( combatNames[i] );
}

int CombatGroup::servID ( void )
{
	return cloud? cloud->servID : -1;
}

void CombatGroup::addCharacter ( WorldObject *obj, int side )
{
	if ( !obj ) {
		logInfo ( _LOG_ALWAYS, "Non-object being added to combat cloud." );
		return;
	}
		
	if( !obj->player ) {
		logInfo ( _LOG_ALWAYS, "**Non-player being added to combat cloud." );
		return;
	}

	if ( obj->combatGroup ) {
//		fatal ( "Object being added to more than one combat group!" );
		logInfo ( _LOG_ALWAYS, "%s: being added to combat cloud more than once", obj->getName() );
		return;
	}

	if ( combatants.contains ( obj ) ) {
		logInfo ( _LOG_ALWAYS, "%s: added to combat cloud more than once", obj->getName() );
		return;
	}

	obj->combatReady = FALSE;
	obj->combatRound = 0;

	switch ( side ) {
		case _COMBAT_ATTACKER: {
			attackers.add ( obj );
			attackerHealth += obj->health;

			int index = attackers.size() - 1;

			if ( obj->combatX == -1 ) {
				int theX = gAttackerPosn[index].x;
				int theY = gAttackerPosn[index].y;

				obj->combatX = theX;
				obj->combatY = theY;
			}

			grid[obj->combatX][obj->combatY] = obj->servID;

			obj->opposition = &defenders;
		}

		break;

		case _COMBAT_DEFENDER: {
			defenders.add ( obj );
			defenderHealth += obj->health;

			int index = defenders.size() - 1;

			if ( obj->combatX == -1 ) {
				int theX = gDefenderPosn[index].x;
				int theY = gDefenderPosn[index].y;

				obj->combatX = theX;
				obj->combatY = theY;
			}

			grid[obj->combatX][obj->combatY] = obj->servID;

			obj->opposition = &attackers;
		}

		break;
	}

	combatants.add ( obj );
	players.add ( obj->player );

	if ( !obj->summoned ) {
		combatPendingList.add ( obj );

		for ( int i=0; i<_MAX_COMBATANT; i++ ) {
			if ( !combatNames[i] ) {
				combatNames[i] = strdup ( obj->getName() );
				break;
			}
		}
	}

	obj->combatGroup = this;
}

void CombatGroup::deleteCharacter ( WorldObject *obj )
{
	if ( combatants.contains ( obj ) ) {
		//mike- fix stuck in combat
		if( grid[obj->combatX][obj->combatY] == obj->servID )
			grid[obj->combatX][obj->combatY] = 0;

		int pendingTurn = turnPendingList.size();
		int pendingCombat = combatPendingList.size();

		attackers.del ( obj );
		defenders.del ( obj );
		combatants.del ( obj );
		players.del ( obj->player );
		turnReadyList.del ( obj );
		turnPendingList.del ( obj );
		combatReadyList.del ( obj );
		combatPendingList.del ( obj );

		BCharacter *bchar = (BCharacter *)obj->getBase ( _BCHARACTER );

		obj->combatGroup = NULL;
		obj->opposition = NULL;

		//
		// if there are people still in the combat and noone is left waiting in the
		// pending list when there was before the deletion, do a combat round
		//
		if ( pendingTurn && combatants.size() && !turnPendingList.size() ) {
			if ( !gPendingCombats.contains ( this ) )
				gPendingCombats.add ( this );
		} 

		else if ( pendingCombat && combatants.size() && !combatPendingList.size() ) {
			if ( !gPendingCombats.contains ( this ) ) 
				gPendingCombats.add ( this );
		}

		// reset ambush time...
		RMPlayer *pPlayer = obj->player;

		if ( pPlayer ) {
			if ( pPlayer->isNPC && ambushAttack ) {
				pPlayer->lastAmbushTime = -1;
			} else {
				// Grant a reprieve after being ambushed of 3 minutea.
				pPlayer->lastAmbushTime = getsecondsfast() + ( 60 * 3 );
			}
		}
	} else {
		logCrash ( "CombatGroup::deleteCharacter did not find character to delete." );
	}

	if ( !combatants.size() ) {
		if ( !gDeadCombats.contains ( this ) ) 
			gDeadCombats.add ( this );
	}
}

void CombatGroup::rewardCombatants( WorldObject* obj ) {
	LinkedElement *element;

	if ( attackers.contains( obj ) ) {
		if ( attackers.size() == 1 ) {
			element = defenders.head();
			
			while ( element ) {
				WorldObject* character = (WorldObject *)element->ptr();
				element = element->next();

				character->changeHealth( character->healthMax - character->health, NULL );
					
				if ( character->hasAffect ( _AFF_POISONED ) ) {
					character->clearAffect ( _AFF_POISONED, _AFF_TYPE_NORMAL );
				}

				if ( !character->player->isNPC ) {
					roomMgr->sendPlayerText( character->player, "%s has disappeared. The gods reward you for your valor.", obj->getName() );
				}
			}
		}
	} else if ( defenders.contains( obj ) ) {
		if ( defenders.size() == 1 ) {
			element = attackers.head();
			
			while ( element ) {
				WorldObject* character = (WorldObject *)element->ptr();
				element = element->next();

				character->changeHealth( character->healthMax - character->health, NULL );
					
				if ( character->hasAffect ( _AFF_POISONED ) ) {
					character->clearAffect ( _AFF_POISONED, _AFF_TYPE_NORMAL );
				}

				if ( !character->player->isNPC ) {
					roomMgr->sendPlayerText( character->player, "%s has disappeared. The gods reward you for your valor.", obj->getName() );
				}
			}
		}
	} else {
		logInfo ( _LOG_ALWAYS, "Tried to reward the combatants and I was not in combat!" );
	}
}

void CombatGroup::makeTurnReady ( WorldObject *obj )
{
	int turnPending = turnPendingList.size();

	turnPendingList.del ( obj );
	turnReadyList.add ( obj );

	if ( turnPending && !turnPendingList.size() ) {
		if ( !gPendingCombats.contains ( this ) )
			gPendingCombats.add ( this );
	}
}

void CombatGroup::makeCombatReady ( WorldObject *obj )
{
	int combatPending = combatPendingList.size();

	combatPendingList.del ( obj );
	combatReadyList.add ( obj );

	if ( combatPending && !combatPendingList.size() ) {
		if ( !gPendingCombats.contains ( this ) )
			gPendingCombats.add ( this );
	}
}

void CombatGroup::addObject ( WorldObject *obj )
{
	objects.add ( obj );
	obj->combatGroup = this;
}

void CombatGroup::deleteObject ( WorldObject *obj )
{
	if ( objects.contains ( obj ) ) {
		objects.del ( obj );
		obj->combatGroup = NULL;
	}
}

int CombatGroup::isOccupied ( int x, int y )
{
	if ( x >= _COMBAT_GRID_WIDTH || x < 0 || y >= _COMBAT_GRID_HEIGHT || y < 0 )
		return 1;

	return grid[x][y];
}

void CombatGroup::displayGrid( void )
{
	for ( int y=0; y<_COMBAT_GRID_HEIGHT; y++ ) {
			for( int x=0; x<_COMBAT_GRID_WIDTH; x++)
				printf( "%c", (grid[x][y] == 1)?'O': (grid[x][y] > 1)?'P':'.' );
			printf("\n");
	}
}

void CombatGroup::findClosestPoint ( int x1, int y1, int x2, int y2, int *pointX, int *pointY  )
{
	int startingX = x1 - 1, startingY = y1 - 1;
	int endingX = x1 + 2, endingY = y1 + 2;

	if ( startingX < 0 )
		startingX = 0;

	else if ( endingX > _COMBAT_GRID_WIDTH )
		endingX = _COMBAT_GRID_WIDTH;	

	if ( startingY < 0 )
		startingY = 0;

	else if ( endingY > _COMBAT_GRID_HEIGHT )
		endingY = _COMBAT_GRID_HEIGHT;

	int closestDist = 100000;

	for ( int theY=startingY; theY<endingY; theY++ ) {
		for ( int theX=startingX; theX<endingX; theX++ ) {
			if ( grid[theX][theY] == 0 ) {
				int distance = getDistance ( theX, theY, x2, y2 );

				int yDiff = theY - y2;
				int xDiff = theX - x2;

				if ( yDiff == 0 || xDiff == 0 )
					distance--;

				if ( distance < closestDist ) {
					closestDist = distance;
					*pointX = theX;
					*pointY = theY;
				}
			}
		}
	}
}

int CombatGroup::positionCharacter ( WorldObject *obj, int x, int y )
{
	int retVal = -1;

	if ( x >= _COMBAT_GRID_WIDTH || x < 0 || y >= _COMBAT_GRID_HEIGHT || y < 0 ) {
		logInfo( _LOG_ALWAYS, "%s:%d CombatGroup::positionCharacter - requested position (%d,%d) is out of bounds. Character: %s of %s at (%d,%d)", __FILE__, __LINE__, x, y, obj->getName(), obj->classID, obj->combatX, obj->combatY );
		return -1;
	}

	if ( obj->combatX >= _COMBAT_GRID_WIDTH || obj->combatX < 0 || obj->combatY >= _COMBAT_GRID_HEIGHT || obj->combatY < 0 ) {
		logInfo( _LOG_ALWAYS, "%s:%d CombatGroup::positionCharacter - character position (%d,%d) is out of bounds. Character: %s of %s. Requested position (%d,%d)", __FILE__, __LINE__, obj->combatX, obj->combatY, obj->getName(), obj->classID, x, y );
		return -1;
	}

	if( grid[obj->combatX][obj->combatY] != obj->servID ) {
		logInfo( _LOG_ALWAYS, "%s:%d CombatGroup::positionCharacter - Character %s of %s servID(%d) is not at their XY(%d, %d), servID at point is %d", __FILE__, __LINE__, obj->getName(), obj->classID, obj->servID, obj->combatX, obj->combatY, grid[obj->combatX][obj->combatY] );
		return -1;
	}

	if ( !grid[x][y] && grid[obj->combatX][obj->combatY] == obj->servID ) {
		grid[obj->combatX][obj->combatY] = 0;

		obj->combatX = x;
		obj->combatY = y;

		grid[obj->combatX][obj->combatY] = obj->servID;

		retVal = 0;
	}

	return retVal;
}

void CombatGroup::processActions ( void )
{
	gCombatActionLogger.reset();

	if ( round >= 5000 ) {
		logInfo ( _LOG_ALWAYS, "combat has taken way too long:" );


		int index = 0;

		while ( combatNames[index] && index < _MAX_COMBATANT ) {
			logInfo ( _LOG_ALWAYS, "combatant = %s", combatNames[index] );
			index++;
		}

		return;
	}

	if ( !cloud )
		return;
	
	if( strcmp( cloud->classID, "CombatCloud" ) ) {
		logInfo( _LOG_ALWAYS, "Combat Cloud object is not of class 'CombatCloud', it's of %s", cloud->classID );
		return;
	}
	
	PackedMsg response;
	response.putLong ( cloud->servID );
	response.putLong ( cloud->getRoom()->number );

	PackedData *packet = &response;

	// calculate the initiative order
	LinkedList turnOrder;
	LinkedElement *element = combatants.head();

	while ( element ) {
		WorldObject *object = (WorldObject*)element->ptr();
		element = element->next();

		if ( !object->player ) {
			logInfo ( _LOG_ALWAYS, "Non-player is a combatant. it is of %s, room %d (12 is kludge that means no room)", object->classID, object->room?object->room->number:12 );
			continue;
		}

		object->initiative = ((object->dexterity + random ( 0, 5 )) * (100 - object->encumberance())) / 100;

		CombatAction *action = object->player->combatAction;

		if ( action ) {

			action->setup();
			object->initiative += action->initiative;

			// handle encumberance
			int enc = object->encumberance();

			if ( enc > 95 )
				action->roundDelay += 3;

			else if ( enc > 75 ) 
				action->roundDelay += 2;
				
			else if ( enc >= 45 ) 
				action->roundDelay += 1;

			// factor in magic holds
			if ( object->hasAffect ( _AFF_BERSERK ) ) {
				object->initiative = 0;
			}
		}

		object->combatRound++;
		object->combatReady = FALSE;
		object->numAttacks = 0;
		object->numDodges = 0;
		object->numBlocks = 0;
		object->numMoves = 0;
	}

	element = combatants.head();

	while ( element ) {
		WorldObject *object = (WorldObject *)element->ptr();
		element = element->next();

		LinkedElement *elementA = turnOrder.head();

		while ( elementA ) {
			WorldObject *obj = (WorldObject *)elementA->ptr();

			if ( object->initiative > obj->initiative ) {
				turnOrder.addBefore ( obj, object );
				break;
			}

			elementA = elementA->next();
		}

		if ( !elementA )
			turnOrder.add ( object );
	}

	LinkedList confusionList, berserkList, stunList, fearList;

	while ( TRUE ) {
		element = turnOrder.head();
		int count = 0;

		while ( element ) {
			WorldObject *obj = (WorldObject *)element->ptr();
			element = element->next();

			// skip an object if dead
			if ( obj->health < 1 )
				continue;

			// check to make sure this object is in combat with us
			if ( obj->combatGroup != this ) {
				continue;
			}

			RMPlayer *player = obj->player;

			// make sure it's a player!
			if ( !player )
				continue;

			CombatAction *action = player->combatAction;

			// handle berserking
			if ( obj->hasAffect ( _AFF_BERSERK ) && !berserkList.contains ( obj ) ) {
				player->setAction ( new CombatBerserk ( obj ) );

				if ( player->nextAction ) {
					delete player->nextAction;
					player->nextAction = NULL;
				}
				
				action = player->combatAction;
				berserkList.add ( obj );
			}

			// handle fear
			else if ( obj->hasAffect ( _AFF_FEAR ) && !fearList.contains ( obj ) ) {
				fearList.add ( obj );

				switch ( random ( 1, 3 ) ) {
					case 1: {
						char *sayTable[] = { "%s quivers in fear...", "%s is too afraid to move...", "%s wails in terror...", "%s nearly faints from fright...", "%s panics and does nothing..." };
						player->setAction ( new CombatSay ( obj, sayTable[random ( 0, 4)], player->character->getName() ) );

						if ( player->nextAction ) {
							delete player->nextAction;
							player->nextAction = NULL;
						}

						action = player->combatAction;
					}

					break;

					case 2: {
						player->setAction ( new CombatMove ( obj, random ( 0, _COMBAT_GRID_WIDTH ), random ( 0, _COMBAT_GRID_HEIGHT ), 1 ) );

						if ( player->nextAction ) {
							delete player->nextAction;
							player->nextAction = NULL;
						}

						action = player->combatAction;
					}

					break;

					case 3: {
						player->setAction ( new CombatFlee ( obj ) );

						if ( player->nextAction ) {
							delete player->nextAction;
							player->nextAction = NULL;
						}

						action = player->combatAction;
					}

					break;
				}
			}

			// handle confusion
			else if ( obj->hasAffect ( _AFF_CONFUSION ) && !confusionList.contains ( obj ) ) {
				confusionList.add ( obj );

				if ( random ( 1, 100 ) >= 50 ) {
					switch ( random ( 0, 5 ) ) {
						case 0:
						case 1: {
							char *sayTable[] = { "%s drools profusely...", "%s stares into space...", "%s stammers stupidly...", "%s looks confused and does nothing...", "%s starts to do something but stops..." };
							player->setAction ( new CombatSay ( obj, sayTable[random ( 0, 4)], player->character->getName() ) );

							if ( player->nextAction ) {
								delete player->nextAction;
								player->nextAction = NULL;
							}

							action = player->combatAction;
						}

						break;

						case 2:
						case 3: {
							player->setAction ( new CombatMove ( obj, random ( 0, _COMBAT_GRID_WIDTH ), random ( 0, _COMBAT_GRID_HEIGHT ), 1 ) );

							if ( player->nextAction ) {
								delete player->nextAction;
								player->nextAction = NULL;
							}

							action = player->combatAction;
						}

						break;

						case 4:
						case 5: {
							player->setAction ( new CombatBerserk ( obj ) );

							if ( player->nextAction ) {
								delete player->nextAction;
								player->nextAction = NULL;
							}

							action = player->combatAction;
						}

						break;
					}
				}
			}

			// handle stun
			else if ( obj->hasAffect ( _AFF_STUN ) && !stunList.contains ( obj ) ) {
				stunList.add ( obj );
	
				player->setAction ( new CombatSay ( obj, "%s reels, unable to take any action!", player->character->getName() ) );

				if ( player->nextAction ) {
					delete player->nextAction;
					player->nextAction = NULL;
				}

				action = player->combatAction;
			}

			if ( action && action->roundDelay ) {
				action->roundDelay--;
				count++;
			}

			else if ( action && action->canDoit() ) {
				if ( !obj->hasAffect ( _AFF_HOLD ) ) {
					action->doit ( packet );

					if ( player->nextAction ) {
						player->setAction ( player->nextAction );
						player->nextAction = NULL;
					}

					if ( player->combatAction && player->combatAction->done && player->combatAction->nextAction ) {
						CombatAction *nextAction = player->combatAction->nextAction;
						player->combatAction->nextAction = NULL;

						player->setAction ( nextAction );
					}

					count++;
				}
			}
		}

		if ( !count ) {
			element = turnOrder.head();

			while ( element ) {
				WorldObject *obj = (WorldObject *)element->ptr();
				element = element->next();

				RMPlayer *player = obj->player;

				if ( !player ) 
					continue;

				if ( player->combatAction ) {
					delete player->combatAction;
					player->combatAction = NULL;
				}

				if ( player->nextAction ) {
					delete player->nextAction;
					player->nextAction = NULL;
				}
			}

			break;
		}
	}

	confusionList.release();
	berserkList.release();
	stunList.release();
	fearList.release();

	// process all affected states -- one minute has passed

	affect_t *cloudAffect = cloud->hasAffect ( _AFF_ANTI_MAGIC ) ;

	if ( cloudAffect ) {
		if ( random ( 0, 100 ) < 50 || cloudAffect->duration == 0 ) {
			element = turnOrder.head();
			if ( element ) {
				WorldObject *character = (WorldObject *)element->ptr();
				putMovieText ( character, &response, "|c43|The anti-magic aura has dissipated.\n" );	
			}
			cloud->delAffect ( cloudAffect, NULL );					
		}
		else
			cloudAffect->duration--;
	}

	element = turnOrder.head();

	while ( element ) {
		WorldObject *character = (WorldObject *)element->ptr();
		element = element->next();

		character->processAffects ( &response );

		LinkedList items; 
		character->makeItemList ( &items, -1 );

		LinkedElement *elementA = items.head();

		while ( elementA ) {
			WorldObject *obj = (WorldObject *)elementA->ptr();
			elementA = elementA->next();

			if ( obj->affectElement )
				obj->processAffects ( &response );
		}

		items.release();
	}

	turnOrder.release();

	// make sure they are all pending for the next turn
	element = combatants.head();

	while ( element ) {
		WorldObject *obj = (WorldObject *)element->ptr();
		element = element->next();

		turnPendingList.add ( obj );
	}

	doNPCLogic ( packet );
	nextRound ( packet );

	response.putByte ( _MOVIE_END );

	roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, response.data(), response.size(), cloud->getRoom() );

	round++;
}

void CombatGroup::nextRound ( PackedData *movie )
{
	int badLevel = 0;

	// check to see if the attackers are all dead or not
	LinkedElement *element = attackers.head();

	if ( gainedEP )
		goto end;

	while ( element ) {
		WorldObject *obj = (WorldObject *)element->ptr();

		if ( obj->health > 0 )
			break;

		element = element->next();
	}

	// all attackers must be dead -- let the defenders know that they won
	if ( !element ) {
		gainedEP = 1;

		// add up the defenderLevel now
		defenderLevel = 0;

		element = defenders.head();
		int defenderCount = 0;

		while ( element ) {
			WorldObject *obj = (WorldObject *)element->ptr();
			element = element->next();

			if ( obj->health > 0 && !obj->summoned ) {
				// 
				// calculate the level based on the experience of the defender
				//
				if ( obj->playerControlled ) {
					BCharacter *bchar = (BCharacter *)obj->getBase ( _BCHARACTER );
					int level = bchar->getLevel();
//					int level = (bchar->experience / _EP_PER_LEVEL) + 1;

					if ( level == -1 )
						badLevel = 1;

					defenderLevel += level;
				} else {
					defenderLevel += obj->level;
				}

				defenderCount++;
			}
		}

		movie->putByte ( _MOVIE_WIN_COMBAT_DEFENDERS );
		movie->putLong ( servID() );

		// start another normal round
		movie->putByte ( _MOVIE_ROUND_BEGIN );
		movie->putLong ( servID() );

		// let each remaining defender gain EP for their battle
		element = defenders.head();

		// SNTODO: make this EP calculation switchable
#if 1
		int ep = (int)(1000.0 * ((double)attackerLevel / (double)(defenderLevel? defenderLevel : 1)));
#else
		// calculate the percentage of damage that each side did 
		double defenderDmgPct = (attackerDamage > 0)? ((double)attackerDamage) / ((double)defenderHealth) : 0.001;
		double attackerDmgPct = (defenderDamage > 0)? ((double)defenderDamage) / ((double)attackerHealth) : 0.001;

		int ep = defenderDamage? (int)(2500.0 * (defenderDmgPct / attackerDmgPct)) : 1;
#endif

		// adjust the experience based on extra members!
		defenderCount--;

		if ( defenderCount ) {
			int bonus = 60 + (30 * (defenderCount - 1));
			ep += (ep * bonus) / 100;
		}

		ep = random ( ep * 3 / 4, ep * 5 / 4 );

		if ( badLevel || !attackerLevel || pvp )
			ep = 0;

		while ( element ) {
			WorldObject *obj = (WorldObject *)element->ptr();

			if ( obj->health > 0 && ep > 0 ) {
				BCharacter *bchar = (BCharacter *)obj->getBase ( _BCHARACTER );

				if ( bchar ) {
					int tempExp = ep;
					if ((obj->level <= 10000 || gExpBoost < 1) && (ep > 0))
						tempExp = ep * gExpBoost;
					
					if ( tempExp > _MAX_EXP_GAIN )
						tempExp = _MAX_EXP_GAIN;

					bchar->gainExperience ( tempExp, movie );
				}
			}

			element = element->next();
		}

		return;
	}

	// check to see if the defenders are all dead or not
	element = defenders.head();

	while ( element ) {
		WorldObject *obj = (WorldObject *)element->ptr();

		if ( obj->health > 0 )
			break;

		element = element->next();
	}

	// all defenders are dead -- let the attackers know that they won
	if ( !element ) {
		gainedEP = 1;

		// add up the attackerLevel now
		attackerLevel = 0;

		element = attackers.head();
		int attackerCount = 0;
		badLevel = 0;
		
		while ( element ) {
			WorldObject *obj = (WorldObject *)element->ptr();
			element = element->next();

			if ( obj->health > 0 && !obj->summoned ) {
				// 
				// calculate the level based on the experience of the defender
				//
				if ( obj->playerControlled ) {
					BCharacter *bchar = (BCharacter *)obj->getBase ( _BCHARACTER );
					int level = bchar->getLevel();
//					int level = (bchar->experience / _EP_PER_LEVEL) + 1;

					if ( level == -1 )
						badLevel = 1;

					attackerLevel += level;

				} else {
					attackerLevel += obj->level;
				}

				attackerCount++;
			}
		}

		movie->putByte ( _MOVIE_WIN_COMBAT_ATTACKERS );
		movie->putLong ( servID() );

		// start another normal round
		movie->putByte ( _MOVIE_ROUND_BEGIN );
		movie->putLong ( servID() );

		// let each of the surviving attackers gain EP for the battle
		element = attackers.head();

		// SNTODO: make this switchable...
#if 1
		int ep = (int)(1000.0 * ((double)defenderLevel / (double)(attackerLevel? attackerLevel : 1)));
#else
		// calculate the percentage of damage that each side did 
		double defenderDmgPct = (attackerDamage > 0)? ((double)attackerDamage) / ((double)defenderHealth) : 0.001;
		double attackerDmgPct = (defenderDamage > 0)? ((double)defenderDamage) / ((double)attackerHealth) : 0.001;

		int ep = attackerDamage? (int)(2500.0 * (attackerDmgPct / defenderDmgPct)) : 1;
#endif

		// adjust the experience based on extra members!
		attackerCount--;

		if ( attackerCount ) {
			int bonus = 60 + (30 * (attackerCount - 1));
			ep += (ep * bonus) / 100;
		}

		ep = random ( ep * 3 / 4, ep * 5 / 4 );
//		ep = max ( ep, 1 );

		if ( badLevel || !defenderLevel || pvp )
			ep = 0;

		while ( element ) {
			WorldObject *obj = (WorldObject *)element->ptr();

			if ( obj->health > 0 && ep > 0 ) {
				BCharacter *bchar = (BCharacter *)obj->getBase ( _BCHARACTER );

				if ( bchar ){
					int tempExp = ep;
					if ((obj->level <= 10000 || gExpBoost < 1) && (ep > 0))
						tempExp = ep * gExpBoost;
					
					if ( tempExp > _MAX_EXP_GAIN )
						tempExp = _MAX_EXP_GAIN;

					bchar->gainExperience ( tempExp, movie );
				}
			}

			element = element->next();
		}

		return;
	}

end:

	element = combatants.head();

	while ( element ) {
		WorldObject *obj = (WorldObject *)element->ptr();
		element = element->next();

		obj->combatReady = FALSE;
	}

	// start another normal round
	movie->putByte ( _MOVIE_ROUND_BEGIN );
	movie->putLong ( servID() );
}

void CombatGroup::doNPCLogic ( PackedData *movie )
{
	LinkedElement *element = combatants.head();

	while ( element ) {
		WorldObject *obj = (WorldObject *)element->ptr();
		element = element->next();

		if ( !obj->player || obj->destructing || obj->playerControlled ) 
			continue;

		if ( obj->health < 1 || !obj->opposition || !obj->opposition->size() ) {
			obj->player->exitCombat ( movie );
			continue;
		}
		((NPC *)obj->player)->combatLogic();
	}
}

int calcStealChance ( WorldObject *attacker, WorldObject *defender )
{
	int attackerSkillMod[_SKILL_LVL_MAX] = {0, 10, 20, 30, 40, 50};
	int defenderSkillMod[_SKILL_LVL_MAX] = {10, 5, 0, -5, -10, -20};

	int attackerSkill = attacker->getSkill ( _SKILL_PICK_POCKETS );
	int defenderSkill = defender->getSkill ( _SKILL_PICK_POCKETS );

	int chance = (attackerSkillMod[attackerSkill] + attacker->calcDexterity() + attacker->calcIntelligence()) + (defenderSkillMod[defenderSkill] - defender->calcDexterity() - defender->calcIntelligence());

	if ( attacker->player && defender->player ) {
		if ( attacker->player->isGroupMember ( defender->player ) )
			chance += 33;
	}

	return chance;
}

// 
// CombatAction
//

CombatAction::CombatAction ( WorldObject *obj )
{
	client = obj;
	type = -1;
	done = FALSE;
	allowRetaliate = TRUE;
	initiative = 0;
	roundDelay = 0;
	nextAction = NULL;
}

CombatAction::CombatAction( const CombatAction& action )
{
	nextAction = NULL;
	allowRetaliate = action.allowRetaliate;
	client = action.client;
	done = action.done;
	initiative = action.initiative;
	roundDelay = action.roundDelay;
	type = action.type;
}

CombatAction::~CombatAction()
{
	if ( nextAction ) {
		delete nextAction;
		nextAction = NULL;
	}
}

int CombatAction::doit ( PackedData *movie )
{
	return TRUE;
}

void CombatAction::setup ( void )
{
}

int CombatAction::canDoit ( void )
{
	return FALSE;
}

// 
// CombatMove
//

CombatMove::CombatMove ( WorldObject *obj, int destX, int destY, int dist )
{
	client = obj;
	x = destX;
	y = destY;
	distance = dist;
	type = _CA_MOVE;
	allowRetaliate = TRUE;
	movePerRound = 3;
}

CombatMove::CombatMove( const CombatMove& action ) : CombatAction( action )
{
	x = action.x;
	y = action.y;
	distance = action.distance;
	movePerRound = action.movePerRound;
}

CombatMove::~CombatMove()
{
}

int CombatMove::doit ( PackedData *movie )
{
	int retVal = FALSE;

	int clientX = client->combatX;
	int clientY = client->combatY;
	
	int curX = clientX;
	int curY = clientY;

	int movementLeft = client->calcNumMoves() - client->numMoves;
	int rate = std::min( movePerRound, movementLeft);
//movePerRound <? movementLeft;

	client->numMoves += rate;

	while ( rate && (curX != x || curY != y) ) {
		int dist = getDistance ( curX, curY, x, y );

		if ( dist <= distance ) {
			retVal = TRUE;
			break;
		}

		client->combatGroup->findClosestPoint( curX, curY, x, y, &curX, &curY );

		rate--;
	}

	if ( client->numMoves >= client->calcNumMoves() )
		retVal = TRUE;

	if ( (curX != clientX) || (curY != clientY) ) {
		//if we aren't where we started, we have moved, lets try to commit the move

		if( client->combatGroup->positionCharacter ( client, curX, curY ) == -1 ) {
			//move failed... try to recover
			client->combatGroup->findClosestPoint( curX, curY, clientX, clientY, &curX, &curY );

			if( client->combatGroup->positionCharacter ( client, curX, curY ) == -1 ) {
				//move *really* failed.
				logInfo( _LOG_ALWAYS, "%s:%d CombatMove::doit error - move to (%d,%d) failed. Character: %s of %s at (%d,%d)", __FILE__, __LINE__, curX, curY, client->getName(), client->classID, client->combatX, client->combatY );
			} else {
				//we were able to recover
				movie->putByte ( _MOVIE_COMBAT_MOVE );
				movie->putLong ( client->servID );
				movie->putByte ( client->combatX );
				movie->putByte ( client->combatY );
				gCombatActionLogger.logAction( this );
			}
		} else {
			//move success
			movie->putByte ( _MOVIE_COMBAT_MOVE );
			movie->putLong ( client->servID );
			movie->putByte ( client->combatX );
			movie->putByte ( client->combatY );
			gCombatActionLogger.logAction( this );
		}
		
	}

	return retVal;
}

int CombatMove::canDoit ( void ) 
{
	if ( client->canMove() )
		return TRUE;

	return FALSE;
}

//
// CombatCharge
//

CombatCharge::CombatCharge ( WorldObject *obj, WorldObject *target, int dist ) : CombatMove ( obj, 0, 0, obj->combatRange() )
{
	type = _CA_CHASE;
	whoToChase = target;
	targetServID = target? target->servID : -1;
	allowRetaliate = FALSE;
	movePerRound = 6;
}

CombatCharge::~CombatCharge()
{
}

CombatCharge::CombatCharge( const CombatCharge& action ) : CombatMove( action )
{
	whoToChase = action.whoToChase;
	targetServID = action.targetServID;
}

void CombatCharge::setup ( void )
{
	movePerRound = 6;
}

int CombatCharge::canDoit ( void )
{
	whoToChase = roomMgr->findObject ( targetServID );

	// make sure this is a character!

	if ( whoToChase && whoToChase->player && 
		(client->health > 0) && ( whoToChase->health > 0) && 
		client->canMove() && ( client->opposition->contains( whoToChase ) || 
		client->hasAffect ( _AFF_BERSERK ) ) )
		return 1;

	return 0;
}

int CombatCharge::doit ( PackedData *movie )
{
	x = whoToChase->combatX;
	y = whoToChase->combatY;

	if ( client->canHit ( whoToChase ) ) {
		CombatAttack *attack = new CombatAttack ( client, whoToChase );
		attack->doit ( movie );
		delete attack;

		RMPlayer *player = client->player;
		player->nextAction = new CombatAttack ( client, whoToChase );
		return TRUE;
	}

	gCombatActionLogger.logAction( this );
	return CombatMove::doit ( movie );
}

//
// CombatChase
//

CombatChase::CombatChase ( WorldObject *obj, WorldObject *target, int dist ) : CombatMove ( obj, 0, 0, dist )
{
	type = _CA_CHASE;
	whoToChase = target;
	targetServID = target->servID;
	allowRetaliate = TRUE;
}

CombatChase::CombatChase( const CombatChase& action ) : CombatMove( action )
{
	whoToChase = action.whoToChase;
	targetServID = action.targetServID;
}

CombatChase::~CombatChase()
{
}

int CombatChase::canDoit ( void )
{
	whoToChase = roomMgr->findObject ( targetServID );

	if ( whoToChase && whoToChase->player && (client->health > 0) && (whoToChase->health > 0) && client->canMove() )
		return 1;

	return 0;
}

int CombatChase::doit ( PackedData *movie )
{
	x = whoToChase->combatX;
	y = whoToChase->combatY;

	if ( client->canHit ( whoToChase ) ) {
		gCombatActionLogger.logAction( this );
		CombatAttack *attack = new CombatAttack ( client, whoToChase );
		attack->doit ( movie );
		delete attack;

		RMPlayer *player = client->player;
		player->nextAction = new CombatAttack ( client, whoToChase );
		return TRUE;
	}

	return CombatMove::doit ( movie );
}

//
// CombatGuard
//

CombatGuard::CombatGuard ( WorldObject *obj, WorldObject *target )
{
	type = _CA_GUARD;
	client = obj;
	whoToGuard = target;
	targetServID = target? target->servID : -1;
	allowRetaliate = TRUE;
}

CombatGuard::CombatGuard( const CombatGuard& action ) : CombatAction( action )
{
	whoToGuard = action.whoToGuard;
	targetServID = action.targetServID;
}

CombatGuard::~CombatGuard()
{
}

void CombatGuard::setup ( void )
{
	// use up combat movement slots so that guarding player is standing still 
	client->numMoves = client->calcNumMoves();
}

int CombatGuard::doit ( PackedData *movie )
{
	RMPlayer *player = client->player;
	whoToGuard = roomMgr->findObject ( targetServID );

	// look for any characters that are close enough to attack
	LinkedElement *element = client->opposition->head();
	WorldObject *whoToAttack = NULL;

	if ( whoToGuard && client->canHit ( whoToGuard ) ) {
		if ( client->canMove() ) {
			player->nextAction = new CombatAttack ( client, whoToGuard );
			player->nextAction->type = type;
			return TRUE;
		}

		whoToAttack = whoToGuard;
	} else {
		while ( element ) {
			WorldObject *obj = (WorldObject *)element->ptr();
			element = element->next();

			if ( client->canHit ( obj ) ) {
				if ( client->canMove() ) {
					player->nextAction = new CombatAttack ( client, obj );
					player->nextAction->type = type;
					return TRUE;
				}

				whoToAttack = obj;
				break;
			}
		}
	}

	  int retaliate = whoToAttack->player->combatAction? whoToAttack->player->combatAction->allowRetaliate : 0;

    client->delAffect ( _AFF_INVISIBILITY, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, movie );

    client->player->attack ( whoToAttack, movie, retaliate, retaliate, retaliate );

     if(client->curWeapon->getBase((_BWEAPON)))
     {
        BWeapon *bWeapon = (BWeapon *)client->curWeapon->getBase ( _BWEAPON );

        if(bWeapon->spellProcID != 0)
        {
           if(rand() % 100 < bWeapon->forwardProcChance)
            {
                int procID = bWeapon->spellProcID;
                    if(procID == -1)
                        procID = getRandomForwardProc();
                
                spell_info *spell = &gSpellTable[procID];

                if(spell)
                    player->castProc(spell, player->character, whoToAttack->servID, whoToAttack->combatX, whoToAttack->combatY, movie);
            }
        }

    	if(bWeapon->reverseProcID != 0)
        {
            if(rand() % 100 < bWeapon->reverseProcChance)
            {
                int procID = bWeapon->spellProcID;
                    if(procID == -1)
                        procID = getRandomForwardProc();
                        
                spell_info *spell = &gSpellTable[procID];

                if(spell)
                    player->castProc(spell, player->character, player->character->servID, player->character->combatX, player->character->combatY, movie);
            }

        }
    }
    return 0;
}

int CombatGuard::canDoit ( void )
{
	// if dead, can't do it
	if ( client->health <= 0 )
		return FALSE;

	// if no attacks are left, don't let guard happen
	if ( client->numAttacks >= client->calcNumAttacks() )
		return FALSE;
		
	// can I hit anyone this round?
	LinkedElement *element = client->opposition->head();

	while ( element ) {
		WorldObject *obj = (WorldObject *)element->ptr();
		element = element->next();

		if ( client->canHit ( obj ) )
			return TRUE;
	}

	return FALSE;
}

// 
// CombatFlee
//

CombatFlee::CombatFlee ( WorldObject *obj )
{
	type = _CA_FLEE;
	client = obj;
	allowRetaliate = FALSE;
}

CombatFlee::CombatFlee( const CombatFlee& action ) : CombatAction( action )
{
}

CombatFlee::~CombatFlee()
{
}

int CombatFlee::canDoit ( void )
{
	if ( client->health < 1 )
		return FALSE;

	return !done;
}

int CombatFlee::doit ( PackedData *movie ) {
	LinkedElement *element;
	BCharacter *bchar = (BCharacter *)client->getBase ( _BCHARACTER );
	CombatGroup *pGroup = client->combatGroup;

	// can we flee or not?
	int bCanFlee = 1;

	// is this a dungeon fight
	int bDungeonFight = 0;

	if ( client->room && client->room->zone && client->room->zone->isDungeon )
		bDungeonFight = 1;

	// set the default flee chance variables...
	int nMaxFleeChance = 100, nMinFleeChance = 30, nFleeChanceDecrement = 10;

	if ( bDungeonFight ) {
		nMaxFleeChance = 5;
		nMinFleeChance = 5;
	}

	// set flee chance for ambushes
	if ( pGroup->ambushAttack ) {
		nMaxFleeChance = 70;
		nMinFleeChance = 10;

		if ( bDungeonFight ) {
			nMinFleeChance = 5;
			nMaxFleeChance = 5;
		}
	}

	// set flee chance for pvp
	else if ( pGroup->pvp ) {
		nMaxFleeChance = 80;
		nMinFleeChance = 20;
		nFleeChanceDecrement = 5;
	}

	// calculate the chance to flee...
	int chance = nMaxFleeChance - ((client->combatRound - 2) * nFleeChanceDecrement); 

	if ( chance < nMinFleeChance )
		chance = nMinFleeChance;

	// handle mind shackle...
	if ( client->hasAffect ( _AFF_SHACKLED ) ) {
		chance = nMinFleeChance;
	}

	// SNTODO: remove this kludge...
	if ( client->player->isNPC && (client->level == 1) ) {
		chance = 100;
	}

	// check for flee chance...
	if ( random ( 1, 100 ) > chance ) {
		bCanFlee = 0;
	}

	if ( !bCanFlee ) {
		putMovieText ( client, movie, "|c60|%s tried to flee but can't! (%d%% chance)|c43| ", client->getName(), chance );
		done = TRUE;
		return TRUE;
	} else {
		putMovieText ( client, movie, "|c60|%s fled like a scared dog!|c43| ", client->getName(), chance );

		if ( pGroup->attackers.contains ( client ) && pGroup->attackers.size() == 1 ) {
			element = pGroup->defenders.head();
			
			while ( element ) {
				WorldObject* character = (WorldObject *)element->ptr();
				element = element->next();

				if ( character->player->isNPC ) {
					if ( character->health < ( character->healthMax * 0.75 ) ) {
						character->changeHealth( character->healthMax - character->health, NULL );
					}
					
					if ( character->hasAffect ( _AFF_POISONED ) ) {
						character->clearAffect ( _AFF_POISONED, _AFF_TYPE_NORMAL );
					}
				}
			}
		}
	}

	if ( bchar && !client->combatGroup->pvp && (!client->combatGroup->freeFlee || (client->combatGroup->round > 1) || client->combatGroup->attackers.contains ( client )) ) 
		bchar->gainExperience ( -500, movie );

	client->player->exitCombat ( movie );

	gCombatActionLogger.logAction( this );
	done = TRUE;
	return TRUE;
}

// 
// CombatEquip
//

CombatEquip::CombatEquip ( WorldObject *obj, WorldObject *target )
{
	client = obj;
	targetServID = target? target->servID : -1;
	type = _CA_EQUIP;
	allowRetaliate = FALSE;
//	allowRetaliate = TRUE;
	item = NULL;
}

CombatEquip::CombatEquip( const CombatEquip& action ) : CombatAction( action )
{
	item = action.item;
	targetServID = action.targetServID;
}

CombatEquip::~CombatEquip()
{
}

int CombatEquip::canDoit ( void )
{
	if ( client->health < 1 )
		return FALSE;

	item = roomMgr->findObject ( targetServID );

	if ( !done && item && !item->destructing ) {

		BWearable *bwear = (BWearable *)item->getBase ( _BWEAR );
		BWeapon *bweapon = (BWeapon *)item->getBase ( _BWEAPON );
		
		if ( ( bwear && !item->getBase ( _BHEAD ) ) || bweapon ) {

		// must be owned object
			WorldObject *owningObj = item->getBaseOwner();

//			if ( !done && ( owningObj && owningObj == client ) ) 
			if ( owningObj && ( owningObj == client ) )
				return TRUE;				
		}
	}

	return FALSE;
}

void CombatEquip::setup ( void )
{
}

int CombatEquip::doit ( PackedData *movie )
{
	done = TRUE;

	item = roomMgr->findObject ( targetServID );

	if ( item && !item->destructing ) {

		BWearable *bwear = (BWearable *)item->getBase ( _BWEAR );
		BWeapon *bweapon = (BWeapon *)item->getBase ( _BWEAPON );
		
		if ( bwear && !item->getBase ( _BHEAD ) ) {
			// check for an item in the same area worn as this one
			WorldObject *oldItem = client->getWornOn ( bwear->areaWorn );
		
			if ( oldItem ) {
				int nResult = client->takeOff ( oldItem, movie );

				if ( (nResult != _WO_ACTION_HANDLED) || (oldItem == item) )
					return TRUE;
			}
		}
		
		else if ( bweapon ) {
			if ( client->curWeapon ) {
				WorldObject *oldItem = client->curWeapon;

				int nResult = client->takeOff ( oldItem, movie );

				if ( (nResult != _WO_ACTION_HANDLED) || (oldItem == item) )
					return TRUE;
			}
		
			// check for a shield and take it off if we need to
			if ( bweapon->hands == 2 ) {
				WorldObject *shield = client->getWornOn ( _WEAR_SHIELD );
	
				if ( shield ) {
					if ( client->takeOff ( shield, movie ) != _WO_ACTION_HANDLED )
						return TRUE;
				}
			}
		}

		int success = client->putOn ( item, movie );

		if ( success != _WO_ACTION_HANDLED ) {
			movie->putByte ( _MOVIE_INFO );	
			movie->putLong ( client->servID );

			char buf[1024];
			sprintf ( sizeof ( buf ), buf, "Can not equip %s!", item->getName() );
			movie->putString ( buf );
		}
		else
			gCombatActionLogger.logAction( this );

		if ( bweapon && (success == _WO_ACTION_HANDLED) )
			client->curWeapon = item;
	}

	return TRUE;
}

//
// CombatAttack
//

CombatAttack::CombatAttack ( WorldObject *obj, WorldObject *target )
{
	client = obj;
	targetServID = target? target->servID : -1;
	type = _CA_ATTACK;
	allowRetaliate = TRUE;
}

CombatAttack::CombatAttack ( const CombatAttack& action ) : CombatAction( action )
{
	whoToAttack = action.whoToAttack;
	targetServID = action.targetServID;
}

CombatAttack::~CombatAttack()
{
}

int CombatAttack::canDoit ( void )
{
	RMPlayer *player = client->player;
	whoToAttack = roomMgr->findObject ( targetServID );

	// if no whoToAttack or all attacks are used up, don't do anything
	if ( !whoToAttack || !whoToAttack->player || !client->canAttack() )
		return FALSE;

	int inOpposition = client->opposition->contains ( whoToAttack );
	int mystOverride = (client->hasAffect ( _AFF_BERSERK ) || client->hasAffect ( _AFF_CONFUSION ) || client->hasAffect ( _AFF_LOYALTY_SHIFT )) ? 1 : 0;

	// you can't attack friends without mystOverride...
	if ( !inOpposition && !mystOverride )
		return FALSE;

	// if we can move or attack, let it go
	if ( client->canMove() || client->canHit ( whoToAttack ) )
		return TRUE;

	return FALSE;
}


int CombatAttack::doit ( PackedData *movie )
{
	RMPlayer *player = client->player;
	whoToAttack = roomMgr->findObject ( targetServID );

	// if my target is dead, change to guarding
	if ( !whoToAttack || !whoToAttack->player || whoToAttack->health < 1 ) {
		player->nextAction = new CombatGuard ( client );
		player->nextAction->type = type;
		return TRUE;
	}

	// if I have no movement points left, guard with my target as priority
	if ( !client->canHit ( whoToAttack ) && !client->canMove() ) {
		player->nextAction = new CombatGuard ( client, whoToAttack );
		player->nextAction->type = type;
		return TRUE;
	}

	// if I can't hit my target from here, move towards it
	if ( !client->canHit ( whoToAttack ) ) {
		CombatMove *move = new CombatMove ( client, whoToAttack->combatX, whoToAttack->combatY, client->combatRange() );
		move->doit ( movie );
		delete move;

		// if I still can't reach my target, try next phase 
		if ( !client->canHit ( whoToAttack ) ) 
			return TRUE;
	}

	int retaliate = whoToAttack->player->combatAction? whoToAttack->player->combatAction->allowRetaliate : 0;

	client->delAffect ( _AFF_INVISIBILITY, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, movie );

	// I can hit my target, let him have it
	player->attack ( whoToAttack, movie, retaliate, retaliate, retaliate );


     if(client->curWeapon->getBase((_BWEAPON)))
     {
        BWeapon *bWeapon = (BWeapon *)client->curWeapon->getBase ( _BWEAPON );

        if(bWeapon->spellProcID != 0)
        {
           if(rand() % 100 < bWeapon->forwardProcChance)
            {
                int procID = bWeapon->spellProcID;
                    if(procID == -1)
                        procID = getRandomForwardProc();
                
                spell_info *spell = &gSpellTable[procID];

                if(spell)
                    player->castProc(spell, player->character, whoToAttack->servID, whoToAttack->combatX, whoToAttack->combatY, movie);
            }
        }

    	if(bWeapon->reverseProcID != 0)
        {
            if(rand() % 100 < bWeapon->reverseProcChance)
            {
                int procID = bWeapon->spellProcID;
                    if(procID == -1)
                        procID = getRandomForwardProc();
                        
                spell_info *spell = &gSpellTable[procID];

                if(spell)
                    player->castProc(spell, player->character, player->character->servID, player->character->combatX, player->character->combatY, movie);
            }

        }
    }
	 
	 

	gCombatActionLogger.logAction( this );

	// if I killed my target, finish up
	if ( whoToAttack->health < 1 ) {
		// if I still have attacks that I can make, guard
		if ( client->canAttack() ) {
			player->nextAction = new CombatGuard ( client );
			player->nextAction->type = type;
		}

		return TRUE;
	}

	return TRUE;
}

void CombatAttack::setup ( void ) 
{
}

int CombatAttack::calcChance(int baseStat) // <- add this
{
    return random(baseStat, 100);
}

// 
// CombatSay
//

CombatSay::CombatSay ( WorldObject *obj, const char *format, ... )
{
	char output[1024];

	va_list args;
	va_start ( args, format );
	vsprintf ( sizeof ( output ), output, format, args );

	text = strdup ( output );

	client = obj;
	type = _CA_SAY;
	allowRetaliate = TRUE;
}

CombatSay::CombatSay ( const CombatSay& action ) : CombatAction( action )
{
	text = strdup( action.text );
}

CombatSay::~CombatSay()
{
	if ( text ) {
		free ( text );
		text = NULL;
	}
}

int CombatSay::canDoit ( void )
{
	return !done;
}

int CombatSay::doit ( PackedData *packet )
{
	packet->putByte ( _MOVIE_TEXT );
	packet->putLong ( client->servID );
	packet->putString ( text );

	done = 1;
	
	gCombatActionLogger.logAction( this );

	return TRUE;
}

// 
// CombatBerserk
//

CombatBerserk::CombatBerserk ( WorldObject *obj ) : CombatAttack ( obj, NULL )
{
	client = obj;

	// find the closest person
	WorldObject *closest = NULL;

	LinkedElement *element = obj->combatGroup->combatants.head();
	int size = obj->combatGroup->combatants.size();
	int stopAt = random ( 1, size );

	if ( size > 1 ) {
		while ( stopAt > 0 ) {
			WorldObject *target = (WorldObject *)element->ptr();
			element = element->next();

			if ( !element )
				element = obj->combatGroup->combatants.head();

			if ( target == obj )
				continue;

			closest = target;
			stopAt--;
		}
	}

	targetServID = closest? closest->servID : -1;
	lastAction = obj->player->combatAction;
	obj->player->combatAction = NULL;

	type = _CA_BERSERK;
	allowRetaliate = TRUE;
}

CombatBerserk::CombatBerserk( const CombatBerserk& action ) : CombatAttack( action )
{
	lastAction = NULL;
}

CombatBerserk::~CombatBerserk()
{
	if ( lastAction ) {
		delete lastAction;
		lastAction = NULL;
	}
}

int CombatBerserk::canDoit ( void )
{
	if ( !client->hasAffect ( _AFF_BERSERK ) ) {
		client->player->nextAction = lastAction;
		lastAction = NULL;
		return 0;
	}

	return CombatAttack::canDoit();
}

//
// CombatCastSpell
//

CombatCastSpell::CombatCastSpell ( WorldObject *obj, int spellID, int servID, int combatX, int combatY )
{
	spell = spellID;
	targetServID = servID;
	casterServID = obj->servID;
	targetX = combatX;
	targetY = combatY;
	client = obj;
	type = _CA_CAST_SPELL;
	didCast = FALSE;
	allowRetaliate = FALSE;

	spell_info *spellPtr = &gSpellTable[spell];
	initiative = -spellPtr->dexMod;

	switch ( spellPtr->dexMod ) {
		case _SPELL_MEDIUM: {
			roundDelay = 1;
		}

		break;

		case _SPELL_SLOW: {
			roundDelay = 2;
		}

		break;

		case _SPELL_SLOWEST: {
			roundDelay = 3;
		}

		break;
	}
}

CombatCastSpell::CombatCastSpell ( WorldObject *obj, int spellID, int casterServID, int servID, int combatX, int combatY )
{
	spell = spellID;
	targetServID = servID;
	this->casterServID = casterServID;
	targetX = combatX;
	targetY = combatY;
	client = obj;
	type = _CA_CAST_SPELL;
	didCast = FALSE;
	allowRetaliate = FALSE;

	spell_info *spellPtr = &gSpellTable[spell];
	initiative = -spellPtr->dexMod;

	switch ( spellPtr->dexMod ) {
		case _SPELL_MEDIUM: {
			roundDelay = 1;
		}

		break;

		case _SPELL_SLOW: {
			roundDelay = 2;
		}

		break;

		case _SPELL_SLOWEST: {
			roundDelay = 3;
		}

		break;
	}
}

CombatCastSpell::CombatCastSpell( const CombatCastSpell& action ) : CombatAction( action )
{
	spell = action.spell;
	targetServID = action.targetServID;
	casterServID = action.casterServID;
	targetX = action.targetX;
	targetY = action.targetY;
	didCast = action.didCast;
}

CombatCastSpell::~CombatCastSpell()
{
}

int CombatCastSpell::canDoit ( void )
{
	if ( client->health < 1 )
		return FALSE;

	WorldObject *targetObj = roomMgr->findObject ( targetServID );

	if ( !targetObj || (targetObj->health <= 0) )
		return FALSE;

	if ( didCast )
		allowRetaliate = TRUE;

	return !didCast;
}

int CombatCastSpell::doit ( PackedData *packet )
{
	spell_info *spell = &gSpellTable[this->spell];

	WorldObject *caster = roomMgr->findObject ( casterServID );

	client->delAffect ( _AFF_INVISIBILITY, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, packet );

	packet->putByte ( _MOVIE_CAST_BEGIN );
	packet->putLong ( client->servID );

	WorldObject *targetObj = roomMgr->findObject ( targetServID );

	if ( caster ) {
		if ( !targetObj || ( targetObj && targetObj->getBaseOwner()->combatGroup == client->combatGroup ) ) {
			client->player->castSpell ( spell, caster, targetServID, targetX, targetY, packet );
		}
	}

	packet->putByte ( _MOVIE_CAST_END );
	packet->putLong ( client->servID );

	didCast = TRUE;
	gCombatActionLogger.logAction( this );
	return TRUE;
}

//
// CombatProcSpell
//

CombatProcSpell::CombatProcSpell ( WorldObject *obj, int spellID, int servID, int combatX, int combatY )
{
    spell = spellID;
    targetServID = servID;
    casterServID = obj->servID;
    targetX = combatX;
    targetY = combatY;
    client = obj;
    type = _CA_CAST_SPELL;
    didCast = FALSE;
    allowRetaliate = FALSE;

    spell_info *spellPtr = &gSpellTable[spell];
    initiative = -spellPtr->dexMod;

    switch ( spellPtr->dexMod ) {
        case _SPELL_MEDIUM: {
            roundDelay = 1;
        }

        break;

        case _SPELL_SLOW: {
            roundDelay = 2;
        }

        break;

        case _SPELL_SLOWEST: {
            roundDelay = 3;
        }

        break;
    }
}

CombatProcSpell::CombatProcSpell ( WorldObject *obj, int spellID, int casterServID, int servID, int combatX, int combatY )
{
    spell = spellID;
    targetServID = servID;
    this->casterServID = casterServID;
    targetX = combatX;
    targetY = combatY;
    client = obj;
    type = _CA_CAST_SPELL;
    didCast = FALSE;
    allowRetaliate = FALSE;

    spell_info *spellPtr = &gSpellTable[spell];
    initiative = -spellPtr->dexMod;

    switch ( spellPtr->dexMod ) {
        case _SPELL_MEDIUM: {
            roundDelay = 1;
        }

        break;

        case _SPELL_SLOW: {
            roundDelay = 2;
        }

        break;

        case _SPELL_SLOWEST: {
            roundDelay = 3;
        }

        break;
    }
}

CombatProcSpell::CombatProcSpell( const CombatCastSpell& action ) : CombatAction( action )
{
    spell = action.spell;
    targetServID = action.targetServID;
    casterServID = action.casterServID;
    targetX = action.targetX;
    targetY = action.targetY;
    didCast = action.didCast;
}

CombatProcSpell::~CombatProcSpell()
{
}

int CombatProcSpell::canDoit ( void )
{
    if ( client->health < 1 )
        return FALSE;

    WorldObject *targetObj = roomMgr->findObject ( targetServID );

    if ( !targetObj || (targetObj->health <= 0) )
        return FALSE;

    if ( didCast )
        allowRetaliate = TRUE;

    return !didCast;
}

int CombatProcSpell::doit ( PackedData *packet )
{
    spell_info *spell = &gSpellTable[this->spell];

    WorldObject *caster = roomMgr->findObject ( casterServID );
    WorldObject *targetObj = roomMgr->findObject ( targetServID );

    if ( caster ) {
        if ( !targetObj || ( targetObj && targetObj->getBaseOwner()->combatGroup == client->combatGroup ) ) {
            client->player->castProc ( spell, caster, targetServID, targetX, targetY, packet );
        }
    }
    didCast = TRUE;
    gCombatActionLogger.logAction( this );
    return TRUE;
}

// 
// CombatEat
//

CombatEat::CombatEat ( WorldObject *obj, WorldObject *target )
{
	type = _CA_EAT;
	targetServID = target? target->servID : -1;
	client = obj;
	didEat = FALSE;
}

CombatEat::CombatEat( const CombatEat& action ) : CombatAction ( action )
{
	targetServID = action.targetServID;
	didEat = action.didEat;
}

CombatEat::~CombatEat()
{
}

int CombatEat::canDoit ( void )
{
	if ( client->health < 1 )
		return FALSE;

//	if ( !didEat && roomMgr->findObject ( targetServID ) )
//		return TRUE;
	
	WorldObject *item = roomMgr->findObject ( targetServID );

	if ( item ) {
		// must be owned object
		WorldObject *owningObj = item->getBaseOwner();

		if ( !didEat && item->getBase ( _BCONSUME ) && ( owningObj && owningObj == client ) ) 
			return TRUE;
	}

	return FALSE;
}

int CombatEat::doit ( PackedData *packet )
{
	WorldObject *item = roomMgr->findObject ( targetServID );

	if ( item )	{

		if ( client->consume ( item, packet ) == _WO_ACTION_HANDLED ) {
 			roomMgr->destroyObj ( item, FALSE, __FILE__, __LINE__ );
			didEat = 1;
			gCombatActionLogger.logAction( this );
		}
	}

	return TRUE;
}

//
// CombatExit
//

CombatExit::CombatExit ( WorldObject *obj )
{
	client = obj;
	didExit = 0;
	type = _CA_EXIT;
}

CombatExit::CombatExit( const CombatExit& action ) : CombatAction( action )
{
	didExit = action.didExit;
}

CombatExit::~CombatExit()
{
}

int CombatExit::canDoit ( void )
{
	return !didExit;
}

int CombatExit::doit ( PackedData *packet )
{
	client->player->exitCombat ( packet );
	didExit = 1;
	gCombatActionLogger.logAction( this );
	return 1;
}

//
// CombatActionLogger
//

CombatActionLogger::CombatActionLogger()
{
}

CombatActionLogger::~CombatActionLogger()
{
}

LinkedList* CombatActionLogger::findActionTypes( int type )
{
	LinkedList* subList = new LinkedList();

	LinkedElement* element = actionList.head();

	while( element )
	{
		if( type == ((CombatAction*)element->ptr())->type )
			subList->add( element->ptr() );

		element = element->next();
	}

	return subList;
}

bool CombatActionLogger::existsActionType( int type )
{
	//see if this type exists anywhere
	LinkedElement* element = actionList.head();

	while( element )
	{
		if( type == ((CombatAction*)element->ptr())->type )
			return true;

		element = element->next();
	}
	return false;
}

void CombatActionLogger::reset( void )
{
	actionList.dispose();
}

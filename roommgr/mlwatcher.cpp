//
// mlwatcher.cpp
//
// This file contains the Watcher AI code.
//
// Author: Stephen Nichols
//

#include "roommgr.hpp"

// enumerate the watcher states
enum {
	_WATCHER_WATCH_PLAYER,
	_WATCHER_COMMUNE,
	_WATCHER_WANDER,
	_WATCHER_COMMUNE_TARGET
};

// this is the linked list of watchers
LinkedList gWatcherList;

Watcher::Watcher()
{
	m_nTargetServID = -1;
	m_nTargetAcquireTime = 0;
	m_nTargetWatchTime = 0;
	m_nWatchState = _WATCHER_WATCH_PLAYER;

	gWatcherList.add ( this );
}

Watcher::~Watcher()
{
	gWatcherList.del ( this );
}

WorldObject *Watcher::chooseTarget ( void )
{
	int nCharListSize = roomMgr->_characters.size();
	WorldObject *pChar = NULL;

	m_nTargetAcquireTime = getsecondsfast();
	m_nTargetWatchTime = random ( 30, 60 );

	if ( 0 == nCharListSize ) {
		m_nTargetServID = -1;
	} else {
		int nCharIndex = random ( 0, nCharListSize - 1 );
		pChar = (WorldObject *)roomMgr->_characters.at ( nCharIndex );
		m_nTargetServID = pChar->servID;

		if ( (pChar->room == NULL) || (pChar->room->picture == 3071) ) {
			m_nTargetServID = -1;
			m_nTargetWatchTime = 5;
			pChar = NULL;
		}
	}

	return pChar;
}

int Watcher::combatLogic ( void )
{
	setAction ( new CombatMove ( character, character->combatX, character->combatY, 0 ) ); 
	return 1;
}

int Watcher::normalLogic ( void )
{
	static int nApproachX[3] = { -30, 80, -80 };
	static int nApproachY[3] = { -40, 0, 0 };
	static int nApproachAngle[3] = { 180, 270, 90 };
	static char *msgs[] = {
		"!!!",
		"???",
		"...",
		"They are not smart enough...",
		"Have they gathered yet?",
		"I have heard that it will begin soon.",
		"The portals are nearing completion.",
		"The mists are too strong to penetrate.",
		"Are they ready?",
		"Have you seen the stoney ones?",
		"They still use mana crystals!",
		"Mabon has forsaken these.",
		"Leinster still burns!",
		"Duach is a powerful foe.",
		"The alarmers will come as well.",
		"I feel a strong magical undercurrent.",
		"Did you seem them cast 'Invisibility'? ha ha ha",
		"The Avengers will be involved.",
		"Enid is powerless to help.",
		"I have seen nothing that can stand in our way.",
		"What have you seen?",
		"These weaklings pose no threat.",
		"He will be quite pleased.",
		"What of the newcomers?",
		"The newcomers will dominate.",
		"Wen is a prime target.",
		"The plan is almost complete.",
		"Their magic is impotent.",
		"They have attacked me several times.",
		"I must observe more closely.",
		"The altars are beginning to surge.",
		"Did you see the wishing well?",
		"Yes.",
		"No.",
		"I agree.",
		"I disagree.",
		"Their armor is no match for us.",
		"The stench of death will be great.",
		"Indeed.",
		"It will be easy.",
		"I feel sorry for them.",
		"The ogres are a bit frustrated.",
		"Have the other temples become active?",
		"BishopSmith's demise was touching.",
		"Do you think they will listen?",
		"Without the proper weapons how can they fight?",
		"The offerings will be consumed.",
		"They grow in strength and number.",
		"The faeries are becoming angry.",
		"Some things are beginning to happen in the desert.",
		"These 'levels' seem quite puny.",
		"The appointed time grows nearer.",
		"There is much still to do.",
		"What is he waiting for?",
		"Will they ever prepare?",
		"The sacred temple will prevail.",
		"I think they can understand us.",
		"The time is not right yet.",
		"The others are coming nearer... they have some powers now.",
		"The mark of the gods will be on them.",
		"Do you smell that?",
		"The attacks will become more frequent.",
		"Vanity.  That is the key."
	};
	static int bCrypted = 0;

	static char aCryptTbl[3][26] = {
		{
			'v', 'q', 'j', 'n', 'x', 'r', 't', 'o', 'z', 'y', 'u', 'p', 's',
			'w', 'h', 'l', 'b', 'f', 'm', 'g', 'k', 'a', 'd', 'e', 'c', 'i' 
		},
		{
			'w', 'h', 'l', 'b', 'f', 'm', 'g', 'k', 'a', 'd', 'e', 'c', 'i',
			'v', 'q', 'j', 'n', 'x', 'r', 't', 'o', 'z', 'y', 'u', 'p', 's' 
		},
		{
			'i', 'w', 'b', 'l', 'm', 'f', 'k', 'g', 'd', 'a', 'c', 'e', 'q',
			's', 'v', 'n', 'j', 'r', 'x', 'o', 't', 'y', 'z', 'p', 'u', 'h' 
		}
	};

	if ( !bCrypted ) {
		// crypt each message
		for ( int i=0; i<(sizeof ( msgs ) / sizeof ( char * )); i++ ) {
			char *msg = msgs[i];
			int offset = 0;

			while ( *msg ) {
				char ch = *msg;

				if ( isalpha ( ch ) ) {
					int upper = isupper ( ch );
					ch = tolower ( ch );

					int index = (int)(ch - 'a');
					int tblIndex = index % 3;
					index += offset;
					index = index % 26;
					ch = aCryptTbl[tblIndex][index];

					if ( upper )
						ch = toupper ( ch );

					*msg = ch;
					offset++;
				}

				msg++;
			}
		}

		bCrypted = 1;
	}

	int retVal = 5;

	if ( !room || character->health <= 0 )
		return -1;

	// check for target change
	int nCurTime = getsecondsfast();
	WorldObject *pChar = NULL;

	// choose a new target if enough time has been spent watching this one...
	if ( (nCurTime - m_nTargetAcquireTime) > m_nTargetWatchTime ) {
		m_nWatchState = random ( _WATCHER_WATCH_PLAYER, _WATCHER_WANDER );

		if ( m_nWatchState == _WATCHER_WANDER ) {
			if ( random ( 0, 3 ) )
				m_nWatchState = _WATCHER_WATCH_PLAYER;
		}

		if ( m_nWatchState == _WATCHER_WATCH_PLAYER ) {
			pChar = chooseTarget();	

			if ( NULL != pChar ) {
				int nAngle = random ( 0, 2 );

				character->x = pChar->x + nApproachX[nAngle] + random ( 0, 10 ) - 5;
				character->y = pChar->y + nApproachY[nAngle] + random ( 0, 8 ) - 4;
				character->loop = character->headingToLoop ( nApproachAngle[nAngle] ); 

				teleport ( pChar->room->number, 0 );

			} else {
				m_nWatchState = _WATCHER_COMMUNE;
			}
		}

		if ( m_nWatchState == _WATCHER_WANDER ) {
			m_nTargetAcquireTime = getsecondsfast();
			m_nTargetWatchTime = random ( 120, 300 );

			if ( random ( 0, 3 ) ) {
				teleport ( random ( 4791, 4793 ), 0 );
			} else {
				teleport ( random ( 3325, 3327 ), 0 );
			}
		}

		if ( m_nWatchState == _WATCHER_COMMUNE ) {
			m_nTargetAcquireTime = getsecondsfast();
			m_nTargetWatchTime = random ( 60, 120 );

			// pick a target for commune...
			Watcher *target = (Watcher *)gWatcherList.at ( random ( 0, gWatcherList.size() - 1 ) );

			if ( (target == this) || (target->m_nWatchState == _WATCHER_COMMUNE) ) {
				m_nWatchState = _WATCHER_WANDER;
			} else {
				pChar = target->character;
				m_nTargetServID = pChar->servID;

				target->m_nWatchState = _WATCHER_COMMUNE_TARGET;
				target->m_nTargetAcquireTime = m_nTargetAcquireTime;
				target->m_nTargetWatchTime = m_nTargetWatchTime;
				target->m_nTargetServID = character->servID;

				character->x = pChar->x + nApproachX[1] + random ( 0, 10 ) - 5;
				character->y = pChar->y + nApproachY[1] + random ( 0, 8 ) - 4;
				character->loop = character->headingToLoop ( nApproachAngle[1] ); 

				teleport ( pChar->room->number, 0 );
			}
		}

		return 2;
	}

	RMRoom *theRoom = character->room;

	// observe the target (if any)
	if ( (m_nWatchState == _WATCHER_WATCH_PLAYER) && (m_nTargetServID != -1) ) {
		retVal = 2;
		pChar = roomMgr->findObject ( m_nTargetServID );

		// default to wander if there is no target...
		if ( NULL == pChar ) {
			m_nTargetServID = -1;
			m_nWatchState = _WATCHER_WANDER;
			return 2;
		}

		RMRoom *targetRoom = pChar->room;

		if ( theRoom == targetRoom ) {
			// get the distance between us and target...
			int dist = getDistance ( character->x, character->y, pChar->x, pChar->y );

			if ( 1 ) {
				retVal = dist / 100;

				if ( retVal < 5 )
					retVal = 5;

				// walk to the target...
				PackedMsg packet;
				packet.putLong ( character->servID );
				packet.putLong ( theRoom->number );

				int nAngle = random ( 0, 2 );
				gotoXY ( pChar->x + nApproachX[nAngle] + random ( 0, 10 ) - 5, pChar->y + nApproachY[nAngle] + random ( 0, 8 ) - 4, &packet );

				// force a heading change...
				packet.putByte ( _MOVIE_HEADING );
				packet.putWord ( nApproachAngle[nAngle]  );
	
				// end the movie packet, you fool!
				packet.putByte ( _MOVIE_END );
	
				roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, packet.data(), packet.size(), theRoom );
			}
		} else {
			// change to an adjacent room, if required
			int targetRoomNum = targetRoom->number;
			int targetDir = -1;

			if ( targetRoomNum == theRoom->north ) {
				targetDir = 0;
			}

			else if ( targetRoomNum == theRoom->south ) {
				targetDir = 1;
			}
	
			else if ( targetRoomNum == theRoom->east ) {
				targetDir = 2;
			}
	
			else if ( targetRoomNum == theRoom->west ) {
				targetDir = 3;
			}
	
			if ( targetDir == -1 ) {
				// teleport to the bastard!
				int nAngle = random ( 0, 2 );

				character->x = pChar->x + nApproachX[nAngle] + random ( 0, 10 ) - 5;
				character->y = pChar->y + nApproachY[nAngle] + random ( 0, 8 ) - 4;
				character->loop = character->headingToLoop ( nApproachAngle[nAngle] ); 

				teleport ( targetRoomNum, 0 );
			} else {
				changeRoom ( targetDir );
				retVal = 5;
			}
		}
	}

	else if ( m_nWatchState == _WATCHER_COMMUNE ) {
		pChar = roomMgr->findObject ( m_nTargetServID );

		// default to wander if there is no target...
		if ( NULL == pChar ) {
			m_nTargetServID = -1;
			m_nWatchState = _WATCHER_WANDER;
			return 1;
		}

		retVal = 2;

		// walk to the target...
		PackedMsg packet;
		packet.putLong ( character->servID );
		packet.putLong ( theRoom->number );

		gotoXY ( pChar->x + nApproachX[1] + random ( 0, 10 ) - 5, pChar->y + nApproachY[1] + random ( 0, 8 ) - 4, &packet );

		// say a random message...
		if ( random ( 0, 4 ) == 0 ) {
			packet.putByte ( _MOVIE_INFO );
			packet.putLong ( character->servID );
			packet.putString ( msgs[random ( 0, (sizeof ( msgs ) / sizeof ( char * )) - 1)] );
			retVal = random ( 5, 9 );
		}

		// force a heading change...
		packet.putByte ( _MOVIE_HEADING );
		packet.putWord ( nApproachAngle[1]  );
	
		// end the movie packet, you fool!
		packet.putByte ( _MOVIE_END );
	
		roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, packet.data(), packet.size(), theRoom );
	}

	else if ( m_nWatchState == _WATCHER_COMMUNE_TARGET ) {
		pChar = roomMgr->findObject ( m_nTargetServID );

		// default to wander if there is no target...
		if ( NULL == pChar ) {
			m_nTargetServID = -1;
			m_nWatchState = _WATCHER_WANDER;
			return 1;
		}

		retVal = 2;

		// walk to the target...
		PackedMsg packet;
		packet.putLong ( character->servID );
		packet.putLong ( theRoom->number );

		gotoXY ( pChar->x + nApproachX[2] + random ( 0, 10 ) - 5, pChar->y + nApproachY[2] + random ( 0, 16 ) - 8, &packet );

		// say a random message...
		if ( random ( 0, 4 ) == 0 ) {
			packet.putByte ( _MOVIE_INFO );
			packet.putLong ( character->servID );
			packet.putString ( msgs[random ( 0, (sizeof ( msgs ) / sizeof ( char * )) - 1)] );
			retVal = 5;
		}

		// force a heading change...
		packet.putByte ( _MOVIE_HEADING );
		packet.putWord ( nApproachAngle[2]  );
	
		// end the movie packet, you fool!
		packet.putByte ( _MOVIE_END );
	
		roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, packet.data(), packet.size(), theRoom );
	}

	else if ( m_nWatchState == _WATCHER_WANDER ) {
		if ( theRoom->number == 4792 ) {
			int nQuakeChance = 400;

			if ( random ( 0, 4 ) ) {
				// walk to the target...
				PackedMsg packet;
				packet.putLong ( character->servID );
				packet.putLong ( theRoom->number );
	
				// force a heading change...
				packet.putByte ( _MOVIE_HEADING );
				packet.putWord ( nApproachAngle[random ( 0, 2 )]  );

				// look for an object on the ground to consume
				LinkedElement *element = theRoom->objects.head();

				while ( element ) {
					WorldObject *obj = (WorldObject *)element->ptr();
					element = element->next();

					if ( random ( 0, 9 ) == 0 ) {
						if ( obj->getBase ( _BCARRY ) && !obj->combatGroup ) {
							int value = obj->netWorth();

							if ( value >= 10000 ) {
								nQuakeChance = 40;
							}

							else if ( value >= 100000 ) {
								nQuakeChance = 9;
							}

							packet.putByte ( _MOVIE_TOSS_OBJECT );
							packet.putLong ( character->servID );
							packet.putLong ( obj->servID );

							roomMgr->destroyObj ( obj, 0, __FILE__, __LINE__ );
						}
					}
				}

				if ( random ( 0, nQuakeChance ) == 0 ) {
					packet.putByte ( _MOVIE_SPECIAL_EFFECT );
					packet.putLong ( character->servID );
					packet.putByte ( _SE_EARTHQUAKE );
					packet.putByte ( 0 );
					packet.putLong ( character->servID );
				}
		
				// end the movie packet, you fool!
				packet.putByte ( _MOVIE_END );
		
				roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, packet.data(), packet.size(), theRoom );
				return 1;
			} else {
				if ( random ( 0, 3 ) == 0 ) {
					changeRoom ( random ( 0, 3 ) );
					return 7;
				} else {
					return gotoXY ( random ( 50, 600 ), random ( 150, 300 ) );
				}
			}
		} else {
			if ( random ( 0, 3 ) == 0 ) {
				changeRoom ( random ( 0, 3 ) );
				return 7;
			} else {
				return gotoXY ( random ( 50, 600 ), random ( 150, 300 ) );
			}
		}
	}

	return retVal;
}

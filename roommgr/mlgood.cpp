//
// mlgood.cpp
//
// Control logic for a 'good' basic monster.
//
// Author: Stephen Nichols
//

#include "roommgr.hpp"

GoodMonster::GoodMonster()
{
}

GoodMonster::~GoodMonster()
{
}

int GoodMonster::normalLogic ( void )
{
	int retVal = 10;

	if ( character->summoned || !room || character->health <= 0 )
		return -1;

	if ( groupLeader && groupLeader != this )
		return 5;

	switch ( random ( 0, 6 ) ) {
		// walk to a random location on screen
		case 0:
		case 1:
		case 2: {
			if ( room->picture != 3071 ) {
				gotoXY ( random ( 50, 600 ), random ( 150, 300 ) );
			}
		}

		break;

		case 4:
		case 3:
		/* removed good-monster jumping code */
#if 0
		{
			LinkedElement *element = room->head();
			retVal = 5;

			while ( element ) {
				RMPlayer *thePlayer = (RMPlayer *)element->ptr();

				int chance = 0;

				if ( room->picture == 3071 )
					chance = random ( 0, 1 );
				else
					chance = random ( 0, 5 );

				if ( !chance &&
					 !thePlayer->character->hasAffect ( _AFF_JAIL )
					 && zone->allowCombat()
					 && !thePlayer->isNPC
					 && !thePlayer->character->hidden
					 && !thePlayer->character->combatGroup
					 && !isGroupMember ( thePlayer )
					 && thePlayer->character->health > 0
					 && (thePlayer->character->coarseAlignment() == _ALIGN_EVIL)
					 && !thePlayer->checkAccess ( _ACCESS_MODERATOR ) )  {

					if ( character->level > 1 ) {
						PackedMsg response;
						response.putLong ( character->servID );
						response.putLong ( character->room->number );
						engage ( thePlayer->character, &response );
						response.putByte ( _MOVIE_END );

						roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, response.data(), response.size(), character->room );
					} else {
						emote ( "watches %s warily.", thePlayer->getName() );
					}

					retVal = 1;

					break;
				}

				element = element->next();
			}
		}
#endif
		break;
		// change rooms
		case 5: {
			if ( room->picture != 3071 ) {
				// pick a random direction
				changeRoom ( random ( 0, 3 ) );
			}
		}

		break;

		case 6: {
			int roll = random ( 0, 100 );

			if ( roll < room->groupChance && groupLeader == NULL ) {
				LinkedElement *element = room->head();

				while ( element ) {
					RMPlayer *thePlayer = (RMPlayer *)element->ptr();
					element = element->next();

					if ( groupLeader == NULL && thePlayer->isNPC && thePlayer != this && !thePlayer->character->combatGroup && (abs ( character->level - thePlayer->character->level ) < 4) ) {
						joinGroup ( thePlayer->groupLeader? thePlayer->groupLeader : thePlayer );
						return 5;
					}
				}
			}
		}
	}
	return retVal;
}


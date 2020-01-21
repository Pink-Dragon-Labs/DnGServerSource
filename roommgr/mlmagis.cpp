//
// mlmagis.cpp
//
// Control logic for magistrate.
//
// Author: Stephen Nichols
//

#include "roommgr.hpp"

Magistrate::Magistrate()
{
}

Magistrate::~Magistrate()
{
}

int Magistrate::normalLogic ( void )
{
	int retVal = 60;

	if ( !room || character->health <= 0 )
		return -1;

	emote ( "rustles through his wanted posters." );

	// look for someone on the wanted list to jail
	LinkedElement *element = gWantedList.head();

	while ( element ) {
		WorldObject *obj = (WorldObject *)element->ptr();

		if ( obj->room && !obj->combatGroup && obj->room->zone == character->room->zone ) {
			say ( "Ahh, yes.  The criminal named %s needs some discipline.", obj->getName() );
			// teleport to this guy and let him have it
			character->x = obj->x;
			character->y = obj->y - 5;
			teleport ( obj->room->number );

			PackedMsg response;
			response.putLong ( character->servID );
			response.putLong ( character->room->number );
			response.putByte ( _MOVIE_CAST_BEGIN );
			response.putByte ( _MOVIE_END );

			roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, response.data(), response.size(), character->room );

			PackedMsg packet;
			packet.putLong ( character->servID );
			packet.putLong ( character->room->number );

			packet.putByte ( _MOVIE_CAST_END );
			packet.putLong ( character->servID );

			packet.putByte ( _MOVIE_END );

			roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, packet.data(), packet.size(), character->room );

			say ( "Yet another criminal dispatched by the magistrate of Leinster!" );

			return 10;
		}

		element = element->next();
	}

	if ( !element && character->room->number != 6051 ) {
		character->x = 320;
		character->y = 200;
		teleport ( 6051 );
	}

	return retVal;
}

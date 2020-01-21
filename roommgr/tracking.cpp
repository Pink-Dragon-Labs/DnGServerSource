//
// tracking
//
// module that handles tracking room changes for every player in the game
//
// author: Stephen Nichols
//

#include "roommgr.hpp"

BinaryTree gRoomTrackTree;

TrackingInfo::TrackingInfo()
{
	name = NULL;
	lastUpdateTime = getseconds();
}

TrackingInfo::~TrackingInfo()
{
	setName ( NULL );
}

void TrackingInfo::setName ( char *str )
{
	if ( name ) {
		free ( name );
		name = NULL;
	}

	if ( str ) 
		name = strdup ( str );
}

void TrackingInfo::addRoom ( int number )
{
	if ( count >= _MAX_TRACK_DISTANCE ) {
		memmove ( &rooms, &rooms[1], sizeof ( rooms ) - sizeof ( rooms[0] ) );
		count--;
	}

	rooms[count] = number;
}

//
// tracking
//
// module that handles tracking room changes for every player in the game
//
// author: Stephen Nichols
//

#ifndef _TRACKING_HPP_
#define _TRACKING_HPP_

#include "../global/system.hpp"

#define _MAX_TRACK_DISTANCE 100

class TrackingInfo : public ListObject
{
public:
	TrackingInfo();
	virtual ~TrackingInfo();

	// add a room to the list of visited rooms
	void addRoom ( int number );

	// set the name of the character 
	void setName ( char *str );

	// name of the character this tracking structure represents
	char *name;

	// last time (in seconds) that this structure was updated...
	int lastUpdateTime;

	// last N rooms this character has visited
	int count;
	int rooms[_MAX_TRACK_DISTANCE];
};

extern BinaryTree gRoomTrackTree;

#endif

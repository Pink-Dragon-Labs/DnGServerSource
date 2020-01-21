//
// SPECIAL.HPP
//
// This is a special SpecialEffect class.
//
// author: Stephen Nichols
//

#include "roommgr.hpp"

SpecialEffect::SpecialEffect ( int theType, int isAsync )
{
	type = theType;
	asynchronous = isAsync;
}

SpecialEffect::~SpecialEffect()
{
	type = -1;
	asynchronous = -1;
}

// build a packet that describes this effect
void SpecialEffect::buildPacket ( WorldObject *source, PackedData *packet, int target )
{
	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( source->servID );
	packet->putByte ( type );
	packet->putByte ( asynchronous? 0 : 1 );
	packet->putLong ( target );
}

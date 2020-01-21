/*
	WorldObjectBase class	
	author: Stephen Nichols
*/

#include "roommgr.hpp"

void WorldObjectBase::setProperty ( int property, ... )
{
	va_list args;
	va_start ( args, property );

	// prepare the outgoing packet
	PackedData packet;

	// put the header
	packet.putLong ( 0 );
	packet.putLong ( 0 );

	// put the servID of self
	packet.putLong ( self->servID );

	while ( property != _PROP_END ) {
		// put the property that we are setting
		packet.putWord ( property );

		int type = propertyTypeTable[property];

		switch ( type ) {
			case _PROP_STRING: 
				packet.putString ( va_arg ( args, char * ) );
				break;

			case _PROP_INT: 
				packet.putLong ( va_arg ( args, int ) );
				break;

			case _PROP_WORD:
				packet.putWord ( va_arg ( args, unsigned int ) );
				break;

			case _PROP_BYTE:
				packet.putWord ( va_arg ( args, unsigned int ) );
				break;
		}

		property = va_arg ( args, int );
	}

	va_end ( args );

	packet.putWord ( _PROP_END );

	// send the property update packet to the room
	self->sendToOwner ( _IPC_SET_PROP, packet.data(), packet.size() );
}

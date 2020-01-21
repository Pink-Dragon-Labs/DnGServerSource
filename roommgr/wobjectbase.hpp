/*
	WorldObject / WorldObjectBase classes	
	author: Stephen Nichols
*/

#ifndef _WOBJECTBASE_HPP_
#define _WOBJECTBASE_HPP_

//#include "system.hpp"
#include "../global/list.hpp"

enum {
	_BCARRY,
	_BCONTAIN,
	_BHEAD,
	_BWEAR,
	_BOPEN,
	_BCYCLE,
	_BCHARACTER,
	_BLOCK,
	_BKEY,
	_BWEAPON,
	_BENTRY,
	_BSHOP,
	_BNPC,
	_BCONSUME,
	_BPASSWORD,
	_BGATE,
	_BSIT,
	_BSCROLL,
	_BTALK,
	_BUSE,
	_BMIX,
	_BSWITCH,
	_BDESCRIBED,
	_BDYE,
	_BSPELLBAG,
	_BMAX,
	_BPLAYER	= 200,
	_BTREASURE,
	_BNULL		= 255
};

enum {
	_WO_ACTION_HANDLED,
	_WO_ACTION_ALLOWED,
	_WO_ACTION_PROHIBITED,
	_WO_ACTION_TRIGGER,
	_WO_ACTION_ERROR
};

class WorldObject;
class PackedData;

class WorldObjectBase : public ListObject
{
public:
	int type;
	WorldObject *self;

	WorldObjectBase() {};
	WorldObjectBase ( WorldObject *object ) { self = object; };
	virtual ~WorldObjectBase() {};

	/* copy another base of the same type */
	virtual void copy ( WorldObjectBase *base ) {};

	/* packet utility methods */
	virtual void buildPacket ( PackedData *packet, int override = 0 ) {};
	virtual void fromPacket ( PackedData *packet ) {};

	/* put my servIDs into a packed data structure */
	virtual void submitServIDs ( PackedData *packet ) {};

	virtual int perform ( int action, va_list *args ) { return _WO_ACTION_ALLOWED; };
	virtual int handlesAction ( int action ) { return 0; };

	/* cleanup this base */
	virtual void cleanup ( void ) {};

	/* send a message to the client requesting properties to be set */
	void setProperty ( int property, ... );

	/* write SCI script representation */
	virtual void writeSCIData ( FILE *file ) {};
};

#endif

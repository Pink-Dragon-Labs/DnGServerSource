/*
	BWearable class
	author: Stephen Nichols
*/

#ifndef _BWEAR_HPP_
#define _BWEAR_HPP_

#include "wobjectbase.hpp"

// define the different masks
#define _WEAR_MASK_MALE				(1 << 0)
#define _WEAR_MASK_FEMALE			(1 << 1)
#define _WEAR_MASK_HUMAN			(1 << 2)
#define _WEAR_MASK_ORC				(1 << 12)
#define _WEAR_MASK_GIANT			(1 << 3)
#define _WEAR_MASK_ELF				(1 << 4)
#define _WEAR_MASK_GOOD				(1 << 5)
#define _WEAR_MASK_NEUTRAL			(1 << 6)
#define _WEAR_MASK_EVIL				(1 << 7)
#define _WEAR_MASK_ADVENTURER		(1 << 8)
#define _WEAR_MASK_WARRIOR			(1 << 9)
#define _WEAR_MASK_WIZARD			(1 << 10)
#define _WEAR_MASK_THIEF			(1 << 11)

#define _WEAR_MASK_EVERYONE		0xFFFFFFFF;

class BWearable : public WorldObjectBase
{
public:
	int areaWorn, mask, layer, level, spellProcID, reverseProcID, forwardProcChance, reverseProcChance;	 // added level
	WorldObject *owner;

	BWearable ( WorldObject *obj );
	virtual ~BWearable();

	void copy ( WorldObjectBase *base );

	virtual void buildPacket ( PackedData *packet, int override = 0 );
	virtual void writeSCIData ( FILE *file );

	// verb interface functions
	int handlesAction ( int action );
	int perform ( int action, va_list *args );
	
	// verb action functions 
	int bePutOn ( WorldObject *object );
	int beTakenOff ( WorldObject *object = NULL );
};

#endif

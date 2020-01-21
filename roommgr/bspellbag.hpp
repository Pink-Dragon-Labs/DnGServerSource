/*
	BSpellBag class
	author: Stephen Nichols
*/

#ifndef _BSPELLBAG_HPP_
#define _BSPELLBAG_HPP_

#include "wobjectbase.hpp"

class SpellComponent : public ListObject
{
public:
	SpellComponent();
	virtual ~SpellComponent();

	WorldObject *object;
	int quantity;
};

class BSpellBag : public WorldObjectBase
{
public:
	LinkedList components;

	BSpellBag ( WorldObject *obj );
	virtual ~BSpellBag();

	void copy ( WorldObjectBase *base );
	void writeSCIData ( FILE *file );
	virtual void buildPacket ( PackedData *packet, int override );

	void addComponent ( WorldObject *object, int quantity );
	void addComponent ( SpellComponent *component );
};

#endif

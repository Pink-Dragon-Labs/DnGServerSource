/*
	BSpellBag class
	author: Stephen Nichols
*/

#include "bspellbag.hpp"
#include "roommgr.hpp"

BSpellBag::BSpellBag ( WorldObject *obj ) : WorldObjectBase ( obj )
{
	type = _BSPELLBAG;
}

BSpellBag::~BSpellBag()
{
}

void BSpellBag::copy ( WorldObjectBase *theBase )
{
	BSpellBag *base = (BSpellBag *)theBase;

	LinkedElement *element = base->components.head();

	while ( element ) {
		SpellComponent *component = (SpellComponent *)element->ptr();
		element = element->next();

		SpellComponent *newComponent = new SpellComponent;
		newComponent->object = component->object;
		newComponent->quantity = component->quantity;
	}
}

void BSpellBag::writeSCIData ( FILE *file )
{
	fprintf ( file, "\t\t(aWhatObj addBase: BSpellBag)\n" );
}

void BSpellBag::buildPacket ( PackedData *packet, int override )
{
	packet->putByte ( components.size() );

	LinkedElement *element = components.head();

	while ( element ) {
		SpellComponent *component = (SpellComponent *)element->ptr();
		element = element->next();

		packet->putWord ( component->object->classNumber );
		packet->putWord ( component->quantity );
	}
}

void BSpellBag::addComponent ( WorldObject *object, int quantity )
{
	LinkedElement *element = components.head();

	while ( element ) {
		SpellComponent *component = (SpellComponent *)element->ptr();

		if ( component->object->classNumber == object->classNumber ) {
			component->quantity += quantity;
			break;
		}

		element = element->next();
	}

	if ( !element ) {
		SpellComponent *component = new SpellComponent;

		if ( object->servID != -1 ) 
			object = roomMgr->findClass ( object->super );

		component->object = object;
		component->quantity = quantity;

		components.add ( component );
	}
}

void BSpellBag::addComponent ( SpellComponent *component )
{
	addComponent ( component->object, component->quantity );
}

SpellComponent::SpellComponent()
{
	object = NULL;
	quantity = 0;
}

SpellComponent::~SpellComponent()
{
}

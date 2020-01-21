#include "squirmy.hpp"

SquirmyObj::SquirmyObj()
{
}

SquirmyObj::~SquirmyObj()
{
}

WorldObject *SquirmyObj::clone ( void )
{
	SquirmyObj *clone = new SquirmyObj;
	clone->copy ( this );

	return (WorldObject *)clone;
}

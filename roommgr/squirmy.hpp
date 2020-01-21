#ifndef _SQUIRMY_HPP_
#define _SQUIRMY_HPP_

#include "roommgr.hpp"

class SquirmyObj : public WorldObject
{
public:
	SquirmyObj();
	virtual ~SquirmyObj();

	virtual WorldObject *clone ( void );
};

#endif

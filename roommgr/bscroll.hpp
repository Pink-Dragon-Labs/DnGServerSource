/*
	BScroll class
	author: Stephen Nichols
*/

#ifndef _BSCROLL_HPP_
#define _BSCROLL_HPP_

#include "wobjectbase.hpp"
#include "wobject.hpp"

class BScroll : public WorldObjectBase
{
public:
	BScroll ( WorldObject *obj );
	virtual ~BScroll();

	void copy ( WorldObjectBase *base );

	virtual void writeSCIData ( FILE *file );

	// action handling methods
	virtual int handlesAction ( int action );
	virtual int perform ( int action, va_list *args );

	// handle being memorized by a player 
	int beMemorized ( WorldObject *object );

	int calcBPCost ( WorldObject *object );

	// describe the magic that this scroll holds
	int spell;

	// describe the memorizing requirements
	int skill;
	int bpCost;

	int level;
};

#endif

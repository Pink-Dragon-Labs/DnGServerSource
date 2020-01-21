/*
	BDye class
	author: Stephen Nichols
*/

#ifndef _BDYE_HPP_
#define _BDYE_HPP_

#include "wobjectbase.hpp"
#include "wobject.hpp"

class BDye : public WorldObjectBase
{
public:
	int	nHairDye;

	BDye ( WorldObject *obj );
	virtual ~BDye();

	void copy ( WorldObjectBase *base );
	virtual void writeSCIData ( FILE *file );
};

#endif

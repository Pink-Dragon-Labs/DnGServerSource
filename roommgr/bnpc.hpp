/*
	BNPC class
	author: Stephen Nichols
*/

#ifndef _BNPC_HPP_
#define _BNPC_HPP_

#include "wobjectbase.hpp"

class BNPC : public WorldObjectBase
{
public:
	BNPC ( WorldObject *obj );
	virtual ~BNPC();

	void copy ( WorldObjectBase *base );

	virtual void writeSCIData ( FILE *file );

	void setCode ( char * );

	char *code;
	int isExternal;
};

#endif

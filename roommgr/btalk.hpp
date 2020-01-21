/*
	BTalk class
	author: Stephen Nichols
*/

#ifndef _BTALK_HPP_
#define _BTALK_HPP_

#include "wobjectbase.hpp"
#include "wobject.hpp"

class BTalk : public WorldObjectBase
{
public:
	BTalk ( WorldObject *obj );
	virtual ~BTalk();

	void copy ( WorldObjectBase *base );

	virtual void writeSCIData ( FILE *file );

	void setFile ( char *str );

	// rumor text file name
	char *file;
	int talkTreeID;
};

#endif

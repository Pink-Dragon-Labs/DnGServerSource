/*
	BPassword class
	author: Stephen Nichols
*/

#ifndef _BPASSWORD_HPP_
#define _BPASSWORD_HPP_

#include "wobjectbase.hpp"
#include "wobject.hpp"

#define _MAX_PASSWORD_SIZE 17

class DBPassword 
{
public:
	int owner;
	char password[_MAX_PASSWORD_SIZE];
};

class BPassword : public WorldObjectBase, public DBPassword
{
public:
	BPassword ( WorldObject *obj );
	virtual ~BPassword();

	void copy ( WorldObjectBase *base );

	virtual void writeSCIData ( FILE *file );
};

#endif

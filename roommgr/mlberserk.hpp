//
// mlberserk.hpp
//
// File that contains Berserker logic code.
//
// Author: Janus Anderson
// modified from mlhunter by Scott Wochholz
//

#ifndef _MLBERSERKER_HPP_
#define _MLBERSERKER_HPP_

#include "mlsmarty.hpp"

class Berserker : public SmartMonster
{
	public:
	Berserker();
	virtual ~Berserker();

	virtual void init ( void );
	virtual int isFriend ( WorldObject *thisPlayer );
};

#endif

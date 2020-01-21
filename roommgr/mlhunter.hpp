//
// mlhunter.hpp
//
// File that contains hunter logic code.
//
// Author: Janus Anderson
//

#ifndef _MLHUNTER_HPP_
#define _MLHUNTER_HPP_

#include "mlsmarty.hpp"

class Hunter : public SmartMonster
{
	public:
	Hunter();
	virtual ~Hunter();

	virtual void init ( void );
	virtual int isFriend ( WorldObject *thisPlayer );
	virtual int isEnemy ( WorldObject *thisPlayer );
	virtual int spellAttack ( void );
};

#endif

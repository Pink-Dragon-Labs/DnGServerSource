//
// mlmouse.hpp
//
// File that contains logic code for mouses
//
// Author: Janus Anderson
//

#ifndef _MLMOUSE_HPP_
#define _MLMOUSE_HPP_

#include "mlsmarty.hpp"

class Mouse : public SmartMonster
{
public:
	Mouse();
	virtual ~Mouse();

	virtual void init ( void );
};

#endif

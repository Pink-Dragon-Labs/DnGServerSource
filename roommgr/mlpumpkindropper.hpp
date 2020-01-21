//
// mlpumpkindropper.hpp
//
// File that contains logic code for the Harvest Festival pumpkin spawners.
//
// Author: Michael Nicolella
//

#ifndef _MLPUMPDROP_HPP_
#define _MLPUMPDROP_HPP_

#include "mlsmarty.hpp"

class PumpkinDropper : public SmartMonster
{
	//keep track of the last 3 players I dropped a pumpkin next to
	RMPlayer* lastPlayers[3];
public:
	PumpkinDropper();
	virtual ~PumpkinDropper();
	virtual void init ( void );
	int normalLogic( void );
};

#endif

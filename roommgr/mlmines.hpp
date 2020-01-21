//
// mlmines.hpp
//
// File that contains logic code for dropping metals in The Mines
//
// Author: Michael Nicolella
//

#ifndef _MLMINES_HPP_
#define _MLMINES_HPP_

#include "mlsmarty.hpp"

class MinesDropper : public SmartMonster
{
	//keep track of the last 3 players I dropped an object next to
	RMPlayer* lastPlayers[3];
	unsigned short invIndex;
public:
	MinesDropper();
	MinesDropper( unsigned short invType );
	virtual ~MinesDropper();
	virtual void init ( void );
	virtual int normalLogic( void );
};

//this guy just kinda wanders around and looks pretty
class Miner : public SmartMonster
{
public:
	Miner();
	virtual ~Miner();
	virtual void init ( void );
};


//change this to MWActionTake, with an option to destroy the object
class MWActionMineTake : public MonsterAction
{
public:
	MWActionMineTake ( int mod, NPC *me, bool dungeonOverride = false );
	virtual int evaluate ( void );
	virtual int execute ( void );
};
#endif

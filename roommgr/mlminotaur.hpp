//
// mlminotaur.hpp
//
// Logic code for Minotaurs
//
// author - Michael Nicolella
//

#ifndef _MLMINOTAUR_HPP_
#define _MLMINOTAUR_HPP_

#include "mlsmarty.hpp"

class Minotaur : public SmartMonster
{
public:
	//bool combatAssigned;

	Minotaur();
	virtual ~Minotaur();

	virtual void init ( void );
	int isFriend ( WorldObject *object );

	void killedTarget ( void );
	void acquiredTarget ( void );

	//int combatLogic( void );
};

class MinotaurWarrior : public Minotaur
{
public:
	MinotaurWarrior();
	virtual ~MinotaurWarrior();

	void init ( void );
	int combatLogic( void );
};

class MinotaurMage : public Minotaur
{
public:
	MinotaurMage();
	virtual ~MinotaurMage();

	void init ( void );
	int combatLogic( void );
};

#endif

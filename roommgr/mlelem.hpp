//
// mlelem.hpp
//
// File that contains elemental logic code headers.
//
// Author: K. Sergent
//

#ifndef _MLELEM_HPP_
#define _MLELEM_HPP_

#include "mlsmarty.hpp"

class EarthElemental : public SmartMonster
{
public:
	EarthElemental();
	virtual ~EarthElemental();
	virtual void init ( void );
	virtual int spellAttack ( void );
};

class WaterElemental : public SmartMonster
{
public:
	WaterElemental();
	virtual ~WaterElemental();
	virtual void init ( void );
	virtual int spellAttack ( void );
};

class AirElemental : public SmartMonster
{
public:
	AirElemental();
	virtual ~AirElemental();
	virtual void init ( void );
	virtual int spellAttack ( void );
};

class FireElemental : public SmartMonster
{
public:
	FireElemental();
	virtual ~FireElemental();
	virtual void init ( void );
	virtual int spellAttack ( void );
};

#endif

//
// mlhousepet.hpp
//
// File that contains logic code for House Pets
//
// Author: Zack Kaiser
//

#ifndef _MLHOUSEPET_HPP_
#define _MLHOUSEPET_HPP_

#include "mlsmarty.hpp"

class HousePet : public SmartMonster
{
public:
	HousePet();
	virtual ~HousePet();

	virtual void init ( void );
};

#endif
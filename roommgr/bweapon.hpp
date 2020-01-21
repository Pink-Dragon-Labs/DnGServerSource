/*
	BWeapon class
	author: Stephen Nichols
*/

#ifndef _BWEAPON_HPP_
#define _BWEAPON_HPP_

#include "wobjectbase.hpp"
#include "dice.hpp"

// define weapon types
enum {
	_WEAPON_ONE_HANDED = 1,
	_WEAPON_TWO_HANDED,
};

class BCharacter;

class Weapon 
{
public:
	int damageType, speed, minDamage, maxDamage, modifier, pierce, hands, bonus, distance, isMissile, skillType, spellProcID, reverseProcID, forwardProcChance, reverseProcChance;

	Weapon();
	virtual ~Weapon();

	int getSkill ( BCharacter *bchar );
};

class BWeapon : public WorldObjectBase, public Weapon
{
public:
	static const int _DAMAGE_NORMAL				= 1;
	static const int _DAMAGE_FIRE				= 2;
	static const int _DAMAGE_COLD				= 4;
	static const int _DAMAGE_LIGHTNING		 	= 8;
	static const int _DAMAGE_ACID				= 16;
	static const int _DAMAGE_POISON				= 32;
	static const int _DAMAGE_STAMINA			= 64;
	static const int _DAMAGE_STEAL_STAMINA		= 128;
	static const int _DAMAGE_EXPERIENCE			= 256;
	static const int _DAMAGE_STEAL_EXPERIENCE	= 512;
	static const int _DAMAGE_STEAL_LIFE			= 1024;
	static const int _DAMAGE_RUST				= 2048;
	static const int _DAMAGE_ETHEREAL			= 4096;
	static const int _DAMAGE_STUN				= 8192;

	WorldObject *owner;
	int mask;

	BWeapon ( WorldObject *obj );
	virtual ~BWeapon();

	void copy ( WorldObjectBase *base );

	virtual void buildPacket ( PackedData *packet, int override = 0 );
	virtual void writeSCIData ( FILE *file );

	// verb interface functions
	int handlesAction ( int action );
	int perform ( int action, va_list *args );

	// verb action functions
	int bePutOn ( WorldObject *object );
	int beTakenOff ( WorldObject *object = NULL );
};

#endif

/*
	BHead class
	author: Stephen Nichols
*/

#ifndef _BHEAD_HPP_
#define _BHEAD_HPP_

#include "wobjectbase.hpp"

class HeadData
{
public:
	HeadData() {
		race = 0;
		sex = 0;
		headNumber = 0;
		eyeNumber = 0;
		hairNumber = 0;
		browNumber = 0;
		noseNumber = 0;
		earNumber = 0;
		mouthNumber = 0;
		faceHairNumber = 0;
		skinColor = 0;
		eyeColor = 0;
		hairColor = 0;
	};

	int race;
	int sex;
	int headNumber;
	int eyeNumber;
	int hairNumber;
	int browNumber;
	int noseNumber;
	int earNumber;
	int mouthNumber;
	int faceHairNumber;
	int skinColor;
	int eyeColor;
	int hairColor;
};

// class DBHead : public HeadData
// {
// };

class BHead : public WorldObjectBase, public HeadData
{
public:
	BHead ( WorldObject *obj );
	virtual ~BHead();

	void copy ( WorldObjectBase *base );

	virtual void buildPacket ( PackedData *packet, int override = 0 );
	virtual void fromPacket ( PackedData *packet );
	virtual void writeSCIData ( FILE *file );

	int valid ( void );
};

#endif

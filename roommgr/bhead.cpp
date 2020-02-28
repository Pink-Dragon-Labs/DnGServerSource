/* 
	BHead class	
	author: Stephen Nichols 
*/

#include "bcarry.hpp"
#include "roommgr.hpp"

BHead::BHead ( WorldObject *obj ) : WorldObjectBase ( obj )
{
	type = _BHEAD;
}

BHead::~BHead()
{
}

void BHead::copy ( WorldObjectBase *theBase )
{
	BHead *base = (BHead *)theBase;

	race = base->race;
	sex = base->sex;
	headNumber = base->headNumber;
	eyeNumber = base->eyeNumber;
	hairNumber = base->hairNumber;
	browNumber = base->browNumber;
	noseNumber = base->noseNumber;
	earNumber = base->earNumber;
	mouthNumber = base->mouthNumber;
	faceHairNumber = base->faceHairNumber;
	skinColor = base->skinColor;
	eyeColor = base->eyeColor;
	hairColor = base->hairColor;
}

int BHead::valid ( void )
{
	if ( sex != _SEX_MALE && sex != _SEX_FEMALE )
		return 0;

	if ( race < 0 || race >= _MAX_RACE ) 
		return 0;

	return 1;
}

//
// WARNING:  This packet format DOES NOT match that of the client and should
// never be changed. I must have been high the day I wrote this.
//

void BHead::buildPacket ( PackedData *packet, int override )
{
	packet->putByte ( race );
	packet->putByte ( sex );
	packet->putByte ( headNumber );
	packet->putByte ( eyeNumber );
	packet->putByte ( hairNumber );
	packet->putByte ( browNumber );
	packet->putByte ( noseNumber );
	packet->putByte ( earNumber );
	packet->putByte ( mouthNumber );
	packet->putByte ( faceHairNumber );
	packet->putByte ( skinColor );
	packet->putByte ( eyeColor );
	packet->putByte ( hairColor );
}

//
// WARNING:  This packet format DOES NOT match that of the client and should
// never be changed. I must have been high the day I wrote this.
//

void BHead::fromPacket ( PackedData *packet )
{
	race = packet->getByte(); 
	sex =  packet->getByte();
	headNumber =  packet->getByte();
	eyeNumber =  packet->getByte();
	hairNumber =  packet->getByte();
	browNumber =  packet->getByte();
	noseNumber =  packet->getByte();
	earNumber =  packet->getByte();
	mouthNumber =  packet->getByte();
	faceHairNumber =  packet->getByte();
	skinColor =  packet->getByte();
	eyeColor =  packet->getByte();
	hairColor =  packet->getByte();
}

void BHead::writeSCIData ( FILE *file )
{
	fprintf ( file, "\t\t(aWhatObj addBase: BHead)\n" );
}

/*
	BCharacter class	
	author: Stephen Nichols
*/

#include "bcharacter.hpp"
#include "roommgr.hpp"

int minStatValues[ _MAX_PROFESSION ][ _MAX_RACE ][ 4 ] = {

	// Adventurer
	{
		{ 7, 6, 6, 6 },			//	Human
		{ 7, 6, 6, 6 },			//	Orc
		{ 7, 6, 6, 6 },			//	Giant
		{ 7, 6, 6, 6 },			//	Elf
	},

	// Warrior
	{
		{ 9, 7,  2, 7 },			//	Human
		{ 9, 7,  2, 7 },			//	Orc
		{ 9, 7,  2, 7 },			//	Giant
		{ 9, 7,  2, 7 },			//	Elf  
	},

	// Wizard
	{
		{  3, 7, 10,  5 },			//	Human
		{  3, 7, 10,  5 },			//	Orc
		{  3, 7, 10,  5 },			//	Giant
		{  3, 7, 10,  5 },			//	Elf  
	},

	// Thief
	{
		{  6, 9,  4, 6 },			//	Human
		{  6, 9,  4, 6 },			//	Orc
		{  6, 9,  4, 6 },			//	Giant
		{  6, 9,  4, 6 },			//	Elf  
	}
};

BCharacter::BCharacter ( WorldObject *obj ) : WorldObjectBase ( obj )
{
	type = _BCHARACTER;
	profession = _PROF_WARRIOR;
	experience = 0;
	race = _RACE_HUMAN;
	sex = _SEX_MALE;
	buildPoints = 0;
	homeTown = -1;
	version = _CHAR_VERSION;

	stealingCount = 0;
	stealingUnserved = 0;
	killingCount = 0;
	killingUnserved = 0;
	peaceful = 0;

	questNumber = 0;
	questState = 0;
	warnCount = 0;
	playerKills = 0;
	npcKills = 0;
	topLevel = 1;

	memset ( properName, 0, sizeof ( properName ) );
	memset ( title, 0, sizeof ( title ) );
	memset ( skills, 0, sizeof ( skills ) );
	memset ( spells, 0, sizeof ( spells ) );

	wearMask = 0;

	lastDungeon = 0;
}

BCharacter::~BCharacter()
{
}

void BCharacter::copy ( WorldObjectBase *theBase )
{
	BCharacter *base = (BCharacter *)theBase;

	profession = base->profession;
	experience = base->experience;
	race = base->race;
	sex = base->sex;
	buildPoints = base->buildPoints;
	homeTown = base->homeTown;
	topLevel = base->topLevel;

	memcpy ( properName, base->properName, sizeof ( properName ) );
	memcpy ( title, base->title, sizeof ( title ) );
	memcpy ( skills, base->skills, sizeof ( skills ) );
	memcpy ( spells, base->spells, sizeof ( spells ) );
	wearMask = base->wearMask;
}

void BCharacter::buildPacket ( PackedData *packet, int override )
{
	packet->putByte ( profession );
	packet->putByte ( race );
	packet->putByte ( sex );
	packet->putString ( properName );
	packet->putByte ( peaceful );

	// adjusted the size of health and healthMax BEW
	packet->putLong ( self->health );
	packet->putLong ( self->healthMax );
}

void BCharacter::writeSCIData ( FILE *file )
{
	fprintf ( file, "\t\t(aWhatObj addBase: BCharacter)\n" );
}

// set the wear mask based on profession, sex, race and alignment
void BCharacter::setWearMask ( void )
{
	wearMask = 0;

	// set the sex bit
	wearMask |= sexWearMaskTable[sex];

	// set the profession bit
	wearMask |= profWearMaskTable[profession];

	// set the race bit
	wearMask |= raceWearMaskTable[race];

	// set the alignment bit
	wearMask |= alignmentWearMaskTable[self->coarseAlignment()];

}

// gain positive or negative experience points
void BCharacter::gainExperience ( int exp, PackedData *movie )
{
	// Serenia change
	// int maxLevel = 100;
	int maxLevel = 10000;

	if ( !self->player || self->player->isNPC )
		return;

	if ( exp >= 0 ) {

		self->level = getLevel();

		// cap at max level
		if ( self->level > maxLevel ) {
			self->level = maxLevel;
		} else{
			experience += exp;
		}

		if ( movie ) {
			movie->putByte ( _MOVIE_GAIN_EXP );
			movie->putLong ( self->servID );
			movie->putLong ( exp );
		}

		// check for level gain
		int neededExp = _EP_PER_LEVEL * self->level;

		while ( experience >= neededExp && self->level < maxLevel ) {
	 		advanceLevel ( movie );
			neededExp = _EP_PER_LEVEL * self->level;
		}

		// store top level 
		if ( self->level > topLevel )
			topLevel = self->level;

	}

	else if ( exp < 0 ) {
		// don't allow a loss of more than _MAX_EXP_LOSS
		experience += exp;

		if ( experience < 0)
			experience = 0;

		if ( movie ) {
			movie->putByte ( _MOVIE_GAIN_EXP );
			movie->putLong ( self->servID );
			movie->putLong ( exp );
		}

		// Set new lower level if they lost enough, never allow less than level 1

		int tLevel = getLevel();

		if ( tLevel >= 1 && tLevel < self->level ) {
			
			if ( self->player ) {
				int amount = self->level - tLevel;
				roomMgr->sendPlayerText ( self->player, "|c60|You have lost %d level%s You are now level %d|c43|", amount, amount > 1? "s!":"!",tLevel );
			}

			if ( !topLevel)
				topLevel = self->level;

			self->level = tLevel;

			self->calcHealth();
			self->calcStamina();

			// always lose health if drop in level and health is over max

			if ( self->health > self->healthMax ) 
				self->health = self->healthMax;
			
			if ( movie ) {
				movie->putByte ( _MOVIE_HEALTH_MAX );
				movie->putLong ( self->servID );
				movie->putLong ( self->servID );

				// adjusted the size of the health  BEW
				movie->putLong ( self->healthMax );
			}
		}
	}
}

// advance to the next level
void BCharacter::advanceLevel ( PackedData *movie )
{
	// increase the level and update my experience to be the minimum required for that level

	int neededExp = _EP_PER_LEVEL * self->level;

	self->level++;

	if ( experience < neededExp )
		experience = neededExp;

// not used?
//	profession_t *ptr = &gProfessionTable[self->profession()];

	self->calcHealth();
	self->calcStamina();

	if ( movie ) {
		movie->putByte ( _MOVIE_GAIN_LEVEL );
		movie->putLong ( self->servID );
		movie->putWord ( self->level );

		// adjusted the size of the health  BEW
		movie->putLong ( self->healthMax );
		movie->putWord ( 0 );
	}

	if ( self->level <= topLevel )
		return;

	// increment the available build points

	buildPoints++; 

	if ( !self->player )
		return;

	if ( self->level < 0 ) {
		roomMgr->sendSystemMsg ( "Congratulations!", self->player, "You are incredibly feeble. However, we are merciful and your death will be painless." );
		self->player->forceLogout();
	}

	if ( self->level == 3 ) {
		WorldObject *object = self->addObject ( "BlueBaldric" ); 

		if ( object )
			object->makeVisible ( 1 );

		roomMgr->sendSystemMsg ( "Congratulations!", self->player, "You have advanced to level three. With this new level comes some additional challenges for you to overcome.  If you are killed from this point on, you will lose 1000 experience points and you may also lose items in your inventory.  May you have much luck!" );
	}

	else if ( self->level == 7 ) {
		WorldObject *object = self->addObject ( "RedBaldric" );

		if ( object )
			object->makeVisible ( 1 );
	}

	else if ( self->level == 15 ) {
		WorldObject *object = self->addObject ( "BrownBaldric" );

		if ( object )
			object->makeVisible ( 1 );
	}

	else if ( self->level == 25 ) {
		WorldObject *object = self->addObject ( "GrayBaldric" );

		if ( object )
			object->makeVisible ( 1 );
	}

	else if ( self->level == 50 ) {
		WorldObject *object = self->addObject ( "GoldBaldric" );

		if ( object )
			object->makeVisible ( 1 );

		roomMgr->sendSystemMsg ( "Congratulations!", self->player, "You have just earned your new permanent house!" );
	}

	else if ( self->level == 100 ) {
		WorldObject *object = self->addObject ( "MagentaBaldric" );

		if ( object )
			object->makeVisible ( 1 );
	}

	else if ( self->level == 150 ) {
		WorldObject *object = self->addObject ( "YellowBaldric" );

		if ( object )
			object->makeVisible ( 1 );
	}

	else if ( self->level == 200 ) {
		WorldObject *object = self->addObject ( "GreenBaldric" );

		if ( object )
			object->makeVisible ( 1 );
	}

	else if ( self->level == 300 ) {
		WorldObject *object = self->addObject ( "AmberBaldric" );

		if ( object )
			object->makeVisible ( 1 );
	}

	else if ( self->level == 500 ) {
		WorldObject *object = self->addObject ( "RoyalBaldric" );

		if ( object )
			object->makeVisible ( 1 );

		object = self->addObject ( "500ChestReward" );

		if ( object )
			object->makeVisible ( 1 );
		
		roomMgr->sendSystemMsg ( "Congratulations!", self->player, "You have hit a milestone and have earned a large chest to store your treasure in!" );
	}

	else if ( self->level == 750 ) {
		WorldObject *object = self->addObject ( "SkyBlueBaldric" );

		if ( object )
			object->makeVisible ( 1 );
	}

	else if ( self->level == 1000 ) {
		roomMgr->sendSystemMsg ( "Retire?", self->player, "Congratulations.. you have reached the zenith of your profession! Consider retirement, for there are no more challenges beyond this level." );
		WorldObject *object = self->addObject ( "SatoriBaldric" );

		if ( object )
			object->makeVisible ( 1 );
	}

	else if ( self->level == 1001 ) {
		roomMgr->sendSystemMsg ( "What?", self->player, "You have surpassed anybody before you. You venture on into uncharted territory of power.. beware." );
	}

	else if ( self->level == 1500 ) {
		roomMgr->sendSystemMsg ( "Illustrious!", self->player, "Incredible accomplishment! There are still more adventures ahead of you traveller, Good Luck and well done!" );
		WorldObject *object = self->addObject ( "IllustriousBaldric" );

		if ( object )
			object->makeVisible ( 1 );
	}

	else if ( self->level == 2000 ) {
		roomMgr->sendSystemMsg ( "Unbelievable!", self->player, "Incredible! Amazing! The dedication to achieve such power! You truly are impressive, you have conquered yet another milestone. Well done!" );
		WorldObject *object = self->addObject ( "AmazingBaldric" );

		if ( object )
			object->makeVisible ( 1 );
	}

	else if ( self->level == 3000 ) {
		WorldObject *object = self->addObject ( "3000Baldric" );

		if ( object )
			object->makeVisible ( 1 );

		object = self->addObject ( "3000ChestReward" );

		if ( object )
			object->makeVisible ( 1 );
		
		roomMgr->sendSystemMsg ( "Unbelievable!", self->player, "What an accomplishment! This milestone comes with magnificent rewards! The Tahali is an exotic sash from overseas, and you have recieved a new, larger storage chest!" );
	}

	else if ( self->level == 4000 ) {
		WorldObject *object = self->addObject ( "4000Baldric" );

		if ( object )
			object->makeVisible ( 1 );

		object = self->addObject ( "4000ChestReward" );

		if ( object )
			object->makeVisible ( 1 );
		
		roomMgr->sendSystemMsg ( "Unbelievable!", self->player, "What an accomplishment! This milestone comes with magnificent rewards! The Magic Ward is a very powerful and mystical sash, and you have recieved a new, even larger storage chest!" );

        
	}

	else if ( self->level == 5000 ) {
		WorldObject *object = self->addObject ( "5000Baldric" );

		if ( object )
			object->makeVisible ( 1 );

		object = self->addObject ( "5000ChestReward" );

		if ( object )
			object->makeVisible ( 1 );
		
		roomMgr->sendSystemMsg ( "Remarkable!", self->player, "What an accomplishment! This milestone comes with magnificent rewards! The Catalyst is said to aid in the ability to enchant armor and weapons, and you have recieved the largest storage chest!" );
	}

	else if ( self->level == 7500 ) {
		WorldObject *object = self->addObject ( "7500Baldric" );

		if ( object )
			object->makeVisible ( 1 );

		object = self->addObject ( "VoidPouch" );

		if ( object )
			object->makeVisible ( 1 );
		
		roomMgr->sendSystemMsg ( "Astounding!", self->player, "Incredible! You just continue to grow in power! Prime is an ancient baldric that will ensure you strike first in combat, and will grant you a signficant boost to all of your stats! You have also earned a Void Pouch! These small pouches are to not be taken lightly.. they can hold a seemingly limitless amount of things.." );
	}
	


//	self->writeLevelData();
}

int BCharacter::getSkill ( int skill )
{
	return skills[skill];
}

void BCharacter::setSkill ( int skill, int value )
{
	skills[skill] = value;
}

int BCharacter::knowsSpell ( int spell ) 
{
	return spells[spell];
}

int BCharacter::learnSpell ( int spell )
{
	spells[spell] = 1;
	return 0;
}

int BCharacter::getLevel ( void )
{
	int level = ( experience / _EP_PER_LEVEL ) + 1;

	// level should now set to 1 if zero or less than zero 
	if ( level <= 0 )
		level = 1;
	
	return level;
}


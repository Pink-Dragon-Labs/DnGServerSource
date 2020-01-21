/*
	BCharacter class
	author: Stephen Nichols
*/

#ifndef _BCHARACTER_HPP_
#define _BCHARACTER_HPP_

#define _CHAR_VERSION 1
#define _EP_PER_LEVEL 10000

#include "wobjectbase.hpp"
#include "magic.hpp"
#include "skills.hpp"
#include "playerstate.hpp"

// client profession types
enum {
	_PROF_ADVENTURER,
	_PROF_WARRIOR,
	_PROF_WIZARD,
	_PROF_THIEF,
	_MAX_PROFESSION
};

#define _MAX_EXP_GAIN	30000 		// 30k | 3 levels
#define _MAX_EXP_LOSS	-50000		// 50k | 5 levels
#define _CHAR_MONEY_START 0 //150
#define _CHAR_MANA_START 0 //50

#define _MAX_ATTRIBUTE	30

// sex types
enum {
	_SEX_MALE,
	_SEX_FEMALE,
	_MAX_SEX
};

// race types
enum {
	_RACE_HUMAN,
	_RACE_DWARF,
	_RACE_GIANT,
	_RACE_ELF,
	_MAX_RACE
};

// stat types
enum {
	_STAT_STRENGTH,
	_STAT_DEXTERITY,
	_STAT_INTELLIGENCE,
	_STAT_ENDURANCE,
	_MAX_STAT
};

extern int minStatValues[ _MAX_PROFESSION ][ _MAX_RACE ][ _MAX_STAT ];

class BCharacter : public WorldObjectBase, public CPlayerState
{
public:
	char properName[32], title[32];

	int profession, experience, race, sex, buildPoints, homeTown, stealingCount, killingCount, stealingUnserved, killingUnserved, peaceful, version, questNumber, questState, warnCount, playerKills, npcKills;

	int topLevel; 
	int lastDungeon;

	char spells[_SPELL_MAX], skills[_SKILL_MAX];
	int wearMask;

	BCharacter ( WorldObject *obj );
	virtual ~BCharacter();

	void copy ( WorldObjectBase *base );

	virtual void buildPacket ( PackedData *packet, int override = 0 );
	virtual void writeSCIData ( FILE *file );

	void setWearMask ( void );

	void gainExperience ( int exp, PackedData *movie = NULL );
	void advanceLevel ( PackedData *movie = NULL );
	int getLevel ( void );

	int getSkill ( int skill );
	void setSkill ( int skill, int value );
	int knowsSpell ( int spell );
	int learnSpell ( int spell );
};

#endif

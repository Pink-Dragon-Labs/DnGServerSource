/*
	global tables
	author: Stephen Nichols
*/

#ifndef _TABLES_HPP_
#define _TABLES_HPP_

#include "bcharacter.hpp"
#include "wobject.hpp"

// this is the conversion table for sex values to wear masks
extern int sexWearMaskTable[];

// this is the conversion table for race values to wear masks
extern int raceWearMaskTable[];

// this is the conversion table for profession values to wear masks
extern int profWearMaskTable[];

// this is the conversion table for coarse alignment values to wear masks
extern int alignmentWearMaskTable[];

// define the title_t struct
typedef struct {
	char *maleTitle, *femaleTitle;
	int experience;
} title_t;

// this is the character class name table
extern char *characterClassTable[_MAX_PROFESSION][_MAX_RACE][_MAX_SEX];

// this is the absorption table for armor
#define _MAX_AC 21
extern int armorClassAbsorptionTable[_MAX_AC];

// this is the endurance to health increase table
extern int enduranceToHealthIncTable[_MAX_ATTRIBUTE];

// this is the intelligence to mana increase table
extern int intelligenceToManaIncTable[_MAX_ATTRIBUTE];

// define the modifier_t struct
typedef struct {
	int healthGain;
	int manaGain;
} modifier_t;

// this is the profession modifier table
extern modifier_t professionModifierTable[_MAX_PROFESSION];

enum {
	_TRAP_POISON,
	_TRAP_FLAME_ORB,
	_TRAP_LIGHT_DART,
	_TRAP_EXPLOSION,
	_TRAP_WHIRLWIND,
	_TRAP_DEATH_BLADE,
	_TRAP_WEAKEN,
	_TRAP_SLOW,
	_TRAP_DRAIN_MANA,
	_TRAP_ICE_STORM,
	_TRAP_LIGHTNING,
	_TRAP_MAX,
};

extern char *gTrapNames[_TRAP_MAX];

typedef struct {
	int basePercent;
	int maxPercent;
	double increment;
} save_t;

extern save_t gSpellSaveTable[_MAX_PROFESSION];
extern save_t gParalysisSaveTable[_MAX_PROFESSION];
extern save_t gPoisonSaveTable[_MAX_PROFESSION];
extern save_t gAttackSaveTable[_MAX_PROFESSION];

typedef struct {
	int level;
	char *name;
} monster_t;

extern monster_t gMonsterTable[];

typedef struct {
	int level;
	char *name;
} treasure_t;

extern treasure_t gTreasureTable[];

#define _SEASONS_PER_YEAR		6
#define _DAYS_PER_SEASON		45
#define _DAYS_PER_YEAR 			(_SEASONS_PER_YEAR * _DAYS_PER_SEASON)

extern char *gSeasonTable[_SEASONS_PER_YEAR];
extern char *gYearTable[];

typedef struct {
	int hitChance;
	int damage;
} strbonus_t;

#define _MAX_STRENGTH	50
extern strbonus_t gStrBonus[_MAX_STRENGTH];

typedef struct {
	int minHealthGain;
	int maxHealthGain;
	int minManaGain;
	int maxManaGain;
} profession_t;

extern profession_t gProfessionTable[_MAX_PROFESSION];

typedef struct {
	int ability;

	union {
		double numAttacks;
		double numDodges;
	};
} skill_t;

extern skill_t gSkillInfo[_SKILL_MAX][_SKILL_LVL_MAX];

typedef struct {
	char *hitStr;
	char *magStr;
} combatStr_t;

#define _MAX_COLOR 28
extern int gColorTable[_MAX_COLOR];

extern combatStr_t gRatCombatStrTbl[5];

extern combatStr_t gUnarmedCombatStrTbl[5];
extern combatStr_t gSwordCombatStrTbl[5];
extern combatStr_t gClubCombatStrTbl[5];

extern char *gAffectLostTbl[];

enum {
	dtPierce,
	dtSlash,
	dtBludgeon,
	dtMax
};

enum {
	atNone,
	atLeather,
	atChain,
	atPlate,
	atMax
};

extern int gArmorModTbl[dtMax][atMax];
extern int gDamagePassTbl[dtMax][atMax];

#endif

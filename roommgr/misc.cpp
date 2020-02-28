/*
	MISC.CPP
	Miscellaneous utility functions.

	Author: Stephen Nichols
*/

#include "roommgr.hpp"

int gValidForwardProc[] =
{
    1,
    3,
    5,
    8,
    18,
    19,
    22,
    23,
    24,
    25,
    26, //10
    27,
    28,
    29,
    30,
    34,
    36,
    37,
    38,
    39,
    40, //20
    41,
    42,
    43,
    44,
    45,
    46,
    47,
    48,
    49,
    50, //30
    51,
    52,
    53,
    54,
    55,
    56,
    59,
    61,
    64,
    66, //40
    67,
    68,
    69,
    70,
    71,
    72,
    76,
    77,
    80,
    90, //50
    91,
    92,
    95,
    106,
    108,
    110,
    111,
    115,
    117,
    120, //60
    125,
    126,
    127,
    128,
    142,
    143,
    144,
    145 //68
};

int gValidReverseProc[] =
{
    11, // 0
    12,
    13,
    14,
    20,
    31,
    63,
    75,
    78,
    79,
    81, // 10
    82,
    84,
    85,
    87,
    93,
    94,
    96,
    97,
    98,
    100, //20
    102,
    103 //22
};

int gZerkProc[] =
{
	25, // Flame Orb
    27, // Ice Orb
	30, // Gust of Wind
    36, // Sandstorm
    37, // Spark
    38, // Ice Storm
    42, // Stoning
    43, // Fireball
    44, // Lightning Bolt
    46, // Crushing Boulder
	47, // Electric Fury
	49, // Earthquake
    53, // Psychic Orb
	75, // Light Dart
    108, // Poison Bolt
    110, // Drain Life
    111, // Acid Cloud
    115, // Acid Raid
    117, // Steal Life
    120, // Death Touch
	124, // Duach's Vengeance
    125, // Death Wish
	126, // Mass Drain
    144 // Burning Mistcloud
};


typedef struct 
{
	char *text;
	int verb;
} VerbEntry;

VerbEntry verbTable[] = {
	{"vTake", vTake},
	{"vBeTakenBy", vBeTakenBy},
	{"vDrop", vDrop},
	{"vBeDroppedBy", vBeDroppedBy},
	{"vPutOn", vPutOn},
	{"vBePutOn", vBePutOn},
	{"vTakeOff", vTakeOff},
	{"vBeTakenOff", vBeTakenOff},
	{"vPutIn", vPutIn},
	{"vBePutIn", vBePutIn},
	{"vClose", vClose},
	{"vBeClosed", vBeClosed},
	{"vOpen", vOpen},
	{"vBeOpened", vBeOpened},
	{"vBeActivated", vBeOpened},
	{"vBeDeactivated", vBeClosed},
	{"vLock", vLock},
	{"vBeLocked", vBeLocked},
	{"vUnlock", vUnlock},
	{"vBeUnlocked", vBeUnlocked},
	{"vGive", vGive},
	{"vBeGivenTo", vBeGivenTo},
	{"vConsume", vConsume},
	{"vBeConsumed", vBeConsumed},
	{"vBeAnswered", vBeAnswered},
	{"vUse", vUse},
	{"vBeUsed", vBeUsed},
	{"vBeSatOn", vBeSatOn},
	{"vBeStoodUpOn", vBeStoodUpOn},
	{"vBeDisarmed", vBeConsumed},
	{"vBeBorn", vBeBorn},
	{NULL, -1}
};

int textToVerb ( char *text )
{
	int index = 0;

	while ( verbTable[index].text ) {
		if ( !strcmp ( text, verbTable[index].text ) )
			return verbTable[index].verb;

		index++;
	}

	return -1;
}

typedef struct {
	char *text;
	int mask;
} MaskEntry;

MaskEntry maskTable[] = {
	{"male",		_WEAR_MASK_MALE},
	{"female",		_WEAR_MASK_FEMALE},
	{"human",		_WEAR_MASK_HUMAN},
	{"orc",			_WEAR_MASK_ORC},
	{"giant",		_WEAR_MASK_GIANT},
	{"elf",			_WEAR_MASK_ELF},
	{"good",		_WEAR_MASK_GOOD},
	{"neutral",		_WEAR_MASK_NEUTRAL},
	{"evil",		_WEAR_MASK_EVIL},
	{"adventurer",	_WEAR_MASK_ADVENTURER},
	{"warrior",		_WEAR_MASK_WARRIOR},
	{"thief",		_WEAR_MASK_THIEF},
	{"wizard",		_WEAR_MASK_WIZARD},
	{NULL, -1}
};

int calcChance(int baseValue)
{
    return random(baseValue, 100);
}

int textToWearMask ( char *text )
{
	int index = 0;

	while ( maskTable[index].text ) {
		if ( !strcmp ( text, maskTable[index].text ) )
			return maskTable[index].mask;

		index++;
	}

	return -1;
}

typedef struct {
	char *name;
	action_t function;
} ActionEntry;

ActionEntry actionTable[] = {
	{"teleport", actTeleport},
	{"damage", actDamage},
	{"heal", actHeal},
	{"setAffect", actSetAffect},
	{"clearAffect", actClearAffect},
	{"kill", actKill},
	{"forceDrop", actForceDrop},
	{"damn", actDamn},
	{"inspire", actInspire},
	{"changeExperience", actChangeExperience},
	{"open", actOpen},
	{"close", actClose},
	{"lock", actLock},
	{"unlock", actUnlock},
	{"activate", actActivate},
	{"deactivate", actDeactivate},
	{"enable", actEnable},
	{"disable", actDisable},
	{"createObj", actCreateObj},
	{"summon", actSummon},
	{"floatingText", actFloatingText},
	{"zoneMsg", actZoneMsg},
	{"roomMsg", actRoomMsg},
	{"playSound", actPlaySound},
	{"playMusic", actPlayMusic},
	{"castSpell", actCastSpell},
	{"castSpellRoom", actCastSpellRoom},
	{"destroyObj", actDestroyObj},
	{"timedOpen", actTimedOpen},
	{"clearActions", actClearActions},
	{"addAction", actAddAction},
	{"enidMark", actEnidMark},
	{"duachMark", actDuachMark},
	{"despothesMark", actDespothesMark},
	{"changeManaDrain", actChangeManaDrain},
	{"changeMeleePhase", actChangeMeleePhase},
	{"changeEvilMDMMod", actChangeEvilMDMMod},
	{"changeGoodMDMMod", actChangeGoodMDMMod},
	{"changeMeleeArmorPiercing", actChangeMeleeArmorPiercing},
	{"changeMystImmunityCount", actChangeMystImmunityCount},
	{"changeCastResistance", actChangeCastResistance},
	{"changeSpellResistance", actChangeSpellResistance},
	{"changeSDM", actChangeSDM},
	{"minotaurSoulConsume", actMinotaurSoulConsume},
    {"addForwardProc", actAddForwardProc},
    {"addForwardProcChance", actAddForwardProcChance},
    {"addReverseProc", actAddReverseProc},
    {"addReverseProcChance", actAddReverseProcChance},
	{"gemChest-vClose", actGemChest_vClose},
	{"gemChest-vLock",  actGemChest_vLock },
	
	{NULL, NULL}
};

action_t getActionFunction ( char *text )
{
	int index = 0;

	while ( actionTable[index].name ) {
		if ( !strcmp ( text, actionTable[index].name ) )
			return actionTable[index].function;

		index++;
	}

	return NULL;
}

typedef struct {
	char *name;
	WorldObject *object;
} ObjectEntry;

ObjectEntry objectTable[] = {
	{"WorldObject", (WorldObject *)1},
	{NULL, NULL}
};

WorldObject *textToObject ( const char *text )
{
	int index = 0;

	while ( objectTable[index].name ) {
		if ( !strcmp ( text, objectTable[index].name ) )
			return objectTable[index].object;

		index++;
	}

	return NULL;
}

typedef struct {
	char *name;
	int skill;
} SkillEntry;

SkillEntry skillLevelTable[] = {
	{"ignorant", _SKILL_LVL_IGNORANT},
	{"familiar", _SKILL_LVL_FAMILIAR},
	{"proficient", _SKILL_LVL_PROFICIENT},
	{"expert", _SKILL_LVL_EXPERT},
	{"master", _SKILL_LVL_MASTER},
	{"grandMaster", _SKILL_LVL_GRAND_MASTER},
	{"paragon", _SKILL_LVL_PARAGON},
	{NULL, -1}
};

int textToSkillLevel ( char *text )
{
	int index = 0;

	while ( skillLevelTable[index].name ) {
		if ( !strcmp ( text, skillLevelTable[index].name ) )
			return skillLevelTable[index].skill;

		index++;
	}

	return -1;
}

SkillEntry skillTable[] = {
	{"shortsword", _SKILL_SHORT_SWORD},
	{"longsword", _SKILL_LONG_SWORD},
	{"twohandedsword", _SKILL_TWOHANDED_SWORD},
	{"dagger", _SKILL_DAGGER},
	{"axe", _SKILL_AXE},
	{"mace", _SKILL_MACE},
	{"unarmed", _SKILL_UNARMED},
	{"throwing", _SKILL_THROWING},
	{"acrobatics", _SKILL_ACROBATICS},
	{"maul", _SKILL_MAUL},
	{"sorcery", _SKILL_SORCERY},
	{"elementalism", _SKILL_ELEMENTALISM},
	{"mysticism", _SKILL_MYSTICISM},
	{"thaumaturgy", _SKILL_THAUMATURGY},
	{"necromancy", _SKILL_NECROMANCY},
	{"theurgism", _SKILL_THEURGISM},
	{"alchemy", _SKILL_ALCHEMY},
	{"weaponsmith", _SKILL_WEAPONSMITH},
	{"armorer", _SKILL_ARMORER},
	{"tracking", _SKILL_TRACKING},
	{"healer", _SKILL_HEALER},
	{"broadsword", _SKILL_BROADSWORD},
	{"pickpockets", _SKILL_PICK_POCKETS},
	{"detecttraps", _SKILL_DETECT_TRAPS},
	{"meditation", _SKILL_MEDITATION},
	{"criticalstriking", _SKILL_CRITICAL_STRIKING},
	{"shielduse", _SKILL_SHIELD_USE},
	{NULL, -1}
};

int textToSkill ( char *text )
{
	int index = 0;

	while ( skillTable[index].name ) {
		if ( !strcmp ( text, skillTable[index].name ) )
			return skillTable[index].skill;

		index++;
	}

	return -1;
}

int exitToDir ( int exit )
{
	switch ( exit ) {
		case 1:
			return 0;

		case 2:
			return 1;

		case 4:
			return 2;

		case 8:
			return 3;
	}

	return 0;
}

int reverseLoop ( int loop )
{
	switch ( loop ) {
		case 0:
			return 1;

		case 1: 
			return 0;

		case 2: 
			return 3;

		case 3: 
			return 2;
	}

	return loop;
}

int projectXViaLoop ( int loop, int len )
{
	switch ( loop ) {
		case 0:
			return len;

		case 1:
			return -len;
	}

	return 0;
}

int projectYViaLoop ( int loop, int len )
{
	switch ( loop ) {
		case 2:
			return len;

		case 3:
			return -len;
	}

	return 0;
}

int directionToLoop ( int direction )
{
	switch ( direction ) {
		case 0:
			return 2;
			break;

		case 1:
			return 3;
			break;

		case 2: 
			return 0;
			break;

		case 3: 
			return 1;
			break;
	}

	return 0;
}

int calcCombatMod ( WorldObject *character )
{
	int percent = 100;

	int diff = character->calcDexterity() - 10;

	if ( diff < 0 ) {
		percent = 100 + (5 * diff);
	} else {
		percent = 100 + (10 * diff);
	}

	return percent;
}

int isValidForwardProcID(int id)
{
    int gSize = sizeof(gValidForwardProc) / sizeof(int);

    int i;

    for(i=0;i <gSize;i++)
    {
        if(gValidForwardProc[i] == id)
            return gValidForwardProc[i];
    }

    return -1;
}


int isValidReverseProcID(int id)
{
    int gSize = sizeof(gValidReverseProc) / sizeof(int);

    int i;

    for(i=0;i <gSize;i++)
    {
        if(gValidReverseProc[i] == id)
            return gValidReverseProc[i];
    }

    return -1;
}

int getRandomForwardProc()
{
    int fProcSize = sizeof(gZerkProc) / sizeof(int);
    return gZerkProc[rand() % fProcSize];
}
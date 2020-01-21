/*
	global tables
	author: Stephen Nichols
*/

#include "roommgr.hpp"

// this is the conversion table for sex values to wear masks
int sexWearMaskTable[] = {
	_WEAR_MASK_MALE,
	_WEAR_MASK_FEMALE,
};

// this is the conversion table for race values to wear masks
int raceWearMaskTable[] = {
	_WEAR_MASK_HUMAN,
	0,	//dwarf
	_WEAR_MASK_GIANT,
	_WEAR_MASK_ELF
};

// this is the conversion table for profession values to wear masks
int profWearMaskTable[] = {
	_WEAR_MASK_ADVENTURER,
	_WEAR_MASK_WARRIOR,
	_WEAR_MASK_WIZARD,
	_WEAR_MASK_THIEF,
};

// this is the conversion table for coarse alignment values to wear masks
int alignmentWearMaskTable[] = {
	_WEAR_MASK_GOOD,
	_WEAR_MASK_NEUTRAL,
	_WEAR_MASK_EVIL,
};

// this is the table of character class names
char *characterClassTable[_MAX_PROFESSION][_MAX_RACE][_MAX_SEX] = {
	// adventurer
	{
		// human
		{"MaleAdventurer", "FemaleAdventurer"},

		// dwarf
		{"MaleAdventurer", "FemaleAdventurer"},

		// giant
		{"MaleAdventurer", "FemaleAdventurer"},

		// elf
		{"MaleAdventurer", "FemaleAdventurer"},
	},

	// warrior
	{
		// human
		{"MaleWarrior", "FemaleWarrior"},

		// dwarf
		{"MaleWarrior", "FemaleWarrior"},

		// giant
		{"MaleWarrior", "FemaleWarrior"},

		// elf
		{"MaleWarrior", "FemaleWarrior"},
	},

	// wizard
	{
		// human
		{"MaleWizard", "FemaleWizard"},

		// dwarf
		{"MaleWizard", "FemaleWizard"},

		// giant
		{"MaleWizard", "FemaleWizard"},

		// elf
		{"MaleWizard", "FemaleWizard"},
	},

	// thief
	{
		// human
		{"MaleThief", "FemaleThief"},

		// dwarf
		{"MaleThief", "FemaleThief"},

		// giant
		{"MaleThief", "FemaleThief"},

		// elf
		{"MaleThief", "FemaleThief"},
	},
};

// this is the profession modifier table
modifier_t professionModifierTable[_MAX_PROFESSION] = {
	// adventurer
	{ 0, 0 },

	// warrior
	{ 4, -2 },

	// wizard
	{ -2, 4 },

	// thief
	{ 1, 0 },
};

char *gTrapNames[_TRAP_MAX] = {
	"poison trap",
	"flame orb trap",
	"light dart trap",
	"explosion trap",
	"whirlwind trap",
	"death blade trap",
	"weaken trap",
	"slow trap",
	"drain mana trap",
	"ice storm trap",
	"lightning trap"
};

save_t gSpellSaveTable[_MAX_PROFESSION] = {
	// adventurer
//	{ 30, 70, 0.40 },
	{ 10, 50, 0.40 },

	// warrior
//	{ 10, 75, 0.65 },
	{ 0, 40, 0.40 },

	// wizard
//	{ 45, 85, 0.40 },
	{ 20, 60, 0.40 },

	// thief
//	{ 30, 80, 0.50 }
	{ 10, 55, 0.45 }
};

save_t gParalysisSaveTable[_MAX_PROFESSION] = {
	// adventurer
	{ 40, 80, 0.40 },

	// warrior
	{ 20, 85, 0.65 },

	// wizard
	{ 40, 80, 0.40 },

	// thief
	{ 45, 70, 0.25 }
};

save_t gPoisonSaveTable[_MAX_PROFESSION] = {
	// adventurer
	{ 55, 90, 0.35 },

	// warrior
	{ 25, 85, 0.60 },

	// wizard
	{ 35, 65, 0.30 },

	// thief
	{ 40, 65, 0.25 }
};

save_t gAttackSaveTable[_MAX_PROFESSION] = {
	// adventurer
	{ 25, 75, 0.50 },

	// warrior
	{ 40, 80, 0.40 },

	// wizard
	{ 15, 65, 0.50 },

	// thief
	{ 30, 75, 0.45 }
};

monster_t gMonsterTable[] = {
	{ 1, "Ratling" },
	{ 1, "Bat" },
	{ 1, "Zombie" },
	{ 2, "ArcticRat" },
	{ 2, "Wasp" },
	{ 3, "WharfRat" },
	{ 4, "SwampWasp" },
	{ 5, "FlameRat" },
	{ 6, "HiveWarrior" },
	{ 7, "Wolf" },
	{ 7, "HiveMage" },
	{ 8, "DemonRat" },
	{ 9, "Troll" },
	{ 10, "BlackWolf" },
	{ 10, "Brownie" },
	{ 11, "SwampImp" },
	{ 12, "Wraith" }, 
	{ 12, "HellHound" },
	{ 13, "TrollWarrior" },
	{ 14, "WaterNixie" },
	{ 15, "ForestImp" },
	{ 15, "WhiteWolf" },
	{ 16, "TrollMage" },
	{ 17, "WoodNymph" },
	{ 17, "TrollChief" },
	{ 18, "IceImp" },
	{ 19, "RockTroll" },
	{ 19, "ShadowFaery" },
	{ 20, "FlameWolf" },
	{ 20, "Banshee" },
	{ 21, "DemonTroll" },
	{ 21, "Faery" },
	{ 22, "Imp" },
	{ 23, "IceFaery" },
	{ 24, "FireImp" },
	{ 25, "StoneImp" },
	{ 26, "StormFaery" },
	{ 27, "FireFaery" },
	{ 28, "FaeryWarrior" },
	{ 29, "FaeryNoble" },
	{ 30, "RoyalFaery" },
	{ 31, "KingImp" },
	{ 32, "Fenris" },
	{ 33, "Ogre" },
	{ 34, "OchreFenris" },
	{ 35, "GrayFenris" },
	{ 36, "OgreWarrior" },
	{ 37, "FireDaemon" },
	{ 38, "OgreMage" },
	{ 39, "IceFenris" },
	{ 40, "GraniteOgre" },
	{ 41, "OgreChief" },
	{ 42, "BloodFenris" },
	{ 43, "BogDaemon" },
	{ 44, "GreenImp" },
	{ 45, "DarkOgre" },
	{ 46, "Daemon" },
	{ 47, "Kilrog" },
	{ 48, "GreySeraph" },
	{ 49, "GraniteDaemon" },
	{ 50, "FrostSeraph" },
	{ 51, "ForestSeraph" },
	{ 52, "FlameSeraph" },
	{ 53, "DarkDaemon" },
	{ 54, "Seraph" },
	{ 55, "ThunderSeraph" },
	{ 56, "SeraphWarrior" },
	{ 56, "DaemonKing" },
	{ 60, "KingKilrog" }
};

//treasure_t gTreasureTable[] = {
//	{
//};

char *gSeasonTable[_SEASONS_PER_YEAR] = {
	"Equality",
	"War",
	"Evil",
	"Justice",
	"Peace",
	"Magic",
};

char *gYearTable[] = {
	"Duach",
	"Finvarra",
	"Mabon",
	"Elphame",
	"Enid",
	"Despothes",
};

strbonus_t gStrBonus[_MAX_STRENGTH] = {
	{-25, -5},
	{-15, -4},
	{-5, -3},
	{0, -2},
	{0, -1},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{5, 1},
	{6, 1},
	{7, 1},
	{8, 2},
	{9, 2},
	{10, 3},
	{10, 3},
	{10, 3},
	{10, 3},
	{10, 3},
	{11, 4},
	{11, 4},
	{11, 4},
	{11, 4},
	{11, 4},
	{11, 4},
	{11, 4},
	{12, 5},
	{12, 5},
	{12, 5},
	{12, 5},
	{12, 5},
	{12, 5},
	{12, 5},
	{12, 5},
	{12, 5},
	{12, 5},
	{13, 5},
	{13, 5},
	{13, 5},
	{13, 5},
	{13, 5},
	{13, 5},
	{13, 5},
	{13, 5},
	{13, 5},
};

profession_t gProfessionTable[_MAX_PROFESSION] = {
	// adventurer
	{ 3, 6, 3, 5 },

	// warrior
	{ 5, 7, 1, 2 },

	// wizard
	{ 2, 4, 5, 8 },

	// thief
	{ 4, 6, 1, 3 }
};

skill_t gSkillInfo[_SKILL_MAX][_SKILL_LVL_MAX] = {
	// short sword
	{
		{0, 1.0},				// ignorant
		{20, 1.6},				// familiar
		{40, 2.2},				// proficient
		{60, 2.8},				// expert
		{80, 3.4},				// master
		{100, 4.0} 				// grand master
	},

	// long sword
	{
		{0, 1.0},				// ignorant
		{20, 1.6},				// familiar
		{40, 2.2},				// proficient
		{60, 2.8},				// expert
		{80, 3.4},				// master
		{100, 4.0} 				// grand master
	},

	// two-handed sword
	{
		{0, 1.0},				// ignorant
		{20, 1.4},				// familiar
		{40, 1.8},				// proficient
		{60, 2.2},				// expert
		{80, 2.6},				// master
		{100, 3.0} 				// grand master
	},

	// dagger 
	{
		{0, 1.0},				// ignorant
		{20, 1.0},				// familiar
		{40, 2.0},				// proficient
		{60, 3.0},				// expert
		{80, 4.0},				// master
		{100, 5.0}				// grand master
	},

	// axe 
	{
		{0, 1.0},				// ignorant
		{20, 1.0},				// familiar
		{40, 1.6},				// proficient
		{60, 2.2},				// expert
		{80, 2.8},				// master
		{100, 3.0} 				// grand master
	},

	// club 
	{
		{0, 1.0},				// ignorant
		{20, 1.6},				// familiar
		{40, 2.2},				// proficient
		{60, 2.8},				// expert
		{80, 3.4},				// master
		{100, 4.0} 				// grand master
	},

	// mace 
	{
		{0, 1.0},				// ignorant
		{20, 1.6},				// familiar
		{40, 2.2},				// proficient
		{60, 2.8},				// expert
		{80, 3.4},				// master
		{100, 4.0} 				// grand master
	},

	// unarmed 
	{
		{0, 1.0},				// ignorant
		{20, 1.0},				// familiar
		{40, 2.0},				// proficient
		{60, 3.0},				// expert
		{80, 4.0},				// master
		{100, 5.0}				// grand master
	},

	// throwing 
	{
		{0, 1.0},				// ignorant
		{20, 1.0},				// familiar
		{40, 2.0},				// proficient
		{60, 3.0},				// expert
		{80, 4.0},				// master
		{100, 5.0}				// grand master
	},

	// acrobatics 
	{
		{0, 1.0},				// ignorant
		{10, 1.0},				// familiar
		{20, 2.0},				// proficient
		{30, 3.0},				// expert
		{40, 4.0},				// master
		{50, 5.0}				// grand master
	},

	// maul 
	{
		{0, 1.0},				// ignorant
		{20, 1.0},				// familiar
		{40, 1.6},				// proficient
		{60, 2.2},				// expert
		{80, 2.8},				// master
		{100, 3.0} 				// grand master
	},
	// sorcery 
	{
		{0, 0},				// ignorant
		{0, 2},				// familiar
		{0, 3},				// proficient
		{0, 4},				// expert
		{0, 5},				// master
		{0, 0}				// grand master
	},

	// elementalism 
	{
		{0, 0},				// ignorant
		{0, 2},				// familiar
		{0, 3},				// proficient
		{0, 4},				// expert
		{0, 5},				// master
		{0, 0}				// grand master
	},

	// mysticism 
	{
		{0, 0},				// ignorant
		{0, 2},				// familiar
		{0, 3},				// proficient
		{0, 4},				// expert
		{0, 5},				// master
		{0, 0}				// grand master
	},

	// thaumaturgy 
	{
		{0, 0},				// ignorant
		{0, 2},				// familiar
		{0, 3},				// proficient
		{0, 4},				// expert
		{0, 5},				// master
		{0, 0}				// grand master
	},

	// necromancy 
	{
		{0, 0},				// ignorant
		{0, 2},				// familiar
		{0, 3},				// proficient
		{0, 4},				// expert
		{0, 5},				// master
		{0, 0}				// grand master
	},

	// theurgism 
	{
		{0, 0},				// ignorant
		{15, 0},				// familiar
		{35, 0},				// proficient
		{55, 0},				// expert
		{75, 0},				// master
		{90, 0}				// grand master
	},

	// alchemy 
	{
		{0, 0},				// ignorant
		{15, 7},				// familiar
		{30, 7},				// proficient
		{45, 6},				// expert
		{70, 5},				// master
		{85, 4}				// grand master
	},

	// weaponsmith 
	{
		{0, 0},				// ignorant
		{15, 7},				// familiar
		{30, 7},				// proficient
		{45, 6},				// expert
		{70, 5},				// master
		{85, 4}				// grand master
	},

	// armorer 
	{
		{0, 0},				// ignorant
		{15, 7},				// familiar
		{30, 7},				// proficient
		{45, 6},				// expert
		{70, 5},				// master
		{85, 4}				// grand master
	},

	// leatherworker 
	{
		{0, 0},				// ignorant
		{15, 7},				// familiar
		{30, 7},				// proficient
		{45, 6},				// expert
		{70, 5},				// master
		{85, 4}				// grand master
	},

	// seamster 
	{
		{0, 0},				// ignorant
		{15, 5},				// familiar
		{30, 5},				// proficient
		{50, 4},				// expert
		{75, 4},				// master
		{90, 3}				// grand master
	},

	// tracking 
	{
		{0, 0},				// ignorant
		{50, 10},			// familiar
		{60, 10},			// proficient
		{70, 9},				// expert
		{80, 9},				// master
		{90, 8}				// grand master
	},

	// healing 
	{
		{0, 0},				// ignorant
		{25, 10},			// familiar
		{35, 9},				// proficient
		{45, 8},				// expert
		{55, 7},				// master
		{65, 6}				// grand master
	},

	// broadsword
	{
		{0, 1.0},				// ignorant
		{20, 1.6},				// familiar
		{40, 2.2},				// proficient
		{60, 2.8},				// expert
		{80, 3.4},				// master
		{100, 4.0} 				// grand master
	},

	// pick pocket 
	{
		{0, 0},				// ignorant
		{10, 7},				// familiar
		{25, 7},				// proficient
		{40, 6},				// expert
		{55, 5},				// master
		{70, 4}				// grand master
	},

	// detect traps 
	{
		{0, 0},				// ignorant
		{15, 7},				// familiar
		{33, 7},				// proficient
		{55, 6},				// expert
		{70, 5},				// master
		{90, 4}				// grand master
	},

	// pick locks 
	{
		{0, 0},				// ignorant
		{15, 7},				// familiar
		{33, 7},				// proficient
		{55, 6},				// expert
		{70, 5},				// master
		{90, 4}				// grand master
	},

	// meditation
	{
		{0, 0},
		{25, 0},
		{50, 0},
		{70, 0},
		{90, 0}
	},

	// critical striking
	{
		{1, 0},
		{5, 0},
		{10, 0},
		{15, 0},
		{20, 0},
		{25, 0}
	},

	// shield use 
	{
		{0, 1.0},				// ignorant
		{10, 1.0},				// familiar
		{20, 2.0},				// proficient
		{30, 3.0},				// expert
		{40, 4.0},				// master
		{50, 5.0}				// grand master
	}
};

combatStr_t gRatCombatStrTbl[5] = { 
	{"nibbles on", ""},
	{"bites", ""},
	{"bites", " hard"},
	{"bites", " very hard"},
	{"shreds", " to pieces"}
};

combatStr_t gUnarmedCombatStrTbl[5] = {
	{"pinches", ""},
	{"slaps", ""},
	{"punches", ""},
	{"uppercuts", ""},
	{"lets loose a haymaker on", ""}
};

combatStr_t gSwordCombatStrTbl[5] = {
	{"nicks", ""},
	{"cuts", ""},
	{"slashes", ""},
	{"deeply cuts", ""},
	{"grievously wounds", ""}
};

combatStr_t gClubCombatStrTbl[5] = {
	{"hits", ""},
	{"clubs", ""},
	{"bashes", ""},
	{"crushes", ""},
	{"demolishes", ""}
};

int gColorTable[_MAX_COLOR] = { 43, 48, 49, 53, 54, 58, 59, 63, 64, 68, 69, 73, 74, 78, 79, 83, 84, 88, 89, 93, 94, 98, 99, 100, 104, 105, 106, 107 };

char *gAffectLostTbl[] = {
	"normal damage",
	"fire damage",
	"cold damage",
	"lightning damage",
	"acid damage",
	"poison damage",
	"stamina damage",
	"steal stamina damage",
	"experience damage",
	"steal experience damage",
	"steal life damage",
	"rust damage",
	"ethereal damage",
	"stun damage",
	"missile damage",
	"magically armored",
	"blessed with improved damage",
	"able to detect invisible",
	"invisible",
	"permanency enchanted",
	"wraithformed",
	"invisible",
	"enchanted",
	"fire immolated",
	"cold immolated",
	"acid immolated",
	"poison immolated",
	"lightning immolated",
	"frozen",
	"held",
	"confused",
	"shackled",
	"identified",
	"berserk",
	"stunned",
	"loyalty shifted",
	"in fear",
	"quickened",
	"slowed",
	"empowered",
	"enfeebled",
	"shielded",
	"shielded",
	"invulnerable",
	"regenerating",
	"indestructible",
	"cursed",
	"jailed",
	"resistant to magic",
	"immune to magic",
	"rust immolated",
	"dated",
	"regenerating stamina",
	"ressurecting",
	"ressurecting",
	"ressurecting",
	"getting extra attacks",
	"getting extra dodges",
	"memory enhanced",
	"dexterity enhanced",
	"dexterity cursed",
	"intelligence enhanced",
	"intelligence cursed",
	"endurance enhanced",
	"endurance cursed",
	"under retention",
	"vulnerable",
	"nourished",
	"gender switched",
	"naked",
	"ugly",
	"disguised",
	"encumberance enhanced",
	"encumberance cursed",
	"engraved",
	"shifting",
	"extended",
	"poisoned",
	"burning",
	"slowed",
	"spell countered",
	"under an anti-magic aura",
	"converted",
	"resetting",
	"resetting",
	"protected from death magic",
	"blessed with free will",
	"identified",
	"blessed by Enid",
	"blessed by Duach",
	"cursed by Enid",
	"cursed by Duach",
	"blessed by Despothes",
	"cursed by Despothes",
	"green skinned",
	"yellow skinned",
	"grey skinned",
	"white haired",
	"black haired",
	"staff"
};

int gArmorModTbl[dtMax][atMax] = {
	// pierce weapon vs atNone, atLeather, atChain, atPlate
	{ 0, 0, 90, -50 },

	// slash weapon vs atNone, atLeather, atChain, atPlate
	{ 0, -25, 25, 0 },

	// bludgeon weapon vs atNone, atLeather, atChain, atPlate
	{ 0, 50, 0, 70 },
};

int gDamagePassTbl[dtMax][atMax] = {
	// pierce weapon vs atNone, atLeather, atChain, atPlate
	{ 110, 80, 50, 30 }, //{ 95, 85, 25, 0 },

	// slash weapon vs atNone, atLeather, atChain, atPlate
	{ 120, 50, 60, 40 }, //{ 97, 30, 50, 20 },

	// bludgeon weapon vs atNone, atLeather, atChain, atPlate
	{ 130, 70, 90, 50 }, //{ 100, 50, 100, 30 },
};

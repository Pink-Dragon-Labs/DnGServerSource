//
// SPECIAL.HPP
//
// Special Effect enum.
//
// author: Stephen Nichols
//

#ifndef _SPECIAL_HPP_
#define _SPECIAL_HPP_

#include "../global/system.hpp"

enum {
	_SE_DEATH_CLOUD,		//0
	_SE_FLAME_ORB,
	_SE_FLASH, 
	_SE_FLASH_GROW,
	_SE_FLASH_FADE,
	_SE_LIGHTNING,
	_SE_GREEN_CLOUD,
	_SE_RED_CLOUD,
	_SE_BLUE_CLOUD,
	_SE_YELLOW_CLOUD,
	_SE_WHITE_CLOUD,		//10
	_SE_PURPLE_CLOUD,
	_SE_ICE_STORM,
	_SE_SUMMON,
	_SE_GREEN_FADE,
	_SE_RED_FADE,
	_SE_BLUE_FADE,
	_SE_YELLOW_FADE,
	_SE_WHITE_FADE,
	_SE_PURPLE_FADE,
	_SE_HEAL,				//20
	_SE_DRAIN_MAGIC,
	_SE_WHIRLWIND,
	_SE_SHIELD_UP,
	_SE_SHIELD_DOWN,
	_SE_POOF,
	_SE_DEATH_BLADE,
	_SE_UNLOCK,
	_SE_MAGIC_DUST,
	_SE_LEVITATE,
	_SE_FIREBALL,			//30
	_SE_MUSIC,
	_SE_SOUND,
	_SE_MULTI_BLADE,
	_SE_DISPEL_MAGIC,
	_SE_PERMANENCY,
	_SE_GATHER_THE_FELLOWSHIP,
	_SE_ACID_SPHERE,
	_SE_LIGHT_DART,
	_SE_PSYCHIC_ORB,
	_SE_GREATER_HEAL,		//40
	_SE_PURIFY,
	_SE_CURE_POISON,
	_SE_SUMMON_ZOMBIE,
	_SE_SUMMON_DAEMON,
	_SE_SUMMON_PIXIE,
	_SE_EARTHQUAKE,
	_SE_ENGRAVE,
	_SE_CORNUCOPIA,
	_SE_IMPROVE_ARMOR,
	_SE_REMOVE_CURSE,		//50
	_SE_ICE_ORB,
	_SE_HOLD_MONSTER,
	_SE_BERSERK,
	_SE_SHIELD,
	_SE_GREATER_SHIELD,
	_SE_FIRE_SHIELD,
	_SE_COLD_SHIELD,
	_SE_LIGHTNING_SHIELD,
	_SE_INVULNERABILITY,
	_SE_POISON_SHIELD,		//60
	_SE_ACID_SHIELD,
	_SE_EXTENSION,
	_SE_SEE_INVISIBILITY,
	_SE_SHIFT,
	_SE_INVISIBILITY,
	_SE_RUST,
	_SE_DEFENSELESSNESS,
	_SE_IMPROVED_INVISIBILITY,
	_SE_ENCHANT_ITEM,
	_SE_FIRE_CURSE,			//70
	_SE_COLD_CURSE,
	_SE_LIGHTNING_CURSE,
	_SE_POISON_CURSE,
	_SE_ACID_CURSE,
	_SE_EMPOWER,
	_SE_ENFEEBLE,
	_SE_POISON_BOLT,
	_SE_SUMMON_WRAITH,
	_SE_SUMMON_UNDEAD,
	_SE_SUMMON_NIGHT_FRIENDS,//80
	_SE_FIRE_GRASP,
	_SE_ARTIC_GRASP,
	_SE_VENOMOUS_TOUCH,
	_SE_FLAME_BLADE,
	_SE_ELECTRIC_CHARGE,
	_SE_COLD_STEEL,
	_SE_VENOM,
	_SE_STEAL_LIFE,
	_SE_NIMBILITY,
	_SE_CLUMSINESS,			//90
	_SE_IMMOLATION,
	_SE_WARP_MIND,
	_SE_DRAIN_LIFE,
	_SE_SPARK,
	_SE_DEATH_TOUCH,
	_SE_BANISHMENT,
	_SE_STONING,
	_SE_ACID_RAIN,
	_SE_MASS_DRAIN,
	_SE_ELECTRIC_FURY,		//100
	_SE_DEATH_WISH,
	_SE_WRATH_OF_THE_GODS,
	_SE_CRUSHING_BOULDER,
	_SE_MISSILE_RESISTANCE,
	_SE_HEAD_OF_DEATH,
	_SE_FREEZE,
	_SE_STUN,
	_SE_CONFUSION,
	_SE_MIND_SHACKLE,
	_SE_LOYALTY_SHIFT,		//110
	_SE_FEAR,
	_SE_ELPHAMES_JUSTICE,
	_SE_INCINERATE,
	_SE_INDESTRUCTION,
	_SE_ENIDS_BLESSING,
	_SE_DUACHS_VENGEANCE,
	_SE_COLD_SNAP,
	_SE_SPELL_BLAST,
	_SE_FORCED_FLEE,
	_SE_ANTI_MAGIC_AURA,	//120
	_SE_FUMBLE,
	_SE_MASS_FUMBLE,
	_SE_GUST_OF_WIND,
	_SE_SAND_STORM,
	_SE_BANG,			// MIKE - please note that only the packet format
	_SE_BANG_VIOLET,	// for SEBang is different than the color
	_SE_BANG_GREEN,		// specific ones in that SEBang requires
	_SE_BANG_YELLOW,	// a color to be specified as a byte
	_SE_BANG_ORANGE,	// before the target.
	_SE_BANG_TEAL,		// note: teal not obtainable through general SEBang.
	_SE_BANG_BLUE,
	_SE_SLIDE_COMBATANT,
	_SE_LIGHTNING_BOLT,
	_SE_POISONCLOUD_TARGET,
	_SE_POISONCLOUD_AREA,
	_SE_FLAME_TARGET,
	_SE_FLAME_AREA,
	_SE_MAX
};

#endif

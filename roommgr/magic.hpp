//
// magic.hpp
//
// generic spell definitions
//

#ifndef _MAGIC_HPP_
#define _MAGIC_HPP_

#define _SPELL_SLOWEST 25
#define _SPELL_SLOW 10
#define _SPELL_MEDIUM 5
#define _SPELL_FAST 0

enum {
	_SPELL_NO_FAILURE,
	_SPELL_SKILL_FAILURE,
	_SPELL_NOTHING_FAILURE,
	_SPELL_MANA_FAILURE,
	_SPELL_FAILURE_MAX
};

enum {
	_SPELL_TYPE_PROJECTILE,
	_SPELL_TYPE_MAX
};

enum {
	_TARGET_OBJ,
	_TARGET_LOCALE,
	_TARGET_NONE,
	_TARGET_MAX
};

enum {
	_SPELL_HOME,
	_SPELL_KILL_STAR,
	_SPELL_UNLOCK,
	_SPELL_DISPEL_MAGIC,
	_SPELL_ENGRAVE,
	_SPELL_MULTI_BLADE,
	_SPELL_GATHER_THE_FELLOWSHIP,
	_SPELL_CORNUCOPIA,
	_SPELL_CLOUD_OF_FOG,
	_SPELL_IMPROVE_ARMOR,
	_SPELL_TELEPORT,
	_SPELL_EXTENSION,
	_SPELL_SEE_INVISIBILITY,
	_SPELL_SHIFT,
	_SPELL_INVISIBILITY,
	_SPELL_COMBAT_TELEPORT,
	_SPELL_TELEPORT_GROUP,
	_SPELL_PERMANENCY,
	_SPELL_RUST,
	_SPELL_DEFENSELESSNESS,
	_SPELL_IMPROVED_INVISIBILITY,
	_SPELL_ENCHANT_ITEM,
	_SPELL_MASS_RUST,
	_SPELL_ELPHAMES_JUSTICE,
	_SPELL_FIRE_GRASP,
	_SPELL_FLAME_ORB,
	_SPELL_ARTIC_GRASP,
	_SPELL_ICE_ORB,
	_SPELL_EARTH_SPIKE,
	_SPELL_INCINERATE,
	_SPELL_GUST_OF_WIND,
	_SPELL_IMMOLATION,
	_SPELL_DANCING_FLAME,
	_SPELL_FLAME_BLADE,
	_SPELL_ARTIC_CHARGE,
	_SPELL_COLD_STEEL,
	_SPELL_SAND_STORM,
	_SPELL_SPARK,
	_SPELL_ICE_STORM,
	_SPELL_FREEZING_WIND,
	_SPELL_HURRICANE,
	_SPELL_CRAWLING_CHARGE,
	_SPELL_STONING,
	_SPELL_FIREBALL,
	_SPELL_LIGHTNING_BOLT,
	_SPELL_FREEZE,
	_SPELL_CRUSHING_BOULDER,
	_SPELL_ELECTRIC_FURY,
	_SPELL_COLD_SNAP,
	_SPELL_EARTHQUAKE,
	_SPELL_DESPOTHES_WRATH,
	_SPELL_HOLD_MONSTER,
	_SPELL_FUMBLE,
	_SPELL_PSYCHIC_ORB,
	_SPELL_CONFUSION,
	_SPELL_FORGET,
	_SPELL_MIND_SHACKLE,
	_SPELL_IDENTIFY,
	_SPELL_BERSERK,
	_SPELL_STUN,
	_SPELL_LOYALTY_SHIFT,
	_SPELL_WARP_MIND,
	_SPELL_ETHEREALIZE,
	_SPELL_SPELL_BLAST,
	_SPELL_MASS_HOLD,
	_SPELL_65,
	_SPELL_MASS_FUMBLE,
	_SPELL_FEAR,
	_SPELL_IRON_CHAINS,
	_SPELL_MASS_BERSERK,
	_SPELL_MASS_STUN,
	_SPELL_MASS_LOYALTY_SHIFT,
	_SPELL_MABONS_FORCED_FLEE,
	_SPELL_ILLUSIONARY_FOE,
	_SPELL_ANTI_MAGIC_AURA,
	_SPELL_LIGHT_DART,
	_SPELL_CURSE_OF_CLUMSINESS,
	_SPELL_NIMBILITY,
	_SPELL_EMPOWER,
	_SPELL_ENFEEBLE,
	_SPELL_MISSILE_RESISTANCE,
	_SPELL_HEAL,
	_SPELL_SUMMON_PIXIE,
	_SPELL_PURIFY,
	_SPELL_CURE_POISON,
	_SPELL_SUMMON_ELEMENTAL,
	_SPELL_GREATER_HEAL,
	_SPELL_REMOVE_CURSE,
	_SPELL_SUMMON_FAERY,
	_SPELL_FIRE_CURSE,
	_SPELL_COLD_CURSE,
	_SPELL_ELECTRIC_CURSE,
	_SPELL_SHIELD,
	_SPELL_GREATER_SHIELD,
	_SPELL_WRATH_OF_THE_GODS,
	_SPELL_FIRE_SHIELD,
	_SPELL_COLD_SHIELD,
	_SPELL_LIGHTNING_SHIELD,
	_SPELL_SUMMON_NYMPH,
	_SPELL_REGENERATION,
	_SPELL_INDESTRUCTION,
	_SPELL_INVULNERABILITY,
	_SPELL_ENIDS_BLESSING,
	_SPELL_BANISHMENT,
	_SPELL_SUMMON_FAERY_QUEEN,
	_SPELL_ACID_SPHERE,
	_SPELL_VENOMOUS_TOUCH,
	_SPELL_107,
	_SPELL_POISON_BOLT,
	_SPELL_109,
	_SPELL_DRAIN_LIFE,
	_SPELL_ACID_CLOUD,
	_SPELL_SUMMON_ZOMBIE,
	_SPELL_VENOM,
	_SPELL_SUMMON_UNDEAD,
	_SPELL_ACID_RAIN,
	_SPELL_NIGHT_FRIENDS,
	_SPELL_STEAL_LIFE,
	_SPELL_118,
	_SPELL_SUMMON_DOPPELGANGER,
	_SPELL_DEATH_TOUCH,
	_SPELL_BANISH,
	_SPELL_SHADOW_WARRIOR,
	_SPELL_SUMMON_DAEMON,
	_SPELL_DUACHS_VENGEANCE,
	_SPELL_DEATH_WISH,
	_SPELL_MASS_DRAIN,
	_SPELL_POISON_CURSE,
	_SPELL_ACID_CURSE,
	_SPELL_POISON_SHIELD,
	_SPELL_ACID_SHIELD,
	_SPELL_HEAD_OF_DEATH,
	_SPELL_GREATER_IDENTIFY,
	_SPELL_MONSTER_SUMMONING_I,
	_SPELL_MONSTER_SUMMONING_II,
	_SPELL_MONSTER_SUMMONING_III,
	_SPELL_SUMMON_RATLING,
	_SPELL_SUMMON_BAT,
	_SPELL_SUMMON_FENRIS,
	_SPELL_SUMMON_IMP,
	_SPELL_SUMMON_OGRE,
	_SPELL_SUMMON_TROLL,
	_SPELL_SUMMON_SERAPH,
	_SPELL_REPEL,
	_SPELL_BURNING_MISTCLOUD,
	_SPELL_NAUSEATING_MISTCLOUD,
	_SPELL_DEADLY_MISTCLOUD,
	_SPELL_IMMOLATION_COLD,
	_SPELL_VITALITY,
	_SPELL_ENLIGHTENMENT,
	_SPELL_IMMOLATION_LIGHTNING,
	_SPELL_SUMMON_DRAGON,
	_SPELL_SUMMON_MIST,
	_SPELL_IMP_INVULNERABILITY,
	_SPELL_DAMNATION,
	_SPELL_IMMOLATION_ACID,
	_SPELL_IMMOLATION_POISON,
	_SPELL_ACID_BLADE,
	_SPELL_SUMMON_SNAKE,
	_SPELL_SUMMON_HOLY_WARRIOR,
	_SPELL_MAX
};

enum {
	_NON_COMBAT_SPELL,
	_BOTH_SPELL,
	_COMBAT_SPELL
};

class WorldObject;

//
// define the structure that represents every spell
//

class affect_t;
class RMPlayer;

typedef affect_t *(*spellfn_t)( int spell, WorldObject *caster, int targetServID, int targetX, int targetY, char *output, PackedData *packet, RMPlayer *caller );

#define SPELL(x) affect_t *x ( int spell, WorldObject *caster, int targetServID, int targetX, int targetY, char *output, PackedData *packet, RMPlayer *caller )

typedef struct {
	spellfn_t function;
	int skillType, skillLevel;
	int targetType;
	char *verbal;
	int manaCost;
	int dexMod;
	int isCombat;
	int castCount;
	int agressive;
	int isDamaging;
} spell_info;

extern spell_info gSpellTable[_SPELL_MAX];

// calculate the mana cost of a spell
int calcSpellCost ( spell_info *spell, WorldObject *caster );
int calcSpellMod ( WorldObject *caster );

int createProjectile ( int special, WorldObject *source, int targetServID, PackedData *packet, int minDamage, int maxDamage, int projectileCount, char *hitText, char *name );

int summonMonster ( int count, char *name, char *text, int special, int targetX, int targetY, WorldObject *caster, char *output, PackedData *packet, int level = 0 );

#endif

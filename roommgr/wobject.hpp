/*
	WorldObject / WorldObjectBase classes	
	author: Stephen Nichols
*/

#ifndef _WOBJECT_HPP_
#define _WOBJECT_HPP_

#include "wobjectbase.hpp"
#include "../global/system.hpp"
#include "verbs.hpp"
#include "misc.hpp"
#include "dice.hpp"
#include "properties.hpp"
#include "actions.hpp"
#include "combat.hpp"
#include "bhead.hpp"
#include "bweapon.hpp"

class WorldObject;
class RMRoom;
class RMPlayer;
class BCharacter;
class MsgProcessor;

extern MemoryAllocator gAffectAllocator;

#define _WEAR_HEAD		0			// counts for 10% of armor rating
#define _WEAR_CHEST		1			// counts for 40% of armor rating
#define _WEAR_ROBE		2			// alternative to legs and chest 
#define _WEAR_NECK		3			// counts for 10% of armor rating
#define _WEAR_BANDS		4			// counts for 10% of armor rating
#define _WEAR_LEGS		6			// counts for 20% of armor rating
#define _WEAR_FEET		7			// counts for 10% of armor rating 
#define _WEAR_SKIRT		9			// alternative to legs
#define _WEAR_SHIELD 	17			// adds to overall rating
#define _WEAR_PACK 		18			// backpack... 

#define _WEAR_RING_L		700		// special area worn that allows 2 to be worn
#define _WEAR_RING_R		701		// special area worn that allows 2 to be worn

#define _JAIL_GM	0
#define _JAIL_LAW 1

enum {
	_MEM_WOBJECT = 10000,
	_MEM_PLAYER,
	_MEM_NPC,
	_MEM_ROOM,
	_MEM_TIMER_EVENT,
	_MEM_COMBAT_GROUP,
	_MEM_BUILDING
};

class CombatGrid 
{
public:
	CombatGrid();
	virtual ~CombatGrid();

	void mapAccessible ( WorldObject *obj );
	void mapAccessible ( CombatGroup *group, int x, int y );
	void findClosestPoint ( int x1, int y1, int x2, int y2, int *pointX, int *pointY );

	void display ( void );

	char data[_COMBAT_GRID_WIDTH][_COMBAT_GRID_HEIGHT];
};

// enumerate crimes
enum {
	_CRIME_THEFT,
	_CRIME_MURDER,
	_CRIME_CLEAR,
};

// enumerate speeds
enum {
	_SPEED_VERY_SLOW,
	_SPEED_SLOW,
	_SPEED_AVERAGE,
	_SPEED_FAST,
	_SPEED_VERY_FAST
};

/* enumerate the movie commands */
enum {
	_MOVIE_MOVETO,
	_MOVIE_POSN,
	_MOVIE_HIDE,
	_MOVIE_SHOW,
	_MOVIE_HEADING,
	_MOVIE_TAKE,
	_MOVIE_DROP,
	_MOVIE_PUT_ON,
	_MOVIE_TAKE_OFF,
	_MOVIE_PUT_IN,
	_MOVIE_OPEN,
	_MOVIE_CLOSE,
	_MOVIE_LOCK,
	_MOVIE_UNLOCK,
	_MOVIE_ATTACH_EFFECT,
	_MOVIE_REMOVE_EFFECT,
	_MOVIE_RUN,
	_MOVIE_ATTACK,
	_MOVIE_COMBAT_MOVE,
	_MOVIE_GIVE_XXX,
	_MOVIE_START_COMBAT,
	_MOVIE_COMBAT_BEGIN,
	_MOVIE_ACTION_DELAY,
	_MOVIE_GAIN_EXP,
	_MOVIE_GAIN_LEVEL,
	_MOVIE_MONEY_DROP,
	_MOVIE_MONEY_PUT,
	_MOVIE_MONEY_GIVE,
	_MOVIE_MONEY_TAKE,
	_MOVIE_CAST_BEGIN,
	_MOVIE_CHANGE_HEALTH,
	_MOVIE_CAST_END,
	_MOVIE_HANDS_OFF,
	_MOVIE_HANDS_ON,
	_MOVIE_COMBAT_FLEE,
	_MOVIE_COMBAT_EXIT,
	_MOVIE_DIE,
	_MOVIE_CHANGE_MANA,
	_MOVIE_CONSUME,
	_MOVIE_SPECIAL_EFFECT,
	_MOVIE_CHANGE_ROOM,
	_MOVIE_COMBAT_READY,
	_MOVIE_TELEPORT,
	_MOVIE_COMBAT_TELEPORT,
	_MOVIE_TEXT,
	_MOVIE_INFO,
	_MOVIE_MANA_MAX,
	_MOVIE_HEALTH_MAX,
	_MOVIE_SIT,
	_MOVIE_STAND,
	_MOVIE_MEMORIZE,
	_MOVIE_GIVE,
	_MOVIE_ROB,
	_MOVIE_ROUND_BEGIN,
	_MOVIE_TURN_BEGIN,
	_MOVIE_MISSILE,
	_MOVIE_SWITCH,
	_MOVIE_QUEST_COMPLETE,
	_MOVIE_CHANGE_STAMINA,
	_MOVIE_WIN_COMBAT_DEFENDERS,
	_MOVIE_WIN_COMBAT_ATTACKERS,
	_MOVIE_DMG_FLASH,
	_MOVIE_TOSS_OBJECT,
	_MOVIE_CREATE_OBJ,
	_MOVIE_CREATE_CHAR,
	_MOVIE_DYE,
	_MOVIE_ENGRAVE,
	_MOVIE_FORCE_DROP,
	_MOVIE_BOUNTY,
	_MOVIE_MURDER,
	_MOVIE_END = 255
};

/* enumerate the different attack results */
#define _ATTACK_MISS			(1 << 0)
#define _ATTACK_DEFEND			(1 << 1)
#define _ATTACK_HIT				(1 << 2)
#define _ATTACK_DIE				(1 << 3)
#define _ATTACK_HIT_ARMOR		(1 << 4)

/* define the different consumable states */
enum {
	_STATE_LIQUID,
	_STATE_SOLID,
	_STATE_MAX
};

/* enumerate the actions */
enum {
	_WOA_FACE,
	_WOA_STAND,
	_WOA_WALK,
	_WOA_SIT,
	_WOA_GIVE,
	_WOA_GET,
	_WOA_THROW,
	_WOA_JUMP,
	_WOA_PUSH,
	_WOA_CAST,
	_WOA_CLUB_STANCE,
	_WOA_CLUB_ADVANCE,
	_WOA_CLUB_STRIKE,
	_WOA_DAGGER_STANCE,
	_WOA_DAGGER_ADVANCE,
	_WOA_DAGGER_STRIKE,
	_WOA_SWORD_STANCE,
	_WOA_SWORD_ADVANCE,
	_WOA_SWORD_STRIKE,
	_WOA_COMBAT_ADVANCE,
	_WOA_HAND_STANCE,
	_WOA_HAND_STRIKE,
	_WOA_HAND_ADVANCE,
	_WOA_KICK_STRIKE,
	_WOA_LONGBOW,
	_WOA_CRAWL,
	_WOA_KICK,
	_WOA_PUNCH,
	_WOA_DIE,
	_WOA_GROUND,
	_WOA_OPEN,
	_WOA_BURN,
	_WOA_EXPLODE,
	_WOA_INVENTORY,
	_WOA_CLOSEUP,
	_WOA_LIQUID_SIZE
};

/* enumerate NAK error codes */
enum {
	_ERR_INTERNAL = 10000,
	_ERR_REDUNDANT,
	_ERR_NO_ROOM,
	_ERR_MUST_REMOVE,
	_ERR_BAD_SERVID,
	_ERR_BAD_ROOM,
	_ERR_SERVICE_NOT_AVAILABLE,
	_ERR_MUST_UNLOCK,
	_ERR_WRONG_KEY,
	_ERR_MUST_OPEN,
	_ERR_WONT_FIT,
	_ERR_CANT_WEAR,
	_ERR_TOO_BULKY,
	_ERR_TOO_HEAVY,
	_ERR_TOO_EXPENSIVE,
	_ERR_DEAD,
	_ERR_ACCESS_LOCKED,
	_ERR_BAD_PASSWORD,
	_ERR_FROZEN,
	_ERR_NONSENSE,
	_ERR_NOT_MAGICAL,
	_ERR_TOO_FAR,
	_ERR_NO_SHIELD,
	_ERR_NO_WEAPON,
	_ERR_BAD_PACKAGE,
	_ERR_TOO_TIRED,
	_ERR_STRENGTH_FAILURE,
	_ERR_DEXTERITY_FAILURE,
	_ERR_INTELLIGENCE_FAILURE,
	_ERR_QUICKNESS_FAILURE,
	_ERR_ENDURANCE_FAILURE,
	_ERR_MEMORIZE_FAILURE,
	_ERR_SKILL_FAILURE,
	_ERR_TOO_VAGUE,
	_ERR_BAD_OBJECT
};

// used for player object only, so is ok to duplicate

#define _STATE_BUSY (1 << 2)

/* define the available physical state bits */
#define _STATE_LOCKED			(1 << 0)
#define _STATE_CLOSED			(1 << 1)
#define _STATE_EVIL				(1 << 2)
#define _STATE_NODROP			(1 << 3)
#define _STATE_MAGIC			(1 << 4)
#define _STATE_TOSS				(1 << 5)
#define _STATE_ANTI_GOOD		(1 << 6)
#define _STATE_ANTI_EVIL		(1 << 7)
#define _STATE_ANTI_NEUTRAL		(1 << 8) 
#define _STATE_TRAPPED			(1 << 9)
#define _STATE_MONEY			(1 << 10)
#define _STATE_RIDDLED			(1 << 11)
#define _STATE_SPECIAL			(1 << 12)
#define _STATE_COMPONENT		(1 << 13)
#define _STATE_OCCUPIED 		(1 << 14)
#define _STATE_WEINER			(1 << 15)
#define _STATE_WHOLESALE		(1 << 16)
#define _STATE_WORTHLESS		(1 << 17)
#define _STATE_DISEASED			(1 << 18)
#define _STATE_HACKER			(1 << 19)
#define _STATE_NOSCALE			(1 << 20)
#define _STATE_TREASURE			(1 << 21)
#define _STATE_MUST_DROP		(1 << 22)
#define _STATE_TESTSERVER_OBJ	(1 << 23)
#define _STATE_HOUSE_DECOR		(1 << 24)

#define _MAX_ACTION_FUNCTIONS 3

enum {
	_AFF_DAMAGE_NORMAL,			// 0
	_AFF_DAMAGE_FIRE,
	_AFF_DAMAGE_COLD,
	_AFF_DAMAGE_LIGHTNING,
	_AFF_DAMAGE_ACID,
	_AFF_DAMAGE_POISON,
	_AFF_DAMAGE_STAMINA,
	_AFF_DAMAGE_STEAL_STAMINA,
	_AFF_DAMAGE_EXPERIENCE,
	_AFF_DAMAGE_STEAL_EXPERIENCE,
	_AFF_DAMAGE_STEAL_LIFE,		// 10
	_AFF_DAMAGE_RUST,
	_AFF_DAMAGE_ETHEREAL,
	_AFF_DAMAGE_STUN,
	_AFF_DAMAGE_MISSILE,
	_AFF_IMPROVE_ARMOR,
	_AFF_IMPROVE_DAMAGE,
	_AFF_SEE_INVISIBLE,
	_AFF_INVISIBILITY,
	_AFF_PERMANENCY,
	_AFF_DEFENSELESS,			// 20
	_AFF_IMPROVED_INVISIBILITY,	
	_AFF_ENCHANT_ITEM,
	_AFF_IMMOLATION_FIRE,
	_AFF_IMMOLATION_COLD,
	_AFF_IMMOLATION_ACID,
	_AFF_IMMOLATION_POISON,
	_AFF_IMMOLATION_LIGHTNING,
	_AFF_FREEZE,
	_AFF_HOLD,
	_AFF_CONFUSION,				// 30
	_AFF_SHACKLED,				
	_AFF_IDENTIFIED,
	_AFF_BERSERK,
	_AFF_STUN,
	_AFF_LOYALTY_SHIFT,
	_AFF_FEAR,
	_AFF_QUICKEN,
	_AFF_SLOW,
	_AFF_EMPOWER,
	_AFF_ENFEEBLE,				// 40
	_AFF_SHIELD,
	_AFF_GREATER_SHIELD,
	_AFF_INVULNERABLE,
	_AFF_REGENERATION,
	_AFF_INDESTRUCTION,
	_AFF_CURSED,
	_AFF_JAIL,
	_AFF_MAGIC_RESISTANCE,
	_AFF_MAGIC_IMMUNITY,
	_AFF_IMMOLATION_RUST,		// 50
	_AFF_OBJ_DATED,
	_AFF_REGENERATE_STAMINA,
	_AFF_RESSURECT_25,
	_AFF_RESSURECT_50,
	_AFF_RESSURECT_100,
	_AFF_EXTRA_ATTACK,
	_AFF_EXTRA_DODGE,
	_AFF_MEMORY,
	_AFF_POS_DEX_MOD,
	_AFF_NEG_DEX_MOD,			// 60
	_AFF_POS_INT_MOD,
	_AFF_NEG_INT_MOD,
	_AFF_POS_END_MOD,
	_AFF_NEG_END_MOD,
	_AFF_RETENTION,
	_AFF_VULNERABLE,
	_AFF_NOURISHED,
	_AFF_SWITCH_GENDER,
	_AFF_NAKED,
	_AFF_UGLY,					// 70
	_AFF_DISGUISED,
	_AFF_ENCUMBERANCE_BLESSING,
	_AFF_ENCUMBERANCE_CURSE,
	_AFF_ENGRAVED,
	_AFF_SHIFT,
	_AFF_EXTENSION,
	_AFF_POISONED,
	_AFF_ACID_BURN,
	_AFF_SLOWED,
	_AFF_SPELL_BLAST,			// 80
	_AFF_ANTI_MAGIC,
	_AFF_CONVERTED,
	_AFF_RESET,
	_AFF_RESET_A,
	_AFF_PROT_DEATH,
	_AFF_FREE_WILL,
	_AFF_ENH_IDENTIFIED,		// 87
	_AFF_MARK_ENID,
	_AFF_MARK_DUACH,
	_AFF_CURSE_ENID,			// 90
	_AFF_CURSE_DUACH,
	_AFF_MARK_DESPOTHES,
	_AFF_CURSE_DESPOTHES,
	_AFF_SKINCOLOR_GREEN,
	_AFF_SKINCOLOR_YELLOW,
	_AFF_SKINCOLOR_GREY,
	_AFF_HAIRCOLOR_WHITE,
	_AFF_HAIRCOLOR_BLACK,
	_AFF_STAFF,					//99
	_AFF_MAX
};

#define _AFF_DAMAGE_MAX (_AFF_DAMAGE_STUN + 1)

// enumerate the affect source types
enum {
	_AFF_SOURCE_ARTIFACT,			// cumulative
	_AFF_SOURCE_SPELL,				// mutex
	_AFF_SOURCE_POTION,				// mutex
	_AFF_SOURCE_GIFT,				// mutex
	_AFF_SOURCE_WEARER,				// mutex -- goes to wearer / wielder of item
	_AFF_SOURCE_PERMANENT,			// mutex -- permanent 
	_AFF_SOURCE_TEMPORARY,
	_AFF_SOURCE_MAX
};

// define affect types
enum {
	_AFF_TYPE_NORMAL,		
	_AFF_TYPE_UNUSED,				// kept for OBJ file compatibility		
	_AFF_TYPE_RESISTANCE,	
	_AFF_TYPE_WEAKNESS,
	_AFF_TYPE_MAX
};

// special bit flag for dormant affects BEW changed from 32
#define _AFF_TYPE_DORMANT (1 << 31)

#undef new

class affect_t : public ListObject
{
public:
	affect_t();
	virtual ~affect_t();

	int id, type, source, duration, value;
	WorldObject *owner;

	void* operator new ( size_t size, char* file, int nLine ) {
		return gAffectAllocator.allocate();
	};

	void operator delete ( void *ptr ) {
		gAffectAllocator.deallocate ( (char *)ptr );
	};
};

#define new new( __FILE__, __LINE__ )

/* define the available coarse alignment types */
enum {
	_ALIGN_GOOD,
	_ALIGN_NEUTRAL,
	_ALIGN_EVIL,
	_ALIGN_MAX
};

/* define the maximum hunger and thirst values */
#define _MAX_HUNGER		56
#define _MAX_THIRST		28

// define the maximum affected types
#define _MAX_AFFECT		64

// this structure is the database representation of a WorldObject
typedef struct 
{
	char *super;	// name of super class
	int x, y;			// position on screen
	int xStep, yStep;
	int color, loop;

	// servID of the object that I am worn on
	int objectWornOn;

	// servID of the object that I am wielded on
	int objectWieldedOn;

	// servID of the object to which I belong
	long ownerID;

	// bitvector of physical states (_STATE_XXX)
	int physicalState;

	// miscellaneous properties
	int strength;
	int dexterity;
	int intelligence;
	int quickness;
	int endurance;

	// old properties
	int oldStrength;
	int oldDexterity;
	int oldIntelligence;
	int oldEndurance;
	
	int level, oldLevel;
	int health;
	int healthMax;
	int alignment;
	int armor, baseArmor;
	int hunger, thirst;
	int isZoneObject;
	int value, creationTime, manaValue;
	int roomNumber;
	int armorType;

	int stamina;
} DBObject;

// database representation of a biography
typedef struct
{
	char text[512];
} DBBiography;

// enumerate ActionInfo targets
enum {
	_ACTION_DIRECT_OBJECT,
	_ACTION_INDIRECT_OBJECT,
	_ACTION_SELF
};

// ActionInfo structure
class ActionInfo : public ListObject
{
public:
	int verb;
	action_t function;
	char **argv;
	int argc;

	ActionInfo() {
		function = NULL;
		argv = NULL;
		argc = 0;
		verb = -1;
	};

	virtual ~ActionInfo() {
		if ( argv ) {
			for ( int i=0; i<argc; i++ )
				free ( argv[i] );

			free ( argv );
			argv = NULL;
		}

		argc = 0;
	};

	ActionInfo *clone ( void ) {
		ActionInfo *retVal = new ActionInfo;

		retVal->function = function;
		retVal->verb = verb;
		retVal->argc = argc;

		retVal->argv = (char **)malloc ( sizeof ( char * ) * argc );

		for ( int i=0; i<argc; i++ ) 
			retVal->argv[i] = strdup ( argv[i] );

		return retVal;
	};
};

// 
// NOTE: Following is the use of each stat in odd cases. :)
//
// 1. If the object has the BEntry base, the strength property really refers to 
//    the destination room number. 
//
// 2. If the object has the _AFF_MYSTIC_LOCK affect, the intelligence property
//    refers to the owner of the lock.
//
// 3. If the object has the BEntry base, the dexterity property really refers to
//		the object that it is linked to. (linkTo)
//

#define _MANA_VIEW 50600

#define _MAX_BASES 10

#undef new

class WorldObject : public DBObject, public HashableObject
{
public:
	static const int _DAMAGE_CUT			= -1;
	static const int _DAMAGE_HEAL			= -2;
	static const int _DAMAGE_POISON			= -3;
	static const int _DAMAGE_INTERNAL		= -4;
	static const int _DAMAGE_EARTHQUAKE		= -5;
	static const int _DAMAGE_EXP_GAIN 		= -6;
	static const int _DAMAGE_SILENCE 		= -7;
	static const int _DAMAGE_PSYCHIC		= -8;

	LinkedList *opposition, *actions, *components, *tokens;

	// base information
	char numBases;
	WorldObjectBase *pBaseList[_MAX_BASES];

	// base properties
	int servID, tempID, baseBits, skill, scratch;
	int view;
	int action;
	int clutStart;
	int playerControlled, thiefProtection;
	RMRoom *room;
	char *name, *classID, *biography, *text, *basicName, *ownerName;
	int combatReady, summoned;
	int _locked, pvpGrace, initiative, girth, height, m_nEnchantResistance, randomChance, treasureTable, sqlID, characterID;

	LinkedElement *objElement, *affectElement, *characterElement;

	// special flags
	int hidden, destructing, makePolygon;

	// object references
	WorldObject *directObject, *indirectObject;
	WorldObject *curWeapon, *curShield;
	WorldObject *superObj, *linkTo, *sittingOn, *referenceObj;
	char *linkToStr;

	int linkToRoom, lastCrimeTime;

	// level range garbage
	int minLevel, maxLevel;

	// my owning player
	RMPlayer *player;

	// my character base (if any)
	BCharacter *character;

	// combat related properties
	int combatX, combatY;
	CombatGroup *combatGroup;

	// object number that I derive from
	int classNumber;

	// is this object visible to it's room
	int isVisible, allowVisible;

	Weapon *hands;
	int attackSkill, dodgeSkill, numAttacks, numDodges, numBlocks, numMoves, canRetaliate, combatRound, soundGroup;

	// affect information
	LinkedList *activeAffects;

	/* constructor */
	WorldObject(); 
	WorldObject ( WorldObject *src );

	virtual ~WorldObject();

	/* reset all of the properties of this object */
	void init ( void );

	/* copy another WorldObject */
	void copy ( WorldObject *object );

	/* clone this WorldObject */
	virtual WorldObject *clone ( void );

	/* set the biography */
	void setBiography ( char *str, int updateDB = 0 );

	/* set the readable text */
	void setText ( char *str );

	void setSuper ( char *str );
	void setClassID ( char *str );

	int isWorldObject ( void );

	void createHands ( void );

	//makes sure character is a valid size
	void validScale ( void );

	//return the age of this object/
	int age ( void );

	//return the first money object within my container base
	WorldObject *goldPile ( void );

	//link this object to another
	void linkWith ( WorldObject *object );
	void linkEntry ( WorldObject *object );

	//hash class interface
	virtual void setIntHashValue ( void ) { _intHashValue = servID; };
	virtual void setCharHashValue ( void ) { _charHashValue = hashString ( name ); };
	virtual int hashMatch ( int val ) { return servID == val; };
	virtual int hashMatch ( char *ptr ) { return !strcmp ( name, ptr ); };
	char *loginName ( void );

	/* base class management */
	WorldObjectBase *getBase ( int type );

	WorldObjectBase *addBase ( int type );

	/* packet manipulation */
	virtual void buildPacket ( PackedData *packet, int override = 0 );
	int calcPacketSize ( void );
	void generateTreasure ( void );
	void buildExtendedPacket ( PackedData *packet, int isMine );
	void fromExtendedPacket ( PackedData *packet );
	int combatRange ( void );
	int calcRepairCost ( void );

	/* message sending */
	void sendToOwner ( int command, void *msg, int size );
	void sendToRoom ( int command, void *msg, int size );
	void sendPlayerText ( const char *format, ... );

	/* movie processing */
//	void processMovie ( PackedData *movie, PackedData *result = NULL );
	int processMovie ( PackedData *movie, PackedData *result = NULL );
	void processTraps ( PackedData *packet, WorldObject *obj );

	/* miscellaneous utility methods */
	int headingToLoop ( int heading );
	int wearMask ( void );
	void writeSCIData ( FILE *file );
	int canAfford ( int val );
	int canAfford ( int val, int currency );
	int canAffordMana ( int val );
	int CanSee ( WorldObject *pObj );
	void changeMana ( int delta, PackedData *movie );
	int netWorth ( int inflation = 100 );
	long long sellWorth ( void );
	unsigned long long getWorth ( void );
	void makeItemList ( LinkedList *list, int value );
	int profession ( void );
	int getSkill ( int skill );
	int canHit ( WorldObject *obj );
	int armorRating ( int dType = -1 );
	int weaponSkillType ( void );
	int weaponSpeed ( void );
	void teleport ( int room, PackedData *packet, int bShowAnim = 1 );
	const char* setName ( const char* newName );
	int weight ( void );
	void addComponent ( char *name );

	/* damage processing */
	int takeDamage ( int type, WorldObject *damager, int amount, char *output, PackedData *movie, int show = 0, int melee = 0, int showMsg = 1 );
	void damageArmor ( int type, WorldObject *damager, WorldObject *weapon, WorldObject *armor, char *output, PackedData *movie, int amount = 0 );

	void writeToBuffer ( PackedData *packet );
	int loadFromBuffer ( WorldObject *owner, char **buffer, int *size ); 
	void writeAffectsToBuffer ( PackedData *packet );
	void loadAffectsFromBuffer ( char **buffer, int *size, int allowArtifacts );

	void writeLevelData ( void );

	affect_t *addAffect ( int id, int type, int source, int duration, int value, PackedData *packet = NULL, int doMakeVisible = 1 );
	int delAffect ( int id, int type, int source, PackedData *packet ) { return delAffect ( hasAffect ( id, type, source ), packet ); };
	int delAffect ( affect_t *affect, PackedData *packet = NULL );
	affect_t *hasAffect ( int id, int type, int source );
	affect_t *hasAffect ( int id, int type = _AFF_TYPE_NORMAL );
	void clearAffect ( int id, int type, PackedData *packet = NULL );
	int countAffects ( int type, int source );
	char * describeAffects ( void );

	int isWeak ( int id ) { return hasAffect ( id, _AFF_TYPE_WEAKNESS ) && !hasAffect ( id, _AFF_TYPE_RESISTANCE ); };
	int isProtected ( int id ) { return !hasAffect ( id, _AFF_TYPE_WEAKNESS ) && hasAffect ( id, _AFF_TYPE_RESISTANCE ); };

	void processAffects ( PackedData *packet = NULL );

	/* room visibility control */
	void makeVisible ( int visibleState, PackedData *packet = NULL );

	/* action handling */
	int processActions ( int verb, PackedData *packet = NULL );
	int addAction ( int verb, action_t action, int argc, char **argv );

	void transferWearerAffects ( WorldObject *obj, PackedData *packet = NULL );
	void removeWearerAffects ( WorldObject *obj, PackedData *packet = NULL );

	/* submit all of my servIDs to a packed data structure */
	void submitServIDs ( PackedData *packet );	

	/* write this object as a character */
	void writeCharacterData ( void );

	int addToDatabase ( void );

	void updateWanted ( WorldObject *obj, int crimeType );

	// affected list management...
	void AddToAffectedObjects ( void );
	void DelFromAffectedObjects ( void );

	/* write this object fully out to the packed database */
	void writePacked ( void );
	void updatePacked ( void );

	/* utility methods */
	void calcAC ( void );
	int calcHealth ( void );
	int calcStamina ( void );
	int owns ( WorldObject *object, int levels = 100000 );
	int isOwnedBy ( WorldObject *object, int levels = 100000 );
	int wears ( WorldObject *object );
	int isWornBy ( WorldObject *object );
	WorldObject *getOwner ( void );
	WorldObject *getBaseOwner ( void );
	RMRoom *getRoom ( void );
	WorldObject *getWornOn ( int areaWorn );
	char *getName ( void );
	int coarseAlignment ( void );
	void deleteFromRoom ( int notify = 1 );
	int isMoney ( void ) { return physicalState & _STATE_MONEY? 1 : 0; };
	void changeHealth ( int delta, WorldObject *killer, int othersSee = 0, int egoSees = 0, PackedData *movie = NULL );
	void changeStamina ( int delta, int othersSee = 0, int egoSees = 0, PackedData *movie = NULL );
	int poisonLevel ( int source = -2 );
	void removeVolatileAffects ( PackedData *packet );

	// calculate the weight capacity of this object
	int calcWeightCap ( void );
	void calcWeight ( void );
	int getWeight ( void );

	// return the total number of attacks that this object can perform
	int calcNumAttacks ( void );

	// return the total number of dodges that this object can perform
	int calcNumDodges ( void );

	// return the total number of times that this object can block an attack
	int calcNumBlocks ( void );

	// return the total number of spaces that this object can move in a round
	int calcNumMoves ( void );

	// calculate (and return) the current attributes
	int calcStrength ( void );
	int calcDexterity ( void );
	int calcIntelligence ( void );
	int calcEndurance ( void );

	// return the percentage encumbrance of this object
	int encumberance ( void );

	// can this object attack
	int canAttack ( void );

	// can this object move
	int canMove ( void );

	// can this object block
	int canBlock ( void );

	// can this object dodge
	int canDodge ( void );

	int changeStructureHealth ( int delta, PackedData *movie );

	char *getPronoun ( int state = 0 );

	// drop until I'm unencumbered
	void dropUntilUnencumbered ( PackedData *packet );

	/* this method is called every game hour to update my properties */
	void doit ( void );
	void heal ( void );

	/* setup methods */
	WorldObject *addObject ( const char *name, int wearIt = 0 );
	WorldObject *addObject ( int count, const char *name, int wearIt = 0 );

	/* perform an action, dispatch to a handling base */
	int perform ( int action, ... );

	/* define all of the verb interface functions */
	int take ( WorldObject *object, PackedData *packet = NULL );
	int beTakenBy ( WorldObject *object );
	int drop ( WorldObject *object, PackedData *packet = NULL, int override = 0 );
	int beDroppedBy ( WorldObject *object );
	int putOn ( WorldObject *object, PackedData *packet = NULL );
	int bePutOn ( WorldObject *object ); 
	int takeOff ( WorldObject *object, PackedData *packet = NULL );
	int beTakenOff ( WorldObject *object );
	int give ( WorldObject *object, WorldObject *to, PackedData *packet = NULL );
	int beGivenTo ( WorldObject *object );
	int putIn ( WorldObject *object, WorldObject *container, PackedData *packet = NULL );
	int bePutIn ( WorldObject *object );
	int open ( WorldObject *object, PackedData *packet = NULL );
	int beOpened ( WorldObject *object );
	int close ( WorldObject *object, PackedData *packet = NULL );
	int beClosed ( WorldObject *object );
	int lock ( WorldObject *object, WorldObject *key = NULL, PackedData *packet = NULL );
	int beLocked ( WorldObject *object, WorldObject *key = NULL );
	int unlock ( WorldObject *object, WorldObject *key = NULL, PackedData *packet = NULL );
	int beUnlocked ( WorldObject *object, WorldObject *key = NULL );
	int consume ( WorldObject *object, PackedData *packet = NULL );
	int beConsumed ( WorldObject *object );
	int sit ( WorldObject *object, PackedData *packet = NULL );
	int beSatOn ( WorldObject *object );
	int stand ( PackedData *packet = NULL );
	int beStoodUpOn ( WorldObject *object );
	int memorize ( WorldObject *object, PackedData *packet = NULL );
	int beMemorized ( WorldObject *object );

	/* define the force verb actions */
	int forceIn ( WorldObject *object, int override = 1 );
	int forceOut ( void );
	int forceOn ( WorldObject *object );
	int forceOff ( void );

	// Scan the inventory for the first object that has BHead base.
	BHead* GetHead();

	// this member updates this object's building as changed
	void markBuildingAsChanged ( void );

	void *operator new ( size_t size, char* file, int nLine ) { return db_malloc ( size, file, nLine ); }

	void operator delete ( void *ptr ) { free ( ptr ); }
};

#define new new( __FILE__, __LINE__ )

#include "bases.hpp"

int skillSuccess ( int actingSkill, int resistingSkill, int modifier = 0 );
int nonCombatSkillSuccess ( int actingSkill, int resistingSkill );

int calcDamage ( int flatDamage, int piercing, WorldObject *obj );

extern WorldObject *gBlackBaldric;
extern WorldObject *gRealBlackBaldric;
extern WorldObject *gPurpleBaldric;
extern WorldObject *gSatoriBaldric;
extern WorldObject *gLiveChristmasTree;
extern WorldObject *gDeadChristmasTree;
extern WorldObject* gStaffItems[11];

#endif

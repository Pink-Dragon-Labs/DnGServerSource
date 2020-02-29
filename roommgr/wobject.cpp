/*
	WorldObject / WorldObjectBase classes
	author: Stephen Nichols
*/

#include <algorithm>
#include "wobject.hpp"
#include "bases.hpp"
#include "roommgr.hpp"
#include "globals.hpp"

MemoryAllocator gAffectAllocator ( sizeof ( affect_t ), 300000 );

const int WorldObject::_DAMAGE_CUT;
const int WorldObject::_DAMAGE_HEAL;
const int WorldObject::_DAMAGE_POISON;
const int WorldObject::_DAMAGE_INTERNAL;
const int WorldObject::_DAMAGE_EARTHQUAKE;
const int WorldObject::_DAMAGE_EXP_GAIN;
const int WorldObject::_DAMAGE_SILENCE;
const int WorldObject::_DAMAGE_PSYCHIC;

WorldObject *gBlackBaldric = NULL;
WorldObject *gRealBlackBaldric = NULL;
WorldObject *gPurpleBaldric = NULL;
WorldObject *gSatoriBaldric = NULL;
WorldObject *gLiveChristmasTree = NULL;
WorldObject *gDeadChristmasTree = NULL;
WorldObject* gStaffItems[11] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };

/* init this WorldObject */
WorldObject::WorldObject()
{
	init();
}

/* init this WorldObject from another WorldObject */
WorldObject::WorldObject ( WorldObject *object )
{
	if ( !object && !object->isWorldObject() ) {
		logDisplay ( "Invalid WorldObject passed to WorldObject copy." );
		return;
	}

	init();
	copy ( object );

	// update the super to be the object, not the object's super
	setSuper ( object->classID );

//	memcpy ( super, object->classID, sizeof ( super ) );
}

void WorldObject::copy ( WorldObject *object ) 
{
	/* copy all of my properties from the object */
//	servID = object->servID;
	view = object->view;
	loop = object->loop;
	action = object->action;
	clutStart = object->clutStart;
	girth = object->girth;
	height = object->height;
	color = object->color;
	x = object->x;
	y = object->y;
	xStep = object->xStep;
	yStep = object->yStep;
	physicalState = object->physicalState;
	numAttacks = object->numAttacks;
	numDodges = object->numDodges;
	numBlocks = object->numBlocks;
	numMoves = object->numMoves;
	canRetaliate = object->canRetaliate;
	attackSkill = object->attackSkill;
	dodgeSkill = object->dodgeSkill;
	skill = object->skill;
	soundGroup = object->soundGroup;

	if ( object->activeAffects ) {
		LinkedElement *element = object->activeAffects->head();

		while ( element ) {
			affect_t *affect = (affect_t *)element->ptr();
			element = element->next();

			addAffect ( affect->id, affect->type, affect->source, affect->duration, affect->value );
		}
	}

	strength = object->strength;
	dexterity = object->dexterity;
	intelligence = object->intelligence;
	quickness = object->quickness;
	endurance = object->endurance;
	value = object->value;
	manaValue = object->value;
	level = object->level;
	oldLevel = object->oldLevel;
	minLevel = object->minLevel;
	maxLevel = object->maxLevel;
	health = object->health;
	healthMax = object->healthMax;
	alignment = object->alignment;
	armor = object->armor;
	armorType = object->armorType;
	baseArmor = object->baseArmor;
	hunger = object->hunger;
	thirst = object->thirst;
	classNumber = object->classNumber;
	isVisible = object->isVisible;
	allowVisible = object->allowVisible;
	hidden = object->hidden;
	m_nEnchantResistance = object->m_nEnchantResistance;

	if ( object->hands ) {
		createHands();
		*hands = *object->hands;
	}

	randomChance = object->randomChance;
	treasureTable = object->treasureTable;

	setSuper ( object->super );
	setClassID ( object->classID );

//	memcpy ( super, object->super, sizeof ( super ) );
//	memcpy ( classID, object->classID, sizeof ( classID ) );

	// this pointer is copied without regard to leaks because the basicName
	// property should only be set on class definions in .OBJ files
	//
	basicName = object->basicName;

	setName ( object->name );

	if ( object->actions ) {
		actions = new LinkedList;

		LinkedElement *element = object->actions->head();

		while ( element ) {
			ActionInfo *info = (ActionInfo *)element->ptr();
			actions->add ( info->clone() );

			element = element->next();
		}
	}

	/* copy the object's bases */
	for ( int i=0; i<object->numBases; i++ ) {
		WorldObjectBase *objBase = object->pBaseList[i];
		WorldObjectBase *base = addBase ( objBase->type );
		base->copy ( objBase );
	}

	superObj = object->superObj;
	referenceObj = object;
}

WorldObject *WorldObject::clone ( void )
{
	if ( !this && !isWorldObject() ) { 
		logDisplay ( "Invalid WorldObject passed to WorldObject clone." );
		return NULL;
	}

	WorldObject *clone = new WorldObject;

	clone->copy ( this );

	return clone;
}

WorldObject::~WorldObject() {
	//if someone is sitting on me, stand them up!
	BSit* bsit = static_cast< BSit*>( getBase( _BSIT ) );

	if( bsit && bsit->owner && bsit->owner->room ) {
		WorldObject* sitter = bsit->owner;

		if( sitter->sittingOn == this ) {

			PackedMsg movie;
			movie.putLong ( sitter->servID );
			movie.putLong ( sitter->room->number );
			
			sitter->stand( &movie );

			movie.putByte ( _MOVIE_END );
			roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, movie.data(), movie.size(), sitter->room );
		}
	}

	DelFromAffectedObjects();

	if ( player && !player->tossing )
		player = NULL;

	if ( classID )
		gObjectTree.del ( classID );

	roomMgr->deleteObject ( this );

	destructing = 1;

	// if this object was sitting, undo it
	if ( sittingOn && isValidPtr ( sittingOn ) && (getPtrType ( sittingOn ) == _MEM_WOBJECT ) && !sittingOn->destructing ) {
		sittingOn->beStoodUpOn ( this );
		sittingOn = NULL;
	}

	deleteFromRoom();

	CombatGroup *group = combatGroup;

	if ( group ) {
		if ( player ) {
			group->deleteCharacter ( this );
		} else {
			group->deleteObject ( this );
		}
	}

	combatGroup = NULL;
	setBiography ( NULL );
	setText ( NULL );

	// toss the actions
	if ( actions && gLinkedListAllocator.isValidPtr ( (char *)actions ) ) {
		LinkedElement *element = actions->head();
	
		while ( element ) {
			ActionInfo *info = (ActionInfo *)element->ptr();
			element = element->next();
			delete info;
		}

		actions->release();
		delete actions;
		actions = NULL;
	}

	if ( servID > -1 )
		freeServID ( servID );

	if ( activeAffects ) {
		LinkedElement *element = activeAffects->head();

		while ( element ) {
			affect_t *affect = (affect_t *)element->ptr();
			element = element->next();
			delete affect;
		}
			
		activeAffects->release();
		delete activeAffects;

		activeAffects = NULL;
	}

	// free the name
	if ( name ) {
		free ( name );
		name = NULL;
	}

	if ( components ) {
		components->release();
		delete components;
		components = NULL;
	}

	if ( tokens ) {
		delete tokens;
		tokens = NULL;
	}

	if ( ownerName ) {
		free ( ownerName );
		ownerName = NULL;
	}

	setClassID ( NULL );
	setSuper ( NULL );

	if ( hands ) {
		delete hands;
		hands = NULL;
	}

	for ( int i=0; i<numBases; i++ )
		delete pBaseList[i];
}

int WorldObject::isWorldObject ( void )
{
	if ( !isValidPtr ( this ) || getPtrType ( this ) != _MEM_WOBJECT ) 
		return 0;
	else
		return 1;
}

/* reset all of the properties of this WorldObject */
void WorldObject::init ( void ) 
{
	setPtrType ( this, _MEM_WOBJECT );
	memset ( pBaseList, 0, sizeof ( pBaseList ) );
	numBases = 0;
	stamina = 250;
	sqlID = -1;
	m_nEnchantResistance = 0;

	objElement = NULL;
	affectElement = NULL;
	characterElement = NULL;
	ownerName = NULL;
	skill = 0;

	super = NULL;
	classID = NULL;

	activeAffects = NULL;
	hands = NULL;
	scratch = 0;
	x = 0;
	y = 0;
	xStep = 0;
	yStep = 0;
	color = 0;
	pvpGrace = 0;
	initiative = 0;
	girth = height = 100;
	lastCrimeTime = 0;
	randomChance = 0;
	treasureTable = -1;
	numAttacks = 0;
	numDodges = 0;
	numBlocks = 0;
	numMoves = 0;
	combatRound = 0;
	canRetaliate = 0;
	attackSkill = 0;
	dodgeSkill = 0;
	soundGroup = -1;

	objectWornOn = -1;
	objectWieldedOn = -1;
	linkToRoom = -1;

	ownerID = -1;
	physicalState = 0;
	thiefProtection = 0;

	strength = 0;
	dexterity = 0;
	intelligence = 0;
	quickness = 0;
	endurance = 0;
	oldStrength = 0;
	oldDexterity = 0;
	oldIntelligence = 0;
	oldEndurance = 0;
	value = 0;
	manaValue = 0;
	level = 0;
	oldLevel = 0;
	minLevel = 1;
	maxLevel = 1;
	creationTime = 0;
	health = 0;
	healthMax = 0;
	alignment = 0;
	baseArmor = 0;
	armor = 0;
	armorType = atNone;
	hunger = 0;
	thirst = 0;

	servID = -1;
	tempID = -1;
	view = 0;
	
	//mike- changed default loop from 2 to 0
	//loop = 2;
	loop = 0;
	
	action = 29;
	clutStart = 0;
	playerControlled = 0;
	room = NULL;
	name = strdup ( "" );
	basicName = "object";
	hidden = 0;
	destructing = 0;
	makePolygon = 1;
	isZoneObject = 0;

	player = NULL;

	biography = NULL;
	text = NULL;

	combatX = -1;
	combatY = -1;
	combatGroup = NULL;
	curWeapon = NULL;
	curShield = NULL;
	opposition = NULL;
	character = NULL;

	classNumber = 0;
	isVisible = 0;
	allowVisible = 1;
	combatReady = FALSE;
	_locked = 0;
	summoned = 0;

	superObj = NULL;
	referenceObj = NULL;
	linkTo = NULL;
	linkToStr = NULL;
	sittingOn = NULL;
	actions = NULL;
	components = NULL;
	tokens = NULL;

	roomNumber = -1;
}

void WorldObject::validScale ( void ) 
{
	BCharacter *bchar = (BCharacter *)getBase ( _BCHARACTER );

	if ( bchar ) {

		switch ( bchar->race ) {
			case 0: // human
			{
				if ( girth < 90 || girth > 110 ) girth = 100;
				if ( height < 90 || height > 110 ) height = 100;
			}
			break;
			case 1: // Orc
			{
				if ( girth < 100 || girth > 120 ) girth = 110;
				if ( height < 95 || height > 115 ) height = 105;
			}
			break;
			case 2:	// giant
			{
				if ( girth < 100 || girth > 120 ) girth = 110;
				if ( height < 100 || height > 120 ) height = 110;
			}
			break;
			case 3:	// elf
			{
				if ( girth < 80 || girth > 100 ) girth = 90;
				if ( height < 80 || height > 100 ) height = 90;
			}
			break;
		}
	}
}

void WorldObject::createHands ( void )
{
	if ( !hands )
		hands = new Weapon;
}

/* set the biography text */
void WorldObject::setBiography ( char *str, int updateDB )
{
	if ( biography ) {
		free ( biography );
		biography = NULL;
	}

	if ( str ) {
		int len = 0;

		len = strlen ( str );

		if ( len < 1 || len > 512 )
			str = "No biography.";

		char *ptr = strrchr ( str, '\n' );

		if ( ptr )
			*ptr = 0;

		ptr = str;

		while ( *ptr ) {
			if ( !isprint ( *ptr ) || *ptr == '|') {
				str = "No biography.";
				break;
			}

			ptr++;
		}

		biography = strdup ( str );
	}
}

/* set the readable text */
void WorldObject::setText ( char *str )
{
	if ( text ) {
		free ( text );
		text = NULL;
	}

	if ( str )
		text = strdup ( str );
}

/* set the class of this object */
void WorldObject::setClassID ( char *str )
{
	if ( classID ) {
		free ( classID );
		classID = NULL;
	}

	if ( str )
		classID = strdup ( str );
}

/* set the super of this object */
void WorldObject::setSuper ( char *str )
{
	if ( super ) {
		free ( super );
		super = NULL;
	}

	if ( str )
		super = strdup ( str );
}

/* return the age of this object in seconds */
int WorldObject::age ( void )
{
	return getseconds() - creationTime;
}

/* return the first money object in my container base */
WorldObject *WorldObject::goldPile ( void )
{
	WorldObject *retVal = NULL;

	BContainer *base = (BContainer *)getBase ( _BCONTAIN );

	if ( base )
		retVal = base->goldPile();

	return retVal;
}

/* search the pBaseList for a base class that matches the passed type */
WorldObjectBase *WorldObject::getBase ( int type )
{
	if ( !this || !isWorldObject() ) {
		return NULL;
	}

	if ( !pBaseList ) {
		logInfo ( _LOG_ALWAYS, "getBase::pBaseList == NULL" );
		return NULL;
	}

	if ( numBases >= _MAX_BASES ) {
		logInfo ( _LOG_ALWAYS,  "getBase::too many bases in object" );
		return NULL;
	}

	if ( numBases < 0 ) {
		logInfo ( _LOG_ALWAYS,  "getBase::too few bases in object" );
		return NULL;
	}

	for ( int i=0; i<numBases; i++ ) {
		WorldObjectBase *base = pBaseList[i];
	
		if ( base ) {
			if ( base->type == type )
				return base;
		} else {
			logInfo ( _LOG_ALWAYS, "getBase::null base in object" );
		}
	}

	return NULL;
}

/* add a new instance of a base class to the pBaseList */
WorldObjectBase *WorldObject::addBase ( int type )
{
	WorldObjectBase *retVal = NULL;

	if ( !this || !isWorldObject() ) {
		return retVal;
	}

	retVal = getBase ( type );

	if ( retVal )
		return retVal;

	switch ( type ) {
		case _BCARRY:
			retVal = new BCarryable ( this );
			break;

		case _BCONTAIN:
			retVal = new BContainer ( this );
			break;

		case _BPLAYER:
			retVal = new BPlayer ( this );
			break;

		case _BHEAD:
			retVal = new BHead ( this );
			break;

		case _BWEAR:
			retVal = new BWearable ( this );
			break;

		case _BOPEN:
			retVal = new BOpenable ( this );
			break;

		case _BDESCRIBED:
			retVal = new BDescribed ( this );
			break;

		case _BCYCLE:
			retVal = new BCycle ( this );
			break;

		case _BCHARACTER:
			retVal = new BCharacter ( this );
			character = (BCharacter *)retVal;
			if( reinterpret_cast< int>( character ) == 0x21 ) { logInfo( _LOG_ALWAYS, "%s:%d - BCharacter value corrupted", __FILE__, __LINE__ ); }
			break;

		case _BLOCK:
			retVal = new BLockable ( this );
			break;

		case _BKEY: 
			retVal = new BKey ( this );
			break;

		case _BWEAPON:
			retVal = new BWeapon ( this );
			break;

		case _BENTRY:
			retVal = new BEntry ( this );
			break;

		case _BSHOP:
			retVal = new BShop ( this );
			break;

		case _BNPC:
			retVal = new BNPC ( this );
			break;

		case _BCONSUME:
			retVal = new BConsumable ( this );
			break;

		case _BPASSWORD:
			retVal = new BPassword ( this );
			break;

		case _BGATE:
			retVal = new BGate ( this );
			break;

		case _BSIT:
			retVal = new BSit ( this );
			break;

		case _BTREASURE:
			retVal = new BTreasure ( this );
			break;

		case _BSCROLL:
			retVal = new BScroll ( this );
			break;

		case _BTALK:
			retVal = new BTalk ( this );
			break;

		case _BUSE:
			retVal = new BUse ( this );
			break;

		case _BMIX:
			retVal = new BMix ( this );
			break;

		case _BSWITCH: 
			retVal = new BSwitch ( this );
			break;

		case _BDYE:
			retVal = new BDye ( this );
			break;

		case _BSPELLBAG:
			retVal = new BSpellBag ( this );
			break;
	}

	if ( retVal ) {
		if ( numBases == _MAX_BASES )
			fatal ( "too many bases in object" );

		pBaseList[numBases] = retVal;
		numBases++;

		if ( retVal->type < _BMAX ) 
			baseBits |= (1 << retVal->type);
	}

	return retVal;
}

/* calculate the size of a packet for this object */
int WorldObject::calcPacketSize ( void )
{
	PackedData packet;
	buildPacket ( &packet );

	return packet.size();
}

int WorldObject::calcRepairCost ( void )
{
	int percent = 100 - ((health * 100) / healthMax); 
	int cost = (percent * value) / 100;

	// modify the repair cost by the number of enchantments this item has on it
	int spellAffects = countAffects ( -1, _AFF_SOURCE_WEARER ) + countAffects ( -1, _AFF_SOURCE_PERMANENT );
//	cost += ( cost * ( ( spellAffects * spellAffects ) * 10 ) ) / 100;
	cost += ( cost * ( spellAffects * 10 ) ) / 100;

//	int discount = (cost * 33) / 100;
//	cost -= discount;

	return std::max(cost, 25);//cost >? 25;
}

/* handle building a packet for this object */
void WorldObject::buildPacket ( PackedData *packet, int override )
{
	if ( isVisible || override ) {
		WorldObject *theOwner = getOwner();

		//
		// NOTE:  Make no format changes between this comment and the corresponding
		// comment below.  The client is specially tied to this first part of this
		// message.  You WILL get bizarre errors if this is changed without care.
		//

		packet->putByte ( 1 );

		// put the class number
		packet->putWord ( classNumber );
		// put the constant properties
		packet->putLong ( servID );

		// 
		// NOTE:  You can add things after this line.
		//

		// put the color of the object
		if ( !getBase ( _BCHARACTER ) ) {
// remove glowie color			packet->putWord ( (color == -1)? gColorTable[0] : color );
			packet->putWord ( (color == -1)? gColorTable[1] : color );
		}
	
		int numAffects = 0;

		if ( hasAffect ( _AFF_ENGRAVED ) || (physicalState & _STATE_SPECIAL) ) {
			packet->putByte ( 1 );
			packet->putString ( name );
		} else {
			packet->putByte ( 0 );
		}

		if ( activeAffects ) {
			LinkedElement *element = activeAffects->head();

			while ( element ) {
				affect_t *affect = (affect_t *)element->ptr();
				element = element->next();
	
				if ( affect->type == _AFF_TYPE_NORMAL )
					packet->putByte ( affect->id );
			}
		}

		packet->putByte ( 255 );	// -1

		// if this object is money or mana, put that here
		if ( physicalState & _STATE_MONEY ) {
			if ( view == _MANA_VIEW ) {
				packet->putByte ( manaValue > 65535? 2 : 1 );

				if ( manaValue > 65535 ) {
					packet->putLong ( manaValue );
				} else {
					packet->putWord ( manaValue );
				}
			} else {
				packet->putByte ( value > 65535? 2 : 1 );

				if ( value > 65535 ) {
					packet->putLong ( value );
				} else {
					packet->putWord ( value );
				}
		 	}	
		} else {
			packet->putByte ( 0 );
		}

		BCharacter *bcharacter = (BCharacter *)getBase ( _BCHARACTER );
		if( reinterpret_cast< int>( character ) == 0x21 ) { logInfo( _LOG_ALWAYS, "%s:%d - BCharacter value corrupted", __FILE__, __LINE__ ); }

		// if this object is a character, put that here
		if ( bcharacter ) {
			// put xStep and yStep
			packet->putByte ( xStep );
			packet->putByte ( yStep );
			packet->putByte ( girth );
			packet->putByte ( height );
	
			bcharacter->buildPacket ( packet, override );
		}
	
		if ( room ) {
			int noScale = (physicalState & _STATE_NOSCALE)? 1 : 0;
			#define _NOSCALE_MASK 16

			// put the room number and x, y
			if ( room->number > 65535 ) {
				packet->putByte ( noScale? (2 | _NOSCALE_MASK) : 2 );
				packet->putLong ( room->number );
			} else {
				packet->putByte ( noScale? (1 | _NOSCALE_MASK) : 1 );
				packet->putWord ( room->number );
			}

			packet->putWord ( x );
			packet->putWord ( y );
			packet->putByte ( loop );
		} else {
			packet->putByte ( 0 );
		}
	
		// if this object is a carryable and it belongs to an object, put it here
		BCarryable *bcarryable = (BCarryable *)getBase ( _BCARRY );

		if ( bcarryable ) 
			bcarryable->buildPacket ( packet, override );
	
		BWearable *bwear = (BWearable *)getBase ( _BWEAR );
		BWeapon *bweapon = (BWeapon *)getBase ( _BWEAPON );
		BHead *bhead = (BHead *)getBase ( _BHEAD );
		BContainer *bcontain = (BContainer *)getBase ( _BCONTAIN );
		BLockable *block = (BLockable *)getBase ( _BLOCK );
		BDescribed *bdescribed = (BDescribed *)getBase ( _BDESCRIBED );
	
		if ( bweapon ) 
			packet->putByte ( bweapon->owner? 1 : 0 );	
	
		else if ( bhead )
			bhead->buildPacket ( packet, override );
	
		else if ( bwear ) 
			packet->putByte ( bwear->owner? 1 : 0 );
			
		//if this is a book, lets say so
		if( bdescribed ) {
			packet->putByte ( bdescribed->isBook? 0x01 : 0x00 );

		//the client crashes strangely (and not consistently) when I don't include a byte regardless...
		//there might be some kind of synchronization issue - it might think something does not
		//have the BDescribed base when it does, or vice versa. 2004-12-14
		} else packet->putByte ( 0x00 );

		// if this object is a container, put that here
		if ( bcontain )
			bcontain->buildPacket ( packet, override );
	
		// if this object is open, put that here
		if ( getBase ( _BOPEN ) )
			packet->putByte ( !(physicalState & _STATE_CLOSED) );

		// if this object is a switch, put information here
		if ( getBase ( _BSWITCH ) )
			packet->putByte ( !(physicalState & _STATE_CLOSED) );
	
		// if this object is locked, put that here
		if ( block ) {
			int mask = 1;

			if ( physicalState & _STATE_LOCKED )
				mask |= 2;

			if ( block->autoLock )
				mask |= 4;

			packet->putByte ( mask );
		} else {
			packet->putByte ( 0 );
		}

		// if this object is sitting on another object
		packet->putByte ( sittingOn? 1 : 0 );

		if ( sittingOn ) 
			packet->putLong ( sittingOn->servID );

		// if this object is talkable, put that here
		packet->putByte ( getBase ( _BTALK )? 1 : 0 );

		// tack on the special flag information
		packet->putByte ( hidden? 1 : 0 );

		// put the combat group ID if there is one
		if ( combatGroup && strcmp ( classID, "CombatCloud" ) ) {
			packet->putByte ( 1 );
			packet->putLong ( combatGroup->servID() );
			packet->putByte ( combatGroup->attackers.contains ( this ) );
		} else {
			packet->putByte ( 0 );
		}
	} else {
		packet->putByte ( 0 );
	}
}

void WorldObject::generateTreasure ( void )
{
	static int wentBad = 0;

	if ( treasureTable > -1 ) {
		int theLevel = (level > 1)? level : 2;
		int value = random ( 15 * theLevel, 40 * theLevel );

		if ( random ( 1, 100 ) == 50 )
			value *= 2;

		if ( !random ( 0, 10 ) ) {
			if ( random ( 0, 2 ) == 0 ) {
				this->manaValue += ((value / 2) / 5) + 1;
			} else {
				this->value += value / 2;
			}
		} else {
			int index = 0;
			WorldObject *obj = NULL;

			while ( (obj = gTreasureTbls[treasureTable][index]) ) {
				if ( obj && obj->isWorldObject() ) {
					if ( obj->value >= value || (!gTreasureTbls[treasureTable][index + 1]) ) {
						// scan for end of table
						int start = index, end = index;
	
						while ( (obj = gTreasureTbls[treasureTable][end]) ) {
							if ( obj->value > value )
								break;
		
							end++;
						}
		
						if ( start == end ) {
							start -= 5;
							if (start < 0)
								start = 0;
						}

						obj = gTreasureTbls[treasureTable][random ( start, end )];
		
						if ( obj ) { 
							WorldObject *pObj = addObject ( obj->classID );

							if ( pObj )
								pObj->physicalState |= _STATE_TREASURE;
						}
		
						break;
					}
		
				} else {
					if ( !wentBad ) {
						wentBad = 1;
						logInfo ( _LOG_ALWAYS, "treasure table entry [%d][%d] is invalid (0x%x)", treasureTable, index, obj );
					}
				}

				index++;
			}
		}
	}
}

/* handle building an extended info packet for this object */
void WorldObject::buildExtendedPacket ( PackedData *packet, int isMine ) 
{
	//mike - Changed this packet format to save bandwith, streamline this a bit, and remove
	//some unused old character properties
	//the client knows how to assume zero's where appropriate

	//mike- put a little red herring here
	unsigned char redByte = random( 0, 255 );
	if( isMine ) {
		//make the 2nd lowest bit a ONE
		redByte |= 0x02;
	} else {
		//make the 2nd lowest bit a ZERO
		redByte &= 0xFD;
	}

	packet->putByte( redByte );

	if ( isMine ) {
		packet->putByte ( calcStrength() );
		packet->putByte ( calcDexterity() ); 
		packet->putByte ( calcIntelligence() );
		packet->putByte ( calcEndurance() );
		packet->putLong ( value );
		packet->putLong ( manaValue );
		packet->putWord ( level );
		packet->putByte ( alignment );
	}

	// common attributes
	packet->putLong ( health );
	packet->putLong ( healthMax );

	BCharacter *bchar = (BCharacter *)getBase ( _BCHARACTER );

	if ( bchar ) {
		packet->putString ( bchar->title );

		if ( isMine ) {
			packet->putLong ( bchar->experience );
			packet->putLong ( bchar->stealingCount );
			packet->putLong ( bchar->stealingUnserved );
			packet->putLong ( bchar->killingCount );
			packet->putLong ( bchar->killingUnserved );

			for ( int i=0; i<_SKILL_MAX; ++i ) 
				packet->putByte ( bchar->skills[i] );

			for ( int i=0; i<_SPELL_MAX; ++i )
				packet->putByte ( bchar->spells[i] );

			unsigned char affectArray[_AFF_MAX];
			memset ( affectArray, 0, sizeof ( affectArray ) );

			if ( activeAffects ) {
				LinkedElement *element = activeAffects->head();

				while ( element ) {
					affect_t *affect = (affect_t *)element->ptr();
					element = element->next();

					affectArray[affect->id] |= (1 << affect->type);
				}
			}

			packet->putArray ( affectArray, _AFF_MAX );
			packet->putWord ( bchar->buildPoints );

		}
	}

	packet->putWord ( armorRating() );

	//pvp status
	if ( bchar )
		packet->putByte ( bchar->peaceful );
	else
		packet->putByte ( 0 );
}

/* handle getting properties from an extended packet */
void WorldObject::fromExtendedPacket ( PackedData *packet )
{
	strength = packet->getByte();
	dexterity = packet->getByte();
	intelligence = packet->getByte();
	endurance = packet->getByte();
	alignment = packet->getByte();
	girth = packet->getByte();
	height = packet->getByte();

	calcWeightCap();
}

/* send a message to my owning player */
void WorldObject::sendToOwner ( int command, void *msg, int size )
{
	if ( !player ) 
		sendToRoom ( command, msg, size );
	else
		roomMgr->sendTo ( command, (IPCPMMessage *)msg, size, player );
}

/* send a message to my room */
void WorldObject::sendToRoom ( int command, void *msg, int size )
{
	if ( room )
		roomMgr->sendToRoom ( command, (IPCPMMessage *)msg, size, room );
}

/* send a text message to my player */
void WorldObject::sendPlayerText ( const char *format, ... )
{
	char text[1024] = "";

	va_list args;
	va_start ( args, format );
	vsprintf ( sizeof ( text ), text, format, args );

	if ( player ) 	
		roomMgr->sendPlayerText ( player, text );
}

int WorldObject::addToDatabase ( void )
{
	if ( servID == -1 ) {
		if ( color == -1 ) {
// removed the odd colors			int newColor = random ( 0, _MAX_COLOR - 1 );
			int newColor = random ( 0, 22 ) + 1;
			color = gColorTable[ newColor ];

			if ( color < 43 || color > 107 ) {
				logInfo ( _LOG_ALWAYS, "got invalid color from color table (index = %d, value = %d)", newColor, color );
				color = 58;
			}
		}

		servID = allocateServID();
		roomMgr->addObject ( this );
	}

	return servID;
}

char *WorldObject::loginName ( void )
{
	if ( player && player->player ) {
		BPlayer *bplayer = (BPlayer *)player->player->getBase ( _BPLAYER );

		if ( bplayer )
			return bplayer->login;
	}

	return NULL;
}

void WorldObject::writeCharacterData ( void )
{
	if ( !playerControlled )
		return;

	if ( player ) 
		player->writeQuestData();

	BCharacter *bchar = (BCharacter *)getBase ( _BCHARACTER );

	if ( bchar ) {
		char *name = NULL;

		if ( bchar->properName )
			name = bchar->properName;

		if ( physicalState & _STATE_HACKER )
			return;

		PackedData *data = new PackedData;
		data->grow ( 100000 );

		data->printf ( "%s\n%d\n", ownerName? ownerName : "", creationTime );
		data->printf ( "%s\n", classID );
		data->printf ( "%s\n", bchar->properName );
		data->printf ( "%s\n", bchar->title );
		data->printf ( "%d:%d\n", level, bchar->topLevel );
		data->printf ( "%d\n", bchar->experience );
		data->printf ( "%d\n", bchar->buildPoints );
		data->printf ( "%d\n", bchar->homeTown );
		data->printf ( "%d\n", strength );
		data->printf ( "%d\n", dexterity );
		data->printf ( "%d\n", intelligence );
		data->printf ( "%d\n", quickness );
		data->printf ( "%d\n", endurance );

		data->printf ( "%d\n", health);
		data->printf ( "%d\n", healthMax );

		data->printf ( "%d\n", hunger );
		data->printf ( "%d\n", thirst );
		data->printf ( "%d\n", alignment );
		data->printf ( "%d\n", value );
		data->printf ( "%d\n", manaValue );
		data->printf ( "%d\n", physicalState );

		int j;

		for ( j=0; j<_SKILL_MAX; j++ )
			data->printf ( "%1d", bchar->skills[j] );

		data->printf ( "\n" );

		for ( j=0; j<_SPELL_MAX; j++ )
			data->printf ( "%1d", bchar->spells[j] );

		data->printf ( "\n" );

		data->printf ( "%d\n", bchar->stealingCount );
		data->printf ( "%d\n", bchar->stealingUnserved );
		data->printf ( "%d\n", bchar->killingCount );
		data->printf ( "%d\n", bchar->killingUnserved );
		data->printf ( "%d\n", bchar->peaceful );
		data->printf ( "%d\n", bchar->warnCount );

		BContainer *bcontain = (BContainer *)getBase ( _BCONTAIN );
		BHead *bhead = NULL; 

		LinkedElement *element = bcontain->contents.head();

		while ( element ) {
			WorldObject *obj = (WorldObject *)element->ptr();
			bhead = (BHead *)obj->getBase ( _BHEAD );

			if ( bhead )
				break;

			element = element->next();
		}

		if ( bhead ) {
			data->printf ( "%d\n", bhead->race );
			data->printf ( "%d\n", bhead->sex );
			data->printf ( "%d\n", bhead->skinColor );
			data->printf ( "%d\n", bhead->headNumber );
			data->printf ( "%d\n", bhead->hairNumber );
			data->printf ( "%d\n", bhead->hairColor );
			data->printf ( "%d\n", bhead->browNumber );
			data->printf ( "%d\n", bhead->faceHairNumber );
			data->printf ( "%d\n", bhead->eyeNumber );
			data->printf ( "%d\n", bhead->eyeColor );
			data->printf ( "%d\n", bhead->noseNumber );
			data->printf ( "%d\n", bhead->mouthNumber );
			data->printf ( "%d\n", bhead->earNumber );
		} else {
			data->printf ( "-1\n" );
		}

		// data->printf ( "%d\n", roomNumber );
		data->printf ( "%d:%d\n", roomNumber, bchar->lastDungeon );
		data->printf ( "%d\n%d\n%d\n", x, y, loop );

		if ( biography ) {
			data->printf ( "%s\n", biography );
		} else {
			data->printf ( "No biography.\n" );
		}

		data->printf ( "%d\n%d\n", girth, height );
		data->printf ( "%d\n", 1 );

		// recalc crime time if expired

		if ( 2592000 - (getseconds() - lastCrimeTime) <= 0 )
			lastCrimeTime = 0;

		data->printf ( "%d\n", lastCrimeTime );

		writeAffectsToBuffer ( data );

		if ( bhead ) 
			bcontain->contents.del ( bhead->self );

		data->printf ( "%d\n", bcontain->contents.size() );

		element = bcontain->contents.head();

		while ( element ) {
			WorldObject *obj = (WorldObject *)element->ptr();
			element = element->next();
			obj->writeToBuffer ( data );
		}

		if ( bhead ) 
			bcontain->contents.add ( bhead->self );

		// put kill counts 
		data->printf ( "%d\n%d\n", bchar->playerKills, bchar->npcKills );
		data->printf ( "%d\n", oldLevel );
		data->printf ( "%d\n", stamina );
		data->printf ( "%d\n", getseconds() );

		// write the character
		if ( sqlID == -1 ) {
			gDataMgr->newCharacter ( this, data );
		} else {
			gDataMgr->writeCharacter ( this, data );
		}

		delete data;
	}
}

void WorldObject::writeAffectsToBuffer ( PackedData *data ) 
{
	data->printf ( "%d\n", activeAffects? activeAffects->size() : 0 );

	if ( activeAffects ) {
		LinkedElement *element = activeAffects->head();

		while ( element ) {
			affect_t *affect = (affect_t *)element->ptr();
			element = element->next();

			data->printf ( "%d %d %d %d %d\n", affect->id, affect->type, affect->source, affect->duration, affect->value );
		}
	}
}

void WorldObject::loadAffectsFromBuffer ( char **buffer, int *size, int allowArtifacts )
{
	LinkedElement *element = NULL;
	LinkedList *tokens = NULL;
	StringObject *token = NULL;
	char str[1024] = "";

	if ( activeAffects ) {
		// toss all affects that I already have -- we're loading from disk!
		element = activeAffects->head();

		while ( element ) {
			affect_t *affect = (affect_t *)element->ptr();
			element = element->next();

			delAffect ( affect, NULL );
		}
	}

	int numAffects = bufgetint ( str, buffer, size );

	int value, type, source, id, duration = 0;

	while ( numAffects ) {

		// get the first line of text
		bufgets ( str, buffer, size, sizeof ( str ) );

		tokens = buildTokenList ( str );

		if ( tokens->size() != 5 ) {
			goto skipAffect;
		}

		element = tokens->head();

		token = (StringObject *)element->ptr();

		id = atoi ( token->data );
		
		element = element->next();
		token = (StringObject *)element->ptr();

		type = atoi ( token->data );

		element = element->next();
		token = (StringObject *)element->ptr();

		source = atoi ( token->data );

		element = element->next();
		token = (StringObject *)element->ptr();

		duration = atoi ( token->data );

		element = element->next();
		token = (StringObject *)element->ptr();

		value = atoi ( token->data );

		// remove invalid affects from characters and heads

		if ( getBase ( _BHEAD ) || getBase ( _BCHARACTER ) )
		{
			if ( source == _AFF_SOURCE_PERMANENT || source == _AFF_SOURCE_WEARER )
				goto skipAffect;
		}

		// check for invalid extension affect

		if ( source != _AFF_SOURCE_ARTIFACT && id == _AFF_EXTENSION )
			goto skipAffect;

		if ( source == _AFF_SOURCE_TEMPORARY )
			goto skipAffect;

		if ( (source == _AFF_SOURCE_ARTIFACT) && !allowArtifacts )
			goto skipAffect;

		// check for duped affects. 
		if ( !hasAffect ( id, type, source ) )
			addAffect ( id, type, source, duration, value );

skipAffect:

		if ( tokens )
			delete tokens;

		numAffects--;
	}
}

int WorldObject::loadFromBuffer ( WorldObject *owner, char **buffer, int *size )
{
	char str[1024];

	// get the class name to base this object off of
	bufgets ( str, buffer, size, sizeof ( str ) );

	WorldObject *superObj = roomMgr->findClass ( str );

	if ( !superObj ) {
		logHack ( "super class of %s could not be found.", str? str : "<unknown>" );
//		fatal ( "super class of '%s' could not be found", str );

		// skip data for this object
		bufgets ( str, buffer, size, sizeof ( str ) );
		bufgets ( str, buffer, size, sizeof ( str ) );
		bufgets ( str, buffer, size, sizeof ( str ) );
		bufgets ( str, buffer, size, sizeof ( str ) );
		bufgets ( str, buffer, size, sizeof ( str ) );
		bufgets ( str, buffer, size, sizeof ( str ) );
		bufgets ( str, buffer, size, sizeof ( str ) );
		bufgets ( str, buffer, size, sizeof ( str ) );
		bufgets ( str, buffer, size, sizeof ( str ) );

		int count = bufgetint ( str, buffer, size );

		while ( count > 0 ) {
			bufgets ( str, buffer, size, sizeof ( str ) );
			count--;
		}

		bufgets ( str, buffer, size, sizeof ( str ) );
		bufgets ( str, buffer, size, sizeof ( str ) );
		bufgets ( str, buffer, size, sizeof ( str ) );
		bufgets ( str, buffer, size, sizeof ( str ) );
		bufgets ( str, buffer, size, sizeof ( str ) );
		bufgets ( str, buffer, size, sizeof ( str ) );
		bufgets ( str, buffer, size, sizeof ( str ) );
		bufgets ( str, buffer, size, sizeof ( str ) );
		bufgets ( str, buffer, size, sizeof ( str ) );

		return 0;
	}

	// change super obj if it's a "real" black baldric
	if ( superObj == gRealBlackBaldric ) {
		superObj = gBlackBaldric;
	}

	// change live trees to dead trees
#if 0
	if ( superObj == gLiveChristmasTree ) {
		superObj = gDeadChristmasTree;
	}
#endif

	copy ( superObj );

	setSuper ( superObj->classID );
//	memcpy ( super, superObj->classID, sizeof ( super ) );

	addToDatabase();

	BWearable *bwear = (BWearable *)getBase ( _BWEAR );
	BWeapon *bweapon = (BWeapon *)getBase ( _BWEAPON );
	BOpenable *bopen = (BOpenable *)getBase ( _BOPEN );
	BLockable *block = (BLockable *)getBase ( _BLOCK );
	BPassword *bpass = (BPassword *)getBase ( _BPASSWORD );
	BEntry *bentry = (BEntry *)getBase ( _BENTRY );
	BUse *buse = (BUse *)getBase ( _BUSE );
	BContainer *bcontain = (BContainer *)getBase ( _BCONTAIN );
	BSpellBag *bspellbag = (BSpellBag *)getBase ( _BSPELLBAG );
	BCarryable *bcarry = (BCarryable *)getBase ( _BCARRY );

	// get any special name for this object
	bufgets ( str, buffer, size, sizeof ( str ) );

	if ( !str )
		sprintf ( sizeof ( str ), str, "Object" );

	if ( strlen ( str ) ) {
		char *ptr = str;

		while ( *ptr ) {
			if ( !isprint ( *ptr ) ) {
				sprintf ( sizeof ( str ), str, "Object" );
				break;
			}	

			ptr++;
		}

		try {
		    setName ( str ); //CHECK
		} catch (int e) {
		    logDisplay("setName %d", e);
		    return -1;
		}
	}

	color = bufgetint ( str, buffer, size );

	if ( superObj == gBlackBaldric ) {
		color = 100;
	}

	else if ( superObj == gPurpleBaldric ) {
		color = 59;
	}

	if ( color < 0 || color > 255 )
		color = 58;

	// check for SatoriBaldric color hack
	 //if ( (color == 30) && (superObj != gSatoriBaldric) )
		//color = 99;

	int nPhysicalState = bufgetint ( str, buffer, size );
	physicalState = ( physicalState & _STATE_TOSS ) | nPhysicalState;

	value = bufgetint ( str, buffer, size );
	manaValue = bufgetint ( str, buffer, size );
	health = bufgetint ( str, buffer, size );
	x = bufgetint ( str, buffer, size );
	y = bufgetint ( str, buffer, size );
	loop = bufgetint ( str, buffer, size );

	// make sure we're in our owner and owner was a valid object
	if ( owner && owner->isWorldObject() )
        try{
		    forceIn ( owner );
        } catch (int e) {
		    logDisplay("forceIn %d", e);
		    return -1;
		}

	// clip the health
	if ( health < 1 )
		health = 1;

	if ( health > healthMax )
		health = healthMax;

	// read affect information
	try{
	    loadAffectsFromBuffer ( buffer, size, 1 );
	}catch (int e) {
        logDisplay("loadAffectsFromBuffer %d", e);
        return -1;
    }
	for (int nAffects = 0;nAffects < 11;nAffects++) {
		if ( superObj == gStaffItems[nAffects] ) {
			if ( !hasAffect( _AFF_STAFF, 0 ) )
				try{
				    addAffect( _AFF_STAFF, 0, 5, -1, 0 );
				} catch (int e) {
		            logDisplay("addAffect %d", e);
		            return -1;
		        }
		}
	}

	// get wearable information
	bufgets ( str, buffer, size, sizeof ( str ) );

	if ( bwear ) {
		if ( str[0] == '1' && owner ) {
			try{
			    forceOn ( owner );
            } catch (int e) {
    		    logDisplay("forceOn1 %d", e);
	    	    return -1;
		    }
		}
	}

	// get weapon information
	bufgets ( str, buffer, size, sizeof ( str ) );

	if ( bweapon ) {
		if ( str[0] == '1' && owner ) {
			try {
			    forceOn ( owner );
            } catch (int e) {
    		    logDisplay("forceOn2 %d", e);
	    	    return -1;
	    	}
		}
	}

	// get openable information
	bufgets ( str, buffer, size, sizeof ( str ) );

	if ( bopen ) {
		if ( str[0] == '1' ) {
		    try{
                bopen->isOpen ( 1 );
                isVisible = 1;
            } catch (int e) {
    		    logDisplay("bopen-1 %d", e);
	    	    return -1;
	    	}
		} else {
			try{
			    bopen->isOpen ( 0 );
			} catch (int e) {
    		    logDisplay("bopen-0 %d", e);
	    	    return -1;
	    	}
		}
	}

	// get lockable information
	bufgets ( str, buffer, size, sizeof ( str ) );

	if ( block ) {
		if ( str[0] == '1' ) {
			try{
			    block->isLocked ( 1 );
			} catch (int e) {
    		    logDisplay("isLocked-1 %d", e);
	    	    return -1;
	    	}
		} else {
			try{
			    block->isLocked ( 0 );
			} catch (int e) {
    		    logDisplay("isLocked-0 %d", e);
	    	    return -1;
	    	}
		}
	}

	// get entry information
	bufgets ( str, buffer, size, sizeof ( str ) );

	if ( bentry ) 
		try{
		    linkTo = (WorldObject *)atoi ( str );
		} catch (int e) {
            logDisplay("linkTo %d", e);
            return -1;
        }

	// get password information
	bufgets ( str, buffer, size, sizeof ( str ) );

	if ( bpass ){
	    if (strlen(str) > 16){
	        str[16] = '\0';
	    }
		try{
		    strcpy ( bpass->password, str );
		} catch (int e) {
            logDisplay("bpass %d", e);
            return -1;
        }
    }
	// get use information
	bufgets ( str, buffer, size, sizeof ( str ) );

	if ( buse )
		try{
		    buse->uses = atoi ( str );
		} catch (int e) {
            logDisplay("buse %d", e);
            return -1;
        }

	// get the container information
	bufgets ( str, buffer, size, sizeof ( str ) );

	// get the spellbag information
	bufgets ( str, buffer, size, sizeof ( str ) );

	if ( bspellbag ) {
		try{
            LinkedList *tokens = buildTokenList ( str );
            LinkedElement *element = tokens->head();

            while ( element ) {
                StringObject *name = (StringObject *)element->ptr();
                element = element->next();
                int quantity = atoi ( ((StringObject *)element->ptr())->data );
                element = element->next();

                WorldObject *superObj = roomMgr->findClass ( name->data );
                bspellbag->addComponent ( superObj, quantity );
            }
            delete tokens;
        } catch (int e) {
            logDisplay("bspellbag %d", e);
            return -1;
        }
	}

	if ( bcontain ) {
        try {
            int numObjects = atoi ( str );

            if ( numObjects < 0 ) {
                logInfo ( _LOG_ALWAYS, "Object has too few items in inventory (%d)", numObjects );
                numObjects = 0;
            }

            if ( numObjects >= 10000 ) {
                logInfo ( _LOG_ALWAYS, "Object has too many items in inventory (%d)", numObjects );
                numObjects = 0;
            }

            while ( numObjects ) {

                WorldObject *obj = new WorldObject;

                if ( !obj->loadFromBuffer ( this, buffer, size ) ) {
                    delete obj;
                }

                if ( !size ) {
                    logInfo ( _LOG_ALWAYS, "Object read past end of buffer!" );
                    return -1;
                }

                numObjects--;
            }
        } catch (int e) {
            logDisplay("bcontain %d", e);
            return -1;
        }
	}
	return 1;
}

void WorldObject::writeToBuffer ( PackedData *data )
{
	BWearable *bwear = (BWearable *)getBase ( _BWEAR );
	BWeapon *bweapon = (BWeapon *)getBase ( _BWEAPON );
	BOpenable *bopen = (BOpenable *)getBase ( _BOPEN );
	BLockable *block = (BLockable *)getBase ( _BLOCK );
	BPassword *bpass = (BPassword *)getBase ( _BPASSWORD );
	BEntry *bentry = (BEntry *)getBase ( _BENTRY );
	BUse *buse = (BUse *)getBase ( _BUSE );
	BContainer *bcontain = (BContainer *)getBase ( _BCONTAIN );
	BSpellBag *bspellbag = (BSpellBag *)getBase ( _BSPELLBAG );

	data->printf ( "%s\n", super );

	// write out special name for the object (if any)
	if ( strcmp ( name, referenceObj? referenceObj->name : name ) ) {
		data->printf ( "%s\n", name );
	} else {
		data->printf ( "\n" );
	}

	// put object color
	data->printf ( "%d\n", color );

	// put object physical state
	data->printf ( "%d\n", physicalState );

	// put object value
	data->printf ( "%d\n", value );
	data->printf ( "%d\n", manaValue );

	// put object health
	data->printf ( "%d\n", health );

	// put object location
	data->printf ( "%d\n%d\n%d\n", x, y, loop );

	// put affect information
	writeAffectsToBuffer ( data );

	// put wearable information
	if ( bwear ) {
		data->printf ( "%d\n", bwear->owner? 1 : 0 );
	} else {
		data->printf ( "-1\n" );
	}

	// put weapon information
	if ( bweapon ) {
		data->printf ( "%d\n", bweapon->owner? 1 : 0 );
	} else {
		data->printf ( "-1\n" );
	}

	// put openable information
	if ( bopen ) {
		data->printf ( "%d\n", bopen->isOpen() );
	} else {
		data->printf ( "-1\n" );
	}

	// put lockable information
	if ( block ) {
		data->printf ( "%d\n", block->isLocked() );
	} else {
		data->printf ( "-1\n" );
	}

	// put entry information
	if ( bentry ) {
		if ( linkTo ) {
			if ( linkTo->tempID == -1 )
				logHack( "bEntry is valid, linkTo is valid, but tempID == -1 <%s>", getBaseOwner()->getName() );

			data->printf ( "%d\n", linkTo->tempID );
		} else {
			logHack( "bEntry is valid, linkTo is NULL <%s>", getBaseOwner()->getName() );
			data->printf ( "-1\n" );
		}
	} else {
		data->printf ( "-1\n" );
	}

	// put password information
	if ( bpass ) {
		data->printf ( "%s\n", bpass->password );
	} else {
		data->printf ( "\n" );
	}

	// put use information
	if ( buse ) {
		data->printf ( "%d\n", buse->uses );
	} else {
		data->printf ( "-1\n" );
	}

	// put spellbag information
	if ( bspellbag ) {
		LinkedElement *element = bspellbag->components.head();

		while ( element ) {
			SpellComponent *component = (SpellComponent *)element->ptr();
			element = element->next();

			data->printf ( "%s %d ", component->object->classID, component->quantity );
		}

		data->printf ( "\n" );
	} else {
		data->printf ( "\n" );
	}

	// put container information
	if ( bcontain ) {
		data->printf ( "%d\n", bcontain->contents.size() );

		LinkedElement *element = bcontain->contents.head();

		while ( element ) {
			WorldObject *obj = (WorldObject *)element->ptr();
			element = element->next();

			obj->writeToBuffer ( data );
		}
	} else {
		data->printf ( "-1\n" );
	}
}

/* submit all of my servIDs into a packed data structure */
void WorldObject::submitServIDs ( PackedData *packet )
{
	// put my servID 
	packet->putLong ( servID );

	// tell all of the bases to put their servIDs
	for ( int i=0; i<numBases; i++ ) {
		WorldObjectBase *base = pBaseList[i]; 
		base->submitServIDs ( packet );
	}
}

/* handle taking another WorldObject */
int WorldObject::take ( WorldObject *object, PackedData *packet )
{
	directObject = object;
	indirectObject = NULL;

	if ( health <= 0 && healthMax )
		return _ERR_DEAD;

	BCarryable *bcarry = (BCarryable *) object->getBase ( _BCARRY );

	if ( bcarry ) {
		char* pName;

		if ( (pName = bcarry->sLastOwner) ) {
			sprintf( sizeof( gCrashMessage ), gCrashMessage, "Picking up item - %s(%s) dropped by %s", object->classID, object->getName(), bcarry->sLastOwner ); 
		} else {
			sprintf( sizeof( gCrashMessage ), gCrashMessage, "Picking up item - %s(%s) dropped by unknown", object->classID, object->getName() ); 
		}
	} else {
		sprintf( sizeof( gCrashMessage ), gCrashMessage, "Picking up item - %s(%s)", object->classID, object->getName() ); 
	}

	int retVal = perform ( vTake, object );

	if ( retVal == _WO_ACTION_ALLOWED ) {
		retVal = object->beTakenBy ( this ); 

		if ( retVal == _WO_ACTION_HANDLED ) {
			if ( packet ) {
				// update the passed packet with the movie information
				packet->putByte ( _MOVIE_TAKE );
				packet->putLong ( servID );
				packet->putLong ( directObject->servID );
			}

			// process any special functions related to this verb
			processActions ( vTake, packet );
			object->processActions ( vBeTakenBy, packet );
		}
	}

	gCrashMessage[ 0 ] = 0;

	return retVal;
}

/* handle being taken by another WorldObject */
int WorldObject::beTakenBy ( WorldObject *object )
{
	directObject = object;
	indirectObject = NULL;

	int retVal = perform ( vBeTakenBy, object );

	return retVal;
}

// drop all items until this object is unencumbered
void WorldObject::dropUntilUnencumbered ( PackedData *packet )
{
	if ( encumberance() <= 100 )
		return;

	BContainer *bcontain = (BContainer *)getBase ( _BCONTAIN );

	if ( !bcontain )
		return;

	LinkedList *items = bcontain->contents.copy();

	while ( encumberance() > 100 && items->size() ) {
		WorldObject *object = (WorldObject *)items->at ( random ( 0, items->size() - 1 ) );

		items->del ( object );

		if ( object->getBase ( _BHEAD ) || object->hasAffect ( _AFF_CURSED ) )
			continue;
		
		object->makeVisible ( 1, packet );

		object->forceOff();
		object->forceOut();

		getBaseOwner()->calcAC();
		getBaseOwner()->calcWeightCap();
		getBaseOwner()->calcHealth();

		object->directObject = this;
		object->indirectObject = NULL;

		packet->putByte ( _MOVIE_FORCE_DROP );
		packet->putLong ( servID );
		packet->putLong ( object->servID );

	}

	items->release();
	delete items;
}

/* handle dropping another WorldObject */
int WorldObject::drop ( WorldObject *object, PackedData *packet, int override )
{
	directObject = object;
	indirectObject = NULL;

	if ( health <= 0 && healthMax )
		return _ERR_DEAD;

	int retVal = perform ( vDrop, object );

	if ( retVal == _WO_ACTION_ALLOWED ) {
		retVal = object->beDroppedBy ( this );

		if ( retVal == _WO_ACTION_HANDLED ) {
			if ( packet ) {
				// update the passed packet with the movie information
				packet->putByte ( _MOVIE_DROP );
				packet->putLong ( servID );
				packet->putLong ( directObject->servID );
			}

			// process any special functions related to this verb
			processActions ( vDrop, packet );
			object->processActions ( vBeDroppedBy, packet );
		}
	}

	return retVal;
}

/* handle being dropped by another WorldObject */
int WorldObject::beDroppedBy ( WorldObject *object )
{
	directObject = object;
	indirectObject = NULL;

	int retVal = perform ( vBeDroppedBy, object );

	return retVal;
}

/* handle putting on another WorldObject */
int WorldObject::putOn ( WorldObject *object, PackedData *packet )
{
	directObject = object;
	indirectObject = NULL;

	if ( health <= 0 && healthMax )
		return _ERR_DEAD;

	int retVal = perform ( vPutOn, object );

	if ( retVal == _WO_ACTION_ALLOWED ) {
		retVal = object->bePutOn ( this );

		if ( retVal == _WO_ACTION_HANDLED ) {
			if ( packet ) {
				// update the passed packet with the movie information
				packet->putByte ( _MOVIE_PUT_ON );
				packet->putLong ( servID );
				packet->putLong ( directObject->servID );
			}

			// process any special functions related to this verb
			processActions ( vPutOn, packet );
			object->processActions ( vBePutOn, packet );

			object->transferWearerAffects ( this, packet );

			calcHealth();
			calcWeightCap();

			if ( health > healthMax )
				health = healthMax;

			if ( packet ) {
				packet->putByte ( _MOVIE_HEALTH_MAX );
				packet->putLong ( servID );
				packet->putLong ( servID );

				// changed size of healthMax  BEW
				packet->putLong ( healthMax );
			}
		}
	}

	return retVal;
}

/* handle being put on another WorldObject */
int WorldObject::bePutOn ( WorldObject *object )
{
	directObject = object;
	indirectObject = NULL;

	int retVal = perform ( vBePutOn, object );

	return retVal;
}

/* handle taking off another WorldObject */
int WorldObject::takeOff ( WorldObject *object, PackedData *packet )
{
	directObject = object;
	indirectObject = NULL;

	if ( health <= 0 && healthMax )
		return _ERR_DEAD;

	int retVal = perform ( vTakeOff, object );

	if ( retVal == _WO_ACTION_ALLOWED ) {
		retVal = object->beTakenOff ( this );

		if ( retVal == _WO_ACTION_HANDLED ) {
			if ( packet ) {
				// update the passed packet with the movie information
				packet->putByte ( _MOVIE_TAKE_OFF );
				packet->putLong ( servID );
				packet->putLong ( directObject->servID );
			}

			// process any special functions related to this verb
			processActions ( vTakeOff, packet );
			object->processActions ( vBeTakenOff, packet );
			object->removeWearerAffects ( this, packet );

			calcHealth();
			calcWeightCap();

			if ( health > healthMax )
				health = healthMax;

			if ( packet ) {
				packet->putByte ( _MOVIE_HEALTH_MAX );
				packet->putLong ( servID );
				packet->putLong ( servID );

				// changed size of healthMax  BEW
				packet->putLong ( healthMax );
			}
		}
	}
	return retVal;
}

/* handle being taken off of a WorldObject */
int WorldObject::beTakenOff ( WorldObject *object )
{
	directObject = object;
	indirectObject = NULL;

	int retVal = perform ( vBeTakenOff, object );

	return retVal;
}

/* handle giving a WorldObject that I own to another WorldObject */
int WorldObject::give ( WorldObject *object, WorldObject *recipient, PackedData *packet )
{
	directObject = object;
	indirectObject = recipient;

	if ( health <= 0 && healthMax )
		return _ERR_DEAD;

	int retVal = perform ( vGive, object, recipient );

	if ( retVal == _WO_ACTION_ALLOWED ) {
		retVal = object->beGivenTo ( recipient );

		if ( retVal == _WO_ACTION_HANDLED ) {
			if ( packet ) {
				// update the passed packet to include the movie info
				packet->putByte ( _MOVIE_GIVE );
				packet->putLong ( servID );
				packet->putLong ( directObject->servID );
				packet->putLong ( indirectObject->servID );
			}

			// process any special functions related to this verb
			processActions ( vGive, packet );
			object->processActions ( vBeGivenTo, packet );
		}
	}

	return retVal;
}

/* handle being given to another WorldObject */
int WorldObject::beGivenTo ( WorldObject *object )
{
	directObject = object;
	indirectObject = NULL;

	int retVal = perform ( vBeGivenTo, object );

	return retVal;
}

/* handle putting a WorldObject into another WorldObject */
int WorldObject::putIn ( WorldObject *object, WorldObject *container, PackedData *packet )
{
	directObject = object;
	indirectObject = container;

	if ( health <= 0 && healthMax )
		return _ERR_DEAD;

	int retVal = perform ( vPutIn, object, container );

	if ( retVal == _WO_ACTION_ALLOWED ) {
		retVal = object->bePutIn ( container );

		if ( retVal == _WO_ACTION_HANDLED ) {
			if ( packet ) {
				// update the passed packet to include the movie info
				packet->putByte ( _MOVIE_PUT_IN );
				packet->putLong ( servID );
				packet->putLong ( directObject->servID );
				packet->putLong ( indirectObject->servID );
			}

			// process any special functions related to this verb
			processActions ( vPutIn, packet );
			object->processActions ( vBePutIn, packet );
		}
	}

	return retVal;
}

/* handle being put into a WorldObject */
int WorldObject::bePutIn ( WorldObject *object )
{
	directObject = object;
	indirectObject = NULL;

	int retVal = perform ( vBePutIn, object );

	return retVal;
}

/* handle opening a WorldObject */
int WorldObject::open ( WorldObject *object, PackedData *packet )
{
	directObject = object;
	indirectObject = NULL;

	if ( health <= 0 && healthMax )
		return _ERR_DEAD;

	int retVal = perform ( vOpen, object );

	if ( retVal == _WO_ACTION_ALLOWED ) {
		retVal = object->beOpened ( this );

		if ( retVal == _WO_ACTION_HANDLED ) {
			if ( object->physicalState & _STATE_TRAPPED ) {
				int skill = getSkill ( _SKILL_DETECT_TRAPS );
				int chance = gSkillInfo[_SKILL_DETECT_TRAPS][skill].ability;

				if ( random ( 1, 100 ) < chance ) {
					if ( packet )
						putMovieText ( this, packet, "|c50|%s detects and disarms a trap before opening the %s.", getName(), object->getName() );

					if ( character )
						character->gainExperience ( random ( 30, 100 ), packet );

					object->physicalState &= ~_STATE_TRAPPED;
				}
			}

			if ( packet ) {
				// update the passed packet to include the movie info
				packet->putByte ( _MOVIE_OPEN );
				packet->putLong ( servID );
				packet->putLong ( directObject->servID );
			}

			// process any special functions related to this verb
			processActions ( vOpen, packet );

			object->processActions ( vBeOpened, packet );

			if ( object->physicalState & _STATE_TRAPPED ) {
				object->processTraps ( packet, this );
			}
		}
	}

	return retVal;
}

/* handle being opened by a WorldObject */
int WorldObject::beOpened ( WorldObject *object )
{
	directObject = object;
	indirectObject = NULL;

	int retVal = perform ( vBeOpened, object );

	return retVal;
}

/* handle closing a WorldObject */
int WorldObject::close ( WorldObject *object, PackedData *packet )
{
	directObject = object;
	indirectObject = NULL;

	if ( health <= 0 && healthMax )
		return _ERR_DEAD;

	int retVal = perform ( vClose, object );

	if ( retVal == _WO_ACTION_ALLOWED ) {
		retVal = object->beClosed ( this );

		if ( retVal == _WO_ACTION_HANDLED ) {
			if ( packet ) {
				// update the passed packet to include the movie info
				packet->putByte ( _MOVIE_CLOSE );
				packet->putLong ( servID );
				packet->putLong ( directObject->servID );
			}

			// process any special functions related to this verb
			processActions ( vClose, packet );
			object->processActions ( vBeClosed, packet );
		}
	}

	return retVal;
}

/* handle being closed by a WorldObject */
int WorldObject::beClosed ( WorldObject *object )
{
	directObject = object;
	indirectObject = NULL;

	int retVal = perform ( vBeClosed, object );

	return retVal;
}

/* handle sitting on another WorldObject */
int WorldObject::sit ( WorldObject *object, PackedData *packet )
{
	directObject = object;
	indirectObject = NULL;

	if ( health <= 0 && healthMax )
		return _ERR_DEAD;

	int retVal = perform ( vSit, object );

	if ( retVal == _WO_ACTION_ALLOWED ) {
		retVal = object->beSatOn ( this );

		if ( retVal == _WO_ACTION_HANDLED ) {
			if ( packet ) {
				// insert our movie data
				packet->putByte ( _MOVIE_SIT );
				packet->putLong ( servID );
				packet->putLong ( directObject->servID );
			}

			// allow the magic action functions
			processActions ( vSit, packet );
			object->processActions ( vBeSatOn, packet );
		}
	}

	return retVal;
}

/* handle being sat on */
int WorldObject::beSatOn ( WorldObject *object )
{
	directObject = object;
	indirectObject = NULL;

	return perform ( vBeSatOn, object );
}

/* handle standing up */
int WorldObject::stand ( PackedData *packet )
{
	if ( !sittingOn ) 
		return _WO_ACTION_PROHIBITED;

	directObject = sittingOn;
	indirectObject = NULL;

	int retVal = perform ( vStand, directObject );

	if ( retVal == _WO_ACTION_ALLOWED ) {
		retVal = directObject->beStoodUpOn ( this );

		if ( retVal == _WO_ACTION_HANDLED ) {
			if ( packet ) {
				// insert our movie data
				packet->putByte ( _MOVIE_STAND );
				packet->putLong ( servID );
			}
		}

		// allow the magic action functions
		processActions ( vStand, packet );
		directObject->processActions ( vBeStoodUpOn, packet );
	}

	return retVal;
}

/* handle being stood up on */
int WorldObject::beStoodUpOn ( WorldObject *object )
{
	directObject = object;
	indirectObject = NULL;

	return perform ( vBeStoodUpOn, object );
}

/* handle memorizing another object */
int WorldObject::memorize ( WorldObject *object, PackedData *packet )
{
	directObject = object;
	indirectObject = NULL;

	if ( health <= 0 && healthMax )
		return _ERR_DEAD;

	int retVal = perform ( vMemorize, directObject );

	if ( retVal == _WO_ACTION_ALLOWED ) {
		retVal = directObject->beMemorized ( this );
		BScroll *scroll = (BScroll *)directObject->getBase ( _BSCROLL );

		if ( retVal == _WO_ACTION_HANDLED ) {
			if ( packet ) {
				// insert our movie data
				packet->putByte ( _MOVIE_MEMORIZE );
				packet->putLong ( servID );
				packet->putLong ( directObject->servID );


				if ( scroll ) {
					packet->putWord ( scroll->spell );
					packet->putWord ( scroll->skill );
				} else {
					packet->putWord ( -1 );
					packet->putWord ( -1 );
				}
			}
		} else {
			if ( retVal == _ERR_MEMORIZE_FAILURE ) {
				BCharacter *bchar = (BCharacter *)getBase ( _BCHARACTER );

				if ( bchar && scroll->skill ) 
					bchar->buildPoints--;
			}
		}

		// allow the action functions to do their things
		processActions ( vMemorize, packet );
		directObject->processActions ( vBeMemorized, packet );
	}

	return retVal;
}

/* handle being memorized */
int WorldObject::beMemorized ( WorldObject *object )
{
	directObject = object;
	indirectObject = NULL;

	return perform ( vBeMemorized, object );
}

/* handle locking a WorldObject */
int WorldObject::lock ( WorldObject *object, WorldObject *key, PackedData *packet )
{
	directObject = object;
	indirectObject = key;

	if ( health <= 0 && healthMax )
		return _ERR_DEAD;

	int retVal = perform ( vLock, object, key );

	if ( retVal == _WO_ACTION_ALLOWED ) {
		retVal = object->beLocked ( this, key );

		if ( retVal == _WO_ACTION_HANDLED ) {
			if ( packet ) {
				// update the passed packet to include movie info
				packet->putByte ( _MOVIE_LOCK );
				packet->putLong ( servID );
				packet->putLong ( directObject->servID );
				packet->putLong ( key? key->servID : -1 );
			}

			// process any special functions related to this verb
			processActions ( vLock, packet );
			object->processActions ( vBeLocked, packet );
		}
	}

	return retVal;
}

/* handle being locked by a WorldObject */
int WorldObject::beLocked ( WorldObject *object, WorldObject *key )
{
	directObject = object;
	indirectObject = NULL;

	int retVal = perform ( vBeLocked, object, key );

	return retVal;
}

/* handle unlocking a WorldObject */
int WorldObject::unlock ( WorldObject *object, WorldObject *key, PackedData *packet )
{
	directObject = object;
	indirectObject = key;

	if ( health <= 0 && healthMax )
		return _ERR_DEAD;

	int retVal = perform ( vUnlock, object, key );

	if ( retVal == _WO_ACTION_ALLOWED ) {
		retVal = object->beUnlocked ( this, key );

		if ( retVal == _WO_ACTION_HANDLED ) {
			if ( packet ) {
				// update the passed packet to include movie info
				packet->putByte ( _MOVIE_UNLOCK );
				packet->putLong ( servID );
				packet->putLong ( directObject->servID );
				packet->putLong ( key? key->servID : -1 );
			}
			// process any special functions related to this verb
			processActions ( vUnlock, packet );
			object->processActions ( vBeUnlocked, packet );
		}
	}

	return retVal;
}

/* handle being unlocked by a WorldObject */
int WorldObject::beUnlocked ( WorldObject *object, WorldObject *key )
{
	directObject = object;
	indirectObject = NULL;

	int retVal = perform ( vBeUnlocked, object, key );

	return retVal;
}

/* handle consuming another WorldObject */
int WorldObject::consume ( WorldObject *object, PackedData *packet )
{
	directObject = object;
	indirectObject = NULL;

	if ( health <= 0 && healthMax )
		return _ERR_DEAD;

	int retVal = perform ( vConsume, object );

	if ( retVal == _WO_ACTION_ALLOWED ) {
		retVal = object->beConsumed ( this );

		if ( retVal == _WO_ACTION_HANDLED ) {
			if ( packet ) {
				// update the passed packet with the movie information
				packet->putByte ( _MOVIE_CONSUME );
				packet->putLong ( servID );
				packet->putLong ( directObject->servID );
			}

			// process any attached action functions
			processActions ( vConsume, packet );
			object->processActions ( vBeConsumed, packet );
		}
	}

	return retVal;
}

/* handle being consumed by another WorldObject */
int WorldObject::beConsumed ( WorldObject *object )
{
	directObject = object;
	indirectObject = NULL;

	int retVal = perform ( vBeConsumed, object );

	return retVal;
}

// this member marks this object's building as changed
void WorldObject::markBuildingAsChanged ( void )
{
	if ( !player ) {
		WorldObject *owner = getBaseOwner();
		RMRoom *theRoom = owner->room;
		
		if ( !owner->player && theRoom && theRoom->building ) {
			theRoom->building->changed = 1;
		}
	}
}

/* handle forcing myself into another world object */
int WorldObject::forceIn ( WorldObject *object, int override )
{
	if ( !object->getBase ( _BCONTAIN ) )
		return _ERR_BAD_SERVID;

	return ((BCarryable *)getBase ( _BCARRY ))->bePutIn ( object, override );
}

/* handle forcing myself out of my owner and onto the ground */
int WorldObject::forceOut ( void )
{
	BCarryable *bcarry = (BCarryable *)getBase ( _BCARRY );
	return bcarry? bcarry->beDroppedBy ( bcarry->owner(), 1 ) : _WO_ACTION_PROHIBITED;
}

/* handle forcing myself to be worn by another world object */
int WorldObject::forceOn ( WorldObject *object )
{
	BWearable *bwear = (BWearable *)getBase ( _BWEAR );
	BWeapon *bweapon = (BWeapon *)getBase ( _BWEAPON );

	int retVal = -1;

	if ( bweapon ) {
		 retVal = bweapon->bePutOn ( object );

	}

	if ( bwear ) {
		retVal = bwear->bePutOn ( object );
	}

	if ( retVal == _WO_ACTION_HANDLED ) {
		directObject = object;
		indirectObject = NULL;

		// process being put on
		processActions ( vBePutOn, NULL );

		// transfer all of my wearer affects
		transferWearerAffects ( object, NULL );

		object->calcHealth();
		object->calcWeightCap();
	}

	return retVal;
}

/* handle forcing myself to be removed from another world object */
int WorldObject::forceOff ( void )
{
	int retVal = -1;

	BWearable *bwear = (BWearable *)getBase ( _BWEAR );

	if ( bwear ) {
		if ( bwear->owner ) {
			directObject = bwear->owner;
			indirectObject = NULL;

			processActions ( vBeTakenOff, NULL );
	
			if ( bwear->owner ) {
				removeWearerAffects ( bwear->owner, NULL );

				bwear->owner->calcHealth();
				bwear->owner->calcWeightCap();
			}

			retVal = bwear->beTakenOff();
		} else {
			retVal = _WO_ACTION_HANDLED;
		}
	} else {
		BWeapon *weapon = (BWeapon *)getBase ( _BWEAPON );

		if ( weapon ) {
			if ( weapon->owner ) {
				directObject = weapon->owner;
				indirectObject = NULL;

				processActions ( vBeTakenOff, NULL );

				if ( weapon->owner ) {
					removeWearerAffects ( weapon->owner, NULL );

					weapon->owner->calcHealth();
					weapon->owner->calcWeightCap();
				}

				retVal = weapon->beTakenOff();
			} else {
				retVal = _WO_ACTION_HANDLED;
			}
		}
	}

	return retVal;
}

int WorldObject::coarseAlignment ( void )
{
	//MIKE-ALIGNMENT - modified alignment table
	//if ( alignment > 175 )
	if ( alignment > 170 )
		return _ALIGN_GOOD;

	if ( alignment < 85 )
		return _ALIGN_EVIL;

	return _ALIGN_NEUTRAL;
}

/* delete this object from its room */
void WorldObject::deleteFromRoom ( int notify )
{
	if ( room ) {
		RMRoom *theRoom = room;
		room->delObject ( this, notify );
	}
}

void WorldObject::changeMana ( int delta, PackedData *movie )
{
	manaValue += delta;

	movie->putByte ( _MOVIE_CHANGE_MANA );
	movie->putLong ( servID );
	movie->putLong ( delta );

	calcWeight();
}

int WorldObject::changeStructureHealth ( int delta, PackedData *movie )
{
	if ( delta < 0 ) {
		int nDmgChance = 5;

		if ( hasAffect ( _AFF_INDESTRUCTION ) ) {
			nDmgChance = 8;
		}

		// randomly abort damage attempt based on nDmgChance...
		if ( random ( 1, nDmgChance ) != 1 ) {
			return 0;
		}
	}

	health += delta;

	if ( health > healthMax )
		health = healthMax;

	if ( health < 0 )
		health = 0;

	if ( health == 0 ) {
		// remove any transferred effects
		BWearable *bwear = (BWearable *)getBase ( _BWEAR );

		if ( bwear ) {
			if ( bwear->owner ) 
				bwear->owner->takeOff ( this, movie );
		}

		movie->putByte ( _MOVIE_TOSS_OBJECT );
		movie->putLong ( getBaseOwner()->servID );
		movie->putLong ( servID );
	}

	return delta;
}

void WorldObject::changeHealth ( int delta, WorldObject *killer, int othersSee, int egoSees, PackedData *movie )
{
//	if ( health <= 0 || !delta )
//		return;

	health += delta;

	// update combat damage tracking...
	if ( combatGroup ) {
		int actualChange = delta;

		if ( health > healthMax ) {
			actualChange -= (health - healthMax);
		} 

#if 0
		else if ( health < 0 ) {
			actualChange += -health;
		}
#endif

		if ( opposition == &combatGroup->attackers ) {
			combatGroup->attackerDamage -= actualChange;
//			logDisplay ( "attackerDamage += %d (%d)", -actualChange, combatGroup->attackerDamage );
		} else {
			combatGroup->defenderDamage -= actualChange;
//			logDisplay ( "defenderDamage += %d (%d)", -actualChange, combatGroup->defenderDamage );
		}
	}

	if ( health > healthMax )
		health = healthMax;

	if ( health < 0 )
		health = 0;

	if ( movie ) {
		movie->putByte ( _MOVIE_CHANGE_HEALTH );
		movie->putLong ( servID );
		// adjusted the size of the health  BEW
		movie->putLong ( delta );

		int mask = 0;

		if ( othersSee )
			mask = 1;

		if ( egoSees )
			mask |= 2;

		movie->putByte ( mask );

		if ( health < 1 ) {
			if ( player )
				player->die ( movie, killer );
		}
	} else {
		PackedMsg response;

		response.putLong ( servID );
		response.putLong ( room->number );
		response.putByte ( _MOVIE_CHANGE_HEALTH );
		response.putLong ( servID );
		// adjusted the size of the health  BEW
		response.putLong ( delta );

		int mask = 0;

		if ( othersSee )
			mask = 1;

		if ( egoSees )
			mask |= 2;

		response.putByte ( mask );

		if ( health < 1 ) {
			if ( player )
				player->die ( &response, killer );
		}

		response.putByte ( _MOVIE_END );

		roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, response.data(), response.size(), room );
	}
}

void WorldObject::changeStamina ( int delta, PackedData *movie )
{
	stamina += delta;

	movie->putByte ( _MOVIE_CHANGE_STAMINA );
	movie->putLong ( servID );
	movie->putLong ( delta );
}

// return the cumulative poison level of this object
int WorldObject::poisonLevel ( int source )
{
	return 0;
}

int calcAttributeMod ( int modifier )
{
	int diff = 0;

	if ( modifier > 0 ) {
		switch ( modifier ) {
			case 1: 
				diff = 3;
				break;

			case 2:
				diff = 5;
				break;

			case 3:
				diff = 6;
				break;

			default: 
				diff = 6 + ( (modifier - 3) >> 1 );
				break;
		}
	}

	else if ( modifier < 0 ) {
		switch ( modifier ) {
			case -1: 
				diff = -3;
				break;

			case -2:
				diff = -5;
				break;

			case -3:
				diff = -6;
				break;

			default:
				diff = -6 + ( (modifier + 3) >> 1 );
				break;
		}
	}

	return diff;
}

int WorldObject::calcStrength ( void )
{
	int modifier = countAffects ( _AFF_EMPOWER, -1 );
	modifier -= countAffects ( _AFF_ENFEEBLE, -1 );

	modifier = calcAttributeMod ( modifier );
	return std::max( (strength + modifier), 1 );
//( strength + modifier ) >? 1;
}

int WorldObject::calcDexterity ( void )
{
	int modifier = countAffects ( _AFF_POS_DEX_MOD, -1 );
	modifier -= countAffects ( _AFF_NEG_DEX_MOD, -1 );

	modifier = calcAttributeMod ( modifier );
	return std::max( ( dexterity + modifier ), 1 );
//( dexterity + modifier ) >? 1;
}

int WorldObject::calcIntelligence ( void )
{
	int modifier = countAffects ( _AFF_POS_INT_MOD, -1 );
	modifier -= countAffects ( _AFF_NEG_INT_MOD, -1 );

	modifier = calcAttributeMod ( modifier );
	return std::max( ( intelligence + modifier ), 1 );
//( intelligence + modifier ) >? 1;
}

int WorldObject::calcEndurance ( void )
{
	int modifier = countAffects ( _AFF_POS_END_MOD, -1 );
	modifier -= countAffects ( _AFF_NEG_END_MOD, -1 );

	modifier = calcAttributeMod ( modifier );

	return std::max( ( endurance + modifier ), 1 );
//( endurance + modifier ) >? 1;
}

void WorldObject::calcWeight ( void )
{
	BContainer *bcontain = (BContainer *)getBase ( _BCONTAIN );

	if ( bcontain )
		bcontain->calculateWeight();
}

int WorldObject::calcWeightCap ( void )
{
	BContainer *bcontain = (BContainer *)getBase ( _BCONTAIN );

	if ( bcontain ) {
		bcontain->weightCapacity = (calcStrength() * 10) * 10;

		if ( hasAffect ( _AFF_ENCUMBERANCE_BLESSING ) )
			bcontain->weightCapacity += bcontain->weightCapacity / 2; 

		if ( hasAffect ( _AFF_ENCUMBERANCE_CURSE ) )
			bcontain->weightCapacity /= 2;

		return bcontain->weightCapacity;
	}

	return 1000;
}

int WorldObject::calcNumAttacks ( void )
{
	int skillType = weaponSkillType();
	int skillLevel = getSkill ( skillType );

	if ( !combatRound )
		combatRound = 1;

	int percent = std::max( 0, encumberance() - 35 );
//0 >? encumberance() - 35;

	double attacksPerRound = gSkillInfo[skillType][skillLevel].numAttacks;

	attacksPerRound += 0.5;
	attacksPerRound = (attacksPerRound * (100 - percent)) / 100;

	if ( attacksPerRound < 1 )
		attacksPerRound = 1;

	int attacksLast = (int)(attacksPerRound * (combatRound - 1));
	int attacksCurr = (int)(attacksPerRound * combatRound);

	int count = attacksCurr - attacksLast;
	count += countAffects ( _AFF_EXTRA_ATTACK, -1 );

	// adjust for berserk
	if ( hasAffect ( _AFF_BERSERK ) )
		count++;

	return count;
}

int WorldObject::calcNumDodges ( void )
{
	int skillLevel = getSkill ( _SKILL_ACROBATICS );

	if ( !combatRound )
		combatRound = 1;

	double dodgesPerRound = gSkillInfo[_SKILL_ACROBATICS][skillLevel].numDodges;

	int percent = std::max( 0, encumberance() - 35 );
//0 >? ( encumberance() - 35 );
	dodgesPerRound = (dodgesPerRound * (100 - percent)) / 100;

	if ( dodgesPerRound < 1 )
		dodgesPerRound = 1;

	int dodgesLast = (int)(dodgesPerRound * (combatRound - 1));
	int dodgesCurr = (int)(dodgesPerRound * combatRound);

	int count = dodgesCurr - dodgesLast;

	count += countAffects ( _AFF_EXTRA_DODGE, -1 );

	return count;
}

int WorldObject::calcNumBlocks ( void )
{
	int skillLevel = getSkill ( _SKILL_SHIELD_USE );

	if ( !combatRound )
		combatRound = 1;

	double dodgesPerRound = gSkillInfo[_SKILL_SHIELD_USE][skillLevel].numDodges;

	int percent = std::max( 0, encumberance() - 35 );
//0 >? ( encumberance() - 35 );
	dodgesPerRound = (dodgesPerRound * (100 - percent)) / 100;

	if ( dodgesPerRound < 1 )
		dodgesPerRound = 1;

	int dodgesLast = (int)(dodgesPerRound * (combatRound - 1));
	int dodgesCurr = (int)(dodgesPerRound * combatRound);

	return dodgesCurr - dodgesLast;
}

int WorldObject::calcNumMoves ( void )
{
	if ( hasAffect ( _AFF_FREEZE ) )
		return 0;

	if ( player && player->isNPC )
		return quickness;

	int average = (calcDexterity() + calcStrength()) / 2;
	int percent = 100 - (encumberance() - 35);

	if ( percent > 100 )
		percent = 100;

	average = (average * percent) / 100;

	if ( hasAffect ( _AFF_SLOWED ) )
		average /= 2;

	if ( player && player->combatAction && player->combatAction->type == _CA_CHARGE )
		average += average / 2;

	return std::max(1, average);//1 >? average;
}

int WorldObject::getWeight ( void )
{
	BContainer *bcontain = (BContainer *)getBase ( _BCONTAIN );

	if ( bcontain )
		return bcontain->weight;

	BCarryable *bcarry = (BCarryable *)getBase ( _BCARRY );

	if ( bcarry ) 
		return bcarry->weight;

	return 0;
}

int WorldObject::encumberance ( void )
{
	BContainer *bcontain = (BContainer *)getBase ( _BCONTAIN );
	calcWeightCap();

	if ( bcontain ) {
		if ( bcontain->weightCapacity )
			return ( (bcontain->weight * 100) / bcontain->weightCapacity );	
		else
			return 100;
	}

	return 0;
}

// can this object see the given target...
int WorldObject::CanSee ( WorldObject *pObj )
{
	if ( pObj == NULL ) {
		return 0;
	}

	if ( hasAffect ( _AFF_SEE_INVISIBLE ) )
		return 1;

	int bInvisible = (pObj->hasAffect ( _AFF_INVISIBILITY ) || pObj->hasAffect ( _AFF_IMPROVED_INVISIBILITY ));

	return !bInvisible;
}

int WorldObject::canAttack ( void )
{
	if ( health < 1 )
		return FALSE;

	return numAttacks < calcNumAttacks();
}

int WorldObject::canDodge ( void )
{
	if ( health < 1 )
		return FALSE;

	return numDodges < calcNumDodges();
}

int WorldObject::canBlock ( void )
{
	if ( health < 1 )
		return FALSE;

	return numBlocks < calcNumBlocks();
}

int WorldObject::canMove ( void )
{
	if ( health < 1 )
		return FALSE;

	return numMoves < calcNumMoves();
}

int WorldObject::calcHealth ( void )
{
    int value = calcEndurance() * (level + 2);
    healthMax = value;

    return value;
}

int WorldObject::calcStamina ( void )
{
	int value = endurance * 3; 

	switch ( profession() ) {
		case _PROF_WARRIOR: {
			value += level * 3;
		}

		break;

		case _PROF_WIZARD: {
			value += level / 2; 
		}

		break;

		case _PROF_ADVENTURER: {
			value += level;
		}

		break;

		case _PROF_THIEF: {
			value += level * 2;
		}

		break;
	}

	return value;
}

// handle healing over time
void WorldObject::heal ( void )
{
	RMRoom *room = getRoom();

	if ( room && !combatGroup && health > 0 ) {
		BCharacter *character = (BCharacter *)getBase ( _BCHARACTER );
		if( reinterpret_cast< int>( character ) == 0x21 ) { logInfo( _LOG_ALWAYS, "%s:%d - BCharacter value corrupted", __FILE__, __LINE__ ); }

		if ( stamina < 250 ) {
			stamina++;

			if ( stamina > 250 )
				stamina = 250;
		}

		int value = std::min(hunger, thirst);//hunger <? thirst;

		if ( player && player->checkAccess ( _ACCESS_MODERATOR ) )
			value = 1;

		if ( value < 0 ) {
			int percent = random ( 15, 25 );
			int damage = (int)((healthMax * percent) / 100) + 1;
			changeHealth ( -damage, NULL );

			if ( hunger < 0 && thirst < 0 )
				roomMgr->sendPlayerText ( player, "|c60|Info> You must eat and drink or you will die!\n" );
			
			else if ( hunger < 0 )
				roomMgr->sendPlayerText ( player, "|c60|Info> You must eat something or you will die!\n" );

			else if ( thirst < 0 )
				roomMgr->sendPlayerText ( player, "|c60|Info> You must drink something or you will die!\n" );
		} else {
			if ( !hasAffect ( _AFF_POISONED ) ) {
				if ( health < healthMax ) {
					int percent = random ( 15, 25 );

					// halve the heal rate out of safe areas...
					if ( player && player->zone && player->zone->allowCombat() ) {
						percent /= 2;
					}

					int increment = (int)((healthMax * percent) / 100) + 1;
					increment = std::max(increment, 0);//increment >? 0;
	
					if ( value > 0 ) 
						changeHealth ( increment, NULL );
				
					else if ( value == 0 )	
						changeHealth ( increment / 2, NULL );
				}
			}
		}	
	}
}

/* process usual time based events -- hunger, thirst, etc */
void WorldObject::doit ( void )
{
	RMRoom *room = getRoom();

	if ( room && health > 0 && !player->checkAccess ( _ACCESS_MODERATOR ) && (!hasAffect ( _AFF_NOURISHED )) ) {
		hunger--;

		char *txt = NULL;

		switch ( hunger ) {
			case 1: 
				txt = "|c67|You are getting hungry.\n";
				break;

			case 0:
				txt = "|c67|You are hungry.\n";
				break;

			case -1:
				txt = "|c65|You are really hungry.\n";
				break;

			case -2: 
				txt = "|c62|Your hunger pains are getting quite serious.\n";
				break;
		}

		if ( hunger < -2 )
			txt = "|c60|You are starving to death!  Eat something or you will die!\n";

		if ( hunger < 2 )
			roomMgr->sendPlayerText ( player, txt );

		thirst--;
		txt = NULL;

		switch ( thirst ) {
			case 1:
				txt = "|c67|You are getting thirsty.\n";
				break;

			case 0: 
				txt = "|c67|You are thirsty.\n";
				break;

			case -1: 
				txt = "|c65|You are really thirsty.\n";
				break;

			case -2:
				txt = "|c62|Your thirst is getting quite serious.\n";
				break;
		}

		if ( thirst < -2 )
			txt = "|c60|You are dying from lack of drink!  Find some water, quick!\n";

		if ( thirst < 2 )
			roomMgr->sendPlayerText ( player, "|c67|You are thirsty.\n" );
	}
}

void WorldObject::addComponent ( char *name )
{
	WorldObject *super = roomMgr->findClass ( name );

	if ( super ) {
		if ( !components ) 
			components = new LinkedList;

		components->add ( super );	
	}
}

WorldObject *WorldObject::addObject ( const char *name, int wearIt )
{
	WorldObject *super = roomMgr->findClass ( name );
	WorldObject *retVal = NULL;

	if ( super ) {
		if ( !gParsingClasses ) {
			if ( super->randomChance ) {
				if ( random ( 1, 100 ) > super->randomChance )
					return NULL;
			}

			BTreasure *treasure = (BTreasure *)super->getBase ( _BTREASURE );

			if ( treasure ) {
				retVal = treasure->makeObj();

				if ( !retVal )
					return NULL;
			}
		}

		if ( !retVal )
			retVal = new WorldObject ( super );

		retVal->addToDatabase();

		if ( retVal->forceIn ( this ) != _WO_ACTION_HANDLED ) {
			if ( room ) {
				retVal->x = x;
				retVal->y = y;
				room->addObject ( retVal );

				return retVal;
			} else {
				roomMgr->destroyObj ( retVal, 1, __FILE__, __LINE__ );
				return NULL;
			}

		}

		if ( wearIt )
			retVal->forceOn ( this );	
	}

	return retVal;
}	

WorldObject *WorldObject::addObject ( int count, const char *name, int wearIt )
{
	WorldObject *retVal = NULL;

	while ( count-- )
		retVal = addObject ( name, wearIt );

	return retVal;
}

/* handle performing an action */
int WorldObject::perform ( int action, ... )
{
	va_list args;
	va_start ( args, action );

	WorldObjectBase *actionHandler = NULL;

	/* find the base that handles this action */
	int i;

	for ( i=0; i<numBases; i++ ) {
		WorldObjectBase *base = pBaseList[i];

		if ( base->handlesAction ( action ) ) {
			actionHandler = base;
			break;
		}
	}

	int result = _WO_ACTION_ALLOWED;

	/* make sure all of the bases say we can proceed */
	for ( i=0; i<numBases; i++ ) {
		WorldObjectBase *base = pBaseList[i];
		
		if ( base != actionHandler ) {
			int newResult = base->perform ( action, &args );

			if ( newResult != _WO_ACTION_ALLOWED ) {
				result = newResult;
				break;
			}
		}
	}

	switch ( result ) {
		/* another base besides the actionHandler took the action */
		case _WO_ACTION_HANDLED: {
			logInfo ( _LOG_ALWAYS, "*** WorldObject::perform() action %d has multiple handlers.", action );
			result = _WO_ACTION_ERROR;
		}

		break;

		/* all of the bases agree, lets do it */
		case _WO_ACTION_ALLOWED:
			if ( actionHandler )
				result = actionHandler->perform ( action, &args );

		break;
	}

	return result;
}

/* get my immediate owner */
WorldObject *WorldObject::getOwner ( void )
{
	WorldObject *owner = NULL;

	BCarryable *base = (BCarryable *)getBase ( _BCARRY );

	if ( base )
		owner = base->owner();

	if ( !owner )
		owner = this;

	return owner;
}

/* get my base owner */
WorldObject *WorldObject::getBaseOwner ( void )
{
	WorldObject *owner = this, *temp = this;
	
	while ( owner != (temp = temp->getOwner()) )
		owner = temp;

	return owner;
}

/* return whether this object owns another particular object */
int WorldObject::owns ( WorldObject *object, int levels )
{
	int retVal = object->isOwnedBy ( this, levels );
	return retVal;
}

/* return whether this object is owned by another particular object */
int WorldObject::isOwnedBy ( WorldObject *object, int levels )
{
	WorldObject *owner = this, *lastOwner = this;
	int retVal = 0;

	while ( lastOwner != (owner = owner->getOwner()) && levels ) {
		if ( object == owner ) {
			retVal = 1;
			break;
		}

		lastOwner = owner;
		levels--;
	}

	return retVal;
}

/* return whether this object is wearing another particular object */
int WorldObject::wears ( WorldObject *object )
{
	return object->isWornBy ( this );
}

/* return whether this object is worn by another particular object */
int WorldObject::isWornBy ( WorldObject *object )
{
	int retVal = 0;

	BWearable *bwear = (BWearable *)getBase ( _BWEAR );
	BWeapon *bweapon = (BWeapon *)getBase ( _BWEAPON );

	if ( bwear && bwear->owner == object ) 
		retVal = 1;

	if ( bweapon && bweapon->owner == object )
		retVal = 1;

	return retVal;
}

/* return the room for the base owner of this object */
RMRoom *WorldObject::getRoom ( void )
{
	RMRoom *retVal = getBaseOwner()->room;
	return retVal;
}

/* return the object worn at a particular place on this object */
WorldObject *WorldObject::getWornOn ( int areaWorn )
{
	WorldObject *retVal = NULL;

	BContainer *container = (BContainer *)getBase ( _BCONTAIN );

	if ( container ) {
		LinkedElement *element = container->contents.head();

		while ( element ) {
			WorldObject *object = (WorldObject *)element->ptr();
			BWearable *wearable = (BWearable *)object->getBase ( _BWEAR );

			if ( wearable ) {
				if ( wearable->owner ) {
					if ( wearable->areaWorn == areaWorn ) {
						retVal = object;
						break;
					}
				}
			}

			element = element->next();
		}
	}

	return retVal;
}

/* return the proper name of this object */
char *WorldObject::getName ( void ) 
{
	char *ret = "<unknown>";

	if ( !this || !isWorldObject() ) {
		return ret;
	}

	BCharacter *bliving = (BCharacter *)getBase ( _BCHARACTER );
	BDescribed *bdescribed = (BDescribed *)getBase ( _BDESCRIBED );

	int canIdentify = 0;

	if ( bdescribed && bdescribed->idText && strlen ( bdescribed->idText ) )
		canIdentify = 1;

	if ( bliving && bliving->properName && strlen ( bliving->properName ) )
		ret = bliving->properName;
	else {
		if ( !canIdentify || hasAffect ( _AFF_IDENTIFIED ) || hasAffect ( _AFF_ENH_IDENTIFIED ) || (physicalState & _STATE_SPECIAL) || bliving ) {
			if ( name && strlen ( name ) )
				ret = name;
		} else {
			if ( basicName && strlen ( basicName ) )
				ret = basicName;
		}
	}
	return ret;
}

/* process a movie for this WorldObject and update it's state */
int WorldObject::processMovie ( PackedData *movie, PackedData *result ) 
{
	int quit = 0, goodMovie = 1;

	unsigned char *data = movie->data();

	int _MIN_X = -111;
	int _MAX_X = 740;
	int _MAX_Y = 480;

	while ( goodMovie && !quit && (movie->data() == data) ) {
		switch ( movie->getByte() ) {
			case _MOVIE_MOVETO: {

				x = movie->getWord();
				y = movie->getWord();

				if ( x > 1000 || y > 1000 ) {
					goodMovie = 0;	
					// if ( player )
					//	player->disable( "character %s sent invalid MOVIE_MOVETO x = %d, y = %d", getName(), x, y  );
					quit = 1;	
//					return 0;
				}

				// NEW .. validate x/y
				if ( x < _MIN_X || x > _MAX_X || y < 0 || y > _MAX_Y ) {
//					logInfo ( _LOG_ALWAYS, "%s: MOVIE_MOVETO: invalid movie x=%d y=%d from remote", getName(), x, y );
					goodMovie = 0;
					quit = 1;
				}

				if ( goodMovie && result ) {
					result->putByte ( _MOVIE_MOVETO );
					result->putWord ( x );
					result->putWord ( y );
				}
			}

			break;

			case _MOVIE_POSN: {
				x = movie->getWord();
				y = movie->getWord();
				loop = movie->getByte();

				// NEW .. validate x/y/loop
				if ( x < _MIN_X || x > _MAX_X || y < 0 || y > _MAX_Y || loop < 0 || loop > 3) {
//					logInfo ( _LOG_ALWAYS, "%s: MOVIE_POSN: invalid movie x=%d y=%d loop=%d from remote", getName(), x, y, loop );
					goodMovie = 0;
					quit = 1;
				}

				if ( result ) {
					result->putByte ( _MOVIE_POSN );
					result->putWord ( x );
					result->putWord ( y );
					result->putByte ( loop ); 
				}
			}

			break;

			case _MOVIE_HIDE: 
				hidden++;	

				if ( result )
					result->putByte ( _MOVIE_HIDE );

				break;

			case _MOVIE_SHOW: 
				hidden--;	

				if ( hidden < 0 )
					hidden = 0;

				if ( result )
					result->putByte ( _MOVIE_SHOW );

				break;

			case _MOVIE_HEADING: {
				int heading = movie->getWord();

				// NEW .. validate heading
				if ( heading < 0 || heading > 359 ) {
//					logInfo ( _LOG_ALWAYS, "%s: MOVIE_HEADING: invalid movie heading=%d from remote", getName(), heading );
					goodMovie = 0;
					quit = 1;
				}

				loop = headingToLoop ( heading );

				if ( result ) {
					result->putByte ( _MOVIE_HEADING );
					result->putWord ( heading );
				}
			}

			break;

			case _MOVIE_RUN: {
				x = movie->getWord();
				y = movie->getWord();

				// NEW .. validate x/y
				if ( x < _MIN_X || x > _MAX_X || y < 0 || y > _MAX_Y ) {
					logInfo ( _LOG_ALWAYS, "%s: MOVIE_RUN: invalid movie x=%d y=%d from remote", getName(), x, y );
					goodMovie = 0;
					quit = 1;
				}

				if ( result ) {
					result->putByte ( _MOVIE_RUN );
					result->putWord ( x );
					result->putWord ( y );
				}
			}

			break;

			case _MOVIE_COMBAT_BEGIN: {

				int theID = movie->getLong();

				// NEW .. validate character
				if ( !this->player ) {
					logInfo ( _LOG_ALWAYS, "%s: COMBAT_BEGIN: invalid character ( no player ).", getName() );
					goodMovie = 0;
					quit = 1;
				}

				if ( result ) {
					result->putByte ( _MOVIE_COMBAT_BEGIN );
					result->putLong ( theID );
				}

				if ( combatGroup ) 
					combatGroup->makeCombatReady ( this );
			}

			break;

			case _MOVIE_TAKE:
			case _MOVIE_DROP: {
				logInfo ( _LOG_ALWAYS, "%s(%d): movie take/drop present for servID %d %s that is invalid for remote to initiate", __FILE__, __LINE__, servID, getName() );

				if ( player )
					player->disable ( "invalid movie type received" );

				goodMovie = 0;
				quit = 1;
			}

			break;

			case _MOVIE_CHANGE_ROOM: {
				int dir = movie->getByte();

				if ( result ) {
					result->putByte ( _MOVIE_CHANGE_ROOM );
					result->putByte ( dir );
				}
			}

			break;

			case _MOVIE_END:
				quit = 1;
				break;

			default: 
				logInfo ( _LOG_ALWAYS, "%s(%d): unknown movie type present for servID %d %s that is invalid for remote to initiate", __FILE__, __LINE__, servID, getName() );
				goodMovie = 0;
				quit = 1;
				break;

				if ( player )
					player->disable ( "invalid movie type received" );
		}
	}
	return goodMovie;
}

void WorldObject::transferWearerAffects ( WorldObject *target, PackedData *packet )
{
	if ( activeAffects ) {
		LinkedElement *element = activeAffects->head();

		while ( element ) {
			affect_t *affect = (affect_t *)element->ptr();
			element = element->next();

			if ( affect->source == _AFF_SOURCE_WEARER ) {
				target->addAffect ( affect->id, affect->type, _AFF_SOURCE_TEMPORARY, affect->duration, affect->value, packet );
			}
		}
	}
}

void WorldObject::removeWearerAffects ( WorldObject *target, PackedData *packet )
{
	if ( activeAffects ) {

		LinkedElement *element = activeAffects->head();

		while ( element ) {
			affect_t *affect = (affect_t *)element->ptr();
			element = element->next();
	
			if ( affect->source == _AFF_SOURCE_WEARER ) {
				target->delAffect ( affect->id, affect->type, _AFF_SOURCE_TEMPORARY, packet );
			}
		}
	}
}

/* convert an angular heading to a SCI compatible loop */
int WorldObject::headingToLoop ( int heading )
{
	int ret = 1;

	if ( heading > 315 || heading < 45 )
		ret = 3;

	else if ( heading > 135 && heading < 225 )
		ret = 2;

	else if ( heading < 180 )
		ret = 0;

	return ret;
}

int WorldObject::profession ( void )
{
	if ( character )
		return character->profession;

	return _PROF_WARRIOR;
}

int WorldObject::getSkill ( int skill )
{
	if ( character )
		return character->getSkill ( skill );

	return level;
}

int WorldObject::armorRating ( int dType )
{
	int rating = armor;

#if 0
	if ( !player ) {
		if ( healthMax ) {
			int percent = (health * 100) / healthMax;

			if ( percent >= 50 ) {
				percent = 100 - ((100 - percent) / 10);
			} else {
				percent = 60 - (50 - percent);
			}

			rating = (rating * percent) / 100;
		}
	}
#endif

	if ( !player && hasAffect ( _AFF_IMPROVE_ARMOR ) ) 
		rating += 10;

	if ( hasAffect ( _AFF_DEFENSELESS ) )
		rating /= 2;

	return rating;
}

int WorldObject::weaponSkillType ( void )
{
	if ( curWeapon ) {
		BWeapon *bweapon = (BWeapon *)curWeapon->getBase ( _BWEAPON );

		if ( bweapon )
			return bweapon->skillType;
	}

	return _SKILL_UNARMED;
}

int WorldObject::weaponSpeed ( void )
{
	if ( curWeapon ) 
		return (int) ( (double) (curWeapon->weight() / 10.0) * 0.3 );

	return 0;
}

int WorldObject::weight ( void )
{
	BCarryable *bcarry = (BCarryable *)getBase ( _BCARRY );

	if ( bcarry )
		return bcarry->weight;

	return 0;
}

/* 
	get this object's wear mask from it's living base. if there is not living base
	return a default value.
*/
int WorldObject::wearMask ( void )
{
	BCharacter *base = (BCharacter *)getBase ( _BCHARACTER );

	if ( base ) 
		return base->wearMask;
	else
		return 0;
}

// write this object's SCI script representation
void WorldObject::writeSCIData ( FILE *file )
{
	BDescribed *bdescribed = (BDescribed *)this->getBase ( _BDESCRIBED );
	int canIdentify = 0;

	if ( bdescribed && strlen ( bdescribed->idText ) )
		canIdentify = 1;

	fprintf ( file, "(instance SOBJ%s of Code\n", classID );
	fprintf ( file, "\t(properties\n" );
	fprintf ( file, "\t\tname \"\"\n" );
	fprintf ( file, "\t)\n\n" );

	fprintf ( file, "\t(method (doit aWhatObj)\n" );
	fprintf ( file, "\t\t(aWhatObj\n" );
	fprintf ( file, "\t\t\tname: \"%s\", \n", classID );

	if ( canIdentify && superObj ) {
		fprintf ( file, "\t\t\tpName: \"%s\",\n", superObj->name );
		fprintf ( file, "\t\t\tpIDName: \"%s\",\n", name );
	} else {
		fprintf ( file, "\t\t\tpName: \"%s\",\n", name );
	}

	fprintf ( file, "\t\t\tloop: %d,\n", loop );
	fprintf ( file, "\t\t\tpBaseView: %d,\n", view );
	fprintf ( file, "\t\t\tpAction: %d,\n", action );
	fprintf ( file, "\t\t\tpClutStart: %d,\n", clutStart );
	fprintf ( file, "\t\t\tpColor: %d,\n", (clutStart == -1 || color)? color : clutStart );
	fprintf ( file, "\t\t\tpBaseBitsLo: %d,\n", baseBits & 0x0000FFFF );
	fprintf ( file, "\t\t\tpBaseBitsHi: %d,\n", (baseBits >> 16) );

	if ( soundGroup != -1 )
		fprintf ( file, "\t\t\tpSoundGroup: %d,\n", soundGroup );

	if ( makePolygon == 0 )
		fprintf ( file, "\t\t\tpPolygon: -1,\n" );

	fprintf ( file, "\t\t)\n\n" );
	fprintf ( file, "\t\t(aWhatObj setAction: (aWhatObj pAction?))\n" );
	fprintf ( file, "\t\t(if gWObjectLite (return))\n" );

	for ( int i=0; i<numBases; i++ ) {
		WorldObjectBase *base = pBaseList[i];

		if ( base->type != _BPLAYER )
			fprintf ( file, "\n" );

		base->writeSCIData ( file );
	}

	fprintf ( file, "\t)\n" );

	fprintf ( file, ")\n" );
}

// room visibility control
void WorldObject::makeVisible ( int state, PackedData *packet )
{
	isVisible = state;

	RMRoom *theRoom = getRoom();

	if ( state && allowVisible && theRoom ) 
		theRoom->sendObjInfo ( this, player, packet );
}

// process attached functions for a particular verb
int WorldObject::processActions ( int verb, PackedData *packet )
{
	int retVal = _WO_ACTION_HANDLED;

	// check for invalid actions
	if ( (int)actions == -1 ) {
		logInfo ( _LOG_ALWAYS, "actions pointer == -1 (%d)", isValidPtr ( this ) );
		return retVal;
	}

	if ( (int) actions & 0x00000003 ) {
		logInfo ( _LOG_ALWAYS, "actions pointer is mis-aligned this %d ptr %d", isValidPtr( this ), isValidPtr( actions ) );
		return retVal;
	}

	if ( verb > -1 && verb < vLastVerb && actions ) {
		LinkedElement *element = actions->head();

		while ( element ) {
			ActionInfo *info = (ActionInfo *)element->ptr();

			if ( info && info->verb == verb ) {
				info->function ( this, packet, info->argc, info->argv );
			}

			element = element->next();
		}
	}
	return retVal;
}

// add an action function for a particular verb
int WorldObject::addAction ( int verb, action_t action, int argc, char **argv )
{
	ActionInfo *info = new ActionInfo();
	info->verb = verb;
	info->function = action;

	info->argc = argc;
	info->argv = (char **)malloc ( sizeof ( char * ) * argc );

	for ( int i=0; i<argc; i++ ) 
		info->argv[i] = strdup ( argv[i] );

	if ( !actions ) 
		actions = new LinkedList;

	actions->add ( info );

	return 0;
}

// add an affect to my activeAffects list
affect_t *WorldObject::addAffect ( int id, int type, int source, int duration, int value, PackedData *movie, int doMakeVisible )
{
	// mark the parent building as changed
	markBuildingAsChanged();

	if ( !isVisible && doMakeVisible ) {
		makeVisible ( 1, movie );
		isVisible = 0;
	}

	// SNTODO: clean this up! lol
	if ( (source == _AFF_SOURCE_SPELL) &&  hasAffect ( _AFF_DAMAGE_ETHEREAL ) ) {
		static affect_t tempAffect;
		return &tempAffect;
	}

	affect_t *newAffect = new affect_t;
	newAffect->id = id;
	newAffect->type = type;
	newAffect->source = source;
	newAffect->duration = duration;
	newAffect->value = value;
	newAffect->owner = this;

	if ( !activeAffects )
		activeAffects = new LinkedList;

	// add us to the global affected object list if the duration is not -1
	if ( duration != -1 )
		AddToAffectedObjects();

	activeAffects->add ( newAffect );

	if ( movie ) {
		movie->putByte ( _MOVIE_ATTACH_EFFECT );
		movie->putLong ( servID );
		movie->putByte ( id );
		movie->putByte ( type );
		movie->putByte ( source );
	}

	return newAffect;
}

void WorldObject::AddToAffectedObjects ( void )
{
	if ( affectElement )
		return;

	affectElement = gAffectedObjects.add ( this );
}

void WorldObject::DelFromAffectedObjects ( void )
{
	if ( !affectElement )
		return;

	gAffectedObjects.delElement ( affectElement );
	affectElement = NULL;
}

void WorldObject::clearAffect ( int id, int type, PackedData *movie )
{
	affect_t *affect = NULL;

	while ( (affect = hasAffect ( id, type )) ) 
		delAffect ( affect, movie );
}

// delete a particular Affect
int WorldObject::delAffect ( affect_t *affect, PackedData *movie )
{
	if ( !affect || !activeAffects || !activeAffects->contains ( affect ) ) 
		return 0;

	// mark the parent building as changed
	markBuildingAsChanged();

	if ( !isVisible ) {
		makeVisible ( 1, movie );
		isVisible = 0;
	}

	activeAffects->del ( affect );

	if( movie ) {
		if ( affect->id == _AFF_INVISIBILITY ) {
			if ( !hasAffect( _AFF_IMPROVED_INVISIBILITY ) && !hasAffect( affect->id, affect->type ) ) {
				movie->putByte ( _MOVIE_REMOVE_EFFECT );
				movie->putLong ( servID );
				movie->putByte ( affect->id );
				movie->putByte ( affect->type );
				movie->putByte ( affect->source );
			}
		} else if ( affect->id == _AFF_IMPROVED_INVISIBILITY ) {
        	if ( !hasAffect( _AFF_INVISIBILITY ) && !hasAffect( affect->id, affect->type ) ) {
				movie->putByte ( _MOVIE_REMOVE_EFFECT );
				movie->putLong ( servID );
				movie->putByte ( affect->id );
				movie->putByte ( affect->type );
				movie->putByte ( affect->source );
			}
		} else if ( !hasAffect( affect->id, affect->type ) ) {
			movie->putByte ( _MOVIE_REMOVE_EFFECT );
			movie->putLong ( servID );
			movie->putByte ( affect->id );
			movie->putByte ( affect->type );
			movie->putByte ( affect->source );
		}
	}

	// 
	// search for affects that are active on this object with a duration...
	// if none, pull us out of the global active affect object list
	//
	LinkedElement *element = activeAffects->head();

	while ( element ) {
		affect_t *theAffect = (affect_t *)element->ptr();

		if ( theAffect->duration != -1 )
			break;

		element = element->next();
	}

	if ( !player && !element ) {
		DelFromAffectedObjects();
	}

	delete affect;

	return 1;
}

// get and describe active affects

char * WorldObject::describeAffects ( void )
{
	static char str[1024];
	
	if ( !activeAffects )
		return NULL;

	str[0] = 0;

	LinkedElement *element = activeAffects->head();

	while ( element ) {

		affect_t *affect = (affect_t *)element->ptr();

		switch ( affect->id ) {

			case _AFF_DAMAGE_NORMAL: 
				goto endDescribe;
				break;
			case _AFF_IDENTIFIED:
				goto endDescribe;
				break;
			case _AFF_ENH_IDENTIFIED:
				goto endDescribe;
				break;
			case _AFF_OBJ_DATED:
				goto endDescribe;
				break;
			case _AFF_JAIL:
				goto endDescribe;
				break;
			default:
				break;
		}

		switch ( affect->source ) {
			case _AFF_SOURCE_PERMANENT: 
				strcat( str, "\nPermed with " );
				break;
			case _AFF_SOURCE_WEARER:
				strcat( str, "\nEnchanted with " );
				break;
			default:
				break;
		}


		switch ( affect->type ) {

			case _AFF_TYPE_WEAKNESS:
				strcat( str, "\nWeak to " );
				break;
			case _AFF_TYPE_RESISTANCE: 
				strcat( str, "\nResistant to " );
				break;
			default:
				break;
		}

		switch ( affect->id ) {

			case _AFF_DAMAGE_FIRE: 
				strcat(str, "fire damage. " );
				break;
			case _AFF_DAMAGE_COLD:
				strcat(str, "cold damage. " );
				break;
			case _AFF_DAMAGE_LIGHTNING:
				strcat(str, "lightning damage. " );
				break;
			case _AFF_DAMAGE_ACID:
				strcat(str, "acid damage. " );
				break;
			case _AFF_DAMAGE_POISON:
				strcat(str, "poison damage. " );
				break;
			case _AFF_DAMAGE_STAMINA:
				strcat(str, "stamina damage. " );
				break;
			case _AFF_DAMAGE_STEAL_STAMINA:
				strcat(str, "steal stamina damage. " );
				break;
			case _AFF_DAMAGE_EXPERIENCE:
				strcat(str, "Experience damage. " );
				break;
			case _AFF_DAMAGE_STEAL_EXPERIENCE:
				strcat(str, "steal experience damage. " );
				break;
			case _AFF_DAMAGE_STEAL_LIFE:
				strcat(str, "steal life damage. " );
				break;
			case _AFF_DAMAGE_RUST:
				strcat(str, "rust damage. " );
				break;
			case _AFF_DAMAGE_ETHEREAL:
				strcat(str, "ethereal damage. " );
				break;
			case _AFF_DAMAGE_STUN:
				strcat(str, "stun damage. " );
				break;
			case _AFF_DAMAGE_MISSILE:
				strcat(str, "missile damage. " );
				break;
			case _AFF_IMPROVE_ARMOR:
				strcat(str, "improved armor. " );
				break;
			case _AFF_IMPROVE_DAMAGE:
				strcat(str, "improved damage. " );
				break;
			case _AFF_SEE_INVISIBLE:
				strcat(str, "see invisible. " );
				break;
			case _AFF_INVISIBILITY:
				strcat(str, "invisibility. " );
				break;
			case _AFF_IMPROVED_INVISIBILITY:
				strcat(str, "improved invisibility. " );
				break;
			case _AFF_PERMANENCY:
				strcat(str, "Ready for permanence. " );
				break;
			case _AFF_DEFENSELESS:
				strcat(str, "Defenseless. " );
				break;
			case _AFF_ENCHANT_ITEM:
				strcat(str, "Ready for enchantment. " );
				break;
			case _AFF_IMMOLATION_FIRE:
				strcat(str, "fire immolation. " );
				break;
			case _AFF_IMMOLATION_COLD:
				strcat(str, "cold immolation. " );
				break;
			case _AFF_IMMOLATION_ACID:
				strcat(str, "acid immolation. " );
				break;
			case _AFF_IMMOLATION_POISON:
				strcat(str, "poison immolation. " );
				break;
			case _AFF_IMMOLATION_LIGHTNING:
				strcat(str, "lightning immolation. " );
				break;
			case _AFF_FREEZE:
				strcat(str, "freeze. " );
				break;
			case _AFF_HOLD:
				strcat(str, "hold. " );
				break;
			case _AFF_CONFUSION:
				strcat(str, "confusion. " );
				break;
			case _AFF_SHACKLED:
				strcat(str, "shackle. " );
				break;
			case _AFF_BERSERK:
				strcat(str, "berserk. " );
				break;
			case _AFF_STUN:
				strcat(str, "stun. " );
				break;
			case _AFF_LOYALTY_SHIFT:
				strcat(str, "loyalty shift. " );
				break;
			case _AFF_FEAR:
				strcat(str, "fear. " );
				break;
			case _AFF_QUICKEN:
				strcat(str, "quicken. " );
				break;
			case _AFF_SLOW:
				strcat(str, "slow. " );
				break;
			case _AFF_EMPOWER:
				strcat(str, "empower. " );
				break;
			case _AFF_ENFEEBLE:
				strcat(str, "enfeeble. " );
				break;
			case _AFF_SHIELD:
				strcat(str, "shield. " );
				break;
			case _AFF_GREATER_SHIELD:
				strcat(str, "greater shield. " );
				break;
			case _AFF_INVULNERABLE:
				strcat(str, "Invulnerable. " );
				break;
			case _AFF_REGENERATION:
				strcat(str, "regeneration. " );
				break;
			case _AFF_INDESTRUCTION:
				strcat(str, "Indestructable. " );
				break;
			case _AFF_CURSED:
				strcat(str, "cursed. " );
				break;
			case _AFF_MAGIC_RESISTANCE:
				strcat(str, "magic resistance. " );
				break;
			case _AFF_MAGIC_IMMUNITY:
				strcat(str, "magic immunity. " );
				break;
			case _AFF_IMMOLATION_RUST:
				strcat(str, "rust immolation. " );
				break;
			case _AFF_REGENERATE_STAMINA:
				strcat(str, "regenerate stamina. " );
				break;
			case _AFF_RESSURECT_25:
				strcat(str, "ressurrect weak. " );
				break;
			case _AFF_RESSURECT_50:
				strcat(str, "ressurect medium. " );
				break;
			case _AFF_RESSURECT_100:
				strcat(str, "ressurect strong. " );
				break;
			case _AFF_EXTRA_ATTACK:
				strcat(str, "extra attack. " );
				break;
			case _AFF_EXTRA_DODGE:
				strcat(str, "extra dodge. " );
				break;
			case _AFF_MEMORY:
				strcat(str, "memory. " );
				break;
			case _AFF_POS_DEX_MOD:
				strcat(str, "dexterity. " );
				break;
			case _AFF_NEG_DEX_MOD:
				strcat(str, "clumsiness. " );
				break;
			case _AFF_POS_INT_MOD:
				strcat(str, "intelligence. " );
				break;
			case _AFF_NEG_INT_MOD:
				strcat(str, "stupidity. " );
				break;
			case _AFF_POS_END_MOD:
				strcat(str, "endurance. " );
				break;
			case _AFF_NEG_END_MOD:
				strcat(str, "weakness. " );
				break;
			case _AFF_RETENTION:
				strcat(str, "retention. " );
				break;
			case _AFF_VULNERABLE:
				strcat(str, "Vulnerable. " );
				break;
			case _AFF_NOURISHED:
				strcat(str, "nourished. " );
				break;
			case _AFF_SWITCH_GENDER:
				strcat(str, "gender switch. " );
				break;
			case _AFF_NAKED:
				strcat(str, "naked. " );
				break;
			case _AFF_UGLY:
				strcat(str, "ugly. " );
				break;
			case _AFF_DISGUISED:
				strcat(str, "disguised. " );
				break;
			case _AFF_ENCUMBERANCE_BLESSING:
				strcat(str, "encumberance blessing. " );
				break;
			case _AFF_ENCUMBERANCE_CURSE:
				strcat(str, "encumberance curse. " );
				break;
			case _AFF_ENGRAVED:
				strcat(str, "Engraved. " );
				break;
			case _AFF_SHIFT:
				strcat(str, "shift. " );
				break;
			case _AFF_EXTENSION:
				strcat(str, "extension. " );
				break;
			case _AFF_POISONED:
				strcat(str, "poisoned. " );
				break;
			case _AFF_ACID_BURN:
				strcat(str, "acid burn. " );
				break;
			case _AFF_SLOWED:
				strcat(str, "slowed. " );
				break;
			case _AFF_SPELL_BLAST:
				strcat(str, "spell blast. " );
				break;
			case _AFF_ANTI_MAGIC:
				strcat(str, "antimagic. " );
				break;
			case _AFF_PROT_DEATH:
				strcat(str, "death protection. " );
				break;
			case _AFF_FREE_WILL:
				strcat(str, "free will. " );
				break;
			default:
				strcat(str, "unidentified property. ");
				break;
		}

endDescribe:

		element = element->next();
	}
	return str;
}

// find an active affect
affect_t *WorldObject::hasAffect ( int id, int type, int source )
{
	if ( !activeAffects )
		return NULL;

	LinkedElement *element = activeAffects->head();

	while ( element ) {
		affect_t *affect = (affect_t *)element->ptr();

		if ( affect->id == id && affect->type == type && affect->source == source )
			return affect;

		element = element->next();
	}

	return NULL;
}

// find an active affect -- no source comparison
affect_t *WorldObject::hasAffect ( int id, int type )
{
	if ( !activeAffects )
		return NULL;

	LinkedElement *element = activeAffects->head();

	while ( element ) {
		affect_t *affect = (affect_t *)element->ptr();

		if ( affect->id == id && (type == -1 || affect->type == type) )
			return affect;

		element = element->next();
	}

	return NULL;
}

// count how many affects there are of a certain type on this object
int WorldObject::countAffects ( int type, int source ) 
{
	if ( !activeAffects )
		return 0;

	LinkedElement *element = activeAffects->head();
	int count = 0;

	while ( element ) {
		affect_t *affect = (affect_t *)element->ptr();

		if ( (affect->id == type || type == -1) && (affect->source == source || source == -1) )
			count++;

		element = element->next();
	}

	return count;
}


// process my affected states
void WorldObject::processAffects ( PackedData *movie )
{
	PackedMsg msg;

	WorldObject *baseOwner = getBaseOwner();

	if ( baseOwner && !baseOwner->room )
		return;

	if ( !movie ) {
		movie = &msg;
		msg.putLong ( baseOwner->servID );
		msg.putLong ( baseOwner->room->number );
	}

	// handle updating our player state (if any)...
	if ( character ) {
		char buf[1024] = "", output[1024] = "";

		// decrement any mysticism immunity...
		if ( character->GetMystImmunityCount() ) {
			character->ChangeMystImmunityCount ( -1 );

			sprintf ( sizeof ( buf ), buf, "Myst Immunity: %d ", character->GetMystImmunityCount() );
			strcat ( output, buf );
		}

		// drain mana...
		character->ProcessManaDrain ( this, movie );

		// put the info over my head...
		if ( strlen ( output ) ) {
			movie->putByte ( _MOVIE_INFO );
			movie->putLong ( servID );
			movie->putString ( output );
		}
	}

	// step through all of the activeAffects and decrement the duration 
	// property if necessary.  if the duration reaches zero, toss it
	//
	LinkedElement *element = activeAffects? activeAffects->head() : NULL;
	int regenerated = 0;

	while ( element ) {
		affect_t *affect = (affect_t *)element->ptr();
		element = element->next();

		switch ( affect->id ) {
			// handle regeneration
			case _AFF_REGENERATION: {
				if ( !regenerated ) {
					if ( player && (!combatGroup || (random ( 1, 3 ) == 2)) && (health < healthMax) ) {
						regenerated = 1;
						int percent = random ( 1, 4 );
						int amount = std::max((healthMax * percent) / 100, 1);//(healthMax * percent) / 100 >? 1;

						takeDamage ( _DAMAGE_HEAL, NULL, -amount, NULL, movie ); 
					}
				}
			}

			break;
		}

		// don't process affects on dead stuff or permanent
		if ( affect->duration != -1 ) {
			switch ( affect->id ) {
				// handle hold
				case _AFF_HOLD: {
					// do nothing for the new ruleset, just let duration work
					if ( affect->duration == 1 ) {
						putMovieText ( this, movie, "|c22|%s is no longer held. ", getName() );
					}
				}

				break;

				// handle loyalty shift
				case _AFF_LOYALTY_SHIFT: {
					// do nothing for the new ruleset, just let duration work
					if ( affect->duration == 1 ) {
						putMovieText ( this, movie, "|c22|%s regains loyalty. ", getName() );
					}
				}

				break;

				// handle fear
				case _AFF_FEAR: {
					// do nothing for the new ruleset, just let duration work
					if ( affect->duration == 1 ) {
						putMovieText ( this, movie, "|c22|%s is less afraid. ", getName() );
					}
				}

				break;

				// handle confusion
				case _AFF_CONFUSION: {
					// do nothing for the new ruleset, just let duration work
					if ( affect->duration == 1 ) {
						putMovieText ( this, movie, "|c22|%s is less confused. ", getName() );
					}
				}

				break;

				// handle shackling
				case _AFF_SHACKLED: {
					// do nothing for the new ruleset, just let duration work
					if ( affect->duration == 1 ) {
						putMovieText ( this, movie, "|c22|%s can flee again. ", getName() );
					}
				}

				break;

				// handle stun
				case _AFF_STUN: {
					// do nothing for the new ruleset, just let duration work
					if ( affect->duration == 1 ) {
						putMovieText ( this, movie, "|c22|%s is less stunned. ", getName() );
					}
				}

				break;

				// handle berserk
				case _AFF_BERSERK: {
					int broken = 0;

					// do nothing for the new ruleset, just let duration work
					if ( affect->duration == 1 ) {
						putMovieText ( this, movie, "|c22|%s is less berserk. ", getName() );
					}
				}

				break;

				// handle poison
				case _AFF_POISONED: {
					if ( affect->value == 1 ) {
						putMovieText ( this, movie, "|c22|%s is no longer poisoned. ", getName() );
					} else {
						char output[1024] = "";
						takeDamage ( _DAMAGE_POISON, NULL, random ( 1, affect->value ), output, movie, 1 );
						putMovieText ( this, movie, "%s ", output );
					}
				}

				break;
			}

			// mark the parent building as changed
			markBuildingAsChanged();
			
			if ( !(combatGroup && (affect->type >= _AFF_MARK_ENID) && (affect->type <= _AFF_CURSE_DESPOTHES)) ) {
				affect->duration--;
			}

			if ( !affect->duration )
				delAffect ( affect, movie );					
		}
	}

	if ( movie == &msg ) {
		msg.putByte ( _MOVIE_END );
		roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, msg.data(), msg.size(), getRoom() );
	}
}

// calculate my armor based combat modifiers
void WorldObject::calcAC ( void )
{
	//BContainer *container = (BContainer *)getBase ( _BCONTAIN );

	double rating = (double)baseArmor;

	WorldObject *obj = NULL; 

	if ( (obj = getWornOn ( _WEAR_HEAD )) ) 
		rating += obj->armorRating() * 0.10;		

	if ( (obj = getWornOn ( _WEAR_CHEST )) ) 
		rating += obj->armorRating() * 0.40;		

	if ( (obj = getWornOn ( _WEAR_NECK )) ) 
		rating += obj->armorRating() * 0.10;		

	if ( (obj = getWornOn ( _WEAR_LEGS )) ) 
		rating += obj->armorRating() * 0.20;		

	if ( (obj = getWornOn ( _WEAR_FEET )) ) 
		rating += obj->armorRating() * 0.10;		

	if ( (obj = getWornOn ( _WEAR_BANDS )) ) 
		rating += obj->armorRating() * 0.10;		

	if ( (obj = getWornOn ( _WEAR_SHIELD )) ) 
		rating += obj->armorRating() * 0.25;	

	armor = (int)rating;
} 

#define _MAX_BUY_VALUE	1500

int WorldObject::netWorth ( int inflation )
{
	BContainer *container = (BContainer *)getBase ( _BCONTAIN );
	int worth;

	if(physicalState & _STATE_WHOLESALE)
	{
		worth = std::max(1, value / 2);
	}
	else
	{
		worth = (value * inflation) / 100;
	}

	if ( inflation < 100 && (physicalState & _STATE_WORTHLESS) )
		return 0;

	int percent = healthMax? (health * 100) / healthMax : 100;

	if ( percent > 100 )
		percent = 100;

	if ( percent < 1 )
		percent = 1;
		
	worth = std::max(1, ( (worth * percent) / 100 ));

	BUse *buse = (BUse *)getBase ( _BUSE );

	if ( buse )
		worth += buse->uses * buse->useCost;

	if ( container ) {
		LinkedElement *element = container->contents.head();

		while ( element ) {
			WorldObject *obj = (WorldObject *)element->ptr();
			worth += obj->netWorth ( inflation );
			element = element->next();
		}
	}

	return worth;
}

long long WorldObject::sellWorth ( void )
{
	BContainer *container = (BContainer *)getBase ( _BCONTAIN );
	long long worth = 0;

	if ( getBase ( _BCHARACTER ) ) {
		worth = value; 
		worth += ((long long)manaValue) * 5;
	} else {
		worth = value / 2 + 1;
	}

	if ( physicalState & _STATE_MONEY ) {
		worth = value;

		if ( view == _MANA_VIEW )
			worth = ((long long)manaValue) * 5;
	}

	if ( container ) {
		LinkedElement *element = container->contents.head();

		while ( element ) {
			WorldObject *obj = (WorldObject *)element->ptr();
			element = element->next();

			if ( getBase ( _BNPC ) && obj->objectWornOn == servID || obj->objectWieldedOn == servID )
				continue;	

			worth += obj->sellWorth();
		}
	}

	return worth;
}

const char* WorldObject::setName ( const char* newName )
{
	if ( name ) {
		free ( name );
		name = NULL;
	}

	if ( newName ) {
		int nLen = strlen( newName );

		if ( nLen && nLen < 100 ) {
		 	name = strdup ( newName );
		} else {
			name = strdup ( "<bad engrave>" );
		}
	} else {
		name = strdup ( "<unknown>" );
	}

	return name;
}

void WorldObject::teleport ( int room, PackedData *packet, int bShowAnim )
{
	PackedMsg msg;

	if ( player ) {
		player->isTeleporting++;
		player->setTeleportRoomNum ( room );
	} 

	if ( !packet ) {
		msg.putLong ( servID );
		msg.putLong ( this->room->number );
		packet = &msg;
	}

	if ( player && sittingOn ) 
		stand ( packet );

	if ( player && combatGroup )
		player->exitCombat ( packet );

	if ( bShowAnim ) {
		packet->putByte ( _MOVIE_TELEPORT );
		packet->putLong ( servID );
		packet->putLong ( servID );
		packet->putLong ( room );
	}

	if ( packet == &msg ) {
		packet->putByte ( _MOVIE_END );
		roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, msg.data(), msg.size(), this->room );
	}
}

void WorldObject::makeItemList ( LinkedList *list, int value )
{
	if ( value == -1 && !getBase ( _BCHARACTER ) ) {
		list->add ( this );
		goto doit;
	}

	if ( value == -2 ) {
		list->add ( this );
		goto doit;
	}

	if ( netWorth() <= value ) {
		BWearable *bwear = (BWearable *)this->getBase ( _BWEAR );
		BWeapon *bweapon = (BWeapon *)this->getBase ( _BWEAPON );
		BCarryable *bcarry = (BCarryable *)this->getBase ( _BCARRY );

		if ( !bcarry )
			goto doit;

		if ( bwear && bwear->owner )
			goto doit;

		if ( bweapon && bweapon->owner )
			goto doit;

		list->add ( this );
	}

doit:

	BContainer *container = (BContainer *)getBase ( _BCONTAIN );

	if ( container ) {
		LinkedElement *element = container->contents.head();

		while ( element ) {
			WorldObject *obj = (WorldObject *)element->ptr();
			element = element->next();

			if ( value == -2  || (!obj->getBase ( _BHEAD ) && !obj->getBase ( _BCHARACTER )) )
				obj->makeItemList ( list, value );
		}
	}

}

void WorldObject::removeVolatileAffects ( PackedData *movie ) 
{
}

void WorldObject::linkWith ( WorldObject *object )
{
	object->linkTo = this;
	linkTo = object;

	linkEntry ( object );
	object->linkEntry ( this );
}

void WorldObject::linkEntry ( WorldObject *object )
{
	BEntry *bentry = (BEntry *)getBase ( _BENTRY );

	if ( bentry && object->room && room ) {
		dexterity = object->servID;

		int destX = object->x;
		int destY = object->y + 1;
		int destLoop = object->loop;

		bentry->startingLoop = reverseLoop ( loop );
		bentry->startingX = destX + projectXViaLoop ( destLoop, 5 );
		bentry->startingY = destY + projectYViaLoop ( destLoop, 3 );

		bentry->endingLoop = destLoop;
		bentry->endingX = destX + projectXViaLoop ( destLoop, 10 );
		bentry->endingY = destY + projectYViaLoop ( destLoop, 10 );

		bentry->room = object->room->number;
	}
}

int WorldObject::canAfford ( int val )
{
	if ( val <= 0 ) {
		return 0;
	}

	if ( (value - val) > 0 )
		return 1;

	if ( (value - val) >= _CHAR_MONEY_START ) 
		return 1;

	return 0;
}

int WorldObject::canAfford ( int val, int currency )
{
	if ( val < 0 ) {
		return 0;
	}

	switch( currency ) {
		case BShop::Gold:
			if( (value - val) >= 0 ) return 1;
			break;
		case BShop::Copper:
			if( player && (player->nCoppers - val) >= 0 ) return 1;
			break;
		default:
			return 0;
	};

	return 0;
}


int WorldObject::canAffordMana ( int val )
{
	if ( val <= 0 ) {
		return 0;
	}

	if ( (manaValue - val) > 0 )
		return 1;

	if ( (manaValue - val) >= _CHAR_MANA_START ) 
		return 1;

	return 0;
}

int WorldObject::canHit ( WorldObject *obj )
{
	int range = 1;

	if ( !obj || !obj->isWorldObject() )
		return 0;

	if ( !obj->player || obj->combatGroup != combatGroup || obj->health < 1 || health < 1 )
		return 0;

	int distance = getDistance ( combatX, combatY, obj->combatX, obj->combatY );
	return ( distance <= combatRange() );
}

int WorldObject::combatRange ( void )
{
	int range = 1;

	if ( curWeapon ) {
		BWeapon *weapon = (BWeapon *)curWeapon->getBase ( _BWEAPON );
		range = weapon->distance;

		if ( weapon->isMissile )
			range = 50;
	} else {
		range = hands? hands->distance : 1;
	}

	return range;
}

void WorldObject::updateWanted ( WorldObject *obj, int crimeType ) {
	if ( !obj || !obj->player || obj->player->isNPC )
		return;

	BCharacter *bchar = (BCharacter *)obj->getBase ( _BCHARACTER );

	if ( !bchar ) {
  		logInfo ( _LOG_ALWAYS, "No character base for %s on updateWanted", obj->getName() );
		return;
	}

	// load existing data, if any
	CrimeData* crimeObj = obj->player->getCrimeData();

	// process the crime
	switch ( crimeType ) {
		case _CRIME_THEFT: {
			crimeObj->criminal = 1;
			crimeObj->pickedPockets++;
		}

		break;

		case _CRIME_MURDER: {
			crimeObj->criminal = 1;
			crimeObj->murders++;
			bchar->killingUnserved = crimeObj->murders;
			bchar->killingCount = crimeObj->murders;
		}

		break;

		case _CRIME_CLEAR: {
			crimeObj->pickedPockets = 0;

			if ( crimeObj->murders ) {
				crimeObj->murders--;
			}

			if ( crimeObj->bountyOnHead )
				crimeObj->bountyOnHead = 0;
			
			bchar->killingUnserved = crimeObj->murders;

			if ( !crimeObj->murders && !crimeObj->pickedPockets )
				crimeObj->criminal = 0;

			if ( !crimeObj->criminal )
				roomMgr->sendPlayerText ( obj->player, "|c85|Info> You have paid for your crimes and are no longer wanted.\n" );
			else
				roomMgr->sendPlayerText ( obj->player, "|c85|Info> You have atoned for 1 murder. To clear your wanted status, you must pay for %d more.\n", crimeObj->murders );
		}

		break;		

		default:
			break;
	}

	// add/delete criminal based on crime data.
	if ( !gWantedList.contains ( obj ) ) {
		if ( crimeObj->criminal ) 
			gWantedList.add ( obj );
	} else 	
		gWantedList.del ( obj );

	// update the database
	obj->player->writeCrimes();
}

int skillSuccess ( int actingSkill, int resistingSkill, int modifier )
{
	double percent = ((double)actingSkill) / ((double)resistingSkill + 50);
	percent *= 100.0;

	int val = (int)percent + modifier;

	if ( val > 90 )
		val = 90;

	if ( val < 0 )
		val = 1;

	if ( random ( 0, 100 ) < val )
		return 1;

	return 0;
}

int nonCombatSkillSuccess ( int actingSkill, int resistingSkill )
{
	double percent = (((double)actingSkill) / (2.0 * (double)resistingSkill));
	percent *= 100.0;

	int val = (int)percent;

	if ( random ( 0, 100 ) < val )
		return 1;

	return 0;	
}

WorldObject *loadCharacterData ( RMPlayer *pPlayer, char *buffer, int bufferSize, char *ownerName, char *characterFileName )
{
	sprintf( sizeof( gCrashMessage ), gCrashMessage, "Loading character %s, owner %s", characterFileName, ownerName ); 

	WorldObject *character = NULL, *retVal = NULL, *super = NULL;
	char *loginName = NULL, *ptr = buffer, *name = characterFileName;

	int creationTime = getseconds() - (86400 * 3), len;
	BCharacter *bchar = NULL;
	char str[1024];

	int bSaveCharacter = 0;

	// get the owning account for this character
	bufgets ( str, &ptr, &bufferSize, sizeof ( str ) );
  
	if ( !str ) {
  		logInfo ( _LOG_ALWAYS, "No owner on loadCharacterData" );
		goto skipCharacterLoad;
	}

	len = strlen ( str );
	loginName = strdup ( str );

	// get the creation time
	creationTime = bufgetint ( str, &ptr, &bufferSize );

	// get the class name for this character and create it
	bufgets ( str, &ptr, &bufferSize, sizeof ( str ) );

	super = roomMgr->findClass ( str );

	if ( !super ) {
		logHack ( "super class of %s not found on loadCharacterData ( %s )", str, name );
		goto skipCharacterLoad;
	}

	character = new WorldObject ( super );

	if ( !character ) {
		logHack ( "invalid character super class of %s on loadCharacterData ( %s )", str, name );
		goto skipCharacterLoad;
	}

	bchar = (BCharacter *)character->getBase ( _BCHARACTER );

	// don't load garbage!
	if ( bchar ) {
		character->addToDatabase();
		character->isVisible = 1;
		character->player = pPlayer;

		// give the loginName to the character and clear out the local
		character->ownerName = loginName;
		loginName = NULL;

		character->creationTime = creationTime;

		// get the propername for this character
		bufgets ( bchar->properName, &ptr, &bufferSize, sizeof ( bchar->properName ) );
		bufgets ( bchar->title, &ptr, &bufferSize, sizeof ( bchar->title ) );

		bchar->properName[0] = toupper( bchar->properName[0] );

		// make sure the character's proper name matches the name of the
		// character file
		if ( strcasecmp ( bchar->properName, characterFileName ) ) {
			logInfo ( _LOG_ALWAYS, "Character file '%s' does not match character name '%s'.", bchar->properName, characterFileName );
			goto skipCharacterLoad;
		}

		// get the character's level
		bufgets ( str, &ptr, &bufferSize, sizeof ( str ) );

		if ( !str || !strlen ( str ) ) {
			logInfo ( _LOG_ALWAYS, "Invalid level loading character %s", character->getName() );
			goto skipCharacterLoad;
		}

		char *strPtr = str;
		char *hiLevelStr;
		char *levelStr;

		// get current level 
		int i;
		
		for ( i = 0; i < strlen ( str ); i++ ) {
			if ( strPtr[i] == ':' ) {
				strncpy ( levelStr, strPtr, i - 1 );
				break;
			}
			else {
				levelStr = strPtr;
			}
		}

		// get highest level achieved.
		hiLevelStr = strchr (strPtr ,':');

		// set hi level to current level if we do not have one.

		if ( hiLevelStr ) 
			hiLevelStr = hiLevelStr + 1;
		else
			hiLevelStr = levelStr;

		character->level = atoi ( levelStr );
		bchar->topLevel = atoi ( hiLevelStr );

		if ( character->level < 1 )
			character->level = 1;

		if ( bchar->topLevel < 1 )
			bchar->topLevel = character->level;

		// get the various properties for this character
		bchar->experience = bufgetint ( str, &ptr, &bufferSize );
		
		// set experience to zero, can never be less than nothing.

		if ( bchar->experience < 0 )
			bchar->experience = 0;

		if ( bchar->experience > 100000000 )
			bchar->experience = 100000000;

		bchar->buildPoints = bufgetint ( str, &ptr, &bufferSize );
		bchar->homeTown = bufgetint ( str, &ptr, &bufferSize );


		character->strength = bufgetint ( str, &ptr, &bufferSize );
		character->dexterity = bufgetint ( str, &ptr, &bufferSize );
		character->intelligence = bufgetint ( str, &ptr, &bufferSize );
		character->quickness = bufgetint ( str, &ptr, &bufferSize );
		character->endurance = bufgetint ( str, &ptr, &bufferSize );

		character->health = bufgetint ( str, &ptr, &bufferSize );
		bufgetint ( str, &ptr, &bufferSize );

		character->hunger = bufgetint ( str, &ptr, &bufferSize );
		character->thirst = bufgetint ( str, &ptr, &bufferSize );
		character->alignment = bufgetint ( str, &ptr, &bufferSize );
		character->value = bufgetint ( str, &ptr, &bufferSize );
		character->manaValue = bufgetint ( str, &ptr, &bufferSize );
		character->physicalState = bufgetint ( str, &ptr, &bufferSize );

		if ( character->value < 0 )
			character->value = 0;

		if ( character->manaValue < 0 )
			character->manaValue = 0;

		bufgets ( str, &ptr, &bufferSize, sizeof ( str ) );

		int j;

		for ( j=0; j<_SKILL_MAX; j++ ) {
			if ( !str[j] )
				break;

			bchar->skills[j] = str[j] - 48;
		}

		bufgets ( str, &ptr, &bufferSize, sizeof ( str ) );

		for ( j=0; j<_SPELL_MAX; j++ ) {
			if ( !str[j] )
				break;

			if ( str[j] == '1' )
				bchar->learnSpell ( j );
		}

		bchar->stealingCount = bufgetint ( str, &ptr, &bufferSize );
		bchar->stealingUnserved = bufgetint ( str, &ptr, &bufferSize );
		bchar->killingCount = bufgetint ( str, &ptr, &bufferSize );
		bchar->killingUnserved = bufgetint ( str, &ptr, &bufferSize );

		bchar->peaceful = bufgetint ( str, &ptr, &bufferSize );
		bchar->warnCount = bufgetint ( str, &ptr, &bufferSize );

		// get the head -- if any
		int headRace = bufgetint ( str, &ptr, &bufferSize );

		int nEye = 31;

		if ( headRace != -1 ) {
			WorldObject *head = character->addObject ( "Head" );
			head->isVisible = 1;

			if ( head ) {
				BHead *bhead = (BHead *)head->getBase ( _BHEAD );

				if ( bhead ) {
					bhead->race = headRace; 
					bhead->sex = bufgetint ( str, &ptr, &bufferSize );
					bhead->skinColor = bufgetint ( str, &ptr, &bufferSize );
					bhead->headNumber = bufgetint ( str, &ptr, &bufferSize );
					bhead->hairNumber = bufgetint ( str, &ptr, &bufferSize );
					bhead->hairColor = bufgetint ( str, &ptr, &bufferSize );
					bhead->browNumber = bufgetint ( str, &ptr, &bufferSize );
					bhead->faceHairNumber = bufgetint ( str, &ptr, &bufferSize );
					nEye = bhead->eyeNumber = bufgetint ( str, &ptr, &bufferSize );
					bhead->eyeColor = bufgetint ( str, &ptr, &bufferSize );
					bhead->noseNumber = bufgetint ( str, &ptr, &bufferSize );
					bhead->mouthNumber = bufgetint ( str, &ptr, &bufferSize );
					bhead->earNumber = bufgetint ( str, &ptr, &bufferSize );

					if ( !bhead->valid() ) 
						character->physicalState |= _STATE_HACKER;

					switch ( character->view ) {
						case 100: {
							if ( bhead->sex != _SEX_MALE ) {
								logHack ( "%s: %s sex mismatch male", ownerName, character->getName() );
								character->physicalState |= _STATE_HACKER;
							}
						}

						break;

						case 200: {
							if ( bhead->sex != _SEX_FEMALE ) {
								logHack ( "%s: %s sex mismatch female", ownerName, character->getName() );
								character->physicalState |= _STATE_HACKER;
							}
						}

						break;
					}

					bchar->race = bhead->race;
				}
			}
		}

		// check for valid values on stats
		// check to make sure total attribute assignments dont exceed 48 
		int diff = 0;
		diff += character->strength;
		diff += character->dexterity;
		diff += character->intelligence;
		diff += character->endurance;

		// Zach: Removed for ST Rebirthing
		//if ( !pPlayer->isNPC && !pPlayer->checkAccess( _ACCESS_IMPLEMENTOR ) ) {
		//	character->oldStrength		=	character->strength;
		//	character->oldDexterity		=	character->dexterity;
		//	character->oldIntelligence	=	character->intelligence;
		//	character->oldEndurance		=	character->endurance;
		//
		//	if ( diff != 48 ) {
		//		if ( diff != 4 ) {
		//			gDataMgr->logPermanent ( ownerName, name, _DMGR_PLOG_IMPCMD, "had hacked stats of S:%d D:%d I:%d E:%d", character->strength, character->dexterity, character->intelligence, character->endurance );
		//			logHack( "6) Account %s - character %s had hacked stats of S:%d D:%d I:%d E:%d", ownerName, name, character->strength, character->dexterity, character->intelligence, character->endurance );
		//		}
		//
		//		character->strength = 1;
		//		character->dexterity = 1;
		//		character->intelligence = 1;
		//		character->endurance = 1;
		//	} else {
		//		if ( ( character->strength		< minStatValues[ bchar->profession ][ bchar->race ][ _STAT_STRENGTH ] ) ||
		//			( character->dexterity		< minStatValues[ bchar->profession ][ bchar->race ][ _STAT_DEXTERITY ] ) ||
		//			( character->intelligence	< minStatValues[ bchar->profession ][ bchar->race ][ _STAT_INTELLIGENCE ] ) ||
		//			( character->endurance		< minStatValues[ bchar->profession ][ bchar->race ][ _STAT_ENDURANCE ] ) ) {
		//				gDataMgr->logPermanent ( ownerName, name, _DMGR_PLOG_IMPCMD, "had hacked stats of S:%d D:%d I:%d E:%d", character->strength, character->dexterity, character->intelligence, character->endurance );
		//				logHack( "7) Account %s - character %s had hacked stats of S:%d D:%d I:%d E:%d", ownerName, name, character->strength, character->dexterity, character->intelligence, character->endurance );
		//
		//				character->strength = 1;
		//				character->dexterity = 1;
		//				character->intelligence = 1;
		//				character->endurance = 1;
		//		}
		//	}
		//}

		// get current room 
		bufgets ( str, &ptr, &bufferSize, sizeof ( str ) );

		if ( !str || !strlen ( str ) ) {
			logInfo ( _LOG_ALWAYS, "Invalid room loading character %s", character->getName() );
			goto skipCharacterLoad;
		}

		strPtr = str;
		char *roomStr;
		char *dungeonStr;

		for (i = 0; i < strlen ( str ); i++ ) {
			if ( strPtr[i] == ':' ) {
				strncpy ( roomStr, strPtr, i - 1 );
				break;
			}
			else {
				roomStr = strPtr;
			}
		}

		// get the dungeon instance number
		dungeonStr = strchr (strPtr ,':');

		if ( dungeonStr ) 
			dungeonStr = dungeonStr + 1;
		else
			dungeonStr = "0";

		// set the character's room/dungeon instance values
		character->roomNumber = atoi ( roomStr );
		bchar->lastDungeon = atoi ( dungeonStr );

		if ( nEye == 31 ) {		// Monster toons spawn forced in Jail #2
			character->roomNumber = 6052;
		}

		// get the last position in the game
		character->x = bufgetint ( str, &ptr, &bufferSize );
		character->y = bufgetint ( str, &ptr, &bufferSize );
		character->loop = bufgetint ( str, &ptr, &bufferSize );

		// get the biography
		bufgets ( str, &ptr, &bufferSize, sizeof ( str ) );
		character->setBiography ( str );

		bufgets ( str, &ptr, &bufferSize, sizeof ( str ) );
		character->girth = atoi ( str );

		bufgets ( str, &ptr, &bufferSize, sizeof ( str ) );
		character->height = atoi ( str );

		character->validScale();

		bufgets ( str, &ptr, &bufferSize, sizeof ( str ) );
//		character->version = atoi ( str );

		bufgets ( str, &ptr, &bufferSize, sizeof ( str ) );
		character->lastCrimeTime = atoi ( str );

		character->loadAffectsFromBuffer ( &ptr, &bufferSize, 0 );

		if ( !character->hasAffect ( _AFF_DAMAGE_NORMAL ) )
			character->addAffect ( _AFF_DAMAGE_NORMAL, _AFF_TYPE_NORMAL, _AFF_SOURCE_GIFT, -1, 0, NULL );

		if ( pPlayer->checkAccess( _ACCESS_IMPLEMENTOR ) ) {
			character->addAffect ( _AFF_SEE_INVISIBLE, _AFF_TYPE_NORMAL, _AFF_SOURCE_ARTIFACT, -1, 0, NULL );
		}

		// Racial placeholder
		//
		//
		//
		//

		int size = bufgetint ( str, &ptr, &bufferSize );

		if ( (size < 0) || (size >= 10000) ) {
			logInfo ( _LOG_ALWAYS, "Character '%s' has too many items in inventory (%d)!", name, size );
			free ( buffer );
			character = NULL;
			goto skipCharacterLoad;
		}

		while ( size ) {
			WorldObject *obj = new WorldObject;

			if ( obj->loadFromBuffer ( character, &ptr, &bufferSize ) > 0 ) {
				obj->forceIn ( character );
			} else {
				logInfo ( _LOG_ALWAYS, "Character '%s' unable to load object from root inventory.", name );
				delete obj;
			}

			size--;

			if ( !bufferSize ) {
				logInfo ( _LOG_ALWAYS, "Character '%s' EOF encountered while loading!", name );
				size = 0;
				character = NULL;
				goto skipCharacterLoad;
			}
		}

		// load player kills
		bchar->playerKills = bufgetint ( str, &ptr, &bufferSize );

		// load NPC kills
		bchar->npcKills = bufgetint ( str, &ptr, &bufferSize );

		bchar->setWearMask();
		character->calcHealth();
		character->calcWeightCap();
		character->calcAC();

		if ( character->health < 1 ) {
			character->health = character->healthMax / 2;
			character->clearAffect ( _AFF_POISONED, _AFF_TYPE_NORMAL );
			character->clearAffect ( _AFF_ACID_BURN, _AFF_TYPE_NORMAL );
		}
	} else {
		logInfo ( _LOG_ALWAYS, "No character base for '%s' while loading character.", name );
		character = NULL;
		goto skipCharacterLoad;
	}

	if ( bufferSize ) 
		character->oldLevel = bufgetint ( str, &ptr, &bufferSize );

	if ( bufferSize )
		character->stamina = bufgetint ( str, &ptr, &bufferSize );

	// load the last play time and upgrade the stamina based on the difference
	if ( bufferSize ) {
		int lastPlayTime = bufgetint ( str, &ptr, &bufferSize );
		int increase = (getseconds() - lastPlayTime) / 120;

		character->stamina += increase;

		if ( character->stamina > 250 )
			character->stamina = 250;
	} else {
		character->stamina = 250;
	}

	// character loaded fine, return him
	retVal = character;

skipCharacterLoad:

	if ( loginName )
		free ( loginName );

	// toss the character if no return value
	if ( !retVal && character )
		delete character;

	gCrashMessage[0] = 0;

	return retVal;
}

void WorldObject::processTraps ( PackedData *packet, WorldObject *obj )
{
	int spells[] = { _SPELL_POISON_BOLT, _SPELL_FLAME_ORB, _SPELL_ICE_ORB, _SPELL_LIGHTNING_BOLT, _SPELL_ACID_SPHERE, _SPELL_ENFEEBLE };
	int numSpells = sizeof ( spells ) / sizeof ( int );

	if ( obj->player ) {
		int trap = random ( 0, numSpells - 1 );
		obj->player->castSpell ( &gSpellTable[spells[trap]], this, obj->servID, 0, 0, packet );
	}

	physicalState &= ~_STATE_TRAPPED;
}

char *WorldObject::getPronoun ( int state )
{
	if ( player )
		return player->getPronoun ( state );

	return "object";
}

int WorldObject::takeDamage ( int type, WorldObject *damager, int amount, char *output, PackedData *movie, int show, int melee, int showMsg ) 
{
	char str[1024] = "";

	// handle special ETHEREAL beasts!
	if ( hasAffect ( _AFF_DAMAGE_ETHEREAL ) ) {
		if ( output ) {
			strcat ( output, getName() );
			strcat ( output, " is unaffected! " );
		}

		return 0;
	}

	affect_t *isCursedByDuach = hasAffect ( _AFF_CURSE_DUACH );
	affect_t *isMarkedByDuach = hasAffect ( _AFF_MARK_DUACH );
	affect_t *isCursedByEnid  = hasAffect ( _AFF_CURSE_ENID );

	//for cold weakness
	affect_t *isMarkedByDespothes  = hasAffect ( _AFF_MARK_DESPOTHES );
	//for lightning weakness
	affect_t *isCursedByDespothes  = hasAffect ( _AFF_CURSE_DESPOTHES );
	
	// only process this if we are not cursed by duach...
	if ( type > -1 ) {
		affect_t *isWeak = hasAffect ( type, _AFF_TYPE_WEAKNESS );
		affect_t *isProtected = hasAffect ( type, _AFF_TYPE_RESISTANCE );

		// override protection if we are cursed by Duach...
		if ( isCursedByDuach ) {
			isProtected = NULL;
			isWeak = isCursedByDuach;
		}

		if ( isWeak )
			amount += amount / 4;
	
		if ( isProtected )
			amount -= amount / 2;
	}


	//--------
	//MIKE - double cold damage if marked by Despothes
	if ( (type == BWeapon::_DAMAGE_COLD) && isMarkedByDespothes )
		amount *= 2;
	//--------
	//MIKE - double lightning damage if cursed by Despothes
	if ( (type == BWeapon::_DAMAGE_LIGHTNING) && isCursedByDespothes )
		amount *= 2;
	//--------

	// reduce poison damage if blessed by duach...
	if ( (type == WorldObject::_DAMAGE_POISON) && isMarkedByDuach ) {
		amount = (amount / 10);
	}

	// negate healing damage if we're cursed by Enid
	if ( (type == WorldObject::_DAMAGE_HEAL) && isCursedByEnid ) {
		amount = 0;
	}

	// SNTODO: remove this kludge!
	// negate all poison damage if we're a wasp...
	if ( (view == 40400) && ((type == WorldObject::_DAMAGE_POISON) || (type == _AFF_DAMAGE_POISON)) ) {
		amount = 0;
	}

	if ( health < 1 )
		return amount;

	if ( !amount ) 
		return amount;

	if ( output && *output )
		strcat ( output, " " );

	switch ( type ) {
		case _AFF_DAMAGE_NORMAL: {
			if ( damager ) {
				sprintf ( sizeof ( str ), str, "|c9|%s hits %s for %dHP! ", damager->getName(), getName(), amount );
			} else {
				sprintf ( sizeof ( str ), str, "|c9|%s is hit for %dHP! ", getName(), amount );
			}
		}

		break;

		case _AFF_DAMAGE_LIGHTNING: {
			sprintf ( sizeof ( str ), str, "|c1|%s is zapped %dHP! ", getName(), amount );
		}

		break;

		case _AFF_DAMAGE_EXPERIENCE: {
			BCharacter *bchar = (BCharacter *)getBase ( _BCHARACTER );
	
			if ( bchar ) 
				bchar->gainExperience ( -amount, movie );
	
			sprintf ( sizeof ( str ), str, "|c11|%s loses %dEXP! ", getName(), abs ( amount ) );
		}
	
		break;
	
		// Test
		case _DAMAGE_EXP_GAIN: {
			int expAmount = amount ? random( amount / 3, amount / 6 ) : 0; 		// Range: Hit Damage Divided by 3 - Hit Damage Divided by 6
			BCharacter *bchar = (BCharacter *)getBase ( _BCHARACTER );

			if ( bchar ) 
				bchar->gainExperience ( abs ( expAmount ), movie );
			sprintf ( sizeof ( str ), str, "|c85|%s leeched %dEXP! ", getName(), abs ( expAmount ) );
			
		}

		// old		
		//case _DAMAGE_EXP_GAIN: {
		//	BCharacter *bchar = (BCharacter *)getBase ( _BCHARACTER );
		//
		//	if ( bchar ) 
		//		bchar->gainExperience ( abs ( amount ), movie );
		//
		//	sprintf ( sizeof ( str ), str, "|c85|%s leeched %dEXP! ", getName(), abs ( amount ) );
		//}
	
		break;
	
		case _AFF_DAMAGE_POISON: {
			int theAmount = amount ? random( 1, amount ) : 0;

			affect_t *affect = hasAffect ( _AFF_POISONED, _AFF_TYPE_NORMAL, _AFF_SOURCE_GIFT );

			if ( theAmount ) {
				if ( affect ) {
					affect->value += theAmount;
					affect->duration = random ( 3, 5 );
				} else {
					affect = addAffect ( _AFF_POISONED, _AFF_TYPE_NORMAL, _AFF_SOURCE_GIFT, random ( 3, 5 ), theAmount, movie );
				}
			}

			sprintf ( sizeof ( str ), str, "|c22|%s gains %d poison (%d total). ", getName(), theAmount, affect? affect->value : 0 );
	
			amount /= 3;
		}
	
		break;
	
		case _AFF_DAMAGE_FIRE: {
			sprintf ( sizeof ( str ), str, "|c14|%s is singed for %dHP! ", getName(), amount );
		}

		break;

		case _AFF_DAMAGE_COLD: {
			sprintf ( sizeof ( str ), str, "|c20|%s is chilled for %dHP! ", getName(), amount );
		}

		case _AFF_DAMAGE_ACID: {
			sprintf ( sizeof ( str ), str, "|c50|%s is acid burned for %dHP! ", getName(), amount );
		}

		break;

		case WorldObject::_DAMAGE_POISON: {
			sprintf ( sizeof ( str ), str, "|c23|%s loses %dHP to poison. ", getName(), amount );
		}

		break;

		case WorldObject::_DAMAGE_HEAL: {
			sprintf ( sizeof ( str ), str, "|c86|%s gains %dHP. ", getName(), -amount );
		}

		break;

		default: {
			sprintf ( sizeof ( str ), str, "|c86|%s loses %dHP! ", getName(), amount );
		}
	}

	// toss the message if not showing it
	if ( !showMsg ) {
		sprintf ( sizeof ( str ), str, "" );
	}
	
	if ( amount >= health ) {
		char temp[1024];
		sprintf ( sizeof ( temp ), temp, "|c60|%s dies.|c43| ", getName() );
		strcat ( str, temp );
	} else {
		if ( amount > 0 ) {
			int curPercent = ((health * 100) / healthMax) - 1;
			int endingPercent = (((health - amount) * 100) / healthMax) - 1;
			endingPercent = std::max(0, endingPercent);
//0 >? endingPercent;

			int curLevel = curPercent / 25;
			int endLevel = endingPercent / 25;

			if ( endLevel > 3 )
				endLevel = 3;

			char *dmgMsgs[] = { "is about to die!", "is hurt badly!", "is hurt!", "is feeling the damage!" };

			if ( curLevel != endLevel ) {
				char temp[1024];
				sprintf ( sizeof ( temp ), temp, "|c60|%s %s|c43| ", getName(), dmgMsgs[endLevel] );
				strcat ( str, temp );
			}
		}
	}

	if ( output )
		strcat ( output, str );

	if ( amount )
		changeHealth ( -amount, damager, show, show, movie );

	delAffect ( _AFF_FREEZE, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, movie );

	return amount;
}

void WorldObject::damageArmor ( int type, WorldObject *damager, WorldObject *weapon, WorldObject *theArmor, char *output, PackedData *movie, int amount ) 
{
	if ( !damager )
		return;

	WorldObject *obj = NULL;

	// make a list of all armor worn
	LinkedList armor;

	if ( theArmor ) {

		if ( !obj->getBase ( _BHEAD ) ) {
			obj = theArmor;
		}

		if ( theArmor == weapon )
			obj = NULL;
	} else {

		if ( (obj = getWornOn ( _WEAR_HEAD )) ) {
			if ( !obj->getBase ( _BHEAD ) )
				armor.add ( obj );
		}

		if ( (obj = getWornOn ( _WEAR_CHEST )) ) {
			armor.add ( obj );
			armor.add ( obj );
			armor.add ( obj );
			armor.add ( obj );
			armor.add ( obj );
		}	

		if ( (obj = getWornOn ( _WEAR_NECK )) )
			armor.add ( obj );

		if ( (obj = getWornOn ( _WEAR_LEGS )) ) {
			armor.add ( obj );
			armor.add ( obj );
			armor.add ( obj );
			armor.add ( obj );
			armor.add ( obj );
		}

		if ( (obj = getWornOn ( _WEAR_FEET )) )
			armor.add ( obj );

		if ( (obj = getWornOn ( _WEAR_BANDS )) )
			armor.add ( obj );

		if ( armor.size() ) 
			obj = (WorldObject *)armor.at ( random ( 0, armor.size() - 1 ) );
		else 
			obj = NULL;
	}

	char str[1024] = "";

	// reduce the weapon health if there is a weapon
	if ( weapon && !weapon->destructing && (weapon != damager) ) {
		weapon->changeStructureHealth ( -1, movie );

		if ( weapon->health < 1 ) 
			roomMgr->destroyObj ( weapon, 0, __FILE__, __LINE__ );
	}

	// reduce the armor health if there is any...
	if ( obj && !obj->destructing ) {
		obj->changeStructureHealth ( -1, movie );

		if ( obj->health < 1 )
			roomMgr->destroyObj ( obj, 0, __FILE__, __LINE__ );
	}

	armor.release();
}

CombatGrid::CombatGrid()
{
	memset ( data, 0, sizeof ( data ) );
}

CombatGrid::~CombatGrid()
{
}

void CombatGrid::mapAccessible ( WorldObject *obj )
{
	CombatGroup *group = obj->combatGroup;
	memset ( data, 0, sizeof ( data ) );

	if ( group ) {
		int theX = obj->combatX, theY = obj->combatY;
		mapAccessible ( group, theX, theY );
	}
}

void CombatGrid::mapAccessible ( CombatGroup *group, int theX, int theY )
{
	if ( group ) {
		int startingX = theX - 1, startingY = theY - 1;
		int endingX = theX + 2, endingY = theY + 2;

		if ( startingX < 0 )
			startingX = 0;

		else if ( endingX > _COMBAT_GRID_WIDTH )
			endingX = _COMBAT_GRID_WIDTH;	

		if ( startingY < 0 )
			startingY = 0;

		else if ( endingY > _COMBAT_GRID_HEIGHT )
			endingY = _COMBAT_GRID_HEIGHT;

		for ( theY=startingY; theY<endingY; theY++ ) {
			for ( theX=startingX; theX<endingX; theX++ ) {
				int occupant = group->grid[theX][theY];
				char used = data[theX][theY];

				if ( !occupant && !used ) {
					data[theX][theY] = 1;
					mapAccessible ( group, theX, theY );
				}
			}
		}
	}
}

void CombatGrid::findClosestPoint ( int x1, int y1, int x2, int y2, int *pointX, int *pointY  )
{
	int startingX = x1 - 1, startingY = y1 - 1;
	int endingX = x1 + 2, endingY = y1 + 2;

	if ( startingX < 0 )
		startingX = 0;

	else if ( endingX > _COMBAT_GRID_WIDTH )
		endingX = _COMBAT_GRID_WIDTH;	

	if ( startingY < 0 )
		startingY = 0;

	else if ( endingY > _COMBAT_GRID_HEIGHT )
		endingY = _COMBAT_GRID_HEIGHT;

	int closestDist = 100000;

	for ( int theY=startingY; theY<endingY; theY++ ) {
		for ( int theX=startingX; theX<endingX; theX++ ) {
			if ( data[theX][theY] ) {
				int distance = getDistance ( theX, theY, x2, y2 );

				int yDiff = theY - y2;
				int xDiff = theX - x2;

				if ( yDiff == 0 || xDiff == 0 )
					distance--;

				if ( distance < closestDist ) {
					closestDist = distance;
					*pointX = theX;
					*pointY = theY;
				}
			}
		}
	}

	if ( (*pointX > -1) && (*pointY > -1) ) {
		x1 = *pointX;
		y1 = *pointY;

		data[x1][y1] = 0;
	}
}

void CombatGrid::display ( void )
{
	for ( int y=0; y<_COMBAT_GRID_HEIGHT; y++ )
		logDisplay ( "%d:%d:%d:%d:%d:%d:%d:%d", data[0][y], data[1][y], data[2][y], data[3][y], data[4][y], data[5][y], data[6][y], data[7][y] );
}

affect_t::affect_t()
{
	id = 0;
	source = 0;
	duration = 0;
	value = 0;
}

affect_t::~affect_t()
{
}

unsigned long long WorldObject::getWorth ( void )
{
	long long worth = 0;
	worth = value + ( manaValue * 5 );

	return worth;
}

BHead* WorldObject::GetHead( void )
{
	//
	// Scan for the head of the object passed in.
	//
	BContainer *bcontain = (BContainer *) getBase ( _BCONTAIN );
	BHead *head = NULL;

	if ( bcontain ) {
		LinkedElement *element = bcontain->contents.head();

		while ( element ) {
			WorldObject *obj = (WorldObject *)element->ptr();

			head = (BHead *)obj->getBase ( _BHEAD );

			if ( head )
				break;

			element = element->next();
		}
	}

	return head;
}

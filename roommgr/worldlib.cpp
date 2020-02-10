/*
	WORLDLIB.CPP
	classes for processing the world library

	author: Stephen Nichols
*/

#include "roommgr.hpp"
#include "../global/system.hpp"
#include "ambushgroup.hpp"

BinaryTree gObjectTree;

#define ROOM_TO_INSTANCE(a) (a * 1000) + instanceNum

// this is the class that all InfoParsers are based on
InfoParser::InfoParser()
{
	state = -1;
	instanceNum = -1;
	input = NULL;
	lastGetIdx = 0;
	index = 0;

	sprintf ( sizeof ( _error ), _error, "unknown error" );
}

InfoParser::~InfoParser()
{
}

// register an error
void InfoParser::registerError ( const char *format, ... )
{
	va_list args;

	va_start ( args, format );
	vsprintf ( sizeof ( _error ), _error, format, args );
	va_end ( args );
}

// rewind the token pointer by one
void InfoParser::prevToken ( void )
{
	index = lastGetIdx;
	numTokens++;
}

// return the current token and advance to the next
char *InfoParser::getToken ( void )
{
	if ( !numTokens )
		return NULL;

	lastGetIdx = index;
	int size = *((unsigned short *)&input[index]);

	if ( size % 2 )
		logDisplay ( "size is not right! (%d)", size );

	index += 2 + size;

	numTokens--;

	if ( size ) {
		return &input[lastGetIdx+2];
	}

	return NULL;
}

// look at the current token and return it's string
char *InfoParser::getTokenText ( void )
{
	return getToken();
}

// scan the current token list for a quoted string
char *InfoParser::getString ( void  )
{
	return getToken();
}

// return the current token's text with error checking
char *InfoParser::getIdentifier ( char *str )
{
	char *retVal = getTokenText();

	if ( !retVal )
		registerError ( str );
		
	return retVal;
}

// return if the passed string matches the current token
int InfoParser::expect ( char *str )
{
	int retVal = -1;

	char *text = getTokenText();

	if ( !text || strcmp ( text, str ) ) 
		registerError ( "'%s' expected.", str );
	else
		retVal = 0;

	return retVal;
}

// look at the current token and it as a converted integer
int InfoParser::getInteger ( int *integer )
{
	int retVal = -1;
	char *str = getTokenText();

	if ( str ) {
		char *ptr = str+1;

		while ( *ptr ) {
			if ( *ptr == '-' ) {
				memmove ( ptr, ptr + 1, strlen ( ptr + 1 ) );
			}

			ptr++;
		}

		*integer = atoi ( str );
		retVal = 0;
	}

	return retVal;
}

// look at the current token and it as a converted char
int InfoParser::getChar( char* nValue ) {
	int retVal = -1;
	char *str = getTokenText();

	if ( str ) {
		if ( isdigit( str[0] ) || ( str && str[0] == '-' && isdigit( str[1] ) ) ) {
			*nValue = atoi ( str );
			retVal = 0;
		}
	}

	return retVal;
}

// look at the current token and it as a converted char
int InfoParser::getChar( unsigned char* nValue ) {
	int retVal = -1;
	char *str = getTokenText();

	if ( str ) {
		if ( isdigit( str[0] ) ) {
			*nValue = atoi ( str );
			retVal = 0;
		}
	}

	return retVal;
}

// look at the current token and determine if it is a long
int InfoParser::getLongNumber ( long *longNumber )
{
	int retVal = -1;
	char *str = getTokenText();

	if ( str ) {
		char *ptr = str+1;

		while ( *ptr ) {

			// check for alpha characters!

//			if ( isalpha( *ptr ) )
//				return retVal;

			if ( *ptr == '-' ) {
				memmove ( ptr, ptr + 1, strlen ( ptr + 1 ) );
			}
			ptr++;
		}

		*longNumber = atol ( str );

		retVal = 0;
	}

	return retVal;
}

// parse a line of tokens
int InfoParser::parseLine ( void )
{
	numTokens = *((unsigned short *)&input[index]);
	index += 2;

	lastGetIdx = index;

	if ( !numTokens )
		return 0;

	int retVal = processTokenList();

	if ( numTokens ) {
		if ( numTokens < 0 )
			logDisplay ( "numTokens = %d", numTokens );

		while ( numTokens > 0 )
			getToken();
	}

	return retVal;
}

// process a list of tokens
int InfoParser::processTokenList ( void ) 
{
	registerError ( "InfoParser::processTokenList should never be called." );
	return -1;
}

QuestParser::QuestParser()
{
	quest = NULL;
	state = _QIP_START;
}

QuestParser::~QuestParser()
{
}

int QuestParser::processTokenList ( void )
{
	int retVal = 0;

	switch ( state ) {
		case _QIP_START: {
			char *text = getTokenText();
			long number = -1;

			if ( !strcmp ( text, "quest" ) ) {
				if ( retVal = getLongNumber ( &number ) != -1 ) {

					quest = new Quest;
					quest->number = number;

					if ( findQuest ( number ) ) {
						registerError ( "quest number %ld is already defined", number );
						retVal = -1;
					}
				}
			}

			else if ( !strcmp ( text, "end" ) ) {
				if ( !quest ) {
					registerError ( "quest specification statement found outside of quest scope" );
					return -1;
				}
				gQuestList.add ( quest );
				quest = NULL;
			}

			else if ( !strcmp ( text, "name" ) ) {
				if ( !quest ) {
					registerError ( "quest specification statement found outside of quest scope" );
					return -1;
				}

				char *str = getString();

				if ( str ) {
					quest->setName ( str );
				} else {
					retVal = -1;
				}
			}

			else if ( !strcmp ( text, "accepted" ) ) {
				if ( !quest ) {
					registerError ( "quest specification statement found outside of quest scope" );
					return -1;
				}

				char *str = getString();

				if ( str ) {
					quest->setAccepted ( str );
				} else {
					retVal = -1;
				}
			}

			else if ( !strcmp ( text, "declined" ) ) {
				if ( !quest ) {
					registerError ( "quest specification statement found outside of quest scope" );
					return -1;
				}

				char *str = getString();

				if ( str ) {
					quest->setDeclined ( str );
				} else {
					retVal = -1;
				}
			}

			else if ( !strcmp ( text, "proposal" ) ) {
				if ( !quest ) {
					registerError ( "quest specification statement found outside of quest scope" );
					return -1;
				}

				char *str = getString();

				if ( str ) {
					quest->setProposal ( str );
				} else {
					retVal = -1;
				}
			}

			else if ( !strcmp ( text, "reminder" ) ) {
				if ( !quest ) {
					registerError ( "quest specification statement found outside of quest scope" );
					return -1;
				}

				char *str = getString();

				if ( str ) {
					quest->setReminder ( str );
				} else {
					retVal = -1;
				}
			}

			else if ( !strcmp ( text, "completed" ) ) {
				if ( !quest ) {
					registerError ( "quest specification statement found outside of quest scope" );
					return -1;
				}

				char *str = getString();

				if ( str ) {
					quest->setCompleted ( str );
				} else {
					retVal = -1;
				}
			}

			else if ( !strcmp ( text, "type" ) ) {
				if ( !quest ) {
					registerError ( "quest specification statement found outside of quest scope" );
					return -1;
				}

				char *str = getTokenText();

				if ( str ) {
					if ( !strcmp ( str, "fedex" ) )
						quest->type = _QUEST_FEDEX;
		
					else {
						registerError ( "%s is an unknown quest type", str );
						retVal = -1;
					}
				} else {
					registerError ( "type name expected" );
					retVal = -1;
				}
			}

			else if ( !strcmp ( text, "getItem" ) ) {
				if ( !quest ) {
					registerError ( "quest specification statement found outside of quest scope" );
					return -1;
				}

				char *str = getTokenText();

				if ( str ) {
					if ( !strcmp ( str, "gold" ) ) {
						char *value = getTokenText();

						if ( value ) {
							WorldObject *obj = new WorldObject ( roomMgr->findClass ( "MoneyBag" ) );
							obj->physicalState |= _STATE_MONEY;
							obj->value = atoi ( value );

							quest->item = obj;
						} else {
							registerError ( "gold amount expected" );
							retVal = -1;
						}
					} else {
						WorldObject *super = roomMgr->findClass ( str );

						if ( super ) {
							quest->item = super;
						} else {
							logDisplay ( "'%s' is not a valid getItem object", str );
						}
					}
				} else {
					registerError ( "object name expected" );
					retVal = -1;
				}
			}

			else if ( !strcmp ( text, "reward" ) ) {
				if ( !quest ) {
					registerError ( "quest specification statement found outside of quest scope" );
					return -1;
				}

				char *str = getTokenText();

				if ( str ) {
					if ( !strcmp ( str, "gold" ) ) {
						char *value = getTokenText();

						if ( value ) {
							WorldObject *obj = new WorldObject ( roomMgr->findClass ( "MoneyBag" ) );
							obj->physicalState |= _STATE_MONEY;
							obj->value = atoi ( value );

							quest->reward = obj;
						} else {
							registerError ( "gold amount expected" );
							retVal = -1;
						}
					} else {
						WorldObject *super = roomMgr->findClass ( str );

						if ( super ) {
							quest->reward = new WorldObject ( super );
						} else {
							logDisplay ( "'%s' is not a valid reward object", str );
						}
					}
				} else {
					registerError ( "reward specification expected" );
					retVal = -1;
				}
			}
		}

		break;
	}

	return retVal;
}

TalkTreeParser::TalkTreeParser()
{
	tree = NULL;
	topic = NULL;
	state = _TTP_START;
}

TalkTreeParser::~TalkTreeParser()
{
}

int TalkTreeParser::processTokenList ( void )
{
	int retVal = 0;

	switch ( state ) {
		case _TTP_START: {
			char *text = getTokenText();

			if ( !strcmp ( text, "id" ) ) 
//				retVal = getInteger ( &tree->id );
				retVal = getLongNumber ( &tree->id );

			else if ( !strcmp ( text, "banner" ) ) {
				char *banner = getString();

				if ( banner ) {
					if ( topic )
						topic->setBanner ( banner );
					else
						tree->setBanner ( banner );
				} else {
					retVal = -1;
				}
			}

			else if ( !strcmp ( text, "topic" ) ) {
				char *title = getString();

				if ( title ) {
					if ( topic )
						topic = topic->addTopic ( title );
					else
						topic = tree->addTopic ( title );

					if ( topic ) {
						char *str = NULL;
						ConditionFilter *filter = NULL;
						
						while ( str = getTokenText() ) {
							// parse a filter object
							if ( !filter )
								filter = new ConditionFilter;

							if ( !filter->fromStr ( str ) ) {
								registerError ( "error with filter text" );
								retVal = -1;
							} else {
								topic->filter = filter;
							}
						}
					}
				} else {
					retVal = -1;
				}
			}

			else if ( !strcmp ( text, "text" ) ) {
				char *str = getString();

				if ( str ) {
					if ( topic ) {
						topic->addText ( str );
					} else {
						registerError ( "can not add text to the root of the talk tree" );
						retVal = -1;
					}
				} else {
					retVal = -1;
				}
			}

			else if ( !strcmp ( text, "quest" ) ) {
//				int number = -1;
				long number = -1;

//				if ( retVal = getInteger ( &number ) != -1 ) {
				if ( retVal = getLongNumber ( &number ) != -1 ) {
					Quest *quest = findQuest ( number );

					if ( quest ) {
						quest->id = tree->id;
						topic->addQuest ( quest );
					} else {
//						registerError ( "can not find quest number %d", number );
//						retVal = -1;
					}
				}
			}

			else if ( !strcmp ( text, "end" ) ) {
				if ( topic ) {
					TalkTreeTopicOwner *owner = topic->owner;

					if ( owner->type == _TREE_NODE )
						topic = (TalkTreeTopic *)owner;
					else
						topic = NULL;
				} else {
					registerError ( "end encountered outside of topic scope" );
					retVal = -1;
				}
			}
		}

		break;
	}

	return retVal;
}

// this class parses object specification information
ObjectInfoParser::ObjectInfoParser()
{
	object = NULL;
	category = NULL;
	base = NULL;
	saveObjects = 0;
	state = _OIP_START;
}

ObjectInfoParser::~ObjectInfoParser()
{
}

int ObjectInfoParser::processTokenList ( void )
{
	int retVal = 0;

	switch ( state ) {
		// this is the starting state; only "object" commands are accepted here
		case _OIP_START: {
			retVal = expect ( "object" );

			if ( retVal == 0 ) {
				char *name = getIdentifier ( "object name expected" );

				if ( name ) {
					if ( textToObject ( name ) ) {
						registerError ( "'%s' is a reserved word and can not be used as an object name", name );
						retVal = -1;
						break;
					}

					if ( numTokens ) {
						retVal = expect ( "of" );

						if ( retVal == 0 ) {
							char *super = getIdentifier ( "super-object name expected" );

							if ( super ) {
								WorldObject *superObj = roomMgr->findClass ( super );
								WorldObject *origSuper = superObj;
								object = NULL;

								if ( superObj ) {
									if ( saveObjects ) {
										// translate class name based on treasure table entry
										BTreasure *btreasure = (BTreasure *)superObj->getBase ( _BTREASURE );

										if ( btreasure ) {
											object = btreasure->makeObj();
										}
									}

									state = _OIP_MAIN_BLOCK;

									if ( !object ) {
										object = new WorldObject;
										object->copy ( superObj );

										object->superObj = superObj;

										if ( instanceNum > -1 ) {
											char classID[256];
											sprintf ( sizeof ( classID ), classID, "%s%03d", name, instanceNum );

											object->setClassID ( classID );
										} else {
											object->setClassID ( name );
										}

										object->setSuper ( superObj->classID );
										gObjectTree.add ( object->classID, object );

									}

									if ( saveObjects ) {
										object->isZoneObject = 1;

										if ( origSuper->randomChance ) {
											if ( random ( 1, 100 ) > origSuper->randomChance ) 
												object->servID = -2;
										}

										if ( object->servID != -2 ) {
											object->addToDatabase();
											roomMgr->addObject ( object );
										}
									}
								} else {
									registerError ( "'%s' is an unknown super-object", super );
									retVal = -1;
								}
							} else {
								retVal = -1;
							}
						}
					} else {
						state = _OIP_MAIN_BLOCK;
						object = new WorldObject;

						object->setClassID ( name );
						object->setSuper ( name );

//						strcpy ( object->classID, name );
//						strcpy ( object->super, name );

						if ( saveObjects ) {
							object->isZoneObject = 1;
							object->addToDatabase();
							roomMgr->addObject ( object );
						}
					}
				} else {
					retVal = -1;
				}
			}
		}

		break;

		case _OIP_MAIN_BLOCK: {
			char *text = getTokenText();

			if ( !strcmp ( text, "properties" ) ) {
				state = _OIP_PROPERTIES;
			}

			else if ( !strcmp ( text, "actions" ) ) {
				state = _OIP_ACTIONS;
			}

			else if ( !strcmp ( text, "components" ) ) {
				state = _OIP_COMPONENTS;
			}

			else if ( !strcmp ( text, "base" ) ) {
				int baseType = -1;

				text = getIdentifier ( "base name expected" );

				if ( text ) {
					if ( !strcmp ( text, "consumable" ) )
						baseType = _BCONSUME;

					if ( !strcmp ( text, "carryable" ) ) 
						baseType = _BCARRY;

					else if ( !strcmp ( text, "openable" ) )
						baseType = _BOPEN;

					else if ( !strcmp ( text, "penable" ) )
						baseType = _BOPEN;

					else if ( !strcmp ( text, "dye" ) )
						baseType = _BDYE;

					else if ( !strcmp ( text, "spellbag" ) )
						baseType = _BSPELLBAG;

					else if ( !strcmp ( text, "switch" ) )
						baseType = _BSWITCH;

					else if ( !strcmp ( text, "use" ) )
						baseType = _BUSE;

					else if ( !strcmp ( text, "container" ) )
						baseType = _BCONTAIN;

					else if ( !strcmp ( text, "mix" ) )
						baseType = _BMIX;

					else if ( !strcmp ( text, "described" ) )
						baseType = _BDESCRIBED;

					else if ( !strcmp ( text, "wearable" ) )
						baseType = _BWEAR;

					else if ( !strcmp ( text, "lockable" ) )
						baseType = _BLOCK;

					else if ( !strcmp ( text, "key" ) )
						baseType = _BKEY;

					else if ( !strcmp ( text, "weapon" ) )
						baseType = _BWEAPON;

					else if ( !strcmp ( text, "cycler" ) )
						baseType = _BCYCLE;

					else if ( !strcmp ( text, "entry" ) )
						baseType = _BENTRY;

					else if ( !strcmp ( text, "character" ) )
						baseType = _BCHARACTER;

					else if ( !strcmp ( text, "head" ) )
						baseType = _BHEAD;

					else if ( !strcmp ( text, "player" ) )
						baseType = _BPLAYER;

					else if ( !strcmp ( text, "shop" ) )
						baseType = _BSHOP;

					else if ( !strcmp ( text, "npc" ) )
						baseType = _BNPC;

					else if ( !strcmp ( text, "password" ) )
						baseType = _BPASSWORD;

					else if ( !strcmp ( text, "gate" ) )
						baseType = _BGATE;

					else if ( !strcmp ( text, "sit" ) )
						baseType = _BSIT;

					else if ( !strcmp ( text, "treasure" ) )
						baseType = _BTREASURE;

					else if ( !strcmp ( text, "scroll" ) )
						baseType = _BSCROLL;

					else if ( !strcmp ( text, "teach" ) )
						baseType = _BSCROLL;

					else if ( !strcmp ( text, "talk" ) )
						baseType = _BTALK;

					if ( baseType != -1 ) {
						base = object->getBase ( baseType );

						if ( !base ) 
							base = object->addBase ( baseType );

						if ( base ) {
							state = _OIP_BASE;
						} else {
							registerError ( "'%s' is parseable but not supported", text );
							retVal = -1;
						}
					} else {
						registerError ( "'%s' is not a supported base", text );
						retVal = -1;
					}
				} else {
					retVal = -1;
				}
			}

			else if ( !strcmp ( text, "end" ) ) {
				state = _OIP_START;

				retVal = 1;
			}

			else {
				registerError ( "'%s' is not a valid main block statement", text );
				retVal = -1;
			}
		}

		break;

		case _OIP_PROPERTIES: {
			char *text = getTokenText();

			if ( !strcmp ( text, "name" ) ) {
				char *name = getString();

				if ( name ) {
					object->setName ( name );
				} else {
					retVal = -1;
				}
			}

			else if ( !strcmp ( text, "basicName" ) ) {
				char *basicName = getString();

				if ( basicName ) {
					object->basicName = strdup ( basicName );
				} else {
					retVal = -1;
				}
			}

			else if ( !strcmp ( text, "random" ) ) {
				retVal = getInteger ( &object->randomChance );
			}

			else if ( !strcmp ( text, "treasure" ) ) {
				retVal = getInteger ( &object->treasureTable );
			}

			else if ( !strcmp ( text, "sittingOn" ) ) {
				char *ownerName = getIdentifier ( "owner name expected" );

				if ( ownerName ) {
//					WorldObject *owner = roomMgr->findObjectByClass ( ownerName ); 

					TreeNode *node = gObjectTree.find ( ownerName );
					WorldObject *owner = node? (WorldObject *)node->data : NULL;

					if ( owner ) {
						object->x = owner->x;
						object->y = owner->y + 1;
						object->loop = owner->loop;
						object->sit ( owner );
					} else {
						registerError ( "can not find owner by the name '%s'", ownerName );
						retVal = -1;
					}
				} else {
					retVal = -1;
				}
			}

			else if ( !strcmp ( text, "trapped" ) )
				object->physicalState |= _STATE_TRAPPED;

			else if ( !strcmp ( text, "toss" ) )
				object->physicalState |= _STATE_TOSS;

			else if ( !strcmp ( text, "drop" ) )
				object->physicalState |= _STATE_MUST_DROP;

			else if ( !strcmp ( text, "component" ) )
				object->physicalState |= _STATE_COMPONENT;

			else if ( !strcmp ( text, "wholesale" ) )
				object->physicalState |= _STATE_WHOLESALE;

			else if ( !strcmp ( text, "worthless" ) )
				object->physicalState |= _STATE_WORTHLESS;

			else if ( !strcmp ( text, "special" ) ) {
				object->physicalState |= _STATE_SPECIAL;
				object->addAffect ( _AFF_ENGRAVED, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, -1, 0, NULL, 0 );
			}

			else if ( !strcmp ( text, "noscale" ) )
				object->physicalState |= _STATE_NOSCALE;

			else if ( !strcmp ( text, "testserver-only" ) ) {
				object->physicalState |= _STATE_TESTSERVER_OBJ;
			}

			else if ( !strcmp ( text, "view" ) ) {
				retVal = getInteger ( &object->view );
			}

			else if ( !strcmp ( text, "loop" ) ) {
				retVal = getInteger ( &object->loop );
			}

			else if ( !strcmp ( text, "action" ) ) {
				retVal = getInteger ( &object->action );
			}

			else if ( !strcmp ( text, "clutStart" ) ) {
				retVal = getInteger ( &object->clutStart );
			}

			else if ( !strcmp ( text, "color" ) ) {
				retVal = getInteger ( &object->color );
			}

			else if ( !strcmp ( text, "xStep" ) ) {
				retVal = getInteger ( &object->xStep );
			}

			else if ( !strcmp ( text, "height" ) ) {
				retVal = getInteger ( &object->height );
			}

			else if ( !strcmp ( text, "girth" ) ) {
				retVal = getInteger ( &object->girth );
			}

			else if ( !strcmp ( text, "yStep" ) ) {
				retVal = getInteger ( &object->yStep );
			}

			else if ( !strcmp ( text, "x" ) ) {
				retVal = getInteger ( &object->x );
			}

			else if ( !strcmp ( text, "y" ) ) {
				retVal = getInteger ( &object->y );
			}

			else if ( !strcmp ( text, "numAttacks" ) ) {
				retVal = getInteger ( &object->numAttacks );
			}

			else if ( !strcmp ( text, "numDodges" ) ) {
				retVal = getInteger ( &object->numDodges );
			}

			else if ( !strcmp ( text, "attackSkill" ) ) {
				retVal = getInteger ( &object->attackSkill );
			}

			else if ( !strcmp ( text, "dodgeSkill" ) ) {
				retVal = getInteger ( &object->dodgeSkill );
			}

			else if ( !strcmp ( text, "linkTo" ) || !strcmp ( text, "link" ) ) {
				char *ownerName = getIdentifier ( "owner name expected" );

				if ( ownerName ) {
					char classID[256];

					if ( instanceNum > -1 ) {
						sprintf ( sizeof ( classID ), classID, "%s%03d", ownerName, instanceNum );
						ownerName = classID;
					}

					object->linkToStr = strdup ( ownerName );
				} else {
					retVal = -1;
				}
			}

			else if ( !strcmp ( text, "skill" ) ) {
				char *skillName = getIdentifier ( "skill name expected" );

				if ( skillName ) {
					int skill = textToSkill ( skillName );

					if ( skill == -1 ) {
						registerError ( "'%s' is not a valid skill name", skillName );
						retVal = -1;
					} else {
						object->skill = skill;
					}
				} else {
					retVal = -1;
				}
			}

			else if ( !strcmp ( text, "soundGroup" ) ) 
				retVal = getInteger ( &object->soundGroup );

			else if ( !strcmp ( text, "strength" ) ) 
				retVal = getInteger ( &object->strength ); 

			else if ( !strcmp ( text, "dexterity" ) ) 
				retVal = getInteger ( &object->dexterity ); 

			else if ( !strcmp ( text, "intelligence" ) ) 
				retVal = getInteger ( &object->intelligence ); 

			else if ( !strcmp ( text, "quickness" ) ) 
				retVal = getInteger ( &object->quickness ); 

			else if ( !strcmp ( text, "endurance" ) ) 
				retVal = getInteger ( &object->endurance ); 

			else if ( !strcmp ( text, "poisonResistance" ) ) 
				retVal = 0;

			else if ( !strcmp ( text, "magicResistance" ) ) 
				retVal = 0; 

			else if ( !strcmp ( text, "enchantResistance" ) ) 
				retVal = getInteger ( &object->m_nEnchantResistance ); 

			else if ( !strcmp ( text, "value" ) ) 
				retVal = getInteger ( &object->value ); 

			else if ( !strcmp ( text, "level" ) ) 
				retVal = getInteger ( &object->level ); 

			else if ( !strcmp ( text, "minLevel" ) )
				retVal = getInteger ( &object->minLevel );

			else if ( !strcmp ( text, "maxLevel" ) )
				retVal = getInteger ( &object->maxLevel );

			else if ( !strcmp ( text, "resistCold" ) )
				retVal = retVal;

			else if ( !strcmp ( text, "mana" ) ) {
				int crap;
				retVal = getInteger ( &crap ); 
			}

			else if ( !strcmp ( text, "health" ) ) {
				retVal = getInteger ( &object->health ); 

				if ( retVal != -1 )
					object->healthMax = object->health;
			}

			else if ( !strcmp ( text, "alignment" ) ) 
				retVal = getInteger ( &object->alignment ); 

			else if ( !strcmp ( text, "armorPercent" ) )
				retVal = 0;

			else if ( !strcmp ( text, "armorDefense" ) )
				retVal = 0;

			else if ( !strcmp ( text, "armorType" ) )
				retVal = getInteger ( &object->armorType ); 

			else if ( !strcmp ( text, "armor" ) || !strcmp ( text, "armorFlat" ) ) {
				retVal = getInteger ( &object->baseArmor ); 
				object->armor = object->baseArmor;
			}

			else if ( !strcmp ( text, "damageType" ) ) {
				object->createHands();
				retVal = getInteger ( &object->hands->damageType ); 
			}

			else if ( !strcmp ( text, "range" ) ) {
				object->createHands();
				retVal = getInteger ( &object->hands->distance );
			}

			else if ( !strcmp ( text, "speed" ) ) {
				object->createHands();
				retVal = getInteger ( &object->hands->speed );
			}

			else if ( !strcmp ( text, "lethality" ) ) {
			}

			else if ( !strcmp ( text, "minDamage" ) ) {
				object->createHands();
				retVal = getInteger ( &object->hands->minDamage );
			}

			else if ( !strcmp ( text, "maxDamage" ) ) {
				object->createHands();
				retVal = getInteger ( &object->hands->maxDamage );
			}

			else if ( !strcmp ( text, "modifier" ) ) {
				object->createHands();
				retVal = getInteger ( &object->hands->modifier );
			}

			else if ( !strcmp ( text, "bonus" ) ) {
				object->createHands();
				retVal = getInteger ( &object->hands->bonus );
			}

			else if ( !strcmp ( text, "physicalState" ) )
				retVal = getInteger ( &object->physicalState );

			else if ( !strcmp ( text, "makePolygon" ) )
				retVal = getInteger ( &object->makePolygon );

			else if ( !strcmp ( text, "affect" ) ) {
				int id, type, source, duration, theVal = 0;

				retVal = getInteger ( &id );
				retVal = getInteger ( &type );
				retVal = getInteger ( &source );
				retVal = getInteger ( &duration );
				getInteger ( &theVal );

				if ( retVal != -1 )
					object->addAffect ( id, type, source, duration, theVal );
			}

			else if ( !strcmp ( text, "daffect" ) ) {
				int id, type, source, duration;

				retVal = getInteger ( &id );
				retVal = getInteger ( &type );
				retVal = getInteger ( &source );
				retVal = getInteger ( &duration );

				if ( retVal != -1 ) 
					object->addAffect ( id, type | _AFF_TYPE_DORMANT, source, duration, 0 );
			}

			else if ( !strcmp ( text, "caffect" ) ) {
				int id = -1;

				retVal = getInteger ( &id );

				if ( retVal != -1 ) {
					affect_t *affect = NULL;

					while ( affect = object->hasAffect ( id, -1 ) ) 
						object->delAffect ( affect );
				}
			}

			else if ( !strcmp ( text, "end" ) ) {
				state = _OIP_MAIN_BLOCK;
			}

			else {
				registerError ( "'%s' is not a valid property for an object", text );
				retVal = -1;
			}
		}

		break;

		case _OIP_COMPONENTS: {
			char *text = getTokenText();

			if ( text ) {
				if ( !strcmp ( text, "end" ) ) {
					object->tokens = buildTokenList ( object->name );
					stripTokens ( object->tokens );

					// calculate the value of this object based on it's components
					LinkedElement *element = object->components->head();
					int value = 0;

					while ( element ) {
						WorldObject *obj = (WorldObject *)element->ptr();
						element = element->next();

						if ( !(obj->physicalState & _STATE_WHOLESALE) )
							fatal ( "non-wholesale component inside of component object" );

						value += obj->netWorth();
					}

					value = (value * 250) / 100;
					object->value = value;

					gComponentObjects[object->skill].add ( object );
					state = _OIP_MAIN_BLOCK;
				} else {
					object->addComponent ( text );
				}
			} else {
				registerError ( "component specification expected" );
				retVal = -1;
			}
		}

		break;

		case _OIP_ACTIONS: {
			char *text = getTokenText();

			if ( text ) {
				if ( !strcmp ( text, "end" ) ) {
					state = _OIP_MAIN_BLOCK;
				} 

				else {
					int verb = textToVerb ( text );

					if ( verb == -1 ) {
						registerError ( "'%s' is not a valid verb name", text );
						retVal = -1;
					} else {
						text = getTokenText();
	
						if ( text ) {
							action_t action = getActionFunction ( text );
	
							if ( action ) {
								int argc = numTokens;
								char **argv = (char **)malloc ( sizeof ( char * ) * argc );
								for ( int t=0; t<argc; t++ )
									argv[t] = getToken();

								object->addAction ( verb, action, argc, argv );

								free ( argv );
							} else {
								registerError ( "'%s' is not a valid action function", text );
								retVal = -1;
							}
						} else {
							registerError ( "action function name expected" );
							retVal = -1;
						}
					}
				}
			} else {
				registerError ( "verb name expected" );
				retVal = -1;
			}
		}

		break;

		case _OIP_STORE_INVENTORY: {
			char *text = getTokenText();
			BShop *bshop = (BShop *)base;

			if ( !strcmp ( text, "end" ) ) {
				if ( category == &bshop->inventory ) 
					state = _OIP_BASE;
				else
					category = (ShopCategory *)category->owner;
			}

			else if ( !strcmp ( text, "testCategory" ) ) {
				//this is a category only active on a test-server
		
				if( !IsThisATestServer() ) {
					//we are not operating as a test server - bypass this whole block
					unsigned short level = 1;

					while( level > 0 ) {
						char* tok = getString();

						if( tok ) {
							if ( !strcmp( tok, "category" ) ) ++level;
							else if ( !strcmp( tok, "end" ) ) { --level; }
						}
						else {
							//advance to the next line
							numTokens = *((unsigned short *)&input[index]);
							index += 2;
							lastGetIdx = index;
							if ( !numTokens ) return -1;
						}
					}

				}
				else {
					// get the name and icon for this category
					char *categoryName = getString();
					int icon;

					if ( categoryName ) {
						retVal = getInteger ( &icon );

						if ( retVal == -1 ) {
							icon = 10233;
							retVal = 0;
						}

						if ( retVal != -1 ) {
							ShopCategory *newCategory = new ShopCategory;
							newCategory->setName ( categoryName );
							newCategory->setIcon ( icon );

							category->addItem ( newCategory );

							category = newCategory;
						}
					} else {
						registerError ( "category name expected" );
						retVal = -1;
					}
				}
			}

			else if ( !strcmp ( text, "category" ) ) {
				// get the name and icon for this category
				char *categoryName = getString();
				int icon;

				if ( categoryName ) {
					retVal = getInteger ( &icon );

					if ( retVal == -1 ) {
						icon = 10233;
						retVal = 0;
					}

					if ( retVal != -1 ) {
						ShopCategory *newCategory = new ShopCategory;
						newCategory->setName ( categoryName );
						newCategory->setIcon ( icon );

						category->addItem ( newCategory );

						category = newCategory;
					}
				} else {
					registerError ( "category name expected" );
					retVal = -1;
				}
			}

			else if ( !strcmp ( text, "skill" ) ) {
				retVal = 0;
			}

			else if ( !strcmp ( text, "mana" ) ) {
				ShopObject *shopObject = new ShopObject ( roomMgr->findClass ( "ManaBag" ) );
				category->addItem ( shopObject );
				bshop->addItem ( shopObject );
			}

			else if ( !strcmp ( text, "object" ) ) {
				if ( category ) {
					// get the name of the object to add
					text = getTokenText();

					if ( text ) {
						WorldObject *obj = roomMgr->findClass ( text );

						if ( obj ) {
							ShopObject *shopObject = new ShopObject ( obj );

							if ( numTokens ) {
								int newCost = 0;
								getInteger ( &newCost );

								logDisplay ( "%s: got new cost of %d", obj->classID, newCost );
								shopObject->cost = newCost;
							}
								
							category->addItem ( shopObject );
							bshop->addItem ( shopObject );
						} else {
							registerError ( "'%s' is not a valid class", text );
							retVal = -1;
						}
					} else {
						registerError ( "object name is expected after object directive" );
						retVal = -1;
					}
				} else {
					registerError ( "object directive requires enclosing category directive" );
					retVal = -1;
				}
			}

			else {
				registerError ( "'%s' is not a valid inventory directive", text );
				retVal = -1;
			}
		}

		break;

		case _OIP_BASE: {
			char *text = getTokenText();

			if ( !strcmp ( text, "end" ) ) {
				state = _OIP_MAIN_BLOCK;
			}

			else {
				switch ( base->type ) {
					case _BCYCLE: {
						BCycle *bcycle = (BCycle *)base;

						if ( !strcmp ( text, "speed" ) )
							retVal = getInteger ( &bcycle->cycleSpeed );
						else if ( !strcmp ( text, "type" ) )
							retVal = getInteger ( &bcycle->cycleType );
					}

					break;

					case _BCARRY: {
						BCarryable *bcarry = (BCarryable *)base;

						if ( !strcmp ( text, "weight" ) ) {
							text = getTokenText();

							if ( text ) {
								double value = atof ( text );
								bcarry->weight = (int)(value * 10);
							} else {
								registerError ( "value expected: 1.0 equals 1 pound" );
								retVal = -1;
							}
						}

						else if ( !strcmp ( text, "restrict" ) ) {
							text = getTokenText();

							if ( text ) {
								if( !strcmp( text, "homeowner" ) ) {
									//this makes the object only droppable at the
									//owners house... same for pick up.
									object->physicalState |= _STATE_HOUSE_DECOR;
								}
							} else {
								registerError ( "expected restriction type" );
								retVal = -1;
							}

						}
						else if ( !strcmp ( text, "bulk" ) ) {
							text = getTokenText();

							if ( text ) {
								double value = atof ( text );
								bcarry->bulk = (int)(value * 10);
							} else {
								registerError ( "value expected: 1.0 equals 1 cubic foot" );
								retVal = -1;
							}
						}

						else if ( !strcmp ( text, "show" ) )
							retVal = getInteger ( &bcarry->show );

						else if ( !strcmp ( text, "owner" ) ) {
							char *ownerName = getIdentifier ( "owner name expected" );

							if ( ownerName ) {
								char classID[256];

								if ( instanceNum > -1 ) {
									sprintf ( sizeof ( classID ), classID, "%s%03d", ownerName, instanceNum );
									ownerName = classID;
								}

								TreeNode *node = gObjectTree.find ( ownerName );
								WorldObject *owner = node? (WorldObject *)node->data : NULL;


								if ( owner ) {
									if ( object->servID > -1 ) 
										object->forceIn ( owner );
								} else {
									registerError ( "can not find owner by the name '%s'", ownerName );
									retVal = -1;
								}
							} else {
								retVal = -1;
							}
						}

						else {
							registerError ( "'%s' is not a valid property for carryable", text );
							retVal = -1;
						}
					}

					break;

					case _BOPEN: {
						BOpenable *bopen = (BOpenable *)base;

						if ( !strcmp ( text, "isOpen" ) ) {
							int isOpen = 0;
							retVal = getInteger ( &isOpen );
							bopen->isOpen ( isOpen );
						}

						else {
							registerError ( "'%s' is not a valid property for openable", text );
							retVal = -1;
						}
					}

					break;

					case _BDYE: {
						BDye *bdye = (BDye *)base;

						if ( !strcmp ( text, "hairDye" ) ) {
							retVal = getInteger ( &bdye->nHairDye );
						}

						else {
							registerError ( "'%s' is not a valid property for dye", text );
							retVal = -1;
						}
					}

					break;

					case _BSPELLBAG:
						break;

					case _BSWITCH: {
						BSwitch *bswitch = (BSwitch *)base;

						if ( !strcmp ( text, "active" ) ) {
							int active = 0;
							retVal = getInteger ( &active );
							bswitch->isOpen ( active );
						}

						else if ( !strcmp ( text, "enabled" ) ) 
							retVal = getInteger ( &bswitch->enabled );

						else {
							registerError ( "'%s' is not a valid property for switch", text );
							retVal = -1;
						}
					}

					break;

					case _BCONSUME: {
						BConsumable *bconsume = (BConsumable *)base;

						if ( !strcmp ( text, "state" ) ) {
							retVal = getInteger ( &bconsume->state );	
						}

						else {
							registerError ( "'%s' is not a valid property for consumable", text );
							retVal = -1;
						}
					}

					break;

					case _BUSE: {
						BUse *buse = (BUse *)base;

						if ( !strcmp ( text, "uses" ) )
							retVal = getInteger ( &buse->uses );

						else if ( !strcmp ( text, "usesMax" ) )
							retVal = getInteger ( &buse->usesMax );

						else if ( !strcmp ( text, "verb" ) )
							retVal = getInteger ( &buse->verb );

						else if ( !strcmp ( text, "useCost" ) )
							retVal = getInteger ( &buse->useCost );

						else if ( !strcmp ( text, "spell" ) )
							retVal = getInteger ( &buse->spell );

						else if ( !strcmp ( text, "theurgismSkill" ) )
							retVal = getInteger ( &buse->theurgism );

						else {
							registerError ( "'%s' is not a valid property for use", text );
							retVal = -1;
						}
					}

					break;

					case _BMIX: {
						BMix *bmix = (BMix *)base;

						if ( !strcmp ( text, "skillLevel" ) ) {
							text = getTokenText();

							int skillLevel = textToSkillLevel ( text );

							if ( skillLevel == -1 ) {
								registerError ( "'%s' is not a valid skill level", text );
								retVal = -1;
							} else {
								bmix->skillLevel = skillLevel;
							}
						}

						else if ( !strcmp ( text, "skill" ) ) {
							text = getTokenText();

							int skill = textToSkill ( text );

							if ( skill == -1 ) {
								registerError ( "'%s' is not a valid skill name", text );
								retVal = -1;
							} else {
								bmix->skill = skill;
							}
						}

						else {
							registerError ( "'%s' is not a valid property for mix", text );
							retVal = -1;
						}
					}

					break;

					case _BCONTAIN: {
						BContainer *bcontain = (BContainer *)base;

						if ( !strcmp ( text, "bulkCapacity" ) ) {
							text = getTokenText();

							if ( text ) {
								double value = atof ( text );
								bcontain->bulkCapacity = (int)(value * 10);
							} else {
								registerError ( "value expected: 1.0 equals 1 cubic foot" );
								retVal = -1;
							}
						}

						else if ( !strcmp ( text, "bulkReduction" ) ) {
							retVal = getInteger ( &bcontain->bulkReduction );	
						}

						else if ( !strcmp ( text, "weightReduction" ) ) {
							retVal = getInteger ( &bcontain->weightReduction );
						}

						else if ( !strcmp ( text, "weightCapacity" ) ) {
							text = getTokenText();

							if ( text ) {
								double value = atof ( text );
								bcontain->weightCapacity = (int)(value * 10);
							} else {
								registerError ( "value expected: 1.0 equals 1 pound" );
								retVal = -1;
							}
						}

						else if ( !strcmp ( text, "object" ) ) {
							text = getIdentifier ( "object name expected" );

							if ( !roomMgr->findClass ( text ) ) {
								registerError ( "'%s' is not a valid object name", text );
								retVal = -1;
							} else {
								object->addObject ( text );
							}
						}

						else if ( !strcmp ( text, "gold" ) ) {
							text = getIdentifier ( "value expected" );

							if ( !text ) {
								retVal = -1;
							} else {
								int value = atoi ( text );

								WorldObject *money = new WorldObject ( roomMgr->findClass ( "MoneyBag" ) );
								money->physicalState |= _STATE_MONEY;
								money->value = value;
								money->addToDatabase();

								money->forceIn ( object );
								money->isVisible = 1;
							}
						}

						else if ( !strcmp ( text, "crystals" ) ) {
							text = getIdentifier ( "value expected" );

							if ( !text ) {
								retVal = -1;
							} else {
								int value = atoi ( text );

								WorldObject *money = new WorldObject ( roomMgr->findClass ( "ManaBag" ) );
								money->physicalState |= _STATE_MONEY;
								money->value = value;
								money->addToDatabase();

								money->forceIn ( object );
								money->isVisible = 1;
							}
						}

						else if ( !strcmp ( text, "worn" ) ) {
							text = getIdentifier ( "object name expected" );

							if ( !roomMgr->findClass ( text ) ) {
								registerError ( "'%s' is not a valid object name", text );
								retVal = -1;
							} else {
								object->addObject ( text, 1 );
							}
						}

						else if ( !strcmp ( text, "head" ) ) {
							WorldObject *head = new WorldObject ( roomMgr->findClass ( "Head" ) );

							BHead *bhead = (BHead *)head->getBase ( _BHEAD );

							getInteger ( &bhead->sex );
							getInteger ( &bhead->skinColor );
							getInteger ( &bhead->headNumber );
							getInteger ( &bhead->hairNumber );
							getInteger ( &bhead->hairColor );
							getInteger ( &bhead->browNumber );
							getInteger ( &bhead->faceHairNumber );

							getInteger ( &bhead->eyeNumber );
							getInteger ( &bhead->eyeColor );

							getInteger ( &bhead->noseNumber );
							getInteger ( &bhead->mouthNumber );
							getInteger ( &bhead->earNumber );
							getInteger ( &bhead->race );

							head->addToDatabase();
							head->forceIn ( object );
							head->isVisible = 1;

							switch ( bhead->race ) {
								case _RACE_GIANT: 
									object->height = 110;
									object->girth = 110;
									break;

								case _RACE_ELF:
									bhead->self->height = 90;
									bhead->self->girth = 90;
									break;
							}
						}

						else if ( !strcmp ( text, "accept" ) ) {
							text = getIdentifier ( "object name expected" );
							acceptRecord* pAccept = bcontain->addAccept( text );

							if ( pAccept ) {
								if ( getChar ( &pAccept->acceptCounter ) == -1 ) {
									registerError ( "Expected counter index and did not get it." );
									retVal = -1;
								}

								if ( getChar ( &pAccept->acceptAmount ) ) {
									registerError ( "Expected counter increment." );
									retVal = -1;
								}
							} else {
								registerError ( "'%s' is not a valid object name or you have more than 10 accepted items.", text );
								retVal = -1;
							}
						}

						else {
							registerError ( "'%s' is not a valid property for container", text );
							retVal = -1;
						}
					}

					break;

					case _BLOCK: {
						BLockable *block = (BLockable *)base;

						if ( !strcmp ( text, "isLocked" ) || !strcmp ( text, "locked" ) ) {
							int isLocked = 0;
							retVal = getInteger ( &isLocked );
							block->isLocked ( isLocked );
						}

						else if ( !strcmp ( text, "lockValue" ) )
							retVal = getInteger ( &block->lockValue );

						else if ( !strcmp ( text, "unlockValue" ) )
							retVal = getInteger ( &block->unlockValue );

						else if ( !strcmp ( text, "skeletonLock" ) )
							retVal = getInteger ( &block->skeletonLock );

						else if ( !strcmp ( text, "skeletonUnlock" ) )
							retVal = getInteger ( &block->skeletonUnlock );

						else if ( !strcmp ( text, "autoLock" ) )
							retVal = getInteger ( &block->autoLock );

						else {
							registerError ( "'%s' is not a valid property for lockable", text );
							retVal = -1;
						}
					}

					break;

					case _BKEY: {
						BKey *key = (BKey *)base;

						if ( !strcmp ( text, "lockValue" ) )
							retVal = getInteger ( &key->lockValue );

						else if ( !strcmp ( text, "unlockValue" ) )
							retVal = getInteger ( &key->unlockValue );

						else if ( !strcmp ( text, "skeletonLock" ) )
							retVal = getInteger ( &key->skeletonLock );

						else if ( !strcmp ( text, "skeletonUnlock" ) )
							retVal = getInteger ( &key->skeletonUnlock );

						else {
							registerError ( "'%s' is not a valid property for key", text );
							retVal = -1;
						}
					}

					break;

					case _BWEAR: {
						BWearable *bwear = (BWearable *)base;

						if ( !strcmp ( text, "areaWorn" ) )
							retVal = getInteger ( &bwear->areaWorn );

						else if ( !strcmp ( text, "mask" ) ) {
							text = getTokenText();

							if ( text ) {
								int mask = textToWearMask ( text );

								if ( mask == -1 ) {
									registerError ( "'%s' is not a valid mask", text );
									retVal = -1;
								} else {
									bwear->mask &= ~mask;
								}
							} else {
								registerError ( "mask value expected" );
								retVal = -1;
							}
						}

						else if ( !strcmp ( text, "layer" ) )
							retVal = getInteger ( &bwear->layer );
// new wear!
                        else if ( !strcmp ( text, "level" ) ) 
                            retVal = getInteger ( &bwear->level );
						
						else if ( !strcmp ( text, "spellProcID" ) )
                            retVal = getInteger ( &bwear->spellProcID );

                        else if ( !strcmp ( text, "reverseProcID" ) )
                            retVal = getInteger ( &bwear->reverseProcID );

						else if ( !strcmp ( text, "forwardProcChance" ) )
							retVal = getInteger ( &bwear->forwardProcChance );

						else if ( !strcmp ( text, "reverseProcChance" ) )
							retVal = getInteger ( &bwear->reverseProcChance );

						else {
							registerError ( "'%s' is not a valid property for wearable", text );
							retVal = -1;
						}
					}

					break;

					case _BDESCRIBED: {
						BDescribed *bdescribed = (BDescribed *)base;

						if ( !strcmp ( text, "text" ) ) {
							char *txt = getString();

							if ( txt ) {
								bdescribed->setText ( txt );
							} else {
								registerError ( "text expected" );
								retVal = -1;
							}
						}

						else if ( !strcmp ( text, "shopText" ) ) {
							char *txt = getString();

							if ( txt ) {
								bdescribed->setShopText ( txt );
							} else {
								registerError ( "text expected" );
								retVal = -1;
							}
						}

						else if ( !strcmp ( text, "book" ) ) {
							char* fileName = getString();

							if ( fileName ) {
								File* bookFile = new File( fileName );

								if( bookFile->isOpen() && bookFile->size() )
								{
									char* fileBuf = new char[ bookFile->size() ];
									bookFile->read( fileBuf, bookFile->size() );
									bdescribed->setText( fileBuf );
									bdescribed->isBook = true;
									delete [] fileBuf;

								} else {
									registerError ( "error reading book file %s", fileName );
									retVal = -1;
								}

								delete bookFile;
								
							} else {
								registerError ( "filename invalid" );
								retVal = -1;
							}
						}

						else if ( !strcmp ( text, "idText" ) ) {
							char *txt = getString();

							if ( txt ) {
								bdescribed->setIDText ( txt );
							} else {
								registerError ( "text expected" );
								retVal = -1;
							}
						}

						else if ( !strcmp ( text, "acceptText" ) ) {
							char *txt = getString();

							if ( txt ) {
								bdescribed->setAcceptText ( txt );

								for (int nRecord = 0;nRecord < _MAX_ACCEPT_COUNTS;nRecord++) {
									if ( getChar ( &bdescribed->acceptDisplay[ nRecord ] ) == -1 ) {
										registerError( "Expected accepted display number on %d of %d", ( nRecord + 1 ), _MAX_ACCEPT_COUNTS );
										retVal = -1;
										break;
									}

									if ( bdescribed->acceptDisplay[ nRecord ] >= _MAX_ACCEPT_COUNTS && bdescribed->acceptDisplay[ nRecord ] < 0 ) {
										registerError( "Accepted display number must be less than %d", _MAX_ACCEPT_COUNTS );
										retVal = -1;
										break;
									}
								}
							} else {
								registerError ( "text expected" );
								retVal = -1;
							}
						}

						else if ( !strcmp ( text, "0" ) ) {
							retVal = retVal;
						}

						else if ( !strcmp ( text, "riddleFile" ) ) {
							char *filename = getString();

							if ( filename ) {
								if ( exists ( filename ) ) {
									bdescribed->self->physicalState |= _STATE_RIDDLED;

									File *file = new File ( filename );
									int bufferSize = file->size();
									char str[1024];

									char *buffer = (char *)malloc ( bufferSize + 1 );
									buffer[bufferSize] = 0;
									char *ptr = buffer;

									file->read ( buffer, bufferSize );

									LinkedList riddles, answers;

									while ( bufferSize ) {
										// get the riddle text
										bufgets ( str, &ptr, &bufferSize );
										riddles.add ( new StringObject ( str ) );

										// get the riddle answer
										bufgets ( str, &ptr, &bufferSize );
										answers.add ( new StringObject ( str ) );
									}

									if ( riddles.size() ) {
										int index = random ( 0, riddles.size() - 1 );

										StringObject *riddle = (StringObject *)riddles.at ( index );
										bdescribed->setText ( riddle->data );
										bdescribed->setRiddleText ( riddle->data );

										StringObject *answer = (StringObject *)answers.at ( index );
										bdescribed->setRiddleAnswer ( answer->data );
									}

									free ( buffer );
									delete file;
								}
							}
						}

						else {
							registerError ( "'%s' is not a valid property for described", text );
							retVal = -1;
						}
					}

					break;

					case _BENTRY: {
						BEntry *bentry = (BEntry *)base;

						if ( !strcmp ( text, "room" ) ) {
							retVal = getInteger ( &bentry->room );

							if ( instanceNum > -1 ) {
								bentry->room = ROOM_TO_INSTANCE ( bentry->room );
							}
						}

						else if ( !strcmp ( text, "startingX" ) ) 
							retVal = getInteger ( &bentry->startingX );
					
						else if ( !strcmp ( text, "startingY" ) )
							retVal = getInteger ( &bentry->startingY );

						else if ( !strcmp ( text, "endingX" ) )
							retVal = getInteger ( &bentry->endingX );

						else if ( !strcmp ( text, "endingY" ) )
							retVal = getInteger ( &bentry->endingY );

						else if ( !strcmp ( text, "startingLoop" ) )
							retVal = getInteger ( &bentry->startingLoop );

						else if ( !strcmp ( text, "endingLoop" ) )
							retVal = getInteger ( &bentry->endingLoop );

						else {
							registerError ( "'%s' is not a valid property for entry", text );
							retVal = -1;
						}
					}

					break;

					case _BCHARACTER: {
						BCharacter *bcharacter = (BCharacter *)base;
						if( (intptr_t)bcharacter  == 0x21 ) { logInfo( _LOG_ALWAYS, "%s:%d - BCharacter value corrupted", __FILE__, __LINE__ ); }

						if ( !strcmp ( text, "profession" ) ) 
							retVal = getInteger ( &bcharacter->profession );	

						else if ( !strcmp ( text, "experience" ) ) 
							retVal = getInteger ( &bcharacter->experience );	

						else if ( !strcmp ( text, "race" ) ) 
							retVal = getInteger ( &bcharacter->race );	

						else if ( !strcmp ( text, "sex" ) ) 
							retVal = getInteger ( &bcharacter->sex );	

						else if ( !strcmp ( text, "buildPoints" ) ) 
							retVal = getInteger ( &bcharacter->buildPoints );	

						else if ( !strcmp ( text, "lightMagicSkill" ) ) 
							retVal = 0;

						else if ( !strcmp ( text, "darkMagicSkill" ) ) 
							retVal = 0;

						else if ( !strcmp ( text, "earthMagicSkill" ) ) 
							retVal = 0;

						else if ( !strcmp ( text, "daggerSkill" ) ) 
							retVal = 0;

						else if ( !strcmp ( text, "swordSkill" ) ) 
							retVal = 0;

						else if ( !strcmp ( text, "axeSkill" ) ) 
							retVal = 0;

						else if ( !strcmp ( text, "unarmedSkill" ) ) 
							retVal = 0;

						else if ( !strcmp ( text, "detectTrapSkill" ) ) 
							retVal = 0;

						else if ( !strcmp ( text, "pickPocketSkill" ) ) 
							retVal = 0;

						else if ( !strcmp ( text, "stealthSkill" ) ) 
							retVal = 0;

						else if ( !strcmp ( text, "defenseSkill" ) ) 
							retVal = 0;

						else if ( !strcmp ( text, "BroadswordSkill" ) ) 
							retVal = 0;
						

						else if ( !strcmp ( text, "skill" ) ) {
							int number = 0, value = 0;

							retVal = getInteger ( &number );
							retVal = getInteger ( &value );

							bcharacter->skills[number] = value;
						}

						else if ( !strcmp ( text, "biography" ) ) {
							char *txt = getString();

							if ( txt ) {
								if ( strlen ( txt ) ) 
									object->setBiography ( txt );
							}
						}

						else if ( !strcmp ( text, "properName" ) ) {
							char *txt = getString();

							if ( txt ) {
								if ( strlen ( txt ) ) 
									strcpy ( bcharacter->properName, txt );
							} else {
								registerError ( "text expected for properName" );
								retVal = -1;
							}	
						}

						else if ( !strcmp ( text, "title" ) ) {
							char *txt = getString();

							if ( txt ) {
								if ( strlen ( txt ) ) 
									strcpy ( bcharacter->title, txt );
							} else {
								registerError ( "text expected for title" );
								retVal = -1;
							}
						}

						else {
							registerError ( "'%s' is not a valid property for character", text );
							retVal = -1;
						}
					}

					break;

					case _BSHOP: {
						BShop *bshop = (BShop *)base;

						if ( !strcmp ( text, "inventory" ) ) {
							category = &bshop->inventory;
							state = _OIP_STORE_INVENTORY;
						}

						else if ( !strcmp ( text, "buyMarkup" ) ) {
							retVal = getInteger ( &bshop->buyMarkup );
						}

						else if ( !strcmp ( text, "sellMarkup" ) ) {
							retVal = getInteger ( &bshop->sellMarkup );
						}

						else if ( !strcmp ( text, "currency" ) ) {
							text = getString();

							if( !strcmp( text, "gold" ) ) {
								bshop->currency = BShop::Gold;
							}
							else if( !strcmp( text, "copper" ) ) {
								bshop->currency = BShop::Copper;
							} else {
								registerError ( "'%s' is not a valid currency for shop", text );
								retVal = -1;
							}
						}

						else {
							registerError ( "'%s' is not a valid property for shop", text );
							retVal = -1;
						}
					}

					break;

					case _BNPC: {
						BNPC *bnpc = (BNPC *)base;

						if ( !strcmp ( text, "code" ) ) {
							text = getIdentifier ( "code name expected" );
							bnpc->setCode ( text );
						}

						else if ( !strcmp ( text, "external" ) ) {
							retVal = getInteger ( &bnpc->isExternal );
						}

						else {
							registerError ( "'%s' is not a valid property for npc", text );
							retVal = -1;
						}
					}

					break;

					case _BPASSWORD: {
						BPassword *bpassword = (BPassword *)base;

						if ( !strcmp ( text, "password" ) ) {
							text = getString();
							if( text )
								sprintf ( sizeof ( bpassword->password ), bpassword->password, "%s", text );
							else registerError ( "expected password in 'base password'" );
						}
					}

					break;

					case _BGATE: {
					}

					break;

					case _BSIT: {
					}

					break;

					case _BTREASURE: {
						BTreasure *btreasure = (BTreasure *)base;

						WorldObject *superObj = roomMgr->findClass ( text );

						if ( superObj ) {
							int minVal = 0, maxVal = 0;
							getInteger ( &minVal );
							getInteger ( &maxVal );

							btreasure->addTreasure ( text, minVal, maxVal );
						} else {
							registerError ( "'%s' is not a valid class name", text );
							retVal = -1;
						}
					}

					break;

					case _BSCROLL: {
						BScroll *bscroll = (BScroll *)base;
						int crap;

						if ( !strcmp ( text, "magic" ) )
							retVal = getInteger ( &crap );

						else if ( !strcmp ( text, "spell" ) )
							retVal = getInteger ( &bscroll->spell );

						else if ( !strcmp ( text, "level" ) )
							retVal = getInteger ( &bscroll->level );

						else if ( !strcmp ( text, "skill" ) )
							retVal = getInteger ( &bscroll->skill );

						else if ( !strcmp ( text, "bp" ) )
							retVal = getInteger ( &bscroll->bpCost );

						else if ( !strcmp ( text, "objectRequired" ) ) {
							char *str = getTokenText();
						}

						else {
							registerError ( "'%s' is not a valid property for scroll", text );
							retVal = -1;
						}
					}

					break;

					case _BTALK: {
						BTalk *btalk = (BTalk *)base;

						if ( !strcmp ( text, "file" ) ) {
							char *name = getString();

							if ( name ) {
								btalk->setFile ( name );
							} else {
								retVal = -1;
							}
						}

						else if ( !strcmp ( text, "talkTreeID" ) ) {
							retVal = getInteger ( &btalk->talkTreeID );
						}

						else {
							registerError ( "'%s' is not a valid property for talk", text );
							retVal = -1;
						}
					}

					break;

					case _BHEAD: {
						BHead *bhead = (BHead *)base;

						if ( !strcmp ( text, "race" ) )
							retVal = getInteger ( &bhead->race );

						else if ( !strcmp ( text, "sex" ) )
							retVal = getInteger ( &bhead->sex );

						else if ( !strcmp ( text, "headNumber" ) )
							retVal = getInteger ( &bhead->headNumber );

						else if ( !strcmp ( text, "eyeNumber" ) )
							retVal = getInteger ( &bhead->eyeNumber );

						else if ( !strcmp ( text, "hairNumber" ) )
							retVal = getInteger ( &bhead->hairNumber );

						else if ( !strcmp ( text, "browNumber" ) )
							retVal = getInteger ( &bhead->browNumber );

						else if ( !strcmp ( text, "noseNumber" ) )
							retVal = getInteger ( &bhead->noseNumber );

						else if ( !strcmp ( text, "earNumber" ) )
							retVal = getInteger ( &bhead->earNumber );

						else if ( !strcmp ( text, "mouthNumber" ) )
							retVal = getInteger ( &bhead->mouthNumber );

						else if ( !strcmp ( text, "faceHairNumber" ) )
							retVal = getInteger ( &bhead->faceHairNumber );

						else if ( !strcmp ( text, "skinColor" ) )
							retVal = getInteger ( &bhead->skinColor );

						else if ( !strcmp ( text, "eyeColor" ) )
							retVal = getInteger ( &bhead->eyeColor );

						else if ( !strcmp ( text, "hairColor" ) )
							retVal = getInteger ( &bhead->hairColor );

						else {
							registerError ( "'%s' is not a valid property for head", text );
							retVal = -1;
						}
					}

					break;

					case _BWEAPON: {
						BWeapon *bweapon = (BWeapon *)base;

						if ( !strcmp ( text, "damageType" ) ) {
							retVal = getInteger ( &bweapon->damageType );  
						}

						else if ( !strcmp ( text, "skillType" ) ) {
							retVal = getInteger ( &bweapon->skillType );
						}

						else if ( !strcmp ( text, "mask" ) ) {
							text = getTokenText();

							if ( text ) {
								int mask = textToWearMask ( text );

								if ( mask == -1 ) {
									registerError ( "'%s' is not a valid mask", text );
									retVal = -1;
								} else {
									bweapon->mask &= ~mask;
								}
							} else {
								registerError ( "mask value expected" );
								retVal = -1;
							}
						}

						else if ( !strcmp ( text, "missile" ) )
							retVal = getInteger ( &bweapon->isMissile );

						else if ( !strcmp ( text, "speed" ) ) 
							retVal = getInteger ( &bweapon->speed );

						else if ( !strcmp ( text, "minDamage" ) )
							retVal = getInteger ( &bweapon->minDamage );

						else if ( !strcmp ( text, "maxDamage" ) )
							retVal = getInteger ( &bweapon->maxDamage );

						else if ( !strcmp ( text, "spellProcID" ) )
                            retVal = getInteger ( &bweapon->spellProcID );
						
						else if ( !strcmp ( text, "reverseProcID" ) )
                            retVal = getInteger ( &bweapon->reverseProcID );

						else if ( !strcmp ( text, "forwardProcChance" ) )
							retVal = getInteger ( &bweapon->forwardProcChance );

						else if ( !strcmp ( text, "reverseProcChance" ) )
							retVal = getInteger ( &bweapon->reverseProcChance );

						else if ( !strcmp ( text, "modifier" ) )
							retVal = getInteger ( &bweapon->modifier );

						else if ( !strcmp ( text, "bonus" ) )
							retVal = getInteger ( &bweapon->bonus );

						else if ( !strcmp ( text, "hands" ) )
							retVal = getInteger ( &bweapon->hands );

						else if ( !strcmp ( text, "range" ) )
							retVal = getInteger ( &bweapon->distance );

						else if ( !strcmp ( text, "lethality" ) )
							retVal;

						else {
							registerError ( "'%s' is not a valid property for weapon", text );
							retVal = -1;
						}
					}

					break;

					default: {
						registerError ( "you may not set properties of this base (%d)", base->type );
						retVal = -1;
					}

					break;
				} 
			}
		}

		break;
	}

	return retVal;	
}

// this class parses zone specification information
ZoneInfoParser::ZoneInfoParser()
{
	zone = new Zone;
	state = _ZIP_START;
}

ZoneInfoParser::~ZoneInfoParser()
{
}

int ZoneInfoParser::processTokenList ( void )
{
	int retVal = 0;

	switch ( state ) {
		case _ZIP_START: {
			char *text = getIdentifier ( "zone specification statement expected" );

			if ( text ) {
				if ( !strcmp ( text, "worldFile" ) ) {
					char *name = getString();
	
					if ( name == NULL ) {
						retVal = -1;
					} else {
						zone->addWorldFile ( name );
					}
				}

				else if ( !strcmp ( text, "resetInterval" ) ) {
					retVal = getInteger ( &zone->resetInterval() );
				}

				else if ( !strcmp ( text, "isDungeon" ) ) {
					retVal = getInteger ( &zone->isDungeon );
				}

				else if ( !strcmp ( text, "isRandom" ) ) {
					retVal = getInteger ( &zone->isRandom );
				}

				else if ( !strcmp ( text, "homeTown" ) ) {
					retVal = getInteger ( &zone->homeTown );
				}
	
				else if ( !strcmp ( text, "maximumObjectAge" ) ) {
					retVal = getInteger ( &zone->maximumObjectAge() );
				}

				else if ( !strcmp ( text, "allowCombat" ) )
					retVal = getInteger ( &zone->allowCombat() );

				else if ( !strcmp ( text, "noAlignmentChange" ) ) {
					zone->allowCombat() |= _COMBAT_NO_ALIGNMENT;
					retVal = 0;
				}

				else if ( !strcmp ( text, "script" ) ) {
					char *pName = getIdentifier ( "zone script name expected" );

					if ( pName ) {
						zone->SetScript ( pName );
						retVal = 0;
					} else {
						retVal = -1;
					}
				}

				else if ( !strcmp ( text, "noTreasureDrop" ) ) {
					zone->allowCombat() |= _COMBAT_NO_TREASURE;
					retVal = 0;
				}

				else if ( !strcmp ( text, "monster" ) ) {
					char *name = getIdentifier ( "monster class name expected" );

					if ( name ) {
						int population = 0;
						retVal = getInteger ( &population );

						if ( retVal != -1 ) {
							zone->addMonsterType ( name, population );
						}
					} else {
						retVal = -1;
					}
				}

				else if ( !strcmp ( text, "monsterGroup" ) ) {
					//mike-groupspawn
					// syntax:
					// monsterGroup population quantity className [quantity className]...
					// example:
					// monsterGroup 10 2 Troll 3 WoodRat 2 Ratling
					
					// this will keep 10 of these groups in the zone, 
					// and the monsters in the group are 2 Trolls, 3 WoodRat's, and 2 Ratlings
					// (the leader of the group is one of the trolls.)

					MonsterGroup* monsterGroup = new MonsterGroup();

					int memberCount = 0;
					char* memberClassName = NULL;
					
					retVal = getInteger( &monsterGroup->population );

					if(retVal != -1) {
						//we have a 'valid' population. get all the memberCount/memberClassName pairs.
						for(;;) {
							retVal = getInteger( &memberCount );
							
							if( retVal == -1 ) {
								break;
							} else {
								memberClassName = getIdentifier( "syntax error (monsterGroup): monster class name expected" );

								//we have a classname and count pair. add this monster type to the group
								monsterGroup->addMonsterType( memberClassName, memberCount );
							}

						}
						//the monster group has all it's members defined in it.
						//add this group definition to the zone.
						retVal = 0;
						zone->addMonsterGroup( monsterGroup );
					} else {
						logDisplay( "syntax error %s(monsterGroup): integer group population expected", &zone->_name );
						delete monsterGroup;
					}
				}

				else if ( !strcmp ( text, "ambush" ) ) {
					CAmbushGroup *pAmbushGroup = new CAmbushGroup;
					zone->addAmbushGroup ( pAmbushGroup );

					// get the chance of this ambush group appearing
					int nChance = 0;
					getInteger ( &nChance );
					pAmbushGroup->SetChance ( nChance );

					for ( ;; ) {
						char *pName = getTokenText();

						if ( NULL == pName ) {
							break;
						} else {
							pAmbushGroup->AddMonster ( pName );	
							logDisplay ( "add monster '%s' (level now %d)", pName, pAmbushGroup->GetLevel() );
						}

						logDisplay ( "ambush group complete (level now %d)", pAmbushGroup->GetLevel() );
					}
				}

				else if ( !strcmp ( text, "ambushInterval" ) ) {
					retVal = getInteger ( &zone->ambushInterval );
				}

				else if ( !strcmp ( text, "entrance" ) ) {
					char *name = getIdentifier ( "entrance object name expected" );

					if ( name ) {
						zone->entranceName = strdup ( name );
					} else {
						retVal = -1;
					}
				}

				else if ( !strcmp ( text, "name" ) ) {
					char *name = getString();

					if ( name == NULL ) {
						retVal = -1;
					} else {
						strcpy ( zone->name(), name );
					}
				}

				else if ( !strcmp ( text, "title" ) ) {
					char *title = getString();

					if ( title == NULL ) {
						retVal = -1;
					} else {
						zone->setTitle ( title );
					}
				}

				else if ( !strcmp ( text, "pkg" ) ) 
					retVal = getInteger ( &zone->pkgInfo );

				else if ( !strcmp ( text, "midiFile" ) )
					retVal = getInteger ( &zone->midiFile );

				else if ( !strcmp ( text, "groupChance" ) )
					retVal = getInteger ( &zone->groupChance );

				else {
					registerError ( "'%s' is not a valid zone statement", text );
					retVal = -1;
				}
			}
		}
	}

	return retVal;
}

WorldInfoParser::WorldInfoParser()
{
	state = _WIP_START;
	room = NULL;
	zone = NULL;
	parser.saveObjects = 1;
}

WorldInfoParser::~WorldInfoParser()
{
}

int WorldInfoParser::processTokenList ( void )
{
	int retVal = 0, number = -1;

	switch ( state ) {
		case _WIP_START: {
			retVal = expect ( "room" );
	
			if ( retVal == 0 ) {
				retVal = getInteger ( &number );

				if ( retVal == 0 ) {

					if( roomMgr->findRoom( number ) != 0 )
					{

						FILE *file = fopen ( "../logs/duplicaterooms.txt", "aw" );

						if( !file ) {
							logDisplay ( "Error opening /logs/duplicaterooms.txt for writing.");
						}
						else {
							fprintf ( file, "Duplicate room: %d  ", number );
							fprintf ( file, "Previous room zone: %s(%s)  ", roomMgr->findRoom( number )->zone->name(), roomMgr->findRoom( number )->zone->title );
							fprintf ( file, "New room zone: %s(%s)\n", zone->name(), zone->title );
							fclose(file);
						}
					}

					state = _WIP_ROOM;
					room = new RMRoom;

					if ( instanceNum > -1 ) {
						number = ROOM_TO_INSTANCE ( number );
						room->instanceNum = instanceNum;
					}

					room->number = number;
					room->zone = zone;
				}
			}
		}

		break;

		case _WIP_ROOM: {
	
			char *text = getIdentifier ( "room specification statement expected" );

			if ( text ) {
				if ( !strcmp ( text, "properties" ) ) {
					state = _WIP_PROPERTIES;
				}

				else if ( !strcmp ( text, "objects" ) ) {
					state = _WIP_OBJECTS;
				}

				else if ( !strcmp ( text, "atpinfo" ) ) {
					state = _WIP_ATPINFOBLOCK;	
				}

				else if ( !strcmp ( text, "end" ) ) {
					state = _WIP_START;
					retVal = 2;
				}

				else {
					registerError ( "'%s' is not a valid room statement", text );
					retVal = -1;
				}
			}
		}

		break;

		case _WIP_PROPERTIES: {
	
			char *text = getIdentifier ( "room property expected" );

			if ( text ) {
				if ( !strcmp ( text, "picture" ) ) {
					retVal = getInteger ( &room->picture );
				}

				else if ( !strcmp ( text, "roomName" ) ) {
					char *name = getString();

					if ( name ) {
						room->setTitle ( name );
					}

					retVal = 1;
				}

				else if ( !strcmp ( text, "midiFile" ) ) 
					retVal = getInteger ( &room->midiFile );

				else if ( !strcmp ( text, "groupChance" ) )
					retVal = getInteger ( &room->groupChance );
				
				else if ( !strcmp ( text, "allowAmbush" ) )
					retVal = getInteger ( &room->bAllowAmbush );

				else if ( !strcmp ( text, "template" ) ) {
					int number;
					retVal = getInteger ( &number );

					if ( retVal != -1 ) {
						RMRoom *theRoom = roomMgr->findRoom ( number );

						if ( theRoom ) {
							room->picture = theRoom->picture;
							LinkedElement *element = theRoom->atpList.head();

							while ( element ) {
								ATPInfo *info = (ATPInfo *)element->ptr();
								room->addATP ( info->type, info->x, info->y, info->z );

								element = element->next();
							}
						} else {
							registerError ( "unable to find template room %d", number );
							retVal = -1;
						}
					}
				}

				else if ( !strcmp ( text, "north" ) ) {
					retVal = getInteger ( &room->north );
					room->exits |= _ROOM_NORTH_BIT;

					if ( instanceNum > -1 )
						room->north = ROOM_TO_INSTANCE ( room->north );

					if ( numTokens == 2 ) {
						int theX, theY; 

						getInteger ( &theX );
						getInteger ( &theY );

						room->exitCoords[0][0] = theX;
						room->exitCoords[0][1] = theY;
					}
				}

				else if ( !strcmp ( text, "south" ) ) {
					retVal = getInteger ( &room->south );
					room->exits |= _ROOM_SOUTH_BIT;

					if ( instanceNum > -1 )
						room->south = ROOM_TO_INSTANCE ( room->south );

					if ( numTokens == 2 ) {
						int theX, theY; 

						getInteger ( &theX );
						getInteger ( &theY );

						room->exitCoords[1][0] = theX;
						room->exitCoords[1][1] = theY;
					} 

					else if ( numTokens == 1 ) {
						text = getIdentifier ( "object name expected" );	

						if ( text ) {
//							WorldObject *obj = roomMgr->findObjectByClass ( text );

							TreeNode *node = gObjectTree.find ( text );
							WorldObject *obj = node? (WorldObject *)node->data : NULL;

							if ( obj ) {
								room->exitCoords[1][0] = obj->x;
								room->exitCoords[1][1] = obj->y + 5;
							} else {
								registerError ( "'%s' is not a valid object name (where is it?)", text );
								retVal = -1;
							}
						} else {
							retVal = -1;
						}
					}
				}

				else if ( !strcmp ( text, "east" ) ) {
					retVal = getInteger ( &room->east );
					room->exits |= _ROOM_EAST_BIT;

					if ( instanceNum > -1 )
						room->east = ROOM_TO_INSTANCE ( room->east );

					if ( numTokens == 2 ) {
						int theX, theY; 

						getInteger ( &theX );
						getInteger ( &theY );

						room->exitCoords[2][0] = theX;
						room->exitCoords[2][1] = theY;
					}
				}

				else if ( !strcmp ( text, "west" ) ) {
					retVal = getInteger ( &room->west );
					room->exits |= _ROOM_WEST_BIT;

					if ( instanceNum > -1 )
						room->west = ROOM_TO_INSTANCE ( room->west );

					if ( numTokens == 2 ) {
						int theX, theY; 

						getInteger ( &theX );
						getInteger ( &theY );

						room->exitCoords[3][0] = theX;
						room->exitCoords[3][1] = theY;
					}
				}

				else if ( !strcmp ( text, "type" ) ) 
					retVal = getInteger ( &room->type );

				else if ( !strcmp ( text, "up" ) ) {
					retVal = getInteger ( &room->up );
					room->exits |= _ROOM_UP_BIT;
				}

				else if ( !strcmp ( text, "down" ) ) {
					retVal = getInteger ( &room->down );
					room->exits |= _ROOM_DOWN_BIT;
				}

				else if ( !strcmp ( text, "flags" ) ) {
					retVal = getInteger ( &room->flags );
				}

				else if ( !strcmp ( text, "end" ) ) {
					state = _WIP_ROOM;
				}

				else {
					registerError ( "'%s' is not a valid room property", text );
					retVal = -1;
				}
			}
		}

		break;

		case _WIP_ATPINFOBLOCK: {
	
			char *text = getIdentifier ( "atp info specification statement expected" );

			if ( text ) {
				if ( !strcmp ( text, "end" ) ) {
					state = _WIP_ROOM;
				}

				else {
					state = _WIP_ATPINFO;
					prevToken();
					retVal = processTokenList();
				}
			}
		}

		break;

		case _WIP_ATPINFO: {
		
			int type, x, y, z = 0;

			if ( retVal = getInteger ( &type ) != -1 ) 
				if ( retVal = getInteger ( &x ) != -1 ) 
					if ( retVal = getInteger ( &y ) != -1 ) {
						getInteger ( &z );

						room->addATP ( type, x, y, z );
					}

			state = _WIP_ATPINFOBLOCK;
		}

		break;

		case _WIP_OBJECTS: {

			char *text = getIdentifier ( "object specification statement expected" );

			if ( text ) {
				if ( !strcmp ( text, "object" ) ) {
					state = _WIP_OBJECT;
					parser.input = input;
					parser.index = lastGetIdx-2; // point to start of line

					retVal = processTokenList();
				}

				else if ( !strcmp ( text, "monster" ) ) {
					int level = 1, x = 0, y = 0;

					if ( retVal = getInteger ( &level ) != -1 ) {
						if ( retVal = getInteger ( &x ) != -1 ) {
							if ( retVal = getInteger ( &y ) != -1 ) {
								int start = 0;

								while ( gMonsterTable[start].level < level )
									start++;

								int end = start;

								while ( gMonsterTable[end].level == level )
									end++;

								int index = random ( start, end );

								WorldObject *super = roomMgr->findClass ( gMonsterTable[index].name );

								if ( super ) {
									WorldObject *object = new WorldObject ( super );

									object->isZoneObject = 1;
									object->addToDatabase();
									object->x = x;
									object->y = y;

									roomMgr->addObject ( object );

									BNPC *npc = (BNPC *)object->getBase ( _BNPC );
									NPC *player = makeNPC ( object );

									player->newRoom ( room );

									// enable AI
									player->aiReady = 1;
								}
							}
						}
					}
				}

				else if ( !strcmp ( text, "treasure" ) ) {
					int level = 1, x = 0, y = 0;

					if ( retVal = getInteger ( &level ) != -1 ) {
						if ( retVal = getInteger ( &x ) != -1 ) {
							if ( retVal = getInteger ( &y ) != -1 ) {
								int roll = random ( 0, 99 );

								// give money 50 percent of the time
								if ( roll < 95 ) {
									WorldObject *money = new WorldObject ( roomMgr->findClass ( "MoneyBag" ) );
									money->physicalState |= _STATE_MONEY;
									money->value = random ( level * 5, level * 15 );
									money->addToDatabase();
									money->isVisible = 1;
									money->x = x;
									money->y = y;

									room->addObject ( money );
								}

								else {
									WorldObject *chest = room->addObject ( "Chest", x, y );

									int items = random ( 1, 10 ); 

									while ( items ) {
										items -= 1;

										roll = random ( 0, 99 );

										if ( roll < 100 ) {
											WorldObject *money = new WorldObject ( roomMgr->findClass ( "MoneyBag" ) );
											money->physicalState |= _STATE_MONEY;
											money->value = random ( 1, level * 20 ) * 2;
											money->addToDatabase();
											money->isVisible = 1;

											money->forceIn ( chest );
										}
									}
								}
							}
						}
					}
				}

				else if ( !strcmp ( text, "door" ) ) {
					int x = 0, y = 0, roomNum = 0, direction = 0;

					if ( retVal = getInteger ( &x ) != -1 ) {
						if ( retVal = getInteger ( &y ) != -1 ) {
							if ( retVal = getInteger ( &roomNum ) != -1 ) {
								if ( retVal = getInteger ( &direction ) != -1 ) {
									WorldObject *door = room->addObject ( "DungeonBackDoor", x, y, directionToLoop ( direction ) );
									door->linkToRoom = roomNum;
								}
							}
						}	
					}
				}

				else if ( !strcmp ( text, "stairs" ) ) {
					int x = 0, y = 0, roomNum = 0, direction = 0, up = 0;

					if ( retVal = getInteger ( &x ) != -1 ) {
						if ( retVal = getInteger ( &y ) != -1 ) {
							if ( retVal = getInteger ( &roomNum ) != -1 ) {
								if ( retVal = getInteger ( &direction ) != -1 ) {
									if ( retVal = getInteger ( &up ) != -1 ) {
										WorldObject *door = room->addObject ( up? (char*) "DungeonBackUp" : (char*) "DungeonBackDown", x, y, directionToLoop ( direction ) );
										door->linkToRoom = roomNum;
									}
								}
							}
						}	
					}
				}

				else if ( !strcmp ( text, "end" ) ) {
					state = _WIP_ROOM;
				}

				else {
					registerError ( "'%s' is not a valid object specification statement", text );
					retVal = -1;
				}
			}
		}

		break;

		case _WIP_OBJECT: {
		
			switch ( parser.parseLine() ) {
				case -1: {
					registerError ( parser.error() );
					retVal = -1;
				}

				break;

				case 1: {
					WorldObject *object = parser.object;

					if ( object->servID == -2 ) {
						delete object;
					} else {
						if ( object->getBase ( _BNPC ) ) {
							NPC *player = makeNPC ( object );
							player->newRoom ( room );

							if ( room->picture == 3071 ) {
								LinkedElement *element = room->head();

								while ( element ) {
									RMPlayer *npc = (RMPlayer *)element->ptr();
									element = element->next();

									if ( npc != player ) {
										player->joinGroup ( npc->groupLeader? npc->groupLeader : npc );
										break;
									}
								}
							}

							// enable AI
							player->aiReady = 1;
						} else {
							WorldObject *owner = object->getOwner();

							if ( owner == object ) {
								if ( room->zone->isDungeon && object->getBase ( _BOPEN ) && (random ( 1, 100 ) <= 20) ) {
									object->physicalState |= _STATE_TRAPPED;
									object->level = random ( 1, 5 );
								}

								room->addObject ( object, 0 );
							}
						}

						// hookup links for enterable objects
						BEntry *bentry = (BEntry *)object->getBase ( _BENTRY );

						if ( bentry && bentry->startingX == 0 && object->linkTo ) {
							object->linkEntry ( object->linkTo );
							object->linkTo->linkEntry ( object );
						}
					}

					state = _WIP_OBJECTS;
				}

				break;
			}

			// update our pointers
			index = parser.index;
			numTokens = parser.numTokens;
			lastGetIdx = parser.lastGetIdx;
		}

		break;
	}

	return retVal;
}

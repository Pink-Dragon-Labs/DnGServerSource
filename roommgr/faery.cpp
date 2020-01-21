//
// faery.cpp
//
// Control logic for faeries.
//
// Author: Stephen Nichols
//

#include "roommgr.hpp"

extern int gEggTimer = 5;
extern int gDoDropEggs = 1;

WorldObject *makeEasterEgg ( WorldObject *character )
{
	char *eggs[] = {
		"OliveEasterEgg",
		"LimeEasterEgg",
		"BlueEasterEgg",
		"AzureEasterEgg",
		"BlueRedEasterEgg",
		"RedEasterEgg",
		"PinkEasterEgg",
		"HotPinkEasterEgg",
		"GoldEasterEgg",
		"VioletEasterEgg",
		"MagentaEasterEgg",
		"LightMagentaEasterEgg",
		"AquaEasterEgg",
		"TealEasterEgg",
		"GreenEasterEgg",
		"JadeEasterEgg",
		"OrangeEasterEgg",
		"RoyalEasterEgg",
		"PurpleEasterEgg",
		"BlackEasterEgg",
		"WhiteEasterEgg"
	};

	WorldObject *egg = character->room->addObject ( eggs[random ( 0, 20)], character->x, character->y );
	egg->physicalState |= _STATE_TOSS;

	char *rareItems[] = {
		"OrbOfMana",
		"OrbOfHealing",
		"OrbOfLightning",
		"MGreaterWardScroll",
		"WhiteBaldric",
		"AquaBaldric",
		"JadeBaldric",
		"VioletBaldric",
		"PinkBaldric"
	};

	char *giftItems[] = {
		"Mirror",
		"Bowl",
		"RedRose",
		"WhiteRose",
		"BlueRose",
		"Daisy",
		"Drum",
		"Flute",
		"Strynx",
		"Lyre"
		"Ring",
	};

	char *peltItems[] = {
		"WolfPelt",
		"BlackWolfPelt",
		"WhiteWolfPelt",
		"HellHoundPelt",
		"FlameWolfPelt",
		"TrollHide",
		"RockTrollHide",
		"DemonTrollHide",
		"Statue",
		"Crystal",
		"RubyChip",
		"Aquamarine",
		"Topaz",
		"GoldNugget"
	};

	int roll = random ( 0, 99 );

	if ( roll > 94 ) {
		egg->addObject ( rareItems[random ( 0, 7 )] );
	}

	else if ( roll > 75 ) {
		egg->addObject ( peltItems[random ( 0, 13 )] );
	}

	else if ( roll > 25 ) {
		egg->addObject ( giftItems[random ( 0, 10 )] );
	}

	else {
		WorldObject *money = new WorldObject ( roomMgr->findClass ( "MoneyBag" ) );
		money->physicalState = _STATE_MONEY;
		money->isVisible = 1;
		money->value = random ( 10, 100 );
		money->addToDatabase();

		money->forceIn ( egg );
	}

	return egg;
}

Faery::Faery()
{
}

Faery::~Faery()
{
}

int Faery::normalLogic ( void )
{
	int retVal = 10;

	if ( !room || character->health <= 0 )
		return -1;

	switch ( random ( 0, 6 ) ) {
		case 0:
		case 1: 
		case 2:
			if ( !groupLeader || groupLeader == this )
				gotoXY ( random ( 50, 600 ), random ( 150, 300 ) );
			break;

		case 3:
		case 4: {
			// heal someone
			LinkedElement *element = room->head();

			while ( element ) {
				RMPlayer *thePlayer = (RMPlayer *)element->ptr();
				element = element->next();

#if 0
				if ( ((thePlayer->character->health < thePlayer->character->healthMax) || thePlayer->character->hasAffect ( _AFF_POISON )) && !thePlayer->character->combatGroup && !thePlayer->isNPC ) {

					spell_t spell = NULL;
					WorldObject *obj = thePlayer->character;

					// cure poison if poisoned
					if ( obj->hasAffect ( _AFF_POISON ) ) {
						spell = spellTable[_MAGIC_LIGHT][_SPELL_UNPOISON];
					}

					else if ( obj->health < obj->healthMax ) {
						spell = spellTable[_MAGIC_LIGHT][_SPELL_HEAL];
					}

					if ( spell ) {
						PackedMsg msg;
						msg.putLong ( character->servID );
						msg.putLong ( character->room->number );

						spell ( character, obj, &msg );

						msg.putByte ( _MOVIE_END );

						roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, msg.data(), msg.size(), character->room );
					}
				}
#endif
			}

#if 0
			if ( gDoDropEggs && !random ( 0, 2 ) ) {
				// create an easter egg - hehe
				makeEasterEgg ( character );

				if ( !groupLeader || groupLeader == this )
					changeRoom ( random ( 0, 3 ) );
			}
#endif

			return 3;
		}

		break;

		case 5:
			if ( !groupLeader || groupLeader == this )
				changeRoom ( random ( 0, 3 ) );

			break;

		case 6: {
			int roll = random ( 0, 100 );

			if ( roll < room->groupChance && !groupLeader ) {
				LinkedElement *element = room->head();

				while ( element ) {
					RMPlayer *player = (RMPlayer *)element->ptr();
					element = element->next();

					if ( player->isNPC && player != this && !player->character->combatGroup && player->room == room && player->character->health > 0 && (abs ( character->level - player->character->level ) < 4) ) {
						joinGroup ( player->groupLeader? player->groupLeader : player );	
						return 5;
					}
				}
			}
		}

		break;
	}

	return retVal;
}

int Faery::spellAttack ( void )
{
	return 0;

#if 0
	if ( !random ( 0, 100 ) )
		return 0;

	spell_t spell = NULL;
	int roll = random ( 0, 100 );

	int wrathChance = 10 + ((character->opposition->size() - 1) * 5);
	int lightningChance = 20 + ((character->opposition->size() - 1) * 10) + character->level;
	int whirlwindChance = 30 + ((character->opposition->size() - 1) * 10) + character->level;

	if ( roll > wrathChance ) {
		roll = random ( 0, 100 );

		if ( roll > lightningChance ) {
			roll = random ( 0, 100 );

			if ( roll > whirlwindChance ) 
				spell = spellTable[_MAGIC_LIGHT][_SPELL_LIGHT_DART];
			else
				spell = spellTable[_MAGIC_EARTH][_SPELL_WHIRLWIND];
		} else {
			spell = spellTable[_MAGIC_EARTH][_SPELL_LIGHTNING];
		}
	} else {
		spell = spellTable[_MAGIC_LIGHT][_SPELL_WRATH];
	}

	PackedMsg msg;
	msg.putLong ( character->servID );
	msg.putLong ( character->room->number );

	spell ( character, combatTarget, &msg );

	msg.putByte ( _MOVIE_END );

	roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, msg.data(), msg.size(), character->room );
#endif

	return 1;
}

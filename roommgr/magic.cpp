//
// magic.cpp
//
// generic spell definitions
//
#include <algorithm>
#include "roommgr.hpp"
#include "callbacks.hpp"
#include "globals.hpp"

// Test
//
// Call this function to append the appropriate information to an 
// outgoing packet and output string for a spell failure.
//

void spellFailed ( int casterServID, int targetServID, PackedData *packet, char *output )
{
	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( casterServID );
	packet->putByte ( _SE_DISPEL_MAGIC );
	packet->putByte ( 0 );
	packet->putLong ( targetServID );

	strcat ( output, "Spell failed. " );
}

//
// Call this function to calculate if a spell is resisted by the target or.
// not.  If it is, show the resistance to the users.
//

int checkMagicResistance ( WorldObject *target, int casterServID, PackedData *packet, char *output )
{
	affect_t *affect = target->hasAffect ( _AFF_MAGIC_RESISTANCE );

	int resistance = affect? affect->value : 0;

	if ( !resistance )
		return 0;

	if ( random ( 0, 100 ) <= resistance ) {
		packet->putByte ( _MOVIE_SPECIAL_EFFECT );
		packet->putLong ( casterServID );
		packet->putByte ( _SE_SPELL_BLAST );
		packet->putByte ( 0 );
		packet->putByte ( 1 );
		packet->putLong ( target->servID );

		strcat ( output, "Resisted! " );

		return 1;
	}

	return 0;
}

//
// Call this function to calculate whether or not a spell is protected against.
// If it is, put the appropriate information in the outgoing packet and
// output string.
//

int checkSpellProtection ( int casterServID, int targetServID, int chance, PackedData *packet, char *output )
{
	if ( random ( 0, 100 ) <= chance ) {
		packet->putByte ( _MOVIE_SPECIAL_EFFECT );
		packet->putLong ( casterServID );
		packet->putByte ( _SE_FUMBLE );
		packet->putByte ( 0 );
		packet->putLong ( targetServID );

		strcat ( output, "Protected! " );

		return 1;
	}

	return 0;
}

// check for success...
int didSpellSucceed ( WorldObject *caster, int a, int b, int equalPct, int maxPct ) 
{
	double ratio = ((double)a) / ((double)b);
	int chance = (int)(((double)equalPct) * ratio);

	if ( chance > maxPct )
		chance = maxPct;

	// calculate a spell success chance penalty based on armor and weapons...
	int penalty = 0; 

	WorldObject *theWeapon = caster->curWeapon;

	if ( theWeapon && !theWeapon->destructing ) {
		BWeapon *bweapon = (BWeapon *)theWeapon->getBase ( _BWEAPON );

		// dagger weapons are allowed always...
		if ( bweapon->skillType != _SKILL_DAGGER ) {
			penalty += bweapon->hands;
		}
	}

	// count for the shield...
	if ( caster->curShield ) {
		penalty++;
	}

	// reduce by one if armor rating is too high...
	if ( caster->armorRating() > 50 ) {
		penalty++;
	}

	// NPCs never suffer a penalty here...
	if ( caster->player && caster->player->isNPC ) {
		penalty = 0;
	}

	// adjust the chance by the number of hands free
	double chanceAdjust = ((double)penalty) * 0.25;
	chance -= (int)(((double)chance) * chanceAdjust); 

	int roll = random ( 1, 100 );

	if ( roll <= chance ) 
		return 1;

	return 0;
}

int didSpellHit ( WorldObject *caster, WorldObject *target )
{
	if ( target->canDodge() ) {
		target->numDodges++;

		int didHit = opposedRoll ( caster->calcDexterity(), target->calcDexterity() );

		return didHit;
	}

	return 1;
}

int calcSpellDuration ( WorldObject *caster, int duration, PackedData *packet )
{
	if ( !caster->player )
		duration *= 5;

	affect_t *extensionAffect = caster->hasAffect ( _AFF_EXTENSION );

	if ( extensionAffect ) {
		duration += duration / 2;

		if ( extensionAffect->source == _AFF_SOURCE_SPELL )
			caster->delAffect ( extensionAffect, packet );
	}

	return duration;
}


int calcSpellMod ( WorldObject *caster )
{
	int intel = caster->calcIntelligence();

	int meditation = caster->getSkill ( _SKILL_MEDITATION );

	int diff = intel-12;
	int percent = 100;

	if ( diff > 0 ) {
		percent += 6 * diff;
	} else {
		percent += 10 * diff; 
	}

	if ( caster->player->isNPC ) {
		
		// Nerf monster spell damage output
		percent += 5 * meditation; 
		
	} else {

		// Standard player character output
		percent += 15 * meditation; 

	}

	return percent;
}

// Original `calcSpellMod` Code

//int calcSpellMod ( WorldObject *caster )
//{
//	int intel = caster->calcIntelligence();
//
//	int meditation = caster->getSkill ( _SKILL_MEDITATION );
//
//	int diff = intel-12;
//	int percent = 100;
//
//	if ( diff > 0 ) {
//		percent += 6 * diff;
//	} else {
//		percent += 10 * diff; 
//	}
//
//	percent += 15 * meditation; 
//
//	return percent;
//}


//
// Intended for use when creating spells for warriors to proc
//

int calcPhysicalMod ( WorldObject *caster )
{
	int strength = caster->calcStrength();

	int crit = caster->getSkill ( _SKILL_CRITICAL_STRIKING );

	int diff = strength-15; // 12
	int percent = 100;

	if ( diff > 0 ) {
		percent += 2 * diff; // 6
	} else {
		percent += 5 * diff; // 10
	}

	percent += 10 * crit; // 15

	return percent;
}

int CalcMystSpellDuration ( WorldObject *pCaster, float fBaseRounds )
{
	float fSpellMod = ((float)calcSpellMod ( pCaster )) / 100;
	fBaseRounds *= fSpellMod;

	return std::max(1, (int) fBaseRounds);
//1 >? (int) fBaseRounds;
}

//
// calculate the caster's 'spell skill - (spell skill modified by int)
//

int calcSpellSkill ( WorldObject *caster, int skillID )
{
	/* check to see if not a player, return default */
	if ( !caster->player ) {
		int ability = caster->getSkill ( skillID );
		//
		// modify this ability by the user's theurgism skill
		//
		WorldObject *owner = caster->getBaseOwner();

		if ( owner && owner != caster && !caster->getBase ( _BCONSUME ) ) {
			int percent = 20 * owner->getSkill ( _SKILL_THEURGISM );
			ability = (ability * percent) / 100; 

			percent = (caster->healthMax * 100) / caster->health;
			ability = (ability * percent) / 100;

			if ( !ability )
				ability = 1;
		}

		return ability;
	}

	int baseAbility = caster->getSkill ( skillID );
	int percent = calcSpellMod ( caster );

	int retVal = std::max(1, (baseAbility * percent) / 100);
//1 >? ( (baseAbility * percent) / 100 );

	// factor in global temporary SDM...
	if ( gTempSDM )
		retVal += (retVal * gTempSDM) / 100;

	return std::max(1, retVal);//1 >? retVal;
}

//
// generic summon macro
//

int summonMonster ( int count, char *className, char *nameTxt, int special, int targetX, int targetY, WorldObject *caster, char *output, PackedData *packet, int levelBonus)
{
	if ( !caster || !caster->opposition )
		return FALSE;

	LinkedElement *element = caster->opposition->head();
	int summonCount = 0;

	if ( element )
		element = ((WorldObject *)element->ptr())->opposition->head();

	while ( element ) {
		WorldObject *object = (WorldObject *)element->ptr();
		element = element->next();

		if ( object->summoned )
			summonCount++;
	}

	if ( summonCount >= 5 ) {		// 5
		char buf[1024];
		sprintf ( sizeof ( buf ), buf, "|c60|The spell fails because there are already five summoned monsters on %s's side.|c43| ", caster->getName() );
		strcat ( output, buf );
		return ( FALSE );
	}

	// cap count
	if ( summonCount + count > 5 ) count = 5 - summonCount;		// 5 , 5

	LinkedList monsters;

	if ( targetX < 0 )
		targetX = 0;

	else if ( targetX > 23 )
		targetX = 23;

	if ( targetY < 0 )
		targetY = 0;

	else if ( targetY > 17 )
		targetY = 17;

	int lastX = targetX, lastY = targetY;

	for ( int i=0; i<count; i++ ) {
		WorldObject *monster = new WorldObject ( roomMgr->findClass ( className ) );
		monster->addToDatabase();
		monster->minLevel+=levelBonus;
		monster->maxLevel+=levelBonus;
		monster->summoned = caster->servID;
		NPC *npc = makeNPC ( monster );
		roomMgr->addObject ( monster );

		CombatGrid grid;
		int theX = targetX, theY = targetY;

		grid.mapAccessible ( caster->combatGroup, targetX, targetY );
		grid.findClosestPoint ( lastX, lastY, caster->combatX, caster->combatY, &theX, &theY );

		monster->combatX = theX;
		monster->combatY = theY;
		lastX = theX;
		lastY = theY;
		monster->x = (monster->combatX * 26) + 13 + 5;
		monster->y = (monster->combatY * 11) + 5 + 110;

		caster->combatGroup->addCharacter ( monster, caster->combatGroup->attackers.contains ( caster )? _COMBAT_ATTACKER : _COMBAT_DEFENDER );

		npc->room = caster->room;
		npc->zone = npc->room->zone;
		monster->room = caster->room;

		monster->hidden++;
		caster->room->addPlayer ( npc, packet );
		monster->hidden--;

		monsters.add ( monster );

		// enable NPC AI
		npc->aiReady = 1;

		summonCount++;
	}

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( special );
	packet->putByte ( 1 );
	packet->putByte ( count );

	element = monsters.head();

	while ( element ) {
		WorldObject *obj = (WorldObject *)element->ptr();
		element = element->next();

		packet->putLong ( obj->servID );
	}

	char buf[1024];
	sprintf ( sizeof ( buf ), buf, "%d %s%s appear%s. ", count, nameTxt, (count > 1)? "s" : "",  (count==1)? "s" : "" );
	strcat ( output, buf );

	monsters.release();

	/* return whether or not we can summon additional monsters */
	if ( summonCount >= 5 ) return ( FALSE );
	return ( TRUE );
}

void summonMonsters ( LinkedList *classNameList, char *nameTxt, int special, int targetX, int targetY, WorldObject *caster, char *output, PackedData *packet )
{
	if ( !caster || !caster->opposition )
		return;

	LinkedElement *element = caster->opposition->head();
	int summonCount = 0;
	int toMake = ( classNameList->size() );

	if ( element )
		element = ((WorldObject *)element->ptr())->opposition->head();

	while ( element ) {
		WorldObject *object = (WorldObject *)element->ptr();
		element = element->next();

		if ( object->summoned )
			summonCount++;
	}

	if ( summonCount >= 5 ) {
		char buf[1024];
		sprintf ( sizeof ( buf ), buf, "|c60|The spell fails because there are already five summoned monsters on %s's side.|c43| ", caster->getName() );
		strcat ( output, buf );
    	return;
	}

	// cap count
	if ( summonCount + toMake > 10 ) toMake = 10 - summonCount;

	LinkedList monsters;

	if ( targetX < 0 )
		targetX = 0;

	else if ( targetX > 23 )
		targetX = 23;

	if ( targetY < 0 )
		targetY = 0;

	else if ( targetY > 17 )
		targetY = 17;

	int lastX = targetX, lastY = targetY;

	for ( int i=0; i<toMake; i++ ) {

		StringObject *monsterClassName = (StringObject *)classNameList->at( i );

		WorldObject *monster = new WorldObject ( roomMgr->findClass ( monsterClassName->data ) );
		monster->addToDatabase();
		monster->summoned = caster->servID;
		NPC *npc = makeNPC ( monster );
		roomMgr->addObject ( monster );

		CombatGrid grid;
		int theX = targetX, theY = targetY;

		grid.mapAccessible ( caster->combatGroup, targetX, targetY );
		grid.findClosestPoint ( lastX, lastY, caster->combatX, caster->combatY, &theX, &theY );

		monster->combatX = theX;
		monster->combatY = theY;
		lastX = theX;
		lastY = theY;
		monster->x = (monster->combatX * 26) + 13 + 5;
		monster->y = (monster->combatY * 11) + 5 + 110;

		caster->combatGroup->addCharacter ( monster, caster->combatGroup->attackers.contains ( caster )? _COMBAT_ATTACKER : _COMBAT_DEFENDER );

		npc->room = caster->room;
		npc->zone = npc->room->zone;
		monster->room = caster->room;

		monster->hidden++;
		caster->room->addPlayer ( npc, packet );
		monster->hidden--;

		monsters.add ( monster );

		// enable monster AI
		npc->aiReady = 1;

		summonCount++;
	}

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( special );
	packet->putByte ( 1 );
	packet->putByte ( toMake );

	element = monsters.head();

	while ( element ) {
		WorldObject *obj = (WorldObject *)element->ptr();
		element = element->next();

		packet->putLong ( obj->servID );
	}

	char buf[1024];
	sprintf ( sizeof ( buf ), buf, "%d %s%s appear%s. ", toMake, nameTxt, (toMake > 1)? "s" : "", (toMake==1)? "s" : "" );
	strcat ( output, buf );

	monsters.release();
}

//
// castHome: send the caster of the spell to his/her house
//

SPELL ( castHome )
{
	caster = caster->getBaseOwner();

	if ( !caster || !caster->player || caster->player->isNPC || caster->player->isTeleporting || caster->player->teleportRoomNum != -1 ) 
		return NULL;

	strcat ( output, "A teleportation aura envelops " );
	strcat ( output, caster->getName() );
	strcat ( output, "." );

	// start the teleport process
	teleportHouse ( caster, caster->getName() );

	return (affect_t *) _SPELL_NO_FAILURE;
}

//
// castKillStar: creates 1-5 killstar projectiles that do normal damage to
// chosen target
//

SPELL ( castKillStar )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target || !target->player ) {
		strcat ( output, "Nothing happens. " );
		return NULL;
	}

	caster = caster->getBaseOwner();

	
	int numOrbs = calcSpellSkill ( caster, _SKILL_ELEMENTALISM );
	int damage = random ( 64 * numOrbs, 124 * numOrbs ); // 24 , 48
	
	// cap the number of orbs
	//numOrbs =  std::min( 124, numOrbs);//30 <? numOrbs;

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_MULTI_BLADE );
	packet->putByte ( 1 );
	packet->putByte ( numOrbs );

	for ( int i=0; i<numOrbs; i++ ) {
		packet->putLong ( targetServID );
	}

	target->takeDamage ( WorldObject::_DAMAGE_CUT, caster, damage, output, packet, 1 );

	return NULL;
}

//
// castUnlock: attempt to unlock locked openable object
//

SPELL ( castUnlock )
{
	WorldObject *target = roomMgr->findObject ( targetServID );
	
	if ( !target )
		return NULL;

	BLockable *block = (BLockable *)target->getBase ( _BLOCK );

	int chance = 15 * calcSpellSkill ( caster, _SKILL_SORCERY );
	caster = caster->getBaseOwner();

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_UNLOCK );
	packet->putByte ( 1 );
	packet->putLong ( target->getBaseOwner()->servID );

	// handle the new ruleset...
	if ( block ) {
		int didUnlock = random ( 1, 100 ) <= chance;

		if ( didUnlock ) {
			WorldObject *key = new WorldObject;
			BKey *bkey = (BKey *)key->addBase ( _BKEY );
			bkey->unlockValue = block->unlockValue;

			char str[1024];

			if ( block->unlockValue > -1 ) {
				sprintf ( sizeof ( str ), str, "The %s unlocks.", target->getName() );
				strcat ( output, str );

				caster->unlock ( target, key, packet );
			} else {
				sprintf ( sizeof ( str ), str, "The %s resists the spell. ", target->getName() );
				strcat ( output, str );
			}

			delete key;

		} else {
			strcat ( output, "Nothing happens." );
		}
	} else {
		strcat ( output, "Nothing happens." );
	}

	return NULL;
}

//
// castDispelMagic: remove non-permanent spell effects from an object
//

SPELL ( castDispelMagic )
{
	int badAffects[] = { _AFF_FREEZE,
						 _AFF_HOLD,
						 _AFF_CONFUSION,
						 _AFF_SHACKLED,
						 _AFF_BERSERK,
						 _AFF_FEAR,
						 _AFF_LOYALTY_SHIFT,
						 _AFF_SLOW,
						 _AFF_ENFEEBLE,
						 _AFF_CURSED,
						 _AFF_NEG_DEX_MOD,
						 _AFF_NEG_END_MOD,
						 _AFF_VULNERABLE,
						 _AFF_SWITCH_GENDER,
						 _AFF_NAKED,
						 _AFF_UGLY,
						 _AFF_ENCUMBERANCE_CURSE,
						 _AFF_DEFENSELESS,
						 _AFF_MEMORY,
						 _AFF_STUN,
						 -1 };

	int goodAffects[] = { _AFF_IMPROVE_ARMOR,
						  _AFF_IMPROVE_DAMAGE,
						  _AFF_SEE_INVISIBLE,
						  _AFF_PERMANENCY,
						  _AFF_IMPROVED_INVISIBILITY,
						  _AFF_INVISIBILITY,
						  _AFF_ENCHANT_ITEM,
						  _AFF_IMMOLATION_FIRE,
						  _AFF_IMMOLATION_COLD,
						  _AFF_IMMOLATION_ACID,
						  _AFF_IMMOLATION_POISON,
						  _AFF_IMMOLATION_LIGHTNING,
						  _AFF_QUICKEN,
						  _AFF_EMPOWER,
						  _AFF_SHIELD,
						  _AFF_GREATER_SHIELD,
						  _AFF_INVULNERABLE,
						  _AFF_REGENERATION,
						  _AFF_INDESTRUCTION,
						  _AFF_MAGIC_RESISTANCE,
						  _AFF_MAGIC_IMMUNITY,
						  _AFF_IMMOLATION_RUST,
						  _AFF_RESSURECT_25,
						  _AFF_RESSURECT_50,
						  _AFF_RESSURECT_100,
						  _AFF_EXTRA_ATTACK,
						  _AFF_EXTRA_DODGE,
						  _AFF_MEMORY,
						  _AFF_POS_DEX_MOD,
						  _AFF_POS_END_MOD,
						  _AFF_POS_INT_MOD,
						  _AFF_RETENTION,
						  _AFF_NOURISHED,
						  _AFF_DISGUISED,
						  _AFF_ENCUMBERANCE_BLESSING,
						  -1 };

	int specialAffects[] = { _AFF_DAMAGE_FIRE,
							 _AFF_DAMAGE_COLD,
							 _AFF_DAMAGE_LIGHTNING,
							 _AFF_DAMAGE_POISON,
							 _AFF_DAMAGE_ACID,
							 -1 };

	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target )
		return NULL;

	int chance = 0;
	
	chance = 10 + (15 * calcSpellSkill ( caster, _SKILL_SORCERY ));

	int skill = calcSpellSkill ( caster, _SKILL_SORCERY );
	int tossedAny = 0;

	caster = caster->getBaseOwner();

	char buf[4096];

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_DISPEL_MAGIC );
	packet->putByte ( 1 );
	packet->putLong ( target->getBaseOwner()->servID );

	int doBad = 0, doGood = 0;

	if ( caster->combatGroup ) {
		if (caster->player->isGroupMember(target->player) || caster->player == target->player){
			doBad = 1;
		}
		else{
			doGood = 1;
		}
	} else {
		doGood = doBad = 1;
	}

	if ( target->activeAffects )
	{
		LinkedElement *element = target->activeAffects->head();

		while ( element ) {
			affect_t *affect = (affect_t *)element->ptr();
			element = element->next();

			if ( affect->source == _AFF_SOURCE_SPELL ) {
				int tossed = 0;

				// scan and see if it's a good spell effect if we're tossing good ones
				if ( doGood ) {
					int index = 0;

					// traverse normal 'good effects'
					while ( goodAffects[index] != -1 ) {
						if ( goodAffects[index] == affect->id ) {
							// toss it!
							if ( random ( 1, 100 ) <= chance ) {
								sprintf ( sizeof ( buf ), buf, "%s%s is no longer %s. ", target->player? "" : "The ", target->getName(), gAffectLostTbl[affect->id] );
								strcat ( output, buf );

								skill--;
								if (skill <= 0) break;
								target->delAffect ( affect, packet );
								tossedAny = 1;
								chance -= 10;
							}

							break;
						}

						index++;
					}

					index = 0;
					// traverse special effects, check to make sure it's a protection
					while ( specialAffects[index] != -1 ) {

						if ( specialAffects[index] == affect->id ) {
							// toss it if it's a protection or normal
							if ( affect->type == _AFF_TYPE_RESISTANCE || affect->type == _AFF_TYPE_NORMAL )
							{
								if ( random ( 1, 100 ) <= chance ) {

									if ( affect->type == _AFF_TYPE_NORMAL ) 
										sprintf ( sizeof ( buf ), buf, "%s%s is no longer enhanced with %s. ", target->player? "" : "The ", target->getName(), gAffectLostTbl[affect->id] );
									else
										sprintf ( sizeof ( buf ), buf, "%s%s is no longer protected from %s. ", target->player? "" : "The ", target->getName(), gAffectLostTbl[affect->id] );

									strcat ( output, buf );

									skill--;
									if (skill <= 0) break;
									target->delAffect ( affect, packet );
									tossedAny = 1;
									chance -= 10;
								}
							}
							break;
						}
						index++;
					}
				}

				if ( doBad ) {
					int index = 0;

					// traverse normal effects
					while ( badAffects[index] != -1 ) {
						if ( badAffects[index] == affect->id ) {
							// toss it!
							if ( random ( 1, 100 ) <= chance ) {
								sprintf ( sizeof ( buf ), buf, "%s%s is no longer %s. ", target->player? "" : "The ", target->getName(), gAffectLostTbl[affect->id] );
								strcat ( output, buf );

								target->delAffect ( affect, packet );

								// check for myst dispel... lose half of immunity
								switch ( affect->id ) {
									case _AFF_HOLD:
									case _AFF_FEAR:
									case _AFF_CONFUSION:
									case _AFF_BERSERK:
									case _AFF_LOYALTY_SHIFT:
									case _AFF_STUN: {
										CPlayerState *pPlayer = target->character;

										if ( pPlayer ) {
											int nMystImmunityCount = pPlayer->GetMystImmunityCount();
											pPlayer->ChangeMystImmunityCount ( -(nMystImmunityCount/2) );
										}
									}

									break;
								}

								skill--;
								if (skill <= 0) break;
								tossedAny = 1;
								chance -= 10;
							}

							break;
						}

						index++;
					}

					// traverse special effects, make sure it's a weakness
					index = 0;
					while ( specialAffects[index] != -1 ) {
						if ( specialAffects[index] == affect->id ) {
							// toss it if it's a weakness!
							if ( affect->type == _AFF_TYPE_WEAKNESS )
							{
								if ( random ( 1, 100 ) <= chance ) {
									sprintf ( sizeof ( buf ), buf, "%s%s is no longer cursed with a weakness to %s. ", target->player? "" : "The ", target->getName(), gAffectLostTbl[affect->id] );
									strcat ( output, buf );

									skill--;
									if (skill <= 0) break;
									target->delAffect ( affect, packet );
									tossedAny = 1;
									chance -= 10;
								}
							}
							break;
						}

						index++;
					}
				}
			}
		}
	}

	if ( !tossedAny )
		strcat ( output, "Nothing happens." );

	return NULL;
}

//
// castEngrave: engrave a new name on an object
//

SPELL ( castEngrave )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target )
		return NULL;

	if ( target->player ) {
		strcat ( output, "Nothing happens. " );
		return NULL;
	}

	caster = caster->getBaseOwner();

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_ENGRAVE );
	packet->putByte ( 1 );
	packet->putLong ( target->servID );

	if ( caster && caster->player && target->isOwnedBy ( caster ) && !target->isMoney() ) {

		RMPlayer *aPlayer = caster->player;

		char *name = aPlayer->engraveName;

		if ( name && !isProfane ( name ) && strlen( name ) < 33 ) {
			packet->putByte ( _MOVIE_ENGRAVE );
			packet->putLong ( caster->servID );
			packet->putLong ( target->servID );
			packet->putString ( name );

			if ( !target->hasAffect ( _AFF_ENGRAVED ) )
				target->addAffect ( _AFF_ENGRAVED, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, -1, 0, packet );

			char buf[1024];
			sprintf ( sizeof ( buf ), buf, "The %s is now named '%s'. ", target->getName(), name );
			strcat ( output, buf );
			target->setName ( name );

			if ( aPlayer ) {
				gDataMgr->logEngrave( aPlayer->getLogin(), aPlayer->getName(), name );
			}

			aPlayer->setEngraveName ( NULL );
		} else {
			strcat ( output, "Nothing happens. " );
		}
	} else {
		strcat ( output, "Nothing happens. " );
	}

	return NULL;
}

//
// castMultiBlade: creates 2-10 killstar projectiles that do normal damage to all
// opponents on combat field
//

SPELL ( castMultiBlade )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target || !target->player ) 
		return NULL;
	
	int numBlades = calcSpellSkill ( caster, _SKILL_SORCERY ) * 2;
	int damage = random ( 64 * numBlades, 124 * numBlades );

	numBlades = std::min(124, numBlades);//24 <? numBlades;

	caster = caster->getBaseOwner();

	if ( !caster || !caster->opposition )
		return NULL;

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_MULTI_BLADE );
	packet->putByte ( 1 );
	packet->putByte ( numBlades );

	LinkedElement *element = caster->opposition->head();

	if ( element ) {
		while ( element ) {
			WorldObject *object = (WorldObject *)element->ptr();
			object->scratch = 0;

			element = element->next();
		}

		element = caster->opposition->elementOf ( target );

		if ( !element )
			element = caster->opposition->head();

		int count = numBlades;

		while ( count ) {
			WorldObject *object = (WorldObject *)element->ptr();

			packet->putLong ( object->servID );
			count--;

			element = element->next();

			if ( !element )
				element = caster->opposition->head();
		}

		// handle the new ruleset...
		element = caster->opposition->head();

		while ( element ) {
			WorldObject *object = (WorldObject *)element->ptr();
			element = element->next();

			object->takeDamage ( WorldObject::_DAMAGE_CUT, caster, damage, output, packet, 1 );
		}
	}

	return NULL;
}

//
// castGatherTheFellowship: calls all group members to caster's location
//

SPELL ( castGatherTheFellowship )
{
	caster = caster->getBaseOwner();

	if ( !caster || !caster->player || caster->player->isNPC || caster->player->isTeleporting || caster->player->teleportRoomNum != -1 ) 
		return NULL;

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_GATHER_THE_FELLOWSHIP );
	packet->putByte ( 1 );
	packet->putLong ( caster->servID );

	if ( !caster->player->groupLeader ) {
		strcat ( output, "Nothing happens because you are not the leader of a group." );
	} else { 
		if ( caster->room->size() > 20 ) {
			strcat ( output, "This area is too crowded to summon more people." );
		} else {
			// step through all of the group members and summon all that can be
			LinkedElement *element = caster->player->groupLeader->group.head();
	
			strcat( output, "|c57|Group> " );

			while ( element ) {
				RMPlayer *player = (RMPlayer *)element->ptr();

				if ( player->character ) {
					WorldObject *member = player->character;
		
					if ( member && member->room != caster->room && member->health > 0 &&
						!member->combatGroup && member->player->teleportRoomNum == -1 ) 
					{
						char buf[1024];
						sprintf ( sizeof ( buf ), buf, "%s summons %s. ", caster->getName(), member->getName() );
						strcat ( output, buf );
		
						member->teleport ( caster->room->number, NULL );
					} else {
						if ( caster != member ) {
							char buf[1024];
							sprintf ( sizeof ( buf ), buf, "%s can not summon %s. ", caster->getName(), member->getName() );

							strcat ( output, buf );
						}
					}
				}
				element = element->next();
			}
		}
	}

	return NULL;
}

//
// castCornucopia: creates food and water for the caster
//

SPELL ( castCornucopia )
{
	caster = caster->getBaseOwner();

	if ( !caster || !caster->player )
		return NULL;
	
	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_CORNUCOPIA );
	packet->putByte ( 1 );
	packet->putLong ( caster->servID );

	WorldObject *food = caster->addObject ( "Bread" );
	WorldObject *drink = caster->addObject ( "WaterBottle" );
	food->makeVisible ( 1, packet );
	drink->makeVisible ( 1, packet );

	char buf[1024];
	sprintf ( sizeof ( buf ), buf, "%s conjures bread and water out of the air! ", caster->getName() );
	strcat ( output, buf );

	return NULL;
}

//
// castCloudOfFog
//

SPELL ( castCloudOfFog )
{
	caster = caster->getBaseOwner();

	switch (random (0, 7) )
	{
		case 0:
			summonMonster ( 1, "godDuach", "god Duach", _SE_SUMMON_DAEMON, targetX, targetY, caster, output, packet );
		break;

		case 1:
			summonMonster ( 1, "godElphame", "goddess Elphame", _SE_SUMMON_DAEMON, targetX, targetY, caster, output, packet );
		break;

		case 2:
			summonMonster ( 1, "GodEnid", "goddess Enid", _SE_SUMMON_PIXIE, targetX, targetY, caster, output, packet );
		break;

		case 3:
			summonMonster ( 1, "GodMabon", "god Mabon", _SE_SUMMON_PIXIE, targetX, targetY, caster, output, packet );
		break;

		case 4:
			summonMonster ( 1, "GodFinvarra", "god Finvarra", _SE_SUMMON_DAEMON, targetX, targetY, caster, output, packet );
		break;

		case 5:
			summonMonster ( 1, "GodDespothes", "god-king Despothes", _SE_SUMMON_PIXIE, targetX, targetY, caster, output, packet );
		break;

		case 6:
			summonMonster ( 1, "WorldBoss", "Xerxes", _SE_SUMMON_DAEMON, targetX, targetY, caster, output, packet );
		break;

		case 7:
			summonMonster ( 1, "WorldBoss2", "Abbadon", _SE_SUMMON_DAEMON, targetX, targetY, caster, output, packet );
		break;
	}

	return NULL;
}


//
// castImproveArmor: increases the armor rating of a particular piece of armor for
// a duration
//

SPELL ( castImproveArmor )
{
	WorldObject *target = roomMgr->findObject ( targetServID );
	int skill = calcSpellSkill (caster, _SKILL_SORCERY );

	if ( !target )
		return NULL;

	int duration = 0;

	duration = calcSpellDuration ( caster, 10 * skill, packet );

	caster = caster->getBaseOwner();

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_IMPROVE_ARMOR );
	packet->putByte ( 1 );
	packet->putLong ( target->getBaseOwner()->servID );

	affect_t *affect = NULL;

	if ( target->getBase ( _BCARRY ) && target->getBase ( _BWEAR ) ) {
		affect = target->hasAffect ( _AFF_IMPROVE_ARMOR, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL );

		if ( affect ) {
			affect->duration = duration;
		} else {
			affect = target->addAffect ( _AFF_IMPROVE_ARMOR, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, duration, 15, packet );
		}

		caster->calcAC();

		char buf[1024];
		sprintf ( sizeof ( buf ), buf, "The %s glows blue for a moment and seems stronger. ", target->basicName );
		strcat ( output, buf );
	} else {
		strcat ( output, "Nothing happens. " );
	}

	return affect;
}

//
// castTeleport: allows the caster to teleport to a specified town
//

SPELL ( castTeleport )
{
	char str[1024];

	// the target of a teleport is always the caller's character
	WorldObject *target = caller? caller->character : NULL; 

	if ( !target )
		target = roomMgr->findObject ( targetServID );

	caster = caster->getBaseOwner();

	if ( !target || !caster || !target->player || target->player->teleportRoomNum != -1 || target->player->isTeleporting )
		return NULL;

	if ( caster->player && !caster->player->isNPC && target->player != caster->player )
		return NULL;

	RMRoom *room = roomMgr->findRoom ( gHouseExits[targetX] );

	sprintf ( sizeof ( str ), str, "%s vanishes in a puff of smoke.", target->getName() );
	strcat ( output, str );

	if ( room && (room->size() > 20) ) {
		roomMgr->sendSystemMsg ( "Teleport Rerouted", target->player, "Your teleport destination was too full, so you are being rerouted to another room in the same area." ); 

		if ( room->zone ) {
			Zone *zone = room->zone;

			while ( room->size() > 20  ) {
				room = (RMRoom *)zone->rooms.at ( random ( 0, zone->rooms.size() - 1 ) );
			}
		}	
	}

	if ( !room ) 
		return NULL;
//		room = target->getRoom();

	target->teleport ( room->number, packet );

	return NULL;
}

//
// castExtension: the next spell cast by the caster has increased duration of 50%
//

SPELL ( castExtension )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target || !target->player ) {
		strcat ( output, "Nothing happens. " );
		return NULL;
	}

	caster = caster->getBaseOwner();

	// cast on self only

	if ( target->servID != caster->servID ) {
		strcat ( output, "Nothing happens. " );
		return NULL;
	}

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_EXTENSION );
	packet->putByte ( 1 );
	packet->putLong ( target->servID );

	affect_t *affect = target->hasAffect ( _AFF_EXTENSION );

	if ( affect ) {
		strcat ( output, "Nothing happens. " );
	} else {
		char str[1024];
		sprintf ( sizeof ( str ), str, "The next spell cast by %s will have an extended duration. ", target->getName() );
		strcat ( output, str );

		affect = target->addAffect ( _AFF_EXTENSION, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, -1, 0, packet );
	}

	return affect;
}

//
// castSeeInvisibility: allows the caster to see invisible objects
//

SPELL ( castSeeInvisibility )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target || !target->player ) {
		strcat ( output, "Nothing happens. " );
		return NULL;
	}

	int skill = calcSpellSkill ( caster, _SKILL_SORCERY );

	int duration = 0;

	duration = calcSpellDuration ( caster, 10 * skill, packet );

	caster = caster->getBaseOwner();

	// cast on self only

	if ( target->servID != caster->servID ) {
		strcat ( output, "Nothing happens. " );
		return NULL;
	}

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_SEE_INVISIBILITY );
	packet->putByte ( 1 );
	packet->putLong ( target->servID ); 

	affect_t *affect = target->hasAffect ( _AFF_SEE_INVISIBLE, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL );

	char str[1024];

	if ( affect ) {
		affect->duration = duration;
		sprintf ( sizeof ( str ), str, "%s's invisibility detection is renewed! ", target->getName() );
		strcat ( output, str );
	} else {
		affect = target->addAffect ( _AFF_SEE_INVISIBLE, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, duration, 0, packet );
		sprintf ( sizeof ( str ), str, "%s can now detect invisibilty! ", target->getName() );
		strcat ( output, str );
	}

	sprintf ( sizeof ( str ), str, "%s's eyes glow green for a moment. ", target->getName() );
	strcat ( output, str );

	return affect;
}

//
// castShift: causes caster to "blink" around the combat field at a random time
// during phase resolution, causing all melee attacks that fall after the blink
// to miss
//

SPELL ( castShift )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target || !target->player ) {
		strcat ( output, "Nothing happens. " );
		return NULL;
	}

	char str[1024];
	int skill = target->player->isNPC? 15 : calcSpellSkill ( caster, _SKILL_SORCERY );

	int duration = 0;

	duration = calcSpellDuration ( caster, 1 + skill, packet );

	caster = caster->getBaseOwner();

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_SHIFT );
	packet->putByte ( 1 );
	packet->putLong ( target->servID );

	affect_t *affect = target->hasAffect ( _AFF_SHIFT, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL );

	if ( affect ) {
		affect->duration = duration;
		sprintf ( sizeof ( str ), str, "%s's shift enchantment is renewed! ", target->getName() );
		strcat ( output, str );
	} else {
		affect = target->addAffect ( _AFF_SHIFT, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, duration , 0, packet );
		sprintf ( sizeof ( str ), str, "%s gains combat shift enchantment! ", target->getName() );
		strcat ( output, str );
	}

	return affect;
}

//
// castInvisibility: causes caster to vanish from normal sight, wearing off after
// a duration or when an agressive combat action is taken
//

SPELL ( castInvisibility )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target || !target->player ) {
		strcat ( output, "Nothing happens. " );
		return NULL;
	}

	int skill = calcSpellSkill ( caster, _SKILL_SORCERY );

	int duration = 0;

	duration = calcSpellDuration ( caster, 3 * skill, packet );

	caster = caster->getBaseOwner();

	// cast on self only
	if ( target->servID != caster->servID ) {
		strcat ( output, "Nothing happens. " );
		return NULL;
	}

	char str[1024];

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_INVISIBILITY );
	packet->putByte ( 1 );
	packet->putLong ( target->servID );

	affect_t *affect = target->hasAffect ( _AFF_IMPROVED_INVISIBILITY, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL );

	if ( !affect ) {

		affect_t *affect = target->hasAffect ( _AFF_INVISIBILITY, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL );

		if ( affect ) {
			affect->duration = duration;
			sprintf ( sizeof ( str ), str, "%s's invisibility enchantment is renewed! ", target->getName() );
			strcat ( output, str );
		} else {
			affect = target->addAffect ( _AFF_INVISIBILITY, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, duration , 0, packet );
			sprintf ( sizeof ( str ), str, "%s vanishes from normal vision! ", target->getName() );
			strcat ( output, str );
		}

	} else {
	 	sprintf ( sizeof ( str ), str, "Nothing happens." );
 		strcat ( output, str );
	}
	return affect;
}

//
// castCombatTeleport: allows caster to teleport unerringly to a destination
// combat grid location
//

SPELL ( castCombatTeleport )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target || !target->player || !target->combatGroup ) {
		strcat ( output, "Nothing happens. " );
		return NULL;
	}

	caster = caster->getBaseOwner();

	if ( target->player != caster->player ) {
		strcat ( output, "Nothing happens. " );
		return NULL;
	}

	// find a valid location
	CombatGrid grid;
	int theX = targetX, theY = targetY;

	// we can't place target at the wanted coordinates, so lets look for a valid
	// position from the targetX, targetY point of view
	//
	if ( target->combatGroup->positionCharacter ( target, targetX, targetY ) == -1 ) {
		theX = -1;
		theY = -1;

		grid.mapAccessible ( caster->combatGroup, targetX, targetY );
		grid.findClosestPoint ( targetX, targetY, target->combatX, target->combatY, &theX, &theY );

		// looking from the targetX, targetY out found nothing, so try from
		// the target's point of view
		if ( theX == -1 || theY == -1 ) {
			theX = target->combatX;
			theY = target->combatY;

			grid.mapAccessible ( caster->combatGroup, targetX, targetY );
			int bestDist = 10000, bestX = theX, bestY = theY, lastDist = 10000;

			while ( 1 ) {
				grid.findClosestPoint ( theX, theY, targetX, targetY, &theX, &theY );
				int dist = getDistance ( theX, theY, targetX, targetY );

				if ( dist == lastDist )
					break;

				if ( dist < bestDist ) {
					bestX = theX;
					bestY = theY;
					bestDist = dist;
				}

				lastDist = dist;
			}

			if( target->combatGroup->positionCharacter ( target, theX, theY ) == -1 )
				logInfo( _LOG_ALWAYS, "%s:%d SPELL(castCombatTeleport) - positionCharacter failed. Obj(%s of %s, servID %d) at (%d,%d), grid servID %d. Destination (%d, %d) with servID %d", __FILE__, __LINE__, target->getName(), target->classID, target->servID, target->combatX, target->combatY, target->combatGroup->grid[target->combatX][target->combatY], theX, theY,target->combatGroup->grid[theX][theY] );
		} else {
			if( target->combatGroup->positionCharacter ( target, theX, theY ) == -1 )
				logInfo( _LOG_ALWAYS, "%s:%d SPELL(castCombatTeleport) - positionCharacter failed. Obj(%s of %s, servID %d) at (%d,%d), grid servID %d. Destination (%d, %d) with servID %d", __FILE__, __LINE__, target->getName(), target->classID, target->servID, target->combatX, target->combatY, target->combatGroup->grid[target->combatX][target->combatY], theX, theY,target->combatGroup->grid[theX][theY] );
		}
	}

	if( target->combatX == theX && target->combatY == theY ) {
		packet->putByte ( _MOVIE_COMBAT_TELEPORT );
		packet->putLong ( caster->servID );
		packet->putLong ( target->servID );
		packet->putByte ( target->combatX );
		packet->putByte ( target->combatY );

		strcat ( output, target->getName() );
		strcat ( output, " teleports to another area of the battlefield! ");
	} else {
		strcat ( output, target->getName() );
		strcat ( output, "'s teleport fails. ");
	}

	return NULL;
}

//
// castTeleportGroup: teleports caster and all group members in the same room to
// a specified city
//

SPELL ( castTeleportGroup )
{
	WorldObject *target = roomMgr->findObject ( targetServID );
	caster = caster->getBaseOwner();

	if ( !target || !caster || !target->player || target->player->isNPC || 
		!caster->player || target->player != caster->player || 
		target->player->teleportRoomNum != -1 || target->player->isTeleporting ||
		caster->health < 1 )
		return NULL;

	char str[1024];

	if ( target->player->groupLeader ) {

		LinkedElement *element = target->player->groupLeader->group.head();

		while ( element ) {
			RMPlayer *player = (RMPlayer *)element->ptr();
			element = element->next();

			if ( player->character ) {
				WorldObject *member = player->character;

				if ( member && member->health > 0 && member->room == target->room &&
					!member->combatGroup && 
					member->player->teleportRoomNum == -1 ) 
				{
					sprintf ( sizeof ( str ), str, "%s vanishes in a puff of smoke. ", member->getName() );
					strcat ( output, str );
					member->teleport ( gHouseExits[targetX], packet );
				}
			}
		}
	} else {
		sprintf ( sizeof ( str ), str, "%s vanishes in a puff of smoke. ", target->getName() );
		strcat ( output, str );
		target->teleport ( gHouseExits[targetX], packet );
	}

	return NULL;
}

//
// castPermanency: allows the next spell cast on an item to become permanent
//

SPELL ( castPermanency )
{
	WorldObject *target = roomMgr->findObject ( targetServID );
	
	if (!target || target->player || target->hasAffect ( _AFF_CURSED ) || target->hasAffect( _AFF_STAFF ) ) {
		strcat ( output, "Nothing happens." );
		return NULL;
	}

	char buf[1024];
	
	caster = caster->getBaseOwner();

	// new
	if ( target->getBaseOwner()->player && caster != target->getBaseOwner() ) {
		strcat ( output, "Nothing happens." );
		return NULL;
	}

	int skill = calcSpellSkill ( caster, _SKILL_SORCERY );

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_PERMANENCY );
	packet->putByte ( 1 );
	packet->putLong ( target->getBaseOwner()->servID );

	// if the target already has permanency on it, bail out
	if ( target->hasAffect ( _AFF_PERMANENCY ) || target->hasAffect ( _AFF_ENCHANT_ITEM ) || !target->getBase ( _BCARRY ) ) {
		strcat ( output, "Nothing happens." );
	} else {
   		int spellAffects = target->countAffects ( -1, _AFF_SOURCE_WEARER );
   		spellAffects += target->countAffects (-1, _AFF_SOURCE_PERMANENT );
   
		int chance;

   		chance = skill * 10;
   		chance -= spellAffects * 35;
   
   		if ( chance > 90 )
   			chance = 90;
   
   		chance -= random ( 1, 100 );
   
   		if ( (chance < 0 && !IsThisATestServer()) || IsThisATestServer() && spellAffects > 10)  {
   			// is the item destroyed?
   			if ( !random ( 0, 3 ) ) {
   				sprintf ( sizeof ( buf ), buf, "|c60|The %s flashes white hot and disintegrates!", target->getName() );
   				strcat ( output, buf );
   
   				packet->putByte ( _MOVIE_TOSS_OBJECT );
   				packet->putLong ( caster->servID );
   				packet->putLong ( target->servID );
   
   				roomMgr->destroyObj ( target, 0, __FILE__, __LINE__ );
   			} else {
   				sprintf ( sizeof ( buf ), buf, "|c248|The %s's glows a weak blue and you feel the spell did not function properly.", target->getName() );
   				strcat ( output, buf );
   			}
   		} else {
   			// put permanency on the item
   			target->addAffect ( _AFF_PERMANENCY, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, -1, 0, packet );
   			sprintf ( sizeof ( buf ), buf, "|c12|The %s glows bright blue!  The %s is now ready to accept a permanent enchantment.", target->getName(), target->getName() );
   			strcat ( output, buf );
   		}
   	}
	return NULL;
}

//
// castRust: causes rust damage to target
//

SPELL ( castRust )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target ) 
		return NULL;

	char str[1024];
	
	int areas[] = { _WEAR_HEAD, _WEAR_CHEST, _WEAR_NECK, _WEAR_LEGS, _WEAR_FEET, _WEAR_BANDS, -1 };
	int destroyedCount = 0;
	int damagedCount = 0;

	int skill = calcSpellSkill ( caster, _SKILL_SORCERY );
	int damage = 10 * skill;

	caster = caster->getBaseOwner();

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_RUST );
	packet->putByte ( 1 );
	packet->putByte ( 1 );
	packet->putLong ( target->getBaseOwner()->servID );

	LinkedList items;

	if ( target->player ) {
		WorldObject *obj;
		int index = 0;

		while ( areas[index] != -1 )
		{
			if ( obj = target->getWornOn ( areas[index] ) ) items.add ( obj );
			index++;
		}

		if ( target->curWeapon ) 
			items.add ( target->curWeapon );
	} else {
		items.add ( target );
	}

	int didDamage = 0;

	if ( items.size() > 0 ) {
		LinkedElement *element = items.head();

		while ( element ) {
			WorldObject *item = (WorldObject *)element->ptr();
			element = element->next();

			int theDamage = damage;

			int isWeak = item->hasAffect ( _AFF_DAMAGE_RUST, _AFF_TYPE_WEAKNESS ) != NULL;
			int isProtected = (item->hasAffect ( _AFF_DAMAGE_RUST, _AFF_TYPE_RESISTANCE )) != NULL;

			if ( isWeak && !isProtected ) {
				theDamage *= 2;
			} 
			
			else if ( isProtected && !isWeak ) {
				theDamage = 0;
			}

			if ( item->changeStructureHealth ( -theDamage, packet ) ) {
				didDamage = 1;

				if ( item->health <= 0 ) {
					destroyedCount++;
					roomMgr->destroyObj ( item, 0, __FILE__, __LINE__ );
				} else {
					damagedCount++;
				}
			}
		}
	}

	items.release();

	if ( damagedCount + destroyedCount == 0 ) {
		strcat ( output, "|c14|Nothing happens." );
	}

	if ( destroyedCount > 0 ) {
		if ( destroyedCount == 1 ) 
			sprintf ( sizeof ( str ), str, "|c14|1 item destroyed! ");
		else 
			sprintf ( sizeof ( str ), str, "|c14|%d items destroyed! ", destroyedCount );

		strcat ( output, str );
	}

	if ( damagedCount > 0 ) {
		if ( damagedCount == 1 ) 
			sprintf ( sizeof ( str ), str, "|c14|1 item rusted! " );
		else 
			sprintf ( sizeof ( str ), str, "|c14|%d items rusted! ", damagedCount );

		strcat ( output, str );
	}

	return NULL;
}

//
// castDefenselessness: causes target's armor rating to halve
//

SPELL ( castDefenselessness )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target )
		return NULL;

	char str[1024];
	int skill = calcSpellSkill ( caster, _SKILL_SORCERY );

	int duration = 0;

	duration = calcSpellDuration ( caster, 5 * skill, packet );

	caster = caster->getBaseOwner();

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_DEFENSELESSNESS );
	packet->putByte ( 1 );
	packet->putLong ( target->servID );

	affect_t *affect = NULL;

	affect = target->hasAffect ( _AFF_DEFENSELESS, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL );

	if ( affect ) {
		affect->duration = duration;

		sprintf ( sizeof ( str ), str, "%s%s's defenselessness is renewed! ", target->player? "" : "The ", target->getName() );
		strcat ( output, str );
	} else {
		affect = target->addAffect ( _AFF_DEFENSELESS, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, duration , 0, packet );

		sprintf ( sizeof ( str ), str, "%s%s glows green for a moment and seems less protected! ", target->player? "" : "The ", target->getName() );
		strcat ( output, str );
	}

	return affect;
}

//
// castImprovedInvisibility: improved invisibility spell that does not wear off
// during agressive combat actions
//

SPELL ( castImprovedInvisibility )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target || !target->player ) {
		strcat ( output, "Nothing happens. " );
		return NULL;
	}

	int skill = calcSpellSkill ( caster, _SKILL_SORCERY );

	int duration = 0;

	duration = calcSpellDuration ( caster, 3 * skill, packet );

	caster = caster->getBaseOwner();

	// cast on self only
	if ( target->servID != caster->servID ) {
		strcat ( output, "Nothing happens. " );
		return NULL;
	}

	char str[1024];

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_IMPROVED_INVISIBILITY );
	packet->putByte ( 1 );
	packet->putLong ( target->servID );

	// if already invis, does nothing
	affect_t *affect = target->hasAffect ( _AFF_INVISIBILITY, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL );

	if ( !affect ) {

		affect_t *affect = target->hasAffect ( _AFF_IMPROVED_INVISIBILITY, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL );

		if ( affect ) {
			affect->duration = duration;
			sprintf ( sizeof ( str ), str, "%s's improved invisibility enchantment is renewed! ", target->getName() );
			strcat ( output, str );
		} else {
			target->addAffect ( _AFF_IMPROVED_INVISIBILITY, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, duration , 0, packet );
			sprintf ( sizeof ( str ), str, "%s vanishes from normal vision! ", target->getName() );
			strcat ( output, str );
		}

	} else {
 		strcat ( output, "Nothing happens." );
	}

	return NULL;
}

//
// castEnchantItem: allows the next spell cast on an item to become permanent and
// transfers to the wearer/wielder of the item
//

SPELL ( castEnchantItem )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target || target->player || target->hasAffect ( _AFF_CURSED ) || target->hasAffect( _AFF_STAFF ) ) {
		strcat ( output, "Nothing happens." );
		return NULL;
	}

	char buf[1024];
	caster = caster->getBaseOwner();

	if ( target->getBaseOwner()->player && caster != target->getBaseOwner() ) {
		strcat ( output, "Nothing happens." );
		return NULL;
	}

	int skill = calcSpellSkill ( caster, _SKILL_SORCERY );

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_ENCHANT_ITEM );
	packet->putByte ( 1 );
	packet->putLong ( target->getBaseOwner()->servID );

	// if the target already has enchant item on it, bail out
	if ( target->hasAffect ( _AFF_ENCHANT_ITEM ) || target->hasAffect ( _AFF_PERMANENCY ) || !target->getBase ( _BCARRY ) ){
		strcat ( output, "Nothing happens." );
	} else {
		int spellAffects = target->countAffects ( -1, _AFF_SOURCE_WEARER );
		spellAffects += target->countAffects (-1, _AFF_SOURCE_PERMANENT );

		int chance;

		chance = skill * 10;
		chance -= spellAffects * 30;

		if ( chance > 90 )
			chance = 90;

		chance -= random ( 1, 100 );

		if ( (chance < 0 && !IsThisATestServer()) || IsThisATestServer() && spellAffects > 10) {
			if ( !random ( 0, 3 ) ) {
				sprintf ( sizeof ( buf ), buf, "|c60|The %s flashes white hot and disintegrates!", target->getName() );
				strcat ( output, buf );

				packet->putByte ( _MOVIE_TOSS_OBJECT );
				packet->putLong ( caster->servID );
				packet->putLong ( target->servID );

				roomMgr->destroyObj ( target, 0, __FILE__, __LINE__ );
			} else {
				sprintf ( sizeof ( buf ), buf, "|c248|The %s glows white for a moment but you sense that the spell did not work properly.", target->getName() );
				strcat ( output, buf );
			}
		} else {
			// put permanency on the item
			target->addAffect ( _AFF_ENCHANT_ITEM, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, -1, 0, packet );
			sprintf ( sizeof ( buf ), buf, "|c12|The %s glows white!  The %s is now ready to accept a permanent enchantment.", target->getName(), target->getName() );
			strcat ( output, buf );
		}
	}
	return NULL;
}

//
// castMassRust: casts the rust spell on every opponent in combat
//

SPELL ( castMassRust )
{
	caster = caster->getBaseOwner();

	if ( !caster || !caster->opposition || !caster->opposition->size() ) {
		strcat ( output, "Nothing happens." );
		return NULL;
	}

	char buf[1024];
	char str[1024];
	int areas[] = { _WEAR_HEAD, _WEAR_CHEST, _WEAR_NECK, _WEAR_LEGS, _WEAR_FEET, _WEAR_BANDS, -1 };
	int destroyedCount = 0;
	int damagedCount = 0;
	
	LinkedList items;

	int skill = calcSpellSkill ( caster, _SKILL_SORCERY );
	int damage = 10 * skill;

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_RUST );
	packet->putByte ( 1 );
	packet->putByte ( caster->opposition->size() );

	LinkedElement *element = caster->opposition->head();

	while ( element ) {
		WorldObject *target = (WorldObject *)element->ptr();
		element = element->next();

		packet->putLong ( target->servID );

		// build target items list

		if ( target->player ) {

			WorldObject *obj;
			int index = 0;

			while ( areas[index] != -1 ) {
				if ( obj = target->getWornOn ( areas[index] ) ) items.add ( obj );
				index++;
			}

			if ( target->curWeapon ) 
				items.add ( target->curWeapon );

		} else {
			items.add ( target );
		}
	}

	// destroy or damage items

	if ( items.size() > 0 ) {
		LinkedElement *element = items.head();

		while ( element ) {
			WorldObject *item = (WorldObject *)element->ptr();
			element = element->next();

			int theDamage = damage;

			int isWeak = item->hasAffect ( _AFF_DAMAGE_RUST, _AFF_TYPE_WEAKNESS ) != NULL;
			int isProtected = (item->hasAffect ( _AFF_DAMAGE_RUST, _AFF_TYPE_RESISTANCE )) != NULL;

			if ( isWeak && !isProtected ) {
				theDamage *= 2;
			}

			else if ( isProtected && !isWeak ) {
				theDamage = 0;
			}
		
			if ( item->changeStructureHealth ( -theDamage, packet ) ) {
				if ( item->health <= 0 ) {
					destroyedCount++;
					roomMgr->destroyObj ( item, 0, __FILE__, __LINE__ );
				} else {
					damagedCount++;
				}
			}
		}
	}

	items.release();

	if ( damagedCount + destroyedCount == 0 ) {
		strcat ( output, "|c14|Nothing happens." );
	}

	if ( destroyedCount > 0 ) {
		if ( destroyedCount == 1 ) 
			sprintf ( sizeof ( str ), str, "|c14|1 item destroyed! ");
		else 
			sprintf ( sizeof ( str ), str, "|c14|%d items destroyed! ", destroyedCount );

		strcat ( output, str );
	}

	if ( damagedCount > 0 ) {
		if ( damagedCount == 1 ) 
			sprintf ( sizeof ( str ), str, "|c14|1 item rusted! ");
		else 
			sprintf ( sizeof ( str ), str, "|c14|%d items rusted! ", damagedCount );

		strcat ( output, str );
	}

	return NULL;
}

//
// castElphamesJustice
// 
//

SPELL ( castElphamesJustice ) 
{
	caster = caster->getBaseOwner();

	if ( !caster || !caster->opposition || !caster->opposition->size() )
		return NULL;

	// get affected list of friendly players

	LinkedList *friendList;
	LinkedElement *element = caster->opposition->head();

	if ( element )
	{
		friendList = ((WorldObject *)element->ptr())->opposition;

		if ( friendList->size() ) {
			strcat ( output, "|c12|The goddess Elphame removes all combat impairing effects from your party! ");

			// iterate through friend list...

			WorldObject *object;

			packet->putByte ( _MOVIE_SPECIAL_EFFECT );
			packet->putLong ( caster->servID );
			packet->putByte ( _SE_ENIDS_BLESSING );
			packet->putByte ( 1 );
			packet->putByte ( friendList->size() );

			element = friendList->head();
			while ( element )
			{
				object = (WorldObject *)element->ptr();
				packet->putLong ( object->servID );
				element = element->next();
			}

			//  now remove CC from friends
			element = friendList->head();
			while ( element )
			{
				object = (WorldObject *)element->ptr();

                // remove CC
                affect_t *affect = object->hasAffect ( _AFF_HOLD, _AFF_TYPE_NORMAL );
				if ( affect ) object->delAffect ( affect, packet );

                affect = object->hasAffect ( _AFF_CONFUSION, _AFF_TYPE_NORMAL );
				if ( affect ) object->delAffect ( affect, packet );

                affect = object->hasAffect ( _AFF_BERSERK, _AFF_TYPE_NORMAL );
				if ( affect ) object->delAffect ( affect, packet );

                affect = object->hasAffect ( _AFF_FEAR, _AFF_TYPE_NORMAL );
				if ( affect ) object->delAffect ( affect, packet );

                affect = object->hasAffect ( _AFF_FREEZE, _AFF_TYPE_NORMAL );
				if ( affect ) object->delAffect ( affect, packet );

                affect = object->hasAffect ( _AFF_STUN, _AFF_TYPE_NORMAL );
				if ( affect ) object->delAffect ( affect, packet );

				// cure Defenseless
				affect = object->hasAffect ( _AFF_DEFENSELESS, _AFF_TYPE_NORMAL );
				if ( affect ) object->delAffect ( affect, packet );

				element = element->next();
			}
		} else {
			strcat ( output, "|c12|Nothing happens. " );
		}
	}
	return NULL;
}

//
// castFIRE_GRASP
//

SPELL ( castFIRE_GRASP )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target ) {
		strcat ( output, "Nothing happens." );
		return NULL;
	}

	int skill = calcSpellSkill ( caster, _SKILL_ELEMENTALISM );

	int duration = 0;

	duration = calcSpellDuration ( caster, 3 * skill, packet );

	caster = caster->getBaseOwner();

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_FIRE_GRASP );
	packet->putByte ( 1 );
	packet->putLong ( target->servID );

	affect_t *affect = target->hasAffect ( _AFF_DAMAGE_FIRE, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL );

	if ( affect ) {
		affect->duration = duration;
	} else {
		affect = target->addAffect ( _AFF_DAMAGE_FIRE, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, duration, 15, packet );
	}

	if ( !target->player ) {
		char buf[1024];
		sprintf ( sizeof ( buf ), buf, "The %s glows red for a moment. ", target->getName() );
		strcat ( output, buf );
	} else {
		char buf[1024];
		sprintf ( sizeof ( buf ), buf, "%s's hands glow bright red!", target->getName() );
		strcat ( output, buf );
	}

	return affect;
}

//
// castFLAME_ORB
//

SPELL ( castFLAME_ORB )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target || !target->player ) {
		strcat ( output, "Nothing happens." );
		return NULL;
	}

	int numOrbs = calcSpellSkill ( caster, _SKILL_ELEMENTALISM );
	int damage = random ( 24 * numOrbs, 48 * numOrbs ); // 24 , 48

	// cap the number of orbs
	numOrbs = std::min(30, numOrbs);//30 <? numOrbs;

	int hitOrbs = 0;

	caster = caster->getBaseOwner();

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_FLAME_ORB );
	packet->putByte ( 1 );
	packet->putByte ( numOrbs );

	for ( int i=0; i<numOrbs; i++ ) {
		packet->putLong ( targetServID );
	}

	target->takeDamage ( _AFF_DAMAGE_FIRE, caster, damage, output, packet, 1 );

	return NULL;
}

//
// castARTIC_GRASP
//

SPELL ( castARTIC_GRASP )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target ) {
		strcat ( output, "Nothing happens." );
		return NULL;
	}

	int skill = calcSpellSkill ( caster, _SKILL_ELEMENTALISM );

	int duration = 0;

	duration = calcSpellDuration ( caster, 3 * skill, packet );

	caster = caster->getBaseOwner();

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_ARTIC_GRASP );
	packet->putByte ( 1 );
	packet->putLong ( target->servID );

	affect_t *affect = target->hasAffect ( _AFF_DAMAGE_COLD, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL );

	if ( affect ) {
		affect->duration = duration;
	} else {
		affect = target->addAffect ( _AFF_DAMAGE_COLD, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, duration, 15, packet );
	}

	char buf[1024];

	if ( !target->player ) {
		sprintf ( sizeof ( buf ), buf, "The %s glows blue for a moment. ", target->getName() );
		strcat ( output, buf );
	} else {
		sprintf ( sizeof ( buf ), buf, "Ice crystals begin to form on %s's hands!", target->getName() );
		strcat ( output, buf );
	}

	return affect;
}

//
// castICE_ORB
//

SPELL ( castICE_ORB )
{
	WorldObject *target = roomMgr->findObject ( targetServID );
	
	if ( !target || !target->player ) {
		strcat ( output, "Nothing happens." );
		return NULL;
	}

	int numBlades = calcSpellSkill ( caster, _SKILL_ELEMENTALISM );
	int damage = random ( 24 * numBlades, 48 * numBlades ); 

	numBlades = std::min(30, numBlades);

	caster = caster->getBaseOwner();

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_ICE_ORB );
	packet->putByte ( 1 );
	packet->putByte ( numBlades );

	for ( int i=0; i<numBlades; i++ )
		packet->putLong ( targetServID );

	target->takeDamage ( _AFF_DAMAGE_COLD, caster, damage, output, packet, 1 );

	return NULL;
}

//
// castEARTH_SPIKE
//

SPELL ( castEARTH_SPIKE )
{
	caster = caster->getBaseOwner();

	summonMonster ( 1, "SummonedEarth", "Conjured Earth Elemental", _SE_SUMMON_DAEMON, targetX, targetY, caster, output, packet );

	return NULL;
}
//
// castINCINERATE
//

SPELL ( castINCINERATE )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target ) {
		strcat ( output, "Nothing happens." );
		return NULL;
	}

	char buf[1024];
	int destroyedCount = 0, damagedCount = 0;
	int skill = calcSpellSkill ( caster, _SKILL_ELEMENTALISM );
	int damage = 10 * skill;
	WorldObject *item = NULL;

	caster = caster->getBaseOwner();

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_INCINERATE );
	packet->putByte ( 1 );
	packet->putByte ( 1 );
	packet->putLong ( target->getBaseOwner()->servID );

	LinkedList items;
	target->makeItemList ( &items, -1 );

	LinkedElement *element = items.head();

	// remove containers and fire-protected items from list
	while ( element )
	{
		item = (WorldObject *)element->ptr();
		LinkedElement *next = element->next();

		if ( item->hasAffect ( _AFF_DAMAGE_FIRE, _AFF_TYPE_RESISTANCE ) ||
			 item->getBase ( _BCONTAIN ) || item->isMoney() 
		) 
			items.delElement ( element );

		element = next;
	}

	// randomly iterate through all items and destroy a few.
	element = items.head();

	while ( element ) {
		item = (WorldObject *)element->ptr();
		element = element->next();

		// damage this item...
		if ( item->changeStructureHealth ( damage, packet ) ) {
			if ( item->health < 1 ) {
				roomMgr->destroyObj ( item, 0, __FILE__, __LINE__ );
				destroyedCount++;
			} else {
				damagedCount++;
			}
		}
	}

	items.release();

	if ( (damagedCount + destroyedCount) == 0 ) {
		strcat ( output, "Nothing happens." );
	}

	if ( destroyedCount > 0 ) {
		if ( destroyedCount == 1 ) 
			sprintf ( sizeof ( buf ), buf, "1 item incinerated! ");
		else 
			sprintf ( sizeof ( buf ), buf, "%d items incinerated! ", destroyedCount );

		strcat ( output, buf );
	}

	if ( damagedCount > 0 ) {
		sprintf ( sizeof ( buf ), buf, "%d item%s burned! ", damagedCount, (damagedCount == 1)? "" : "s" );
		strcat ( output, buf );
	}

	return NULL;
}

//
// castGUST_OF_WIND
//

SPELL ( castGUST_OF_WIND )
{
	WorldObject *target = roomMgr->findObject ( targetServID );
	caster = caster->getBaseOwner();

	if ( !target || !caster || !caster->opposition || !caster->opposition->size() )
		return NULL;

	int distance = 0;
	int damage = 0;

	caster = caster->getBaseOwner();
	int skill = calcSpellSkill ( caster, _SKILL_ELEMENTALISM );

	// set up special effect
	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_GUST_OF_WIND );
	packet->putByte ( 1 );
	packet->putLong ( target->servID );

	targetX = target->combatX;
	targetY = target->combatY;

	LinkedElement *element = caster->opposition->head();

	// assign damage and slow bit
	while ( element ) {
		WorldObject *target = (WorldObject *)element->ptr();
		element = element->next();

		// see if this guy is near the impact
		distance = getDistance ( targetX, targetY, target->combatX, target->combatY );

		// this guy should take damage
		if ( distance < 4 )	{
			damage = random ( 48 * skill, 65 * skill ); 
			target->takeDamage ( _AFF_DAMAGE_STEAL_LIFE, caster, damage, output, packet, 1 );

			if ( !target->hasAffect ( _AFF_SLOWED ) ) {
				target->addAffect ( _AFF_SLOWED, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, 2, 0, packet );
			}
		}
	}
	return NULL;
}

//
// castIMMOLATION
//

SPELL ( castIMMOLATION )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target ) {
		strcat ( output, "Nothing happens." );
		return NULL;
	}

	char buf[1024];
	int skill = calcSpellSkill ( caster, _SKILL_ELEMENTALISM );

	int duration = 0;

	duration = calcSpellDuration ( caster, 3 * skill, packet );

	caster = caster->getBaseOwner();

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_IMMOLATION );
	packet->putByte ( 1 );
	packet->putLong ( target->servID );

	affect_t *affect = target->hasAffect ( _AFF_IMMOLATION_FIRE, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL );

	if ( affect ) {
		affect->duration = duration;
	} else {
		affect = target->addAffect ( _AFF_IMMOLATION_FIRE, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, duration, skill, packet );
	}

	if ( !target->player ) {
		sprintf ( sizeof ( buf ), buf, "|c21|The %s is surrounded by magical flame for a moment. ", target->getName() );
		strcat ( output, buf );
	} else {
		sprintf ( sizeof ( buf ), buf, "%s glows red hot!", target->getName() );
		strcat ( output, buf );
	}

	return affect;
}

//
// castDANCING_FLAME
//

SPELL ( castDANCING_FLAME )
{
	caster = caster->getBaseOwner();

	summonMonster ( 1, "SummonedFire", "Conjured Fire Elemental", _SE_SUMMON_DAEMON, targetX, targetY, caster, output, packet );

	return NULL;
}

//
// castFLAME_BLADE
//

SPELL ( castFLAME_BLADE )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target || target->player ) {
		strcat ( output, "Nothing happens. " );
		return NULL;
	}

	int skill = calcSpellSkill (caster, _SKILL_ELEMENTALISM );

	int duration = 0;

	duration = calcSpellDuration ( caster, 10 * skill, packet );

	caster = caster->getBaseOwner();

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_FLAME_BLADE );
	packet->putByte ( 1 );
	packet->putLong ( target->getBaseOwner()->servID );

	affect_t *affect = NULL;

	if ( target->getBase ( _BWEAPON ) ) {
		affect = target->hasAffect ( _AFF_DAMAGE_FIRE, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL );

		if ( affect ) {
			affect->duration = duration;
		} else {
			affect = target->addAffect ( _AFF_DAMAGE_FIRE, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, duration, 15, packet );
		}

		char buf[1024];
		sprintf ( sizeof ( buf ), buf, "A shimmering red flame surrounds the %s. ", target->basicName );
		strcat ( output, buf );
	} else {
		strcat ( output, "Nothing happens. " );
	}

	return affect;
}

//
// castARTIC_CHARGE
//

SPELL ( castARTIC_CHARGE )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target || target->player ) {
		strcat ( output, "Nothing happens. " );
		return NULL;
	}

	int skill = calcSpellSkill (caster, _SKILL_ELEMENTALISM );

	int duration = 0;

	duration = calcSpellDuration ( caster, 10 * skill, packet );

	caster = caster->getBaseOwner();

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_ELECTRIC_CHARGE );
	packet->putByte ( 1 );
	packet->putLong ( target->getBaseOwner()->servID );

	affect_t *affect = NULL;

	if ( target->getBase ( _BWEAPON ) ) {
		affect = target->hasAffect ( _AFF_DAMAGE_LIGHTNING, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL );

		if ( affect ) {
			affect->duration = duration;
		} else {
			affect = target->addAffect ( _AFF_DAMAGE_LIGHTNING, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, duration, 15, packet );
		}
		char buf[1024];
		sprintf ( sizeof ( buf ), buf, "A yellow electrical field surrounds the %s! ", target->basicName );
		strcat ( output, buf );
	} else {
		strcat ( output, "Nothing happens. " );
	}

	return affect;
}

//
// castCOLD_STEEL
//

SPELL ( castCOLD_STEEL )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target || target->player ) {
		strcat ( output, "Nothing happens. " );
		return NULL;
	}

	int skill = calcSpellSkill (caster, _SKILL_ELEMENTALISM );

	int duration = 0;

	duration = calcSpellDuration ( caster, 10 * skill, packet );

	caster = caster->getBaseOwner();

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_COLD_STEEL );
	packet->putByte ( 1 );
	packet->putLong ( target->getBaseOwner()->servID );

	affect_t *affect = NULL;

	if ( target->getBase ( _BWEAPON ) ) {
		affect = target->hasAffect ( _AFF_DAMAGE_COLD, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL  );

		if ( affect ) {
			affect->duration = duration;
		} else {
			affect = target->addAffect ( _AFF_DAMAGE_COLD, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, duration, 15, packet );
		}
			
		char buf[1024];
		sprintf ( sizeof ( buf ), buf, "Ice crystals begin to form on the %s!. ", target->basicName );
		strcat ( output, buf );
	} else {
		strcat ( output, "Nothing happens. " );
	}

	return affect;
}

//
// castSAND_STORM
//

SPELL ( castSAND_STORM )
{
	WorldObject *target = roomMgr->findObject ( targetServID );
	caster = caster->getBaseOwner();

	if ( !target || !caster || !caster->opposition || !caster->opposition->size() )
		return NULL;

	int distance = 0;
	int damage = 0;

	caster = caster->getBaseOwner();
	int skill = calcSpellSkill ( caster, _SKILL_ELEMENTALISM );

	// set up special effect
	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_SAND_STORM );
	packet->putByte ( 1 );
	packet->putLong ( target->servID );

	targetX = target->combatX;
	targetY = target->combatY;

	LinkedElement *element = caster->opposition->head();

	// assign damage only 
	while ( element ) {
		WorldObject *target = (WorldObject *)element->ptr();
		element = element->next();

		// see if this guy is near the impact
		distance = getDistance ( targetX, targetY, target->combatX, target->combatY );

		// this guy should take damage
		if ( distance < 4 )	{
			damage = random ( 48 * skill, 65 * skill ); // 12 , 40
			target->takeDamage ( _AFF_DAMAGE_STEAL_LIFE, caster, damage, output, packet, 1 );
			// add armor damage ?
		}
	}
	return NULL;
}

//
// castSPARK
//

SPELL ( castSPARK )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target || !target->player ) {
		strcat ( output, "Nothing happens." );
		return NULL;
	}

	int skill = calcSpellSkill ( caster, _SKILL_ELEMENTALISM );
	int damage = random ( skill * 10, skill * 25 ); // 10 * skill
	int distance = getDistance ( caster->combatX, caster->combatY, target->combatX, target->combatY );

	caster = caster->getBaseOwner();

	if ( distance < gTouchDistance ) {
		packet->putByte ( _MOVIE_SPECIAL_EFFECT );
		packet->putLong ( caster->servID );
		packet->putByte ( _SE_SPARK );
		packet->putByte ( 1 );
		packet->putLong ( target->servID );

		target->takeDamage ( _AFF_DAMAGE_LIGHTNING, caster, damage, output, packet, 1 );
	} else {

		char buf[1024];
		sprintf ( sizeof ( buf ), buf, "The spell fails because %s is not close enough to touch! ", target->getName() );
		strcat ( output, buf );
	}

	return NULL;
}

//
// castICE_STORM
//

SPELL ( castICE_STORM )
{
	WorldObject *target = roomMgr->findObject ( targetServID );
	caster = caster->getBaseOwner();

	if ( !target || !caster->combatGroup )
		return NULL;

	int distance = 0;
	int skill = calcSpellSkill ( caster, _SKILL_ELEMENTALISM );

	// set up special effect
	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_ICE_STORM );
	packet->putByte ( 1 );
	packet->putByte ( targetX );
	packet->putByte ( targetY );

	// do some damage
	LinkedElement *element = caster->combatGroup->combatants.head();

	while ( element )
	{
		WorldObject *object = (WorldObject *)element->ptr();
		element = element->next();

		// see if this guy is near the impact
		distance = getDistance ( targetX, targetY, object->combatX, object->combatY );

		// this guy should take damage
		if ( distance < 3 )	{
			int damage = random ( skill * 24, skill * 32 );
			object->takeDamage ( _AFF_DAMAGE_COLD, caster, damage, output, packet, 1 );
		}
	}
	return NULL;
}

//
// castFREEZING_WIND
//

SPELL ( castFREEZING_WIND )
{
	caster = caster->getBaseOwner();

	summonMonster ( 1, "SummonedWater", "Conjured Water Elemental", _SE_SUMMON_DAEMON, targetX, targetY, caster, output, packet );

	return NULL;
}

//
// castHURRICANE
//

SPELL ( castHURRICANE )
{
	caster = caster->getBaseOwner();

	summonMonster ( 1, "SummonedAir", "Conjured Air Elemental", _SE_SUMMON_DAEMON, targetX, targetY, caster, output, packet );

	return NULL;
}

//
// castCRAWLING_CHARGE
//

SPELL ( castCRAWLING_CHARGE )
{
	//This spell will affect the area surrounding the caster, but will not harm the caster.
	//It affects an area from 5 left to 5 right of the caster, and from 4 above to 4 below.
	//It does damage approximately equally to every combatant (friend or foe) in that area.
	//
	//If there are not at least 5 combatants that it hurts, it generates special effects
	//in random unoccupied squares, for effect.
	//

	//heres a list of player targets
	LinkedList targetList;

	short casterX = caster->combatX;
	short casterY = caster->combatY;

	caster = caster->getBaseOwner();
	char skill = caster->getSkill( _SKILL_ELEMENTALISM );
	char intel = caster->intelligence;

	for(short combatX = casterX - 5; combatX <= casterX + 5; ++combatX)
	{
		if( combatX < 0 ) continue;
		else if( combatX >= _COMBAT_GRID_WIDTH ) continue;

		for(short combatY = casterY - 4; combatY <= casterY + 4; ++combatY)
		{
			if (combatY < 0) continue;
			else if( combatY >= _COMBAT_GRID_HEIGHT ) continue;
			if( combatX == casterX && combatY == casterY ) continue;
            
			//get the servID of the object in this square
			unsigned int servID = caster->combatGroup->grid[combatX][combatY];

			if( servID <= 1 ) continue;
			if( servID == caster->servID ) continue;

			//add it to our target list if there is a valid servID
			if( servID ) {
				WorldObject* obj = roomMgr->getObject( servID );
				if( obj && obj->character )
					targetList.add( obj );
				else logDisplay( "%s:%d castAREA_LIGHTNING - target error. servid %d, obj %d  obj->character %d", __FILE__, __LINE__, servID, (int)obj, obj?(int)obj->character:0 );
			} 
		}
	}

	char numTargets = targetList.size();

	unsigned short totalDamage = skill * intel * 7;
	unsigned short damageEach = totalDamage / (numTargets?numTargets:1);
	LinkedElement* element = targetList.head();

	while( element ) {
		WorldObject* obj = (WorldObject*)element->ptr();
		element = element->next();

		///unsigned short damage = random((int)(damageEach * .9), (int)(damageEach * 1.1)); Old - Zach
		unsigned short damage = random((int)(damageEach * .9), (int)(damageEach * 1.1));	// 7 , 9
		if( obj ) {
			packet->putByte ( _MOVIE_SPECIAL_EFFECT );
			packet->putLong ( caster->servID );
			packet->putByte ( _SE_LIGHTNING );
			packet->putByte ( 0 );
			packet->putLong ( obj->servID );

			obj->takeDamage( _AFF_DAMAGE_LIGHTNING, caster, damage, output, packet, 1 );
		}
	}

	short numEffects = 5;
	numEffects -= numTargets;
	
	targetList.release(); //this protects from deleting the objects pointed to by the list!

	char effectX;
	char effectY;

	while( numEffects > 0 ) {
		effectX = random( std::max(0, targetX -4), std::min(targetX +4, _COMBAT_GRID_WIDTH) );
		effectY = random( std::max(0, targetY -4), std::min(targetY +4, _COMBAT_GRID_HEIGHT) );
		//effectX = random( 0 >? casterX - 5, casterX + 5 <? _COMBAT_GRID_WIDTH );
		//effectY = random( 0 >? casterY - 4, casterY + 4 <? _COMBAT_GRID_HEIGHT );

		unsigned int servID = caster->combatGroup->grid[effectX][effectY];
			
		if( servID == 0) {
			packet->putByte ( _MOVIE_SPECIAL_EFFECT );
			packet->putLong ( caster->servID );
			packet->putByte ( _SE_LIGHTNING_BOLT );
			packet->putByte ( 0x00 );
			packet->putByte ( effectX );
			packet->putByte ( effectY );

			numEffects--;
		}
	}

	return NULL;
}

//
// castSTONING
//

SPELL ( castSTONING )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target )
		return NULL;

	int distance;
	int skill = calcSpellSkill ( caster, _SKILL_ELEMENTALISM );

	caster = caster->getBaseOwner();

	if ( !caster || !caster->combatGroup )
		return NULL;

	// set up special effect
	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_STONING );
	packet->putByte ( 1 );
	packet->putByte ( targetX );
	packet->putByte ( targetY );

	// do some damage
	LinkedElement *element = caster->combatGroup->combatants.head();

	while ( element )
	{
		WorldObject *object = (WorldObject *)element->ptr();
		element = element->next();

		// see if this guy is near the impact
		distance = getDistance ( targetX, targetY, object->combatX, object->combatY );

		if ( distance < 3 )
		{
			// this guy should take damage
			int damage = random ( 25*skill, 50*skill ); // 25 , 50
			object->takeDamage ( _AFF_DAMAGE_STUN, caster, damage, output, packet, 1 );
		}
	}
	return NULL;
}

//
// castFIREBALL
//

SPELL ( castFIREBALL )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target ) 
		return NULL;
	
	int distance;
	int skill = calcSpellSkill ( caster, _SKILL_ELEMENTALISM );
	
	caster = caster->getBaseOwner();

	if ( !caster || !caster->combatGroup )
		return NULL;

	// set up special effect
	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_FIREBALL );
	packet->putByte ( 1 );
	packet->putByte ( targetX );
	packet->putByte ( targetY );

	// do some damage
	LinkedElement *element = caster->combatGroup->combatants.head();

	while ( element )
	{
		WorldObject *object = (WorldObject *)element->ptr();
		element = element->next();

		// see if this guy is near the impact
		distance = getDistance ( targetX, targetY, object->combatX, object->combatY );

		if ( distance < 5 ) {
			// check for resistance...
			CPlayerState *pPlayerState = object->character;

			if ( pPlayerState && pPlayerState->TestSpellResistance ( _SKILL_ELEMENTALISM - _SKILL_SORCERY ) ) {
				packet->putByte ( _MOVIE_SPECIAL_EFFECT );
				packet->putLong ( caster->servID );
				packet->putByte ( _SE_SPELL_BLAST );
				packet->putByte ( 0 );
				packet->putByte ( 1 );
				packet->putLong ( object->servID );
			} else {
				// this guy should take damage
				int damage = random ( 20*skill, 35*skill );	// 20 , 35
				object->takeDamage ( _AFF_DAMAGE_FIRE, caster, damage, output, packet, 1 );
			}
		}
	}

	return NULL;
}

//
// castLIGHTNING_BOLT
//

SPELL ( castLIGHTNING_BOLT )
{
	WorldObject *target = roomMgr->findObject ( targetServID );
	
	if ( !target || !target->player ) {
		strcat ( output, "Nothing happens." );
		return NULL;
	}

	int skill = calcSpellSkill ( caster, _SKILL_ELEMENTALISM );
	caster = caster->getBaseOwner();

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_LIGHTNING );
	packet->putByte ( 1 );
	packet->putLong ( target->servID );

	int damage = random ( 18 * skill, 38 * skill ); // 18 , 38
	target->takeDamage ( _AFF_DAMAGE_LIGHTNING, caster, damage, output, packet, 1 );

	return NULL;
}

//
// castFREEZE
//

SPELL ( castFREEZE )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target || !target->player ) {
		strcat ( output, "Nothing happens." );
		return NULL;
	}

	char buf[1024];
	int skill = calcSpellSkill ( caster, _SKILL_ELEMENTALISM );
	int chance = 20 + (skill * 10);

	int duration = 0;

	duration = calcSpellDuration ( caster, skill + 1, packet );

	caster = caster->getBaseOwner();

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_FREEZE );
	packet->putByte ( 1 );
	packet->putByte ( 1 );
	packet->putLong ( targetServID );

	affect_t *affect = target->hasAffect ( _AFF_FREEZE );

	if ( affect ) {
		if ( random (0, 100) < chance )
		{
			sprintf ( sizeof ( buf ), buf, "|c20|%s's freeze hold on %s is renewed. ", caster->getName(), target->getName() );
			strcat ( output, buf );
			affect->duration = duration;
		} else {
			sprintf (sizeof ( buf ), buf, "|c20|%s shrugs off the spell. ", target->getName() );
			strcat ( output, buf );
		}
	} else {
		// check random failure
		if ( random (0, 100) < chance )
		{
			target->addAffect ( _AFF_FREEZE, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, duration, caster->calcIntelligence(), packet );

			sprintf ( sizeof ( buf ), buf, "|c20|%s is magically frozen in place! ", target->getName() );
			strcat ( output, buf );
		}
		else
		{
			sprintf (sizeof ( buf ), buf, "|c20|%s shrugs off the spell. ", target->getName() );
			strcat ( output, buf );
		}
	}

	return NULL;
}

//
// castCRUSHING_BOULDER
//

SPELL ( castCRUSHING_BOULDER )
{
	WorldObject *target = roomMgr->findObject ( targetServID );
	caster = caster->getBaseOwner();

	if ( !target || !target->player ) {
		strcat ( output, "Nothing happens." );
		return NULL;
	}

	int skill = calcSpellSkill ( caster, _SKILL_ELEMENTALISM );

	int damage = random ( skill * 28, skill * 35 ); // 28 , 35

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_CRUSHING_BOULDER );
	packet->putByte ( 1 );
	packet->putLong ( target->servID );

	target->takeDamage ( _AFF_DAMAGE_STUN, caster, damage, output, packet, 1 );

	return NULL;
}

//
// castELECTRIC_FURY
//

SPELL ( castELECTRIC_FURY )
{
	caster = caster->getBaseOwner();

	if ( !caster || !caster->opposition || !caster->opposition->size() )
		return NULL;

	int skill = calcSpellSkill ( caster, _SKILL_ELEMENTALISM );

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_ELECTRIC_FURY );
	packet->putByte ( 1 );
	packet->putByte ( caster->opposition->size() );

	LinkedElement *element = caster->opposition->head();

	// build effect
	while ( element )
	{
		WorldObject *target = (WorldObject *)element->ptr();
		element = element->next();

		packet->putLong ( target->servID );
	}

	// handle new electric fury...
	element = caster->opposition->head();

	while ( element ) {
		WorldObject *target = (WorldObject *)element->ptr();
		element = element->next();

		CPlayerState *pPlayerState = target->character;
		int nResisted = 0;

		if ( pPlayerState ) {
			if ( pPlayerState->TestSpellResistance ( _SKILL_ELEMENTALISM - _SKILL_SORCERY ) ) {
				nResisted = 1;

				packet->putByte ( _MOVIE_SPECIAL_EFFECT );
				packet->putLong ( caster->servID );
				packet->putByte ( _SE_SPELL_BLAST );
				packet->putByte ( 0 );
				packet->putByte ( 1 );
				packet->putLong ( target->servID );
			}
		}

		if ( !nResisted ) {
			int damage = random ( skill * 28, skill * 35 ); // 28 , 35
			target->takeDamage ( _AFF_DAMAGE_LIGHTNING, caster, damage, output, packet, 1 );
		}
	}

	return NULL;
}

//
// castCOLD_SNAP
//

SPELL ( castCOLD_SNAP )
{
	caster = caster->getBaseOwner();

	if ( !caster || !caster->opposition || !caster->opposition->size() )
		return NULL;

	int skill = calcSpellSkill ( caster, _SKILL_ELEMENTALISM );

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_COLD_SNAP );
	packet->putByte ( 1 );
	packet->putByte ( caster->opposition->size() );

	LinkedElement *element = caster->opposition->head();

	// build effect
	while ( element )
	{
		WorldObject *target = (WorldObject *)element->ptr();
		element = element->next();

		packet->putLong ( target->servID );
	}

	// handle new cold snap...
	element = caster->opposition->head();

	while ( element ) {
		WorldObject *target = (WorldObject *)element->ptr();
		element = element->next();

		CPlayerState *pPlayerState = target->character;
		int nResisted = 0;

		if ( pPlayerState ) {
			if ( pPlayerState->TestSpellResistance ( _SKILL_ELEMENTALISM - _SKILL_SORCERY ) ) {
				nResisted = 1;

				packet->putByte ( _MOVIE_SPECIAL_EFFECT );
				packet->putLong ( caster->servID );
				packet->putByte ( _SE_SPELL_BLAST );
				packet->putByte ( 0 );
				packet->putByte ( 1 );
				packet->putLong ( target->servID );
			}
		}

		if ( !nResisted ) {
			int damage = random ( skill * 28, skill * 35 ); // 28 , 35
			target->takeDamage ( _AFF_DAMAGE_COLD, caster, damage, output, packet, 1 );
		}

		// add slow bit if not protected
			if ( !target->hasAffect ( _AFF_DAMAGE_COLD, _AFF_TYPE_RESISTANCE ) && !target->hasAffect ( _AFF_SLOWED ) ) {
				target->addAffect ( _AFF_SLOWED, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, 2, 0, packet );
			}
	}

	return NULL;
}

//
// castEARTHQUAKE
//

SPELL ( castEARTHQUAKE )
{
	caster = caster->getBaseOwner();
 
	if ( !caster || !caster->opposition || !caster->opposition->size() )
		return NULL;

	LinkedList *list = caster->opposition;

	int skill = calcSpellSkill ( caster, _SKILL_ELEMENTALISM );

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_EARTHQUAKE );
	packet->putByte ( 0 );
	packet->putLong ( caster->servID );

	LinkedElement *element = list->head();

	while ( element ) {
		WorldObject *object = NULL;
		object = (WorldObject *)element->ptr();
		element = element->next();

		CPlayerState *pPlayerState = object->character;

		if ( pPlayerState && pPlayerState->TestSpellResistance ( _SKILL_ELEMENTALISM - _SKILL_SORCERY ) ) {
			packet->putByte ( _MOVIE_SPECIAL_EFFECT );
			packet->putLong ( caster->servID );
			packet->putByte ( _SE_SPELL_BLAST );
			packet->putByte ( 0 );
			packet->putByte ( 1 );
			packet->putLong ( object->servID );

			int damage = random ( skill * 20, skill * 30 ); // 18 , 28
			object->takeDamage ( WorldObject::_DAMAGE_EARTHQUAKE, caster, damage, output, packet, 1 );
		} else {
			int damage = random ( skill * 21, skill * 34 ); // 21 , 34
			object->takeDamage ( WorldObject::_DAMAGE_EARTHQUAKE, caster, damage, output, packet, 1 );
		}
	}

	return NULL;
}

//
// castDESPOTHES_WRATH
//

SPELL ( castDESPOTHES_WRATH )
 {
 	WorldObject *target = roomMgr->findObject ( targetServID );
 	
 	if ( !target || !target->player ) {
 		strcat ( output, "Nothing happens." );
 		return NULL;
 	}
 
 	int skill = calcSpellSkill ( caster, _SKILL_ELEMENTALISM );
 
 	char buf[1024];
 	sprintf ( sizeof ( buf ), buf, "|c20|Despothes aids %s, inflicting massive damage!!", target->getName());
 	strcat ( output, buf );
 
 	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
 	packet->putLong ( caster->servID );
 	packet->putByte ( _SE_ELECTRIC_FURY );
 	packet->putByte ( 1 );
 	packet->putByte ( caster->opposition->size() );
 
 	LinkedElement *element = caster->opposition->head();
 
 	// build effect
 	while ( element )
 	{
 		WorldObject *target = (WorldObject *)element->ptr();
 		element = element->next();
 
 		packet->putLong ( target->servID );
 	}
 
 	// handle new electric fury...
 	element = caster->opposition->head();
 
 	while ( element ) {
 		WorldObject *target = (WorldObject *)element->ptr();
 		element = element->next();
 
 		CPlayerState *pPlayerState = target->character;
 		int nResisted = 0;
 
 		if ( pPlayerState ) {
 			if ( pPlayerState->TestSpellResistance ( _SKILL_ELEMENTALISM - _SKILL_SORCERY ) ) {
 				nResisted = 1;
 
 				packet->putByte ( _MOVIE_SPECIAL_EFFECT );
 				packet->putLong ( caster->servID );
 				packet->putByte ( _SE_SPELL_BLAST );
 				packet->putByte ( 0 );
 				packet->putByte ( 1 );
 				packet->putLong ( target->servID );
 			}
 		}
 
 		if ( !nResisted ) {
			//int damage = random ( skill * 45, skill * 65 );
 			int damage = random ( skill * 80, skill * 105 ); // 75 , 95
 			target->takeDamage ( _AFF_DAMAGE_LIGHTNING, caster, damage, output, packet, 1 );
 		}
 	}
 
 	return NULL;
 }

//
// castHOLD_MONSTER
//

SPELL ( castHOLD_MONSTER )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target || !target->getBase ( _BNPC ) ) {
		strcat ( output, "Nothing happens." );
		return NULL;
	}

	char buf[1024];
	int skill = calcSpellSkill ( caster, _SKILL_MYSTICISM );

	int duration = CalcMystSpellDuration ( caster, 3.0 );
	duration = calcSpellDuration ( caster, duration, packet );

	// halve duration if has free will
//	if ( target->hasAffect ( _AFF_FREE_WILL, _AFF_TYPE_NORMAL ) )
//		duration /= 2;

	caster = caster->getBaseOwner();

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_HOLD_MONSTER );
	packet->putByte ( 1 );
	packet->putByte ( 1 );
	packet->putLong ( targetServID );

	// get the target's player state
	CPlayerState *pPlayerState = target->character;
	int nMystImmunity = 0;

	if ( pPlayerState ) {
		nMystImmunity = pPlayerState->GetMystImmunityCount();
	}

	affect_t *affect = target->hasAffect ( _AFF_HOLD );

	if ( nMystImmunity ) {
		sprintf ( sizeof ( buf ), buf, "|c22|Nothing happens, immune for %d round(s)! ", nMystImmunity );
		strcat ( output, buf );
	}

	else if ( affect ) {
		strcat ( output, "|c22|Nothing happens, already held! " );
	} 

	else {
		// increase the myst immunity...
		if ( pPlayerState ) {
			pPlayerState->ChangeMystImmunityCount ( duration * 2 );
		}

		target->addAffect ( _AFF_HOLD, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, duration, caster->calcIntelligence(), packet );

		sprintf ( sizeof ( buf ), buf, "|c22|%s held for %d rounds! ", target->getName(), duration );
		strcat ( output, buf );
	}

	return NULL;
}

//
// castFUMBLE
//

SPELL ( castFUMBLE )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target || !target->player ) {
		strcat ( output, "Nothing happens." );
		return NULL;
	}

	caster = caster->getBaseOwner();

	char buf[1024];
	int success = 0;

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_FUMBLE );
	packet->putByte ( 1 );
	packet->putLong ( targetServID );

	WorldObject *targetObj = target->curWeapon;

	affect_t *hasAffect = target->hasAffect ( _AFF_RETENTION, _AFF_TYPE_RESISTANCE ); 

	if ( targetObj && !hasAffect ) {

		BWeapon *bWeapon = (BWeapon *)targetObj->getBase ( _BWEAPON );

		if ( bWeapon ) {

			Weapon weapon = *(Weapon *)bWeapon;

			int skill = calcSpellSkill ( caster, _SKILL_MYSTICISM );
			int chance = skill * 10;

			int weaponSkill = target->getSkill ( weapon.skillType ) * 20;
	   
	   		if ( chance > 90 )
	   			chance = 90;

			if ( random (0, 100) > 50 )	{
				if ( opposedRoll ( chance, weaponSkill ) ) {
					if ( target->takeOff ( targetObj, packet ) == _WO_ACTION_HANDLED ) 
    					success = 1;
				}
			}
		}
	}

	if ( success ) {
		sprintf ( sizeof ( buf ), buf, "|c22|%s loses %s grip on the %s!", target->getName(), target->getPronoun ( _PRONOUN_HIS ), targetObj->getName() );
		strcat ( output, buf );
	} else 
		strcat ( output, "|c22|Nothing happens." );

	return NULL;
}

//
// castPSYCHIC_ORB
//

SPELL ( castPSYCHIC_ORB )
{
	WorldObject *target = roomMgr->findObject ( targetServID );
	
	if ( !target || !target->player ) {
		strcat ( output, "Nothing happens." );
		return NULL;
	}

	int numBlades = calcSpellSkill ( caster, _SKILL_MYSTICISM );
	int damage = random ( 24 * numBlades, 48 * numBlades ); // 24 , 48

	numBlades = std::min(30, numBlades);//30 <? numBlades;

	caster = caster->getBaseOwner();

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_PSYCHIC_ORB );
	packet->putByte ( 1 );
	packet->putByte ( numBlades );

	for ( int i=0; i<numBlades; i++ )
		packet->putLong ( targetServID );

	target->takeDamage ( WorldObject::_DAMAGE_PSYCHIC, caster, damage, output, packet, 1 );

	return NULL;
}

//
// castCONFUSION
//

SPELL ( castCONFUSION )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target || !target->player ) {
		strcat ( output, "Nothing happens." );
		return NULL;
	}

	char buf[1024];
	int skill = calcSpellSkill ( caster, _SKILL_MYSTICISM );

	int duration = CalcMystSpellDuration ( caster, 6.0 );
	duration = calcSpellDuration ( caster, duration, packet );

	// halve duration if has free will
//	if ( target->hasAffect ( _AFF_FREE_WILL, _AFF_TYPE_NORMAL ) )
//		duration /= 2;

	caster = caster->getBaseOwner();

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_CONFUSION );
	packet->putByte ( 1 );
	packet->putByte ( 1 );
	packet->putLong ( targetServID );

	// get the target's player state
	CPlayerState *pPlayerState = target->character;
	int nMystImmunity = 0;

	if ( pPlayerState ) {
		nMystImmunity = pPlayerState->GetMystImmunityCount();
	}

	affect_t *affect = target->hasAffect ( _AFF_CONFUSION );

	if ( nMystImmunity ) {
		sprintf ( sizeof ( buf ), buf, "|c22|Nothing happens, immune for %d round(s)! ", nMystImmunity );
		strcat ( output, buf );
	}

	else if ( affect ) {
		strcat ( output, "|c22|Nothing happens, already confused! " );
	} 

	else {
		// increase the myst immunity...
		if ( pPlayerState ) {
			pPlayerState->ChangeMystImmunityCount ( duration * 2 );
		}

		target->addAffect ( _AFF_CONFUSION, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, duration, caster->calcIntelligence(), packet );

		sprintf ( sizeof ( buf ), buf, "|c22|%s confused for %d rounds! ", target->getName(), duration );
		strcat ( output, buf );
	}

	return NULL;
}

//
// castFORGET
//

SPELL ( castFORGET )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target )
		return NULL;

	char buf[1024];
	int skill = calcSpellSkill ( caster, _SKILL_MYSTICISM );

	int duration = 0;

	duration = calcSpellDuration ( caster, 7 * skill, packet );

	caster = caster->getBaseOwner();

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_HOLD_MONSTER );
	packet->putByte ( 1 );
	packet->putLong ( target->servID );

	affect_t *affect = target->hasAffect ( _AFF_POS_INT_MOD, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL );

	if ( affect ) {
		affect->duration = duration;
	} else {
		affect = target->addAffect ( _AFF_POS_INT_MOD, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, duration, 15, packet );
	}

	if ( !target->player ) {
		sprintf ( sizeof ( buf ), buf, "|c21|The %s is surrounded by a mystical aura! ", target->getName() );
		strcat ( output, buf );
	} else {
		sprintf ( sizeof ( buf ), buf, "%s has been blessed with Intelligence! ", target->getName() );
		strcat ( output, buf );
	}	
	return affect;
}

//
// castMIND_SHACKLE
//

SPELL ( castMIND_SHACKLE )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target || !target->player ) {
		strcat ( output, "Nothing happens." );
		return NULL;
	}

	char buf[1024];
	int skill = calcSpellSkill ( caster, _SKILL_MYSTICISM );

	int duration = CalcMystSpellDuration ( caster, 7.0 );
	duration = calcSpellDuration ( caster, duration, packet );

	caster = caster->getBaseOwner();

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_MIND_SHACKLE );
	packet->putByte ( 1 );
	packet->putByte ( 1 );
	packet->putLong ( targetServID );

	affect_t *affect = target->hasAffect ( _AFF_SHACKLED );

	if ( affect ) {
		strcat ( output, "|c22|Nothing happens, already shackled! " );
	} else {
		target->addAffect ( _AFF_SHACKLED, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, duration, caster->calcIntelligence(), packet );
		sprintf ( sizeof ( buf ), buf, "|c22|%s shackled %d round(s)! ", target->getName(), duration );
		strcat ( output, buf );
	}

	return NULL;
}

//
// castIDENTIFY
//

SPELL ( castIDENTIFY )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	caster = caster->getBaseOwner();

	if ( !target || !caster )
		return NULL;

	if ( !target->getBase ( _BCARRY ) || target->hasAffect ( _AFF_IDENTIFIED ) ) {
		strcat ( output, "Nothing happens." );
	} else {
		char buf[1024];
		sprintf ( sizeof ( buf ), buf, "%s's %s is now identified.", caster->getName(), target->getName() );
		strcat ( output, buf );

		target->addAffect ( _AFF_IDENTIFIED, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, -1, 0, packet );
	}

	return NULL;
}


//
// castGREATER_IDENTIFY
//

SPELL ( castGREATER_IDENTIFY )
{
	WorldObject *target = roomMgr->findObject ( targetServID );
	caster = caster->getBaseOwner();

	if ( !target || !caster )
		return NULL;

	if ( !target->getBase ( _BCARRY ) || target->hasAffect ( _AFF_ENH_IDENTIFIED ) ) {
		strcat ( output, "Nothing happens." );
	} else {
		char buf[1024];
		sprintf ( sizeof ( buf ), buf, "%s's %s is now identified.", caster->getName(), target->getName() );
		strcat ( output, buf );

		target->addAffect ( _AFF_ENH_IDENTIFIED, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, -1, 0, packet );
	}

	return NULL;
}


//
// castBERSERK
//

SPELL ( castBERSERK )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target || !target->player ) {
		strcat ( output, "Nothing happens." );
		return NULL;
	}

	char buf[1024];
	int skill = calcSpellSkill ( caster, _SKILL_MYSTICISM );

	int duration = CalcMystSpellDuration ( caster, 3.0 );
	duration = calcSpellDuration ( caster, duration, packet );

	caster = caster->getBaseOwner();

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_BERSERK );
	packet->putByte ( 1 );
	packet->putByte ( 1 );
	packet->putLong ( targetServID );

	// get the target's player state
	CPlayerState *pPlayerState = target->character;
	int nMystImmunity = 0;

	if ( pPlayerState ) {
		nMystImmunity = pPlayerState->GetMystImmunityCount();
	}

	affect_t *affect = target->hasAffect ( _AFF_BERSERK );

	if ( nMystImmunity ) {
		sprintf ( sizeof ( buf ), buf, "|c22|Nothing happens, immune for %d round(s)! ", nMystImmunity );
		strcat ( output, buf );
	}

	else if ( affect ) {
		strcat ( output, "|c22|Nothing happens, already berserk! " );
	} 

	else {
		// increase the myst immunity...
		if ( pPlayerState ) {
			pPlayerState->ChangeMystImmunityCount ( duration * 2 );
		}

		target->addAffect ( _AFF_BERSERK, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, duration, caster->calcIntelligence(), packet );

		sprintf ( sizeof ( buf ), buf, "|c22|%s berserked %d round(s)! ", target->getName(), duration );
		strcat ( output, buf );
	}

	return NULL;
}

//
// castSTUN
//

SPELL ( castSTUN )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target || !target->player ) {
		strcat ( output, "Nothing happens." );
		return NULL;
	}

	char buf[1024];
	int skill = calcSpellSkill ( caster, _SKILL_MYSTICISM );

	int duration = CalcMystSpellDuration ( caster, 3.0 );
	duration = calcSpellDuration ( caster, duration, packet );

	caster = caster->getBaseOwner();

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_STUN );
	packet->putByte ( 1 );
	packet->putByte ( 1 );
	packet->putLong ( targetServID );

	// get the target's player state...
	CPlayerState *pPlayerState = target->character;
	int nMystImmunity = 0;

	if ( pPlayerState ) {
		nMystImmunity = pPlayerState->GetMystImmunityCount();
	}

	affect_t *affect = target->hasAffect ( _AFF_STUN );

	if ( nMystImmunity ) {
			sprintf ( sizeof ( buf ), buf, "|c22|Nothing happens, immune for %d round(s). ", nMystImmunity );
			strcat ( output, buf );
	}
		
	else if ( affect ) {
		strcat ( output, "|c22|Nothing happens, already stunned! " );
	}

	else {
		// increase myst immunity...
		if ( pPlayerState ) {
			pPlayerState->ChangeMystImmunityCount ( duration * 2 );
		}

		target->addAffect ( _AFF_STUN, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, duration, caster->calcIntelligence(), packet );

		sprintf ( sizeof ( buf ), buf, "|c22|%s stunned %d round(s)! ", target->getName(), duration );
		strcat ( output, buf );
	}

	return NULL;
}

//
// castLOYALTY_SHIFT
//

SPELL ( castLOYALTY_SHIFT )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target )
		return NULL;

	char buf[1024];
	int skill = calcSpellSkill ( caster, _SKILL_MYSTICISM );

	int duration = CalcMystSpellDuration ( caster, 6.0 );
	duration = calcSpellDuration ( caster, duration, packet );

	caster = caster->getBaseOwner();

	if ( target->getBase ( _BNPC ) && target->summoned ) {
		packet->putByte ( _MOVIE_SPECIAL_EFFECT );
		packet->putLong ( caster->servID );
		packet->putByte ( _SE_LOYALTY_SHIFT );
		packet->putByte ( 1 );
		packet->putByte ( 1 );
		packet->putLong ( targetServID );

		// get the target's player state...
		CPlayerState *pPlayerState = target->character;
		int nMystImmunity = 0;

		if ( pPlayerState )
			nMystImmunity = pPlayerState->GetMystImmunityCount();

		affect_t *affect = target->hasAffect ( _AFF_LOYALTY_SHIFT );

		if ( nMystImmunity ) {
			sprintf ( sizeof ( buf ), buf, "|c22|Nothing happens, immune for %d round(s). ", nMystImmunity );
			strcat ( output, buf );
		}

		else if ( affect ) {
			strcat ( output, "|c22|Nothing happens, already unloyal! " );
		}

		else {
			// increase the myst immunity...
			if ( pPlayerState )
				pPlayerState->ChangeMystImmunityCount ( duration * 2 );

			target->addAffect ( _AFF_LOYALTY_SHIFT, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, duration, caster->calcIntelligence(), packet );

			sprintf ( sizeof ( buf ), buf, "|c22|%s possessed %d round(s)! ", target->getName(), duration );
			strcat ( output, buf );
		}
	} else {
		sprintf ( sizeof ( buf ), buf, "|c22|Nothing happens, target not summoned. ", target->getName() );
		strcat ( output, buf );
	}

	return NULL;
}

//
// castWARP_MIND
//

SPELL ( castWARP_MIND )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target || !target->player ) {
		strcat ( output, "Nothing happens." );
		return NULL;
	}

	int skill = calcSpellSkill ( caster, _SKILL_MYSTICISM );
	int damage = 50 * skill;

	caster = caster->getBaseOwner();

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_WARP_MIND );
	packet->putByte ( 1 );
	packet->putLong ( target->servID );

	target->takeDamage ( WorldObject::_DAMAGE_PSYCHIC, caster, damage, output, packet, 1 );

	return NULL;
}

//
// castETHEREALIZE
//

SPELL ( castETHEREALIZE )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target || target->player ) {
		strcat ( output, "Nothing happens. " );
		return NULL;
	}

	int skill = calcSpellSkill (caster, _SKILL_MYSTICISM );

	int duration = 0;

	duration = calcSpellDuration ( caster, 15 * skill, packet );

	caster = caster->getBaseOwner();

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_ANTI_MAGIC_AURA );
	packet->putByte ( 1 );
	packet->putLong ( target->getBaseOwner()->servID );

	affect_t *affect = NULL;

	if ( target->getBase ( _BWEAPON ) ) {
		affect = target->hasAffect ( _AFF_DAMAGE_STEAL_EXPERIENCE, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL );

		if ( affect ) {
			affect->duration = duration;
		} else {
			affect = target->addAffect ( _AFF_DAMAGE_STEAL_EXPERIENCE, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, duration, 15, packet );
		}

		char buf[1024];
		sprintf ( sizeof ( buf ), buf, "|c22|A foul mystical aura surrounds the %s. ", target->basicName );
		strcat ( output, buf );
	} else {
		strcat ( output, "Nothing happens. " );
	}

	return affect;
}

//
// castSPELL_BLAST
//

SPELL ( castSPELL_BLAST )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target || !target->player ) {
		strcat ( output, "Nothing happens" );
		return NULL;
	}

	char buf[1024];

	caster = caster->getBaseOwner();

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_SPELL_BLAST );
	packet->putByte ( 1 );
	packet->putByte ( 1 );
	packet->putLong ( targetServID );

	affect_t *affect = target->hasAffect ( _AFF_SPELL_BLAST );

	if ( affect ) {
		sprintf ( sizeof ( buf ), buf, "|c22|%s has already been affected by this spell. ", caster->getName(), target->getName() );
		strcat ( output, buf );
	} else {
		int intelligence = target->calcIntelligence();

		if ( target->hasAffect ( _AFF_FREE_WILL, _AFF_TYPE_NORMAL ) )
			intelligence *= 2;

		if ( opposedRoll ( caster->calcIntelligence() * 2, intelligence ) ) {
			target->addAffect ( _AFF_SPELL_BLAST, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, 999, 0, packet );

			sprintf ( sizeof ( buf ), buf, "|c22|%s is surrounded with a blue aura! ", target->getName() );
			strcat ( output, buf );
		} else {
			sprintf ( sizeof ( buf ), buf, "|c22|%s resists the spell! ", target->getName() );
			strcat ( output, buf );
		}
	}

	return NULL;
}

//
// castMASS_HOLD
//

SPELL ( castMASS_HOLD )
{
	int skill = calcSpellSkill ( caster, _SKILL_MYSTICISM );

	caster = caster->getBaseOwner();

	if ( !caster || !caster->opposition || !caster->opposition->size() )
		return NULL;

	LinkedList heldList;
	int holdCount = 0;
	int extendCount = 0;
	char buf[1024];

	/* build affected list */
	LinkedElement *element = caster->opposition->head();

	while ( element ) {
		WorldObject *target = (WorldObject *)element->ptr();
		element = element->next();

		if ( target->getBase ( _BNPC ) && (target->health > 0) ) {
			heldList.add ( target );
		}
	}

	// build effect packet and damage creatures

	if ( !heldList.size() ) 
		strcat ( output, "Nothing happens. " );
	else {
		WorldObject *object;

		packet->putByte ( _MOVIE_SPECIAL_EFFECT );
		packet->putLong ( caster->servID );
		packet->putByte ( _SE_HOLD_MONSTER );
		packet->putByte ( 1 );
		packet->putByte ( heldList.size() );

		element = heldList.head();
		while ( element )
		{
			object = (WorldObject *)element->ptr();
			packet->putLong ( object->servID );
			element = element->next();
		}

		//  now set held affect for creatures
		element = heldList.head();

		while ( element ) {
			object = (WorldObject *)element->ptr();

			// get the target's player state
			CPlayerState *pPlayerState = object->character;
			int nMystImmunity = 0, nResisted = 0;

			if ( pPlayerState ) {
				nMystImmunity = pPlayerState->GetMystImmunityCount();

				if ( pPlayerState->TestSpellResistance ( _SKILL_MYSTICISM - _SKILL_SORCERY ) ) {
					nResisted = 1;

					packet->putByte ( _MOVIE_SPECIAL_EFFECT );
					packet->putLong ( caster->servID );
					packet->putByte ( _SE_SPELL_BLAST );
					packet->putByte ( 0 );
					packet->putByte ( 1 );
					packet->putLong ( object->servID );
				}
			}

			if ( !nResisted ) {
				if ( nMystImmunity ) {
					sprintf ( sizeof ( buf ), buf, "|c22|Nothing happens, immune for %d round(s)! ", nMystImmunity );
					strcat ( output, buf );
				} else {
					int duration = CalcMystSpellDuration ( caster, 3.0 );
					duration = random( std::max(( duration / 2 ), 1), duration );
//random ( ( duration / 2 ) >? 1, duration );
	
					if ( duration ) {
						// increase the myst immunity...
						if ( pPlayerState ) {
							pPlayerState->ChangeMystImmunityCount ( duration * 2 );
						}
	
						object->addAffect ( _AFF_HOLD, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, duration, caster->calcIntelligence(), packet );
	
						holdCount++;
					}
				}
			}

			element = element->next();
		}

		if ( holdCount == 1 ) 
			strcat ( output, "|c22|1 hold! " );
		else {
			if ( holdCount > 1 ) {
				char buf[1024];
				sprintf ( sizeof ( buf ), buf, "|c22|%d holds! ", holdCount );
				strcat ( output, buf );
			}
		}
	}

	heldList.release();

	return NULL;
}


//
// cast65
//

SPELL ( cast65 )
{
	caster = caster->getBaseOwner();

	summonMonster ( 1, "SummonedDragon", "Dragon from the mists", _SE_SUMMON_DAEMON, targetX, targetY, caster, output, packet );

	return NULL;
}

//
// castMASS_FUMBLE
//

SPELL ( castMASS_FUMBLE )
{
	caster = caster->getBaseOwner();

	if ( !caster || !caster->opposition || !caster->opposition->size() )
		return NULL;

	char buf[1024];
	LinkedList disarmedList;
	int disarmedCount = 0;

	int skill = calcSpellSkill ( caster, _SKILL_MYSTICISM );
	int chance = skill * 10;

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_MASS_FUMBLE );
	packet->putByte ( 1 );
	packet->putByte ( caster->opposition->size() );

	WorldObject *target;

	LinkedElement *element = caster->opposition->head();

	while ( element ) {
		target = (WorldObject *)element->ptr();
		element = element->next();
		packet->putLong ( target->servID );
	}

	element = caster->opposition->head();

	while ( element ) {

		target = (WorldObject *)element->ptr();
		element = element->next();

		WorldObject *targetObj = target->curWeapon;

		affect_t *hasAffect = target->hasAffect ( _AFF_RETENTION, _AFF_TYPE_RESISTANCE ); 

		if ( targetObj && target->player && !hasAffect ) {

			BWeapon *bWeapon = (BWeapon *)targetObj->getBase ( _BWEAPON );

			if ( bWeapon ) {

				Weapon weapon = *(Weapon *)bWeapon;

				int weaponSkill = target->getSkill ( weapon.skillType ) * 20;
	   
		   		if ( chance > 90 )
	   				chance = 90;

				if ( random (0, 100) > 50 )	{
					if ( opposedRoll ( chance, weaponSkill ) ) {
						if ( target->takeOff ( targetObj, packet ) == _WO_ACTION_HANDLED ) {
							disarmedCount ++;
							sprintf ( sizeof ( buf ), buf, "|c22|%s loses %s grip on the %s!", target->getName(), target->getPronoun ( _PRONOUN_HIS ), targetObj->getName() );
							strcat ( output, buf );
						}
					}
				}
			}
		}
	}

	if ( disarmedCount == 0 )
		strcat ( output, "Nothing happens." );

	return NULL;
}

//
// castFEAR
//

SPELL ( castFEAR )
{
	caster = caster->getBaseOwner();

	if ( !caster || !caster->opposition || !caster->opposition->size() )
		return NULL;

	int skill = calcSpellSkill ( caster, _SKILL_MYSTICISM );

	char buf[1024];

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_FEAR );
	packet->putByte ( 1 );
	packet->putByte ( caster->opposition->size() );

	LinkedElement *element = caster->opposition->head();
	int holdCount = 0;

	// build effect
	while ( element ) {
		WorldObject *target = (WorldObject *)element->ptr();
		element = element->next();

		packet->putLong ( target->servID );
	}

	element = caster->opposition->head();

	while ( element ) {
		WorldObject *target = (WorldObject *)element->ptr();
		element = element->next();

		// get the target's player state
		CPlayerState *pPlayerState = target->character;
		int nMystImmunity = 0, nResisted = 0;

		if ( pPlayerState ) {
			nMystImmunity = pPlayerState->GetMystImmunityCount();

			if ( pPlayerState->TestSpellResistance ( _SKILL_MYSTICISM - _SKILL_SORCERY ) ) {
				nResisted = 1;

				packet->putByte ( _MOVIE_SPECIAL_EFFECT );
				packet->putLong ( caster->servID );
				packet->putByte ( _SE_SPELL_BLAST );
				packet->putByte ( 0 );
				packet->putByte ( 1 );
				packet->putLong ( target->servID );
			}
		}

		if ( !nResisted ) {
			int duration = CalcMystSpellDuration ( caster, 5.0 );
			duration = random( std::max(( duration / 2 ), 1), duration);
//random ( ( duration / 2 ) >? 1, duration );
	
			if ( duration && !nMystImmunity ) {
				// increase the myst immunity...
				if ( pPlayerState ) {
					pPlayerState->ChangeMystImmunityCount ( duration * 2 );
				}
	
				target->addAffect ( _AFF_FEAR, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, duration, caster->calcIntelligence(), packet );
	
				holdCount++;
			}
		}
	}

	if ( !holdCount ) 
		strcat ( output, "Nothing happens. " );
	else 
		strcat ( output, "|c22|An aura of fear pervades the battlefield! ");

	return NULL;
}

//
// castIRON_CHAINS
//

SPELL ( castIRON_CHAINS )
{
	caster = caster->getBaseOwner();

	if ( !caster || !caster->opposition || !caster->opposition->size() )
		return NULL;

	char buf[1024];

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_MIND_SHACKLE );
	packet->putByte ( 1 );
	packet->putByte ( caster->opposition->size() );

	int skill = calcSpellSkill ( caster, _SKILL_MYSTICISM );

	LinkedElement *element = caster->opposition->head();
	int holdCount = 0;

	// build effect
	while ( element ) {
		WorldObject *target = (WorldObject *)element->ptr();
		element = element->next();

		packet->putLong ( target->servID );
	}

	element = caster->opposition->head();

	while ( element ) {
		WorldObject *target = (WorldObject *)element->ptr();
		element = element->next();

		int duration = CalcMystSpellDuration ( caster, 7.0 );
		duration = random( std::max(( duration / 2 ), 1), duration);
//random ( ( duration / 2 ) >? 1, duration );
	
		affect_t *affect = target->hasAffect ( _AFF_SHACKLED );

		if ( !affect ) {
			target->addAffect ( _AFF_SHACKLED, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, duration, caster->calcIntelligence(), packet );
			holdCount++;
		}
	}

	if ( !holdCount ) {
		strcat ( output, "Nothing happens. " );
	} else {
		sprintf ( sizeof ( buf ), buf, "|c22|%d opponents shackled! ", holdCount );
		strcat ( output, buf );
	}

	return NULL;
}

//
// castMASS_BERSERK
//

SPELL ( castMASS_BERSERK )
{
	int skill = calcSpellSkill ( caster, _SKILL_MYSTICISM );

	caster = caster->getBaseOwner();

	if ( !caster || !caster->opposition || !caster->opposition->size() )
		return NULL;

	char buf[1024];

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_BERSERK );
	packet->putByte ( 1 );
	packet->putByte ( caster->opposition->size() );

	LinkedElement *element = caster->opposition->head();
	int holdCount = 0;

	// build effect
	while ( element )	{
		WorldObject *target = (WorldObject *)element->ptr();
		element = element->next();

		packet->putLong ( target->servID );
	}

	element = caster->opposition->head();

	while ( element ) {
		WorldObject *target = (WorldObject *)element->ptr();
		element = element->next();

		// get the target's player state
		CPlayerState *pPlayerState = target->character;
		int nMystImmunity = 0, nResisted = 0;

		if ( pPlayerState ) {
			nMystImmunity = pPlayerState->GetMystImmunityCount();

			if ( pPlayerState->TestSpellResistance ( _SKILL_MYSTICISM - _SKILL_SORCERY ) ) {
				nResisted = 1;

				packet->putByte ( _MOVIE_SPECIAL_EFFECT );
				packet->putLong ( caster->servID );
				packet->putByte ( _SE_SPELL_BLAST );
				packet->putByte ( 0 );
				packet->putByte ( 1 );
				packet->putLong ( target->servID );
			}
		}

		if ( !nResisted ) {
			affect_t *affect = target->hasAffect ( _AFF_BERSERK );
	
			if ( !nMystImmunity && !affect ) {
				int duration = CalcMystSpellDuration ( caster, 3.0 );
				duration = random( std::max(( duration / 2 ), 1), duration);
//random ( ( duration / 2 ) >? 1, duration );
	
				// increase the myst immunity...
				if ( pPlayerState ) {
					pPlayerState->ChangeMystImmunityCount ( duration * 2 );
				}
	
				target->addAffect ( _AFF_BERSERK, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, duration, caster->calcIntelligence(), packet );
				holdCount++;
			}
		}
	}

	if ( !holdCount )
		strcat ( output, "Nothing happens. " );
	else if ( holdCount == 1 ) {
		strcat ( output, "|c22|1 opponent is berserked. " );
	} else {
		sprintf ( sizeof ( buf ), buf, "|c22|%d opponents are berserked! ", holdCount );
		strcat ( output, buf );
	}

	return NULL;
}

//
// castMASS_STUN
//

SPELL ( castMASS_STUN )
{
	int skill = calcSpellSkill ( caster, _SKILL_MYSTICISM );

	caster = caster->getBaseOwner();

	if ( !caster || !caster->opposition || !caster->opposition->size() )
		return NULL;

	char buf[1024];

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_STUN );
	packet->putByte ( 1 );
	packet->putByte ( caster->opposition->size() );

	LinkedElement *element = caster->opposition->head();
	int holdCount = 0;

	// build effect
	while ( element )	{
		WorldObject *target = (WorldObject *)element->ptr();
		element = element->next();

		packet->putLong ( target->servID );
	}

	element = caster->opposition->head();

	while ( element ) {
		WorldObject *target = (WorldObject *)element->ptr();
		element = element->next();

		// get the target's player state
		CPlayerState *pPlayerState = target->character;
		int nMystImmunity = 0, nResisted = 0;

		if ( pPlayerState ) {
			nMystImmunity = pPlayerState->GetMystImmunityCount();

			if ( pPlayerState->TestSpellResistance ( _SKILL_MYSTICISM - _SKILL_SORCERY ) ) {
				nResisted = 1;

				packet->putByte ( _MOVIE_SPECIAL_EFFECT );
				packet->putLong ( caster->servID );
				packet->putByte ( _SE_SPELL_BLAST );
				packet->putByte ( 0 );
				packet->putByte ( 1 );
				packet->putLong ( target->servID );
			}
		}

		if ( !nResisted ) {
			affect_t *affect = target->hasAffect ( _AFF_STUN );
	
			if ( !nMystImmunity && !affect ) {
				int duration = CalcMystSpellDuration ( caster, 3.0 );
				duration = random( std::max(( duration / 2 ), 1), duration);
//random ( ( duration / 2 ) >? 1, duration );
	
				// increase the myst immunity...
				if ( pPlayerState ) {
					pPlayerState->ChangeMystImmunityCount ( duration * 2 );
				}
	
				target->addAffect ( _AFF_STUN, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, duration, caster->calcIntelligence(), packet );
	
				holdCount++;
			}
		}
	}

	if ( !holdCount )
		strcat ( output, "Nothing happens. " );
	else if ( holdCount == 1 ) {
		strcat ( output, "|c22|1 opponent is stunned. " );
	} else {
		sprintf ( sizeof ( buf ), buf, "|c22|%d opponents are stunned! ", holdCount );
		strcat ( output, buf );
	}

	return NULL;
}

//
// castMASS_LOYALTY_SHIFT
//

SPELL ( castMASS_LOYALTY_SHIFT )
{
	int skill = calcSpellSkill ( caster, _SKILL_MYSTICISM );
	caster = caster->getBaseOwner();

	if ( !caster || !caster->opposition || !caster->opposition->size() )
		return NULL;

	char buf[1024];
	LinkedList heldList;
	int holdCount = 0;

	/* build affected list */
	LinkedElement *element = caster->opposition->head();

	while ( element ) {
		WorldObject *target = (WorldObject *)element->ptr();
		element = element->next();

		if ( target->getBase ( _BNPC ) && target->summoned ) {
			// add to list to hold
			heldList.add ( target );
		}
	}

	// build effect packet and damage creatures

	if ( !holdCount ) {
		strcat ( output, "Nothing happens. " );
	} else {
		WorldObject *object;

		packet->putByte ( _MOVIE_SPECIAL_EFFECT );
		packet->putLong ( caster->servID );
		packet->putByte ( _SE_LOYALTY_SHIFT );
		packet->putByte ( 1 );
		packet->putByte ( heldList.size() );

		element = heldList.head();

		while ( element ) {
			object = (WorldObject *)element->ptr();
			packet->putLong ( object->servID );
			element = element->next();
		}

		//  now set held affect for creatures
		element = heldList.head();

		while ( element ) {
			WorldObject *target = (WorldObject *)element->ptr();

			// get the target's player state
			CPlayerState *pPlayerState = target->character;
			int nMystImmunity = 0, nResisted = 0;
	
			if ( pPlayerState ) {
				nMystImmunity = pPlayerState->GetMystImmunityCount();
	
				if ( pPlayerState->TestSpellResistance ( _SKILL_MYSTICISM - _SKILL_SORCERY ) ) {
					nResisted = 1;
	
					packet->putByte ( _MOVIE_SPECIAL_EFFECT );
					packet->putLong ( caster->servID );
					packet->putByte ( _SE_SPELL_BLAST );
					packet->putByte ( 0 );
					packet->putByte ( 1 );
					packet->putLong ( target->servID );
				}
			}
	
			if ( !nResisted ) {
				affect_t *affect = target->hasAffect ( _AFF_LOYALTY_SHIFT );
	
				if ( !nMystImmunity && !affect ) {
					int duration = CalcMystSpellDuration ( caster, 6.0 );
					duration = random( std::max(( duration / 2 ), 1), duration);
//random ( ( duration / 2 ) >? 1, duration );
	
					// increase the myst immunity...
					if ( pPlayerState ) {
						pPlayerState->ChangeMystImmunityCount ( duration * 2 );
					}
	
					target->addAffect ( _AFF_LOYALTY_SHIFT, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, duration, caster->calcIntelligence(), packet );
				}
			}

			element = element->next();
		}

		if ( holdCount == 1 ) 
			sprintf ( sizeof ( buf ), buf, "|c22|%d creature affected! ", holdCount );
		else 
			sprintf ( sizeof ( buf ), buf, "|c22|%d creatures affected! ", holdCount );

		strcat ( output, buf );
	}

	heldList.release();

	return NULL;
}

//
// castMABONS_FORCED_FLEE
//

SPELL ( castMABONS_FORCED_FLEE )
{
    caster = caster->getBaseOwner();

    if ( !caster || !caster->opposition || !caster->opposition->size() )
        return NULL;

    packet->putByte ( _MOVIE_SPECIAL_EFFECT );
    packet->putLong ( caster->servID );
    packet->putByte ( _SE_FORCED_FLEE );
    packet->putByte ( 1 );
    packet->putByte ( caster->opposition->size() );

    LinkedElement *element = caster->opposition->head();

    // build effect
    while ( element ) {
        WorldObject *target = (WorldObject *)element->ptr();
        element = element->next();

        packet->putLong ( target->servID );
    }

    element = caster->opposition->head();

    // force all opponents to flee from combat
    
    int choice = rand() % 3;
            char buf[1024];
            
            
         switch(choice)
        {
            case 0:
            {
                // Force Flee
                strcat(output, "|c22|Mabon fills the battle with fear!" );
                while ( element ) {
            	WorldObject *target = (WorldObject *)element->ptr();
            	element = element->next();
                
                target->player->exitCombat ( packet );
            
                }
            }
            break;

            case 1:
            {
                // Mabon Refuses your request
                strcat(output, "|c14|Mabon refuses to serve you!");
            }
            break;

            case 2:
            {
                // Spell backfires
                strcat(output, "|c60|Mabon: 'You DARE reach out to me mortal?!" );
                caster->player->exitCombat ( packet );
            }
            break;
			

        }

    return NULL;
}

//
// castILLUSIONARY_FOE
//

SPELL(castILLUSIONARY_FOE)
{
    if (!caster || !caster->opposition)
        return NULL;

    LinkedList monsters;
    LinkedElement *element = caster->opposition->head();
    WorldObject *target;
    int summonCount = 0;
    int skill = calcSpellSkill(caster, _SKILL_NECROMANCY);

    if (element)
        for (element = ((WorldObject *)element->ptr())->opposition->head(); element; element = element->next())
        {
            WorldObject *object = (WorldObject *)element->ptr();
            if (object->summoned)
                ++summonCount;
        }
    if (summonCount >= 5)
    {
        char buf[1024];
        sprintf(sizeof(buf), buf, "|c60|The spell fails because there are already five summoned monsters on %s's side.|c43| ", caster->getName());
        strcat(output, buf);
        return(NULL);
    }

    //  choose a random foe
    int i = 0;
    for (element = caster->opposition->head(); element; element = element->next())
    {
        WorldObject *object = (WorldObject *)element->ptr();
        if (object->health > 0)
            ++i;
    }
    if (!i)
    {
        strcat(output, "Nothing happens. ");
        return NULL;
    }
    i = random(0, i - 1);
    for (element = caster->opposition->head(); element; element = element->next())
    {
        WorldObject *object = (WorldObject *)element->ptr();
        if (object->health > 0)
            if (i-- == 0)
            {
                target = (WorldObject *)element->ptr();
                break;
            }
    }

    if (targetX < 0)
        targetX = 0;
    else if (targetX > 23)
        targetX = 23;

    if (targetY < 0)
        targetY = 0;
    else if (targetY > 17)
        targetY = 17;

    int lastX = targetX, lastY = targetY;


    char *data;
    StringObject *classID = (StringObject *)"NPC";

    char *theClass = classID->data;

    //  make summoned monster an exact copy of original
    WorldObject *monster = new WorldObject(roomMgr->findClass("NPC"));
    monster->copy(target);

    monster->player->isNPC;

    monster->minLevel = target->level;
    monster->maxLevel = target->level;
	


	// Zach - Removing level cap on ill foe targets
	//
    //if (monster->level > caster->level)
    //{
    //    monster->level = caster->level;
    //    monster->minLevel = caster->level;
    //    monster->maxLevel = caster->level;
    //}



    //  remove non permanent and other unwanted effects
    if (monster->activeAffects)
        for (element = monster->activeAffects->head(); element; )
        {
            affect_t *affect = (affect_t *) element->ptr();
            element = element->next();
            if (affect->source == _AFF_SOURCE_SPELL || affect->source == _AFF_SOURCE_POTION || (affect->id >= _AFF_FREEZE && affect->id <= _AFF_INVULNERABLE) ||
             affect->id == _AFF_MAGIC_RESISTANCE || affect->id == _AFF_MAGIC_IMMUNITY || (affect->id >= _AFF_POS_DEX_MOD && affect->id <= _AFF_RETENTION))
                monster->delAffect(affect, packet);
        }

        // We need to remove things like poison or def, remove all debuffs, the above is supposed to but does not seem to work ( old note -zach )

    double ratio = ((double)skill * log10((skill > 10) ? (double)skill : 10)) / 1200;
    if (ratio > .5)
        ratio = .5;

    //  reduce attributes
    monster->strength = target->calcStrength();
    //monster->strength = (int)ceil((double)monster->strength * ratio);
    monster->dexterity = target->calcDexterity();
    //monster->dexterity = (int)ceil((double)monster->dexterity * ratio);
    monster->intelligence = target->calcIntelligence();
    //monster->intelligence = (int)ceil((double)monster->intelligence * ratio);
    monster->endurance = target->calcEndurance();
    //if (monster->endurance > 30)
    //    monster->endurance = 30;
    //monster->endurance = (int)ceil((double)monster->endurance * ratio);
    monster->quickness = target->calcDexterity();

    //  reduce unarmed damage
    //if (monster->hands)
    //{
    //    monster->hands->minDamage = target->hands->minDamage;
    //    if (monster->hands->minDamage > 2000)
    //        monster->hands->minDamage = 2000;
    //    monster->hands->minDamage = (int)ceil((double)monster->hands->minDamage * ratio);
    //    monster->hands->maxDamage = target->hands->maxDamage;
    //    if (monster->hands->maxDamage > 4000)
    //        monster->hands->maxDamage = 4000;
    //    monster->hands->maxDamage = (int)ceil((double)monster->hands->maxDamage * ratio);
    //}

    if (monster->character)
    {
        //  reduce spell resistance
        monster->character->ChangeSpellResistance(0, (int)-(monster->character->m_anSpellResistance[0] * (1 - ratio)));
        monster->character->ChangeSpellResistance(1, (int)-(monster->character->m_anSpellResistance[1] * (1 - ratio)));
        monster->character->ChangeSpellResistance(2, (int)-(monster->character->m_anSpellResistance[2] * (1 - ratio)));
        monster->character->ChangeSpellResistance(3, (int)-(monster->character->m_anSpellResistance[3] * (1 - ratio)));
        monster->character->ChangeSpellResistance(4, (int)-(monster->character->m_anSpellResistance[4] * (1 - ratio)));

        //  set myst immunity to 0
        monster->character->ChangeMystImmunityCount(-monster->character->GetMystImmunityCount());
    }

    monster->addToDatabase();
    monster->summoned = caster->servID;
    NPC *npc = makeNPC(monster);
    roomMgr->addObject(monster);

    CombatGrid grid;
    int theX = targetX, theY = targetY;

    grid.mapAccessible(caster->combatGroup, targetX, targetY);
    grid.findClosestPoint(lastX, lastY, caster->combatX, caster->combatY, &theX, &theY);

    monster->combatX = theX;
    monster->combatY = theY;
    lastX = theX;
    lastY = theY;
    monster->x = (monster->combatX * 26) + 13 + 5;
    monster->y = (monster->combatY * 11) + 5 + 110;

    caster->combatGroup->addCharacter(monster, caster->combatGroup->attackers.contains(caster) ? _COMBAT_ATTACKER : _COMBAT_DEFENDER);
    npc->room = caster->room;
    npc->zone = npc->room->zone;
    monster->room = caster->room;
    monster->hidden++;
    caster->room->addPlayer(npc, packet);
    monster->hidden--;
    monsters.add(monster);

    // enable NPC AI
    npc->aiReady = 1;

    packet->putByte(_MOVIE_SPECIAL_EFFECT);
    packet->putLong(caster->servID);
    packet->putByte(_SE_SUMMON_ZOMBIE);
    packet->putByte(1);
    packet->putByte(1);

    for (element = monsters.head(); element; element = element->next())
    {
        WorldObject *obj = (WorldObject *)element->ptr();
        packet->putLong(obj->servID);
    }


	// Zach - Removing Health cap on ill foe
	//
    //int amount = (int)((double)monster->healthMax * ((double)target->health / (double)target->healthMax));
    //amount = monster->healthMax - amount;
    //monster->changeHealth(-amount, monster, 1, 1, packet);

    char buf[1024];
    sprintf(sizeof(buf), buf, "A mirror image of %s appears. ", target->getName());
    strcat(output, buf);

    //  debugging
    /*sprintf(sizeof(buf), buf, "\nstr:  %d, dex:  %d, int:  %d,  end:  %d, qui:  %d, lev:  %d, hp:  %d / %d\n",
        monster->calcStrength(), monster->calcDexterity(), monster->calcIntelligence(), monster->calcEndurance(),
        monster->quickness, monster->level, monster->health, monster->healthMax);
    strcat(output, buf);*/

    monsters.release();
    return NULL;
}

//
// castANTI_MAGIC_AURA
//

SPELL ( castANTI_MAGIC_AURA )
{
	caster = caster->getBaseOwner();

	if ( !caster->player || !caster->combatGroup )
		return NULL;

	int skill = calcSpellSkill (caster, _SKILL_MYSTICISM );

	int duration = 0;

	int highDuration = calcSpellDuration ( caster, skill, packet );
	duration = random ( 1, highDuration );

	int chance = 20 + (skill * 10);

	// build effect packet
	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_ANTI_MAGIC_AURA );
	packet->putByte ( 0 );
	packet->putLong ( caster->servID );

	// set anti-magic bit on combat cloud
	WorldObject *cloud = caster->combatGroup->cloud;

	if ( cloud ) {

		affect_t *affect = cloud->hasAffect ( _AFF_ANTI_MAGIC, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL );
	
		if ( affect ) {

			strcat ( output, "|c22|The spell fails because an anti-magic aura is still present. " );

		} else {

			if ( random (0, 100) < chance ) {

				affect = cloud->addAffect ( _AFF_ANTI_MAGIC, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, duration, 0, packet );
				strcat ( output, "|c22|The air shimmers for a moment. An aura of magic suppression fills the area. " );
			}
		}
	
	}
	return NULL;
}

//
// castLIGHT_DART
//

SPELL ( castLIGHT_DART )
{
	WorldObject *target = roomMgr->findObject ( targetServID );
	
	if ( !target || !target->player ) {
		strcat ( output, "Nothing happens" );
		return NULL;
	}

	int numBlades = calcSpellSkill ( caster, _SKILL_THAUMATURGY );
	int damage = random ( 24 * numBlades, 48 * numBlades );  // 24 , 40

	numBlades = std::min(30, numBlades);//30 <? numBlades;

	if ( target->coarseAlignment() == _ALIGN_EVIL )
		damage *= 2;

	caster = caster->getBaseOwner();

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_LIGHT_DART );
	packet->putByte ( 1 );
	packet->putByte ( numBlades );

	for ( int i=0; i<numBlades; i++ )
		packet->putLong ( targetServID );

	target->takeDamage ( WorldObject::_DAMAGE_CUT, caster, damage, output, packet, 1 );

	return NULL;
}

//
// castCURSE_OF_CLUMSINESS
//

SPELL ( castCURSE_OF_CLUMSINESS )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target )
		return NULL;

	char buf[1024];
	int skill = calcSpellSkill ( caster, _SKILL_NECROMANCY );

	int duration = 0;

	duration = calcSpellDuration ( caster, 3 * skill, packet );

	caster = caster->getBaseOwner();

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_CLUMSINESS );
	packet->putByte ( 1 );
	packet->putLong ( target->servID );

	affect_t *affect = target->hasAffect ( _AFF_NEG_DEX_MOD, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL );

	if ( affect ) {
		affect->duration = duration;
	} else {
		affect = target->addAffect ( _AFF_NEG_DEX_MOD, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, duration, 15, packet );
	}

	if ( !target->player ) {
		sprintf ( sizeof ( buf ), buf, "|c21|The %s is surrounded by a magical aura! ", target->getName() );
		strcat ( output, buf );
	} else {
		sprintf ( sizeof ( buf ), buf, "|c248|%s has been cursed with clumsiness! ", target->getName() );
		strcat ( output, buf );
	}

	return affect;
}

//
// castNIMBILITY
//

SPELL ( castNIMBILITY )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target )
		return NULL;

	char buf[1024];
	int skill = calcSpellSkill ( caster, _SKILL_THAUMATURGY );

	int duration = 0;

	duration = calcSpellDuration ( caster, 3 * skill, packet );

	caster = caster->getBaseOwner();

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_NIMBILITY );
	packet->putByte ( 1 );
	packet->putLong ( target->servID );

	affect_t *affect = target->hasAffect ( _AFF_POS_DEX_MOD, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL );

	if ( affect ) {
		affect->duration = duration;
	} else {
		affect = target->addAffect ( _AFF_POS_DEX_MOD, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, duration, 15, packet );
	}

	if ( !target->player ) {
		sprintf ( sizeof ( buf ), buf, "|c21|The %s is surrounded by a magical aura! ", target->getName() );
		strcat ( output, buf );
	} else {
		sprintf ( sizeof ( buf ), buf, "%s has been blessed with dexterity! ", target->getName() );
		strcat ( output, buf );
	}	
	return affect;
}

//
// castEMPOWER
//

SPELL ( castEMPOWER )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target )
		return NULL;

	char buf[1024];
	int skill = calcSpellSkill ( caster, _SKILL_THAUMATURGY );

	int duration = 0;

	duration = calcSpellDuration ( caster, 3 * skill, packet );

	caster = caster->getBaseOwner();

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_EMPOWER );
	packet->putByte ( 1 );
	packet->putLong ( target->servID );

	affect_t *affect = target->hasAffect ( _AFF_EMPOWER, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL );

	if ( affect ) {
		affect->duration = duration;
	} else {
		affect = target->addAffect ( _AFF_EMPOWER, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, duration, 15, packet );
	}

	target->calcWeightCap();

	if ( !target->player ) {
		sprintf ( sizeof ( buf ), buf, "|c21|The %s is surrounded by a magical aura! ", target->getName() );
		strcat ( output, buf );
	} else {
		sprintf ( sizeof ( buf ), buf, "%s has been blessed with strength! ", target->getName() );
		strcat ( output, buf );
	}
	return affect;
}

//
// castENFEEBLE
//

SPELL ( castENFEEBLE )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target )
		return NULL;

	char buf[1024];
	int skill = calcSpellSkill ( caster, _SKILL_NECROMANCY );

	int duration = 0;

	duration = calcSpellDuration ( caster, 3 * skill, packet );

	caster = caster->getBaseOwner();

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_ENFEEBLE );
	packet->putByte ( 1 );
	packet->putLong ( target->servID );

	affect_t *affect = target->hasAffect ( _AFF_ENFEEBLE, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL );

	if ( affect ) {
		affect->duration = duration;
	} else {
		affect = target->addAffect ( _AFF_ENFEEBLE, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, duration, 15, packet );
	}

	target->calcWeightCap();

	if ( !target->player ) {
		sprintf ( sizeof ( buf ), buf, "|c21|The %s is surrounded by a magical aura! ", target->getName() );
		strcat ( output, buf );
	} else {
		sprintf ( sizeof ( buf ), buf, "|c248|%s has been cursed with weakness!", target->getName() );
		strcat ( output, buf );
	}
	return affect;
}

//
// castMISSILE_RESISTANCE
//

SPELL ( castMISSILE_RESISTANCE )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target )
		return NULL;

	char buf[1024];
	int skill = calcSpellSkill ( caster, _SKILL_THAUMATURGY );

	int duration = 0;

	duration = calcSpellDuration ( caster, 3 * skill, packet );

	caster = caster->getBaseOwner();

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_MISSILE_RESISTANCE );
	packet->putByte ( 1 );
	packet->putLong ( target->servID );

	affect_t *affect = target->hasAffect ( _AFF_DAMAGE_MISSILE, _AFF_TYPE_RESISTANCE, _AFF_SOURCE_SPELL );

	if ( affect ) {
		affect->duration = duration;
	} else {
		affect = target->addAffect ( _AFF_DAMAGE_MISSILE, _AFF_TYPE_RESISTANCE, _AFF_SOURCE_SPELL, duration, 15, packet );
	}

	if ( !target->player ) {
		sprintf ( sizeof ( buf ), buf, "|c21|The %s is surrounded by a magical aura! ", target->getName() );
		strcat ( output, buf );
	} else {
		sprintf ( sizeof ( buf ), buf, "A glowing aura surrounds %s! ", target->getName() );
		strcat ( output, buf );
	}
	return affect;
}

//
// castHEAL
//

SPELL ( castHEAL )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target || !target->player ) {
		strcat ( output, "Nothing happens" );
		return NULL;
	}

	caster = caster->getBaseOwner();
	int skill = calcSpellSkill ( caster, _SKILL_THAUMATURGY );

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_HEAL );
	packet->putByte ( 1 );
	packet->putLong ( target->getBaseOwner()->servID );

	double ratio = ((double)caster->level) / ((double)target->level); 
	int minPct = (int)(25.0 * ratio);
	int maxPct = (int)(40.0 * ratio);

	int percent = random ( minPct, maxPct );
	int damage = (target->healthMax * percent) / 100;

	if ( target->player && (target->health < target->healthMax) )
		target->takeDamage ( WorldObject::_DAMAGE_HEAL, caster, -damage, output, packet );

	return NULL;
}


//
// castSUMMON_PIXIE
//

SPELL ( castSUMMON_PIXIE )
{
	caster = caster->getBaseOwner();

	summonMonster ( 1, "Pixie", "pixie", _SE_SUMMON_PIXIE, targetX, targetY, caster, output, packet );

	return NULL;
}

//
// castPURIFY
//

SPELL ( castPURIFY )
{
	WorldObject *target = roomMgr->findObject ( targetServID );
	
	if ( !target || !target->player ) {
		strcat ( output, "Nothing happens." );
		return NULL;
	}

	char buf[1024];
	int skill = caster->getSkill ( _SKILL_THAUMATURGY );
	caster = caster->getBaseOwner();

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_PURIFY );
	packet->putByte ( 1 );
	packet->putLong ( target->getBaseOwner()->servID );

	affect_t *affect = target->hasAffect ( _AFF_POISONED, _AFF_TYPE_NORMAL );
	
  	if ( affect ) {
  		affect->value /= 2;
   
  		if ( !affect->value ) {
  			sprintf ( sizeof ( buf ), buf, "%s's poison is cured! ", target->getName() );
  			strcat ( output, buf );
 
  			target->delAffect ( affect, packet );
  		} else {
  			sprintf ( sizeof ( buf ), buf, "%s looks less ill. ", target->getName() );
  			strcat ( output, buf );
  		}
	} else {
		strcat ( output, "Nothing happens." );
	}

	return NULL;
}

//
// castCURE_POISON
//

SPELL ( castCURE_POISON )
{
	WorldObject *target = roomMgr->findObject ( targetServID );
	
	if ( !target || !target->player ) {
		strcat ( output, "Nothing happens." );
		return NULL;
	}

	int skill = caster->getSkill ( _SKILL_THAUMATURGY );
	caster = caster->getBaseOwner();

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_CURE_POISON );
	packet->putByte ( 1 );
	packet->putLong ( target->getBaseOwner()->servID );

	affect_t *affect = target->hasAffect ( _AFF_POISONED, _AFF_TYPE_NORMAL );

	if ( affect ) {
		char buf[1024];
		sprintf ( sizeof ( buf ), buf, "%s's poison is cured! ", target->getName() );
		strcat ( output, buf );

		target->delAffect ( affect, packet );
	} else {
		strcat ( output, "Nothing happens." );
	}
	return NULL;
}

//
// castSUMMON_ELEMENTAL - 85
//

SPELL ( castSUMMON_ELEMENTAL )
{
	caster = caster->getBaseOwner();

	switch ( random ( 1, 4 ) )
	{
		case 1:
			summonMonster ( 1, "EarthElemental", "EarthElemental", _SE_SUMMON_DAEMON, targetX, targetY, caster, output, packet );
			break;
		case 2:
			summonMonster ( 1, "FireElemental", "FireElemental", _SE_SUMMON_DAEMON, targetX, targetY, caster, output, packet );
			break;
		case 3:
			summonMonster ( 1, "WaterElemental", "WaterElemental", _SE_SUMMON_DAEMON, targetX, targetY, caster, output, packet );
			break;
		case 4:
			summonMonster ( 1, "AirElemental", "AirElemental", _SE_SUMMON_DAEMON, targetX, targetY, caster, output, packet );
			break;
	}

	return NULL;
}

//
// Death by ImpHead
//

SPELL ( castHEAD_OF_DEATH )
{
	WorldObject *target = roomMgr->findObject ( targetServID );
	
	if ( !target || !caster->player ) 
		return NULL;

	caster = caster->getBaseOwner();

	if ( caster->player->checkAccess ( _ACCESS_IMPLEMENTOR ) ) {
		int damage = target->health;

		packet->putByte ( _MOVIE_SPECIAL_EFFECT );
		packet->putLong ( caster->servID );
		packet->putByte ( _SE_HEAD_OF_DEATH );
		packet->putByte ( 1 );
		packet->putLong ( target->servID );

		target->takeDamage ( BWeapon::_DAMAGE_LIGHTNING, caster, damage, output, packet, 1 );
	}

	return NULL;
}

//
// castGREATER_HEAL
//

SPELL ( castGREATER_HEAL )
{
	WorldObject *target = roomMgr->findObject ( targetServID );
	
	if ( !target || !target->player ) {
		strcat ( output, "Nothing happens. " );
		return NULL;
	}

	int skill = calcSpellSkill ( caster, _SKILL_THAUMATURGY );
	caster = caster->getBaseOwner();

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_GREATER_HEAL );
	packet->putByte ( 1 );
	packet->putLong ( target->getBaseOwner()->servID );

	double ratio = ((double)caster->level) / ((double)target->level); 
	int minPct = (int)(40.0 * ratio);
	int maxPct = (int)(75.0 * ratio);

	int percent = random ( minPct, maxPct );
	int damage = (target->healthMax * percent) / 100;

	if ( target->player && (target->health < target->healthMax) )
		target->takeDamage ( WorldObject::_DAMAGE_HEAL, caster, -damage, output, packet );

	return NULL;
}

//
// castREMOVE_CURSE
//

SPELL ( castREMOVE_CURSE )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target )
		return NULL;

	char buf[1024];
	caster = caster->getBaseOwner();

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_REMOVE_CURSE );
	packet->putByte ( 1 );
	packet->putLong ( target->servID );

	// if a specific item is chosen, remove the curse on it (if any)
	if ( !target->player ) {
		affect_t *affect = target->hasAffect ( _AFF_CURSED );

		if ( affect ) {
			target->delAffect ( affect, packet );

			sprintf ( sizeof ( buf ), buf, "The %s is no longer cursed. ", target->basicName );
			strcat ( output, buf );
		}

	// step through all worn items and try to remove the curses
	} else {

		BContainer *base = (BContainer *)target->getBase ( _BCONTAIN );

		if ( base ) {
			LinkedElement *element = base->contents.head();

			while ( element ) {
				WorldObject *object = (WorldObject *)element->ptr();
				element = element->next();

				if ( object->isWornBy ( target ) ) {
					affect_t *affect = object->hasAffect ( _AFF_CURSED );

					if ( affect ) {
						object->delAffect ( affect, packet );

						sprintf ( sizeof ( buf ), buf, "%s's %s is no longer cursed. ", target->getName(), object->basicName );
						strcat ( output, buf );
					}
				}
			}
		}
	}
	return NULL;
}

//
// castSUMMON_FAERY
//

SPELL ( castSUMMON_FAERY )
{
	caster = caster->getBaseOwner();

	summonMonster ( 1, "Faery", "faery", _SE_SUMMON_PIXIE, targetX, targetY, caster, output, packet );

	return NULL;
}


//
// castSUMMON_RATLING
//

SPELL ( castSUMMON_RATLING )
{
	caster = caster->getBaseOwner();

	switch ( random ( 0, 2 ) )
	{
		case 0:
			summonMonster ( 1, "ArcticRat", "arctic rat", _SE_SUMMON_DAEMON, targetX, targetY, caster, output, packet );
		break;
		case 1:
			summonMonster ( 1, "FlameRat", "flame rat", _SE_SUMMON_DAEMON, targetX, targetY, caster, output, packet );
		break;
		case 2:
			summonMonster ( 1, "PlagueRat", "plague rat", _SE_SUMMON_DAEMON, targetX, targetY, caster, output, packet );
		break;
	}

	return NULL;
}


//
// castSUMMON_BAT
//

SPELL ( castSUMMON_BAT )
{
	caster = caster->getBaseOwner();

	switch ( random ( 0, 2 ) )
	{
		case 0:
			summonMonster ( 1, "CryptBat", "crypt bat", _SE_SUMMON_DAEMON, targetX, targetY, caster, output, packet );
		break;
		case 1:
			summonMonster ( 1, "VampireBat", "vampire bat", _SE_SUMMON_DAEMON, targetX, targetY, caster, output, packet );
		break;
		case 2:
			summonMonster ( 1, "StormBat", "storm bat", _SE_SUMMON_DAEMON, targetX, targetY, caster, output, packet );
		break;
	}

	return NULL;
}

//
// castSUMMON_FENRIS
//

SPELL ( castSUMMON_FENRIS )
{
	caster = caster->getBaseOwner();

	switch ( random ( 0, 2 ) )
	{
		case 0:
			summonMonster ( 1, "YoungFenris", "young fenris", _SE_SUMMON_ZOMBIE, targetX, targetY, caster, output, packet );
		break;
		case 1:
			summonMonster ( 1, "Fenris", "fenris", _SE_SUMMON_ZOMBIE, targetX, targetY, caster, output, packet );
		break;
		case 2:
			summonMonster ( 1, "BloodFenris", "blood fenris", _SE_SUMMON_ZOMBIE, targetX, targetY, caster, output, packet );
		break;
	}

	return NULL;
}

//
// castSUMMON_FENRIS
//

SPELL ( castSUMMON_IMP )
{
	caster = caster->getBaseOwner();

	switch ( random ( 0, 2 ) )
	{
		case 0:
			summonMonster ( 1, "ImpGuard", "imp guard", _SE_SUMMON_DAEMON, targetX, targetY, caster, output, packet );
		break;
		case 1:
			summonMonster ( 1, "ImpThaumaturgist", "imp thaumaturgist", _SE_SUMMON_DAEMON, targetX, targetY, caster, output, packet );
		break;
		case 2:
			summonMonster ( 1, "ImpWarrior", "imp warrior", _SE_SUMMON_DAEMON, targetX, targetY, caster, output, packet );
		break;
	}

	return NULL;
}


//
// castSUMMON_OGRE
//

SPELL ( castSUMMON_OGRE )
{
	caster = caster->getBaseOwner();

	switch ( random ( 0, 2 ) )
	{
		case 0:
			summonMonster ( 1, "Ogre", "ogre", _SE_SUMMON_WRAITH, targetX, targetY, caster, output, packet );
		break;
		case 1:
			summonMonster ( 1, "UglyOgre", "ugly ogre", _SE_SUMMON_WRAITH, targetX, targetY, caster, output, packet );
		break;
		case 2:
			summonMonster ( 1, "OgreMage", "ogre mage", _SE_SUMMON_WRAITH, targetX, targetY, caster, output, packet );
		break;
	}

	return NULL;
}

//
// castSUMMON_TROLL
//

SPELL ( castSUMMON_TROLL )
{
	caster = caster->getBaseOwner();

	switch ( random ( 0, 2 ) )
	{
		case 0:
			summonMonster ( 1, "TrollElementalist", "troll elementalist", _SE_SUMMON_WRAITH, targetX, targetY, caster, output, packet );
		break;
		case 1:
			summonMonster ( 1, "TrollSorcerer", "troll sorcerer", _SE_SUMMON_WRAITH, targetX, targetY, caster, output, packet );
		break;
		case 2:
			summonMonster ( 1, "TrollWarrior", "troll warrior", _SE_SUMMON_WRAITH, targetX, targetY, caster, output, packet );
		break;
	}

	return NULL;
}

//
// castSUMMON_SERAPH
//

SPELL ( castSUMMON_SERAPH )
{
	caster = caster->getBaseOwner();

	switch ( random ( 0, 2 ) )
	{
		case 0:
			summonMonster ( 1, "Seraph", "seraph", _SE_SUMMON_PIXIE, targetX, targetY, caster, output, packet );
		break;
		case 1:
			summonMonster ( 1, "SeraphWarrior", "seraph warrior", _SE_SUMMON_PIXIE, targetX, targetY, caster, output, packet );
		break;
		case 2:
			summonMonster ( 1, "SeraphThaumaturgist", "seraph thaumaturgist", _SE_SUMMON_PIXIE, targetX, targetY, caster, output, packet );
		break;
	}

	return NULL;
}

//
// castMONSTER_SUMMONING_I
//

SPELL ( castMONSTER_SUMMONING_I )
{
	caster = caster->getBaseOwner();

	switch (random (0, 4) )
	{
		case 0:
			summonMonster ( 1, "Pixie", "pixie", _SE_SUMMON_PIXIE, targetX, targetY, caster, output, packet );
		break;

		case 1:
			summonMonster ( 1, "DesertRat", "desert ratling", _SE_SUMMON_PIXIE, targetX, targetY, caster, output, packet );
		break;

		case 2:
			summonMonster ( 1, "FlyingRat", "flying rat", _SE_SUMMON_PIXIE, targetX, targetY, caster, output, packet );
		break;

		case 3:
			summonMonster ( 1, "Skeleton", "skeleton", _SE_SUMMON_PIXIE, targetX, targetY, caster, output, packet );
		break;

		case 4:
			summonMonster ( 1, "Ghoul", "ghoul", _SE_SUMMON_PIXIE, targetX, targetY, caster, output, packet );
		break;
	}

	return NULL;
}



SPELL ( castMONSTER_SUMMONING_II )
{
	caster = caster->getBaseOwner();

	switch (random (0, 3) )
	{
		case 0:
			summonMonster ( 1, "LostSailor", "lost sailor", _SE_SUMMON_ZOMBIE, targetX, targetY, caster, output, packet );
		break;

		case 1:
			summonMonster ( 1, "WaspWorker", "wasp worker", _SE_SUMMON_ZOMBIE, targetX, targetY, caster, output, packet );
		break;

		case 2:
			summonMonster ( 1, "Wolf", "wolf", _SE_SUMMON_ZOMBIE, targetX, targetY, caster, output, packet );
		break;

		case 3:
			summonMonster ( 1, "UndeadNecromancer", "undead necromancer", _SE_SUMMON_ZOMBIE, targetX, targetY, caster, output, packet );
		break;

		case 4:
			summonMonster ( 1, "CryptBat", "crypt bat", _SE_SUMMON_ZOMBIE, targetX, targetY, caster, output, packet );
		break;
	}

	return NULL;
}


SPELL ( castMONSTER_SUMMONING_III )
{
	caster = caster->getBaseOwner();

	switch (random (0, 7) )
	{
		case 0:
			summonMonster ( 1, "FlameRat", "flame ratling", _SE_SUMMON_DAEMON, targetX, targetY, caster, output, packet );
		break;

		case 1:
			summonMonster ( 1, "StormBat", "storm bat", _SE_SUMMON_DAEMON, targetX, targetY, caster, output, packet );
		break;

		case 2:
			summonMonster ( 1, "WoodNymph", "wood nymph", _SE_SUMMON_DAEMON, targetX, targetY, caster, output, packet );
		break;

		case 3:
			summonMonster ( 1, "DarkFaery", "dark faery", _SE_SUMMON_DAEMON, targetX, targetY, caster, output, packet );
		break;

		case 4:
			summonMonster ( 1, "YoungFenris", "young fenris", _SE_SUMMON_DAEMON, targetX, targetY, caster, output, packet );
		break;

		case 5:
			summonMonster ( 1, "DamnedOne", "damned one", _SE_SUMMON_DAEMON, targetX, targetY, caster, output, packet );
		break;

		case 6:
			summonMonster ( 1, "WaspWarrior", "wasp warrior", _SE_SUMMON_DAEMON, targetX, targetY, caster, output, packet );
		break;

		case 7:
			summonMonster ( 1, "EntombedOne", "entombed one", _SE_SUMMON_DAEMON, targetX, targetY, caster, output, packet );
		break;
	}

	return NULL;
}

//
// castFIRE_CURSE
//

SPELL ( castFIRE_CURSE )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target )
		return NULL;

	char buf[1024];
	int skill = calcSpellSkill ( caster, _SKILL_NECROMANCY );

	int duration = 0;

	duration = calcSpellDuration ( caster, 3 * skill, packet );

	caster = caster->getBaseOwner();

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_FIRE_CURSE );
	packet->putByte ( 1 );
	packet->putLong ( target->servID );

	affect_t *affect = target->hasAffect ( _AFF_DAMAGE_FIRE, _AFF_TYPE_WEAKNESS, _AFF_SOURCE_SPELL );

	if ( affect ) {
		affect->duration = duration;
	} else {
		affect = target->addAffect ( _AFF_DAMAGE_FIRE, _AFF_TYPE_WEAKNESS, _AFF_SOURCE_SPELL, duration, 15, packet );
	}

	if ( !target->player ) {
		sprintf ( sizeof ( buf ), buf, "|c21|The %s is surrounded by a magical aura! ", target->getName() );
		strcat ( output, buf );
	} else {
		sprintf ( sizeof ( buf ), buf, "|c248|%s has been cursed with a fire weakness!", target->getName() );
		strcat ( output, buf );
	}

	return affect;
}

//
// castCOLD_CURSE
//

SPELL ( castCOLD_CURSE )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target )
		return NULL;

	char buf[1024];
	int skill = calcSpellSkill ( caster, _SKILL_NECROMANCY );

	int duration = 0;

	duration = calcSpellDuration ( caster, 3 * skill, packet );

	caster = caster->getBaseOwner();

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_COLD_CURSE );
	packet->putByte ( 1 );
	packet->putLong ( target->servID );

	affect_t *affect = target->hasAffect ( _AFF_DAMAGE_COLD, _AFF_TYPE_WEAKNESS, _AFF_SOURCE_SPELL );

	if ( affect ) {
		affect->duration = duration;
	} else {
		affect = target->addAffect ( _AFF_DAMAGE_COLD, _AFF_TYPE_WEAKNESS, _AFF_SOURCE_SPELL, duration, 15, packet );
	}

	if ( !target->player ) {
		sprintf ( sizeof ( buf ), buf, "|c21|The %s is surrounded by a magical aura! ", target->getName() );
		strcat ( output, buf );
	} else {
		sprintf ( sizeof ( buf ), buf, "|c248|%s has been cursed with a cold weakness!", target->getName() );
		strcat ( output, buf );
	}

	return affect;
}

//
// castELECTRIC_CURSE
//

SPELL ( castELECTRIC_CURSE )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target )
		return NULL;

	char buf[1024];
	int skill = calcSpellSkill ( caster, _SKILL_NECROMANCY );

	int duration = 0;

	duration = calcSpellDuration ( caster, 3 * skill, packet );

	caster = caster->getBaseOwner();

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_LIGHTNING_CURSE );
	packet->putByte ( 1 );
	packet->putLong ( target->servID );

	affect_t *affect = target->hasAffect ( _AFF_DAMAGE_LIGHTNING, _AFF_TYPE_WEAKNESS, _AFF_SOURCE_SPELL );

	if ( affect ) {
		affect->duration = duration;
	} else {
		affect = target->addAffect ( _AFF_DAMAGE_LIGHTNING, _AFF_TYPE_WEAKNESS, _AFF_SOURCE_SPELL, duration, 15, packet );
	}

	if ( !target->player ) {
		sprintf ( sizeof ( buf ), buf, "|c21|The %s is surrounded by a magical aura! ", target->getName() );
		strcat ( output, buf );
	} else {
		sprintf ( sizeof ( buf ), buf, "|c248|%s has been cursed with a weakness to lightning!", target->getName() );
		strcat ( output, buf );
	}

	return affect;
}

//
// castSHIELD
//
SPELL ( castSHIELD )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target || !target->player ) {
		strcat ( output, "Nothing happens. " );
		return NULL;
	}

	caster = caster->getBaseOwner();

	//if ( caster->character->profession == _PROF_WIZARD ) {

		int skill = calcSpellSkill ( caster, _SKILL_THAUMATURGY );

		int duration = 0;

		duration = calcSpellDuration ( caster, 10 * skill, packet );

		packet->putByte ( _MOVIE_SPECIAL_EFFECT );
		packet->putLong ( caster->servID );
		packet->putByte ( _SE_SHIELD );
		packet->putByte ( 1 );
		packet->putLong ( target->servID );

		affect_t *affect = target->hasAffect ( _AFF_SHIELD );

		if ( affect ) {
			affect->duration = duration;
			affect->value = 50;
		} else {
			target->addAffect ( _AFF_SHIELD, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, duration, 50, packet );
		}

		target->calcAC();

		char buf[1024];
		sprintf ( sizeof ( buf ), buf, "A lesser protective aura envelops %s! ", target->getName() );
		strcat ( output, buf );

	//}else{ 
	//	char buf[1024];
	//	sprintf ( sizeof ( buf ), buf, "|c60|Your class cannot use this spell!" );
   	//	strcat ( output, buf );
	//}

	return NULL;
}

//
// castGREATER_SHIELD
//

SPELL ( castGREATER_SHIELD )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target || !target->player ) {
		strcat ( output, "Nothing happens. " );
		return NULL;
	}

	caster = caster->getBaseOwner();

	//if ( caster->character->profession == _PROF_WIZARD ) {

		int skill = calcSpellSkill ( caster, _SKILL_THAUMATURGY );

		int duration = 0;

		duration = calcSpellDuration ( caster, 10 * skill, packet );

	

		packet->putByte ( _MOVIE_SPECIAL_EFFECT );
		packet->putLong ( caster->servID );
		packet->putByte ( _SE_GREATER_SHIELD );
		packet->putByte ( 1 );
		packet->putLong ( target->servID );

		affect_t *affect = target->hasAffect ( _AFF_GREATER_SHIELD );

		if ( affect ) {
			affect->value = 60;
			affect->duration = duration;
		} else {
			target->addAffect ( _AFF_GREATER_SHIELD, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, duration, 60, packet );
		}

		target->calcAC();

		char buf[1024];
		sprintf ( sizeof ( buf ), buf, "A greater protective aura envelops %s! ", target->getName() );
		strcat ( output, buf );

	//}else{ 
	//	char buf[1024];
	//	sprintf ( sizeof ( buf ), buf, "|c60|Your class cannot use this spell!" );
   	//	strcat ( output, buf );
	//}
	return NULL;
}


SPELL ( castWRATH_OF_THE_GODS )
{
	caster = caster->getBaseOwner();

	if ( !caster || !caster->opposition || !caster->opposition->size() )
		return NULL;

	LinkedList wrathedList;

	/* build affected list */
	int skill = calcSpellSkill ( caster, _SKILL_THAUMATURGY );

	// don't take out your own team!
	LinkedElement *element = caster->opposition->head();

	while ( element )
	{
		WorldObject *target = (WorldObject *)element->ptr();
		element = element->next();

		/* target is not good... add to the wrath list */

		//MIKE-ALIGNMENT - changed to reflect alignment table
		//if ( target->health > 0 && target->alignment <= 175 )
		if ( target->health > 0 && target->alignment < 171 )
		{
			wrathedList.add ( target );
		}
	}

	// if you're no good, you take damage

	//MIKE-ALIGNMENT - changed to reflect alignment table
	//if ( caster->alignment <= 175 )
	if ( caster->alignment < 171 )
		wrathedList.add ( caster );

	// build effect packet and damage creatures

	if ( !wrathedList.size() ) 
		strcat ( output, "Nothing happens. " );
	else
	{
		WorldObject *object;

		packet->putByte ( _MOVIE_SPECIAL_EFFECT );
		packet->putLong ( caster->servID );
		packet->putByte ( _SE_WRATH_OF_THE_GODS );
		packet->putByte ( 1 );
		packet->putByte ( wrathedList.size() );

		element = wrathedList.head();

		while ( element ) {
			object = (WorldObject *)element->ptr();
			packet->putLong ( object->servID );
			element = element->next();
		}

		//  now damage wrathed creatures
		element = wrathedList.head();
		while ( element )
		{
			object = (WorldObject *)element->ptr();

			CPlayerState *pPlayerState = object->character;
			int nResisted = 0;
	
			if ( pPlayerState ) {
				if ( pPlayerState->TestSpellResistance ( _SKILL_THAUMATURGY - _SKILL_SORCERY ) ) {
					nResisted = 1;
	
					packet->putByte ( _MOVIE_SPECIAL_EFFECT );
					packet->putLong ( caster->servID );
					packet->putByte ( _SE_SPELL_BLAST );
					packet->putByte ( 0 );
					packet->putByte ( 1 );
					packet->putLong ( object->servID );
				}
			}
	
			if ( !nResisted ) {
				int damage = random ( 48 * skill, 65 * skill ); // 48 , 64

				if ( object->coarseAlignment() == _ALIGN_EVIL )
					damage *= 2.5;

				object->takeDamage ( WorldObject::_DAMAGE_PSYCHIC, caster, damage, output, packet, 1 );
			}

			element = element->next();
		}

	}

	wrathedList.release();

	return NULL;
}

//
// castFIRE_SHIELD
//

SPELL ( castFIRE_SHIELD )
{
	WorldObject *target = roomMgr->findObject ( targetServID );
	int skill = calcSpellSkill ( caster, _SKILL_THAUMATURGY );

	if ( !target )
		return NULL;

	char buf[1024];

	int duration = 0;

	duration = calcSpellDuration ( caster, 3 * skill, packet );

	caster = caster->getBaseOwner();

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_FIRE_SHIELD );
	packet->putByte ( 1 );
	packet->putLong ( target->servID );

	affect_t *affect = target->hasAffect ( _AFF_DAMAGE_FIRE, _AFF_TYPE_RESISTANCE, _AFF_SOURCE_SPELL );

	if ( affect ) {
		affect->duration = duration;
	} else {
		affect = target->addAffect ( _AFF_DAMAGE_FIRE, _AFF_TYPE_RESISTANCE, _AFF_SOURCE_SPELL, duration, 15, packet );
	}

	if ( !target->player ) {
		sprintf ( sizeof ( buf ), buf, "|c21|The %s is surrounded by a fire shield! ", target->getName() );
		strcat ( output, buf );
	} else {
		sprintf ( sizeof ( buf ), buf, "A fire shield surrounds %s! ", target->getName() );
		strcat ( output, buf );
	}

	return affect;
}

//
// castCOLD_SHIELD
//

SPELL ( castCOLD_SHIELD )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target )
		return NULL;

	char buf[1024];
	int skill = calcSpellSkill ( caster, _SKILL_THAUMATURGY );

	int duration = 0;

	duration = calcSpellDuration ( caster, 3 * skill, packet );

	caster = caster->getBaseOwner();

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_COLD_SHIELD );
	packet->putByte ( 1 );
	packet->putLong ( target->servID );

	affect_t *affect = target->hasAffect ( _AFF_DAMAGE_COLD, _AFF_TYPE_RESISTANCE, _AFF_SOURCE_SPELL );

	if ( affect ) {
		affect->duration = duration;
	} else {
		affect = target->addAffect ( _AFF_DAMAGE_COLD, _AFF_TYPE_RESISTANCE, _AFF_SOURCE_SPELL, duration, 15, packet );
	}

	if ( !target->player ) {
		sprintf ( sizeof ( buf ), buf, "|c21|The %s is surrounded by a cold shield! ", target->getName() );
		strcat ( output, buf );
	} else {
		sprintf ( sizeof ( buf ), buf, "A cold shield surrounds %s! ", target->getName() );
		strcat ( output, buf );
	}

	return affect;
}

//
// castLIGHTNING_SHIELD
//

SPELL ( castLIGHTNING_SHIELD )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target )
		return NULL;

	char buf[1024];
	int skill = calcSpellSkill ( caster, _SKILL_THAUMATURGY );

	int duration = 0;

	duration = calcSpellDuration ( caster, 3 * skill, packet );

	caster = caster->getBaseOwner();

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_LIGHTNING_SHIELD );
	packet->putByte ( 1 );
	packet->putLong ( target->servID );

	affect_t *affect = target->hasAffect ( _AFF_DAMAGE_LIGHTNING, _AFF_TYPE_RESISTANCE, _AFF_SOURCE_SPELL );

	if ( affect ) {
		affect->duration = duration;
	} else {
		affect = target->addAffect ( _AFF_DAMAGE_LIGHTNING, _AFF_TYPE_RESISTANCE, _AFF_SOURCE_SPELL, duration, 15, packet );
	}

	if ( !target->player ) {
		sprintf ( sizeof ( buf ), buf, "|c21|The %s is surrounded by a lightning shield! ", target->getName() );
		strcat ( output, buf );
	} else {
		sprintf ( sizeof ( buf ), buf, "A lightning shield surrounds %s! ", target->getName() );
		strcat ( output, buf );
	}

	return affect;
}

//
// castSUMMON_NYMPH
//

SPELL ( castSUMMON_NYMPH )
{
	caster = caster->getBaseOwner();

	summonMonster ( 1, "WoodNymph", "nymph", _SE_SUMMON_PIXIE, targetX, targetY, caster, output, packet );

	return NULL;
}

//
// castREGENERATION
//

SPELL ( castREGENERATION )
{
	WorldObject *target = roomMgr->findObject ( targetServID );
	
	if ( !target )
		return NULL;

	char buf[1024];
	int skill = calcSpellSkill ( caster, _SKILL_THAUMATURGY );

	int duration = 0;

	duration = calcSpellDuration ( caster, 5 * skill, packet );

	caster = caster->getBaseOwner();

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_HEAL );
	packet->putByte ( 1 );
	packet->putLong ( target->getBaseOwner()->servID );

	affect_t *affect = NULL;

	affect = target->hasAffect ( _AFF_REGENERATION, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL );

	if ( affect ) {
		affect->duration = duration;
	} else {
		affect = target->addAffect ( _AFF_REGENERATION, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, duration, 0, packet );
	}

	if ( !target->player ) {
		sprintf ( sizeof ( buf ), buf, "An aura of regeneration surrounds the %s. ", target->getName() );
		strcat ( output, buf );
	} else {
		sprintf ( sizeof ( buf ), buf, "An aura of regeneration surrounds %s. ", target->getName() );
		strcat ( output, buf );
	}

	return affect;
}

//
// castINDESTRUCTION
//

SPELL ( castINDESTRUCTION )
{
	WorldObject *target = roomMgr->findObject ( targetServID );
	caster = caster->getBaseOwner();
	
	if ( !target || target->player ) {
		strcat ( output, "Nothing happens. " );
		return NULL;
	}

	int skill = calcSpellSkill ( caster, _SKILL_THAUMATURGY );

	int duration = 0;

	duration = calcSpellDuration ( caster, 10 * skill, packet );

	affect_t *affect=NULL;

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_INDESTRUCTION );
	packet->putByte ( 1 );
	packet->putLong ( target->getBaseOwner()->servID );

	// if the target already has indestruction on it, bail out
	affect = target->hasAffect ( _AFF_INDESTRUCTION, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL );

	if ( !target->getBase ( _BCARRY ) ) {
		strcat ( output, "Nothing happens." );
	} else {
	  
		if ( affect ) 
			affect->duration = duration;
		else {
			// put indestruction on the item
			affect = target->addAffect ( _AFF_INDESTRUCTION, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, duration, 0, packet );
		}
		char buf[1024];
		sprintf ( sizeof ( buf ), buf, "The %s glows bright white for a moment.", target->getName() );
		strcat ( output, buf );
	}
	return NULL;
}

//
// castINVULNERABILITY
//

SPELL ( castINVULNERABILITY )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target || !target->player ) {
		strcat ( output, "Nothing happens. " );
		return NULL;
	}

	caster = caster->getBaseOwner();

	//if ( caster->character->profession == _PROF_WIZARD ) {

		int skill = calcSpellSkill ( caster, _SKILL_THAUMATURGY );
		int duration = 0;

		duration = calcSpellDuration ( caster, 10 * skill, packet );

		packet->putByte ( _MOVIE_SPECIAL_EFFECT );
		packet->putLong ( caster->servID );
		packet->putByte ( _SE_INVULNERABILITY );
		packet->putByte ( 1 );
		packet->putLong ( target->servID );

		affect_t *affect = target->hasAffect ( _AFF_INVULNERABLE );

		if ( affect ) {
			affect->value = 70;
			affect->duration = duration;
		} else {
			target->addAffect ( _AFF_INVULNERABLE, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, duration, 70, packet );
		}

		target->calcAC();

		char buf[1024];
		sprintf ( sizeof ( buf ), buf, "A invulnerability aura surrounds %s! ", target->getName() );
		strcat ( output, buf );

	/*}else{ 
		char buf[1024];
		sprintf ( sizeof ( buf ), buf, "|c60|Your class cannot use this spell!" );
   		strcat ( output, buf );
	}*/
	return NULL;
}

//
// castENIDS_BLESSING
//

SPELL ( castENIDS_BLESSING )
{
	caster = caster->getBaseOwner();

	if ( !caster || !caster->opposition || !caster->opposition->size() )
		return NULL;

	// get affected list of friendly players

	LinkedList *friendList;
	LinkedElement *element = caster->opposition->head();

	if ( element )
	{
		friendList = ((WorldObject *)element->ptr())->opposition;

		if ( friendList->size() ) {
			strcat ( output, "|c12|The goddess Enid blesses your party with health! ");

			// iterate through friend list...

			WorldObject *object;

			packet->putByte ( _MOVIE_SPECIAL_EFFECT );
			packet->putLong ( caster->servID );
			packet->putByte ( _SE_ENIDS_BLESSING );
			packet->putByte ( 1 );
			packet->putByte ( friendList->size() );

			element = friendList->head();
			while ( element )
			{
				object = (WorldObject *)element->ptr();
				packet->putLong ( object->servID );
				element = element->next();
			}

			//  now heal/cure all friends
			element = friendList->head();
			while ( element )
			{
				object = (WorldObject *)element->ptr();

				// heal
				if ( object->health < object->healthMax )
				{
					object->takeDamage ( WorldObject::_DAMAGE_HEAL, caster, -( object->healthMax - object->health ), output, packet, 1 );
				}

				// cure poison
				affect_t *affect = object->hasAffect ( _AFF_POISONED, _AFF_TYPE_NORMAL );
				if ( affect ) object->delAffect ( affect, packet );

				// cure acid-burns
				affect = object->hasAffect ( _AFF_ACID_BURN, _AFF_TYPE_NORMAL );
				if ( affect ) object->delAffect ( affect, packet );

				element = element->next();
			}
		} else {
			strcat ( output, "|c12|Nothing happens. " );
		}
	}
	return NULL;
}

//
// castBANISHMENT
//

SPELL ( castBANISHMENT )
{
	caster = caster->getBaseOwner();

	if ( !caster || !caster->opposition || !caster->opposition->size() )
		return NULL;

	LinkedList banishList;
	int banishCount = 0;

	LinkedElement *element = caster->opposition->head();

	// build list of creatures to banish

	while ( element )
	{
		WorldObject *target = (WorldObject *)element->ptr();
		element = element->next();

		if ( target->getBase ( _BNPC ) &&
			 target->summoned )
		{
			banishList.add ( target );
			banishCount++;
		}
	}

	// build effect packet and damage creatures

	if ( !banishCount ) 
		strcat ( output, "Nothing happens. " );
	else
	{
		WorldObject *object;
		int damage=0;

		packet->putByte ( _MOVIE_SPECIAL_EFFECT );
		packet->putLong ( caster->servID );
		packet->putByte ( _SE_BANISHMENT );
		packet->putByte ( 1 );
		packet->putByte ( banishCount );

		element = banishList.head();
		while ( element )
		{
			object = (WorldObject *)element->ptr();
			packet->putLong ( object->servID );
			element = element->next();
		}

		//  now damage creatures
		element = banishList.head();
		while ( element )
		{
			object = (WorldObject *)element->ptr();
			damage = object->health;
			object->takeDamage ( WorldObject::_DAMAGE_PSYCHIC, caster, damage, output, packet, 1 );
			element = element->next();
		}
		char buf[1024];
		sprintf ( sizeof ( buf ), buf, "%d creatures are banished! ", banishCount );
		strcat ( output, buf );
	}
	banishList.release();

	return NULL;
}

//
// castSUMMON_FAERY_QUEEN
//

SPELL ( castSUMMON_FAERY_QUEEN )
{
	caster = caster->getBaseOwner();

	summonMonster ( 1, "FaeryQueen", "faery queen", _SE_SUMMON_PIXIE, targetX, targetY, caster, output, packet );

	return NULL;
}

//
// castACID_SPHERE
//

SPELL ( castACID_SPHERE )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target || !target->player ) {
		strcat ( output, "Nothing happens. " );
		return NULL;
	}

	int numBlades = calcSpellSkill ( caster, _SKILL_NECROMANCY );
	int damage = random ( 24 * numBlades, 48 * numBlades );		// 24 , 48

	numBlades = std::min(24, numBlades);//24 <? numBlades;

	caster = caster->getBaseOwner();

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_ACID_SPHERE );
	packet->putByte ( 1 );
	packet->putByte ( numBlades );

	for ( int i=0; i<numBlades; i++ )
		packet->putLong ( targetServID );

	target->takeDamage ( _AFF_DAMAGE_ACID, caster, damage, output, packet, 1 );

	return NULL;
}

//
// castVENOMOUS_TOUCH
//

SPELL ( castVENOMOUS_TOUCH )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target ) 
		return NULL;

	char buf[1024];
	int skill = calcSpellSkill ( caster, _SKILL_NECROMANCY );

	int duration = 0;

	duration = calcSpellDuration ( caster, 3 * skill, packet );

	caster = caster->getBaseOwner();

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_VENOMOUS_TOUCH );
	packet->putByte ( 1 );
	packet->putLong ( target->servID );

	affect_t *affect = target->hasAffect ( _AFF_DAMAGE_POISON, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL );

	if ( affect ) {
		affect->duration = duration;
	} else {
		affect = target->addAffect ( _AFF_DAMAGE_POISON, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, duration, 15, packet );
	}

	if ( !target->player ) {
		sprintf ( sizeof ( buf ), buf, "|c21|The %s is surrounded by a sickly green aura! ", target->getName() );
		strcat ( output, buf );
	} else {
		sprintf ( sizeof ( buf ), buf, "A disgusting green liquid begins to drip from %s's fingers!", target->getName() );
		strcat ( output, buf );
	}
	return NULL;
}

//
// cast107 (Raid Boss Summon Spell)
//

SPELL ( cast107 )
{
	caster = caster->getBaseOwner();

	switch (random (0, 5) )
	{
		case 0:
			summonMonster ( 1, "godDuach", "god Duach", _SE_SUMMON_DAEMON, targetX, targetY, caster, output, packet );
		break;

		case 1:
			summonMonster ( 1, "godElphame", "goddess Elphame", _SE_SUMMON_DAEMON, targetX, targetY, caster, output, packet );
		break;

		case 2:
			summonMonster ( 1, "GodEnid", "goddess Enid", _SE_SUMMON_PIXIE, targetX, targetY, caster, output, packet );
		break;

		case 3:
			summonMonster ( 1, "GodMabon", "god Mabon", _SE_SUMMON_PIXIE, targetX, targetY, caster, output, packet );
		break;

		case 4:
			summonMonster ( 1, "GodFinvarra", "god Finvarra", _SE_SUMMON_DAEMON, targetX, targetY, caster, output, packet );
		break;

		case 5:
			summonMonster ( 1, "GodDespothes", "god-king Despothes", _SE_SUMMON_PIXIE, targetX, targetY, caster, output, packet );
		break;
	}

	return NULL;
}


//
// castPOISON_BOLT
//

SPELL ( castPOISON_BOLT )
{
	WorldObject *target = roomMgr->findObject ( targetServID );
	
	if ( !target || !target->player ) {
		strcat ( output, "Nothing happens. " );
		return NULL;
	}

	int numBlades = calcSpellSkill ( caster, _SKILL_NECROMANCY );

	if ( !numBlades )
		numBlades = 10;

	int damage = random ( 24 * numBlades, 48 * numBlades ); // 24 , 48

	numBlades = std::min(40, numBlades);//40 <? numBlades;

	caster = caster->getBaseOwner();

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_POISON_BOLT );
	packet->putByte ( 1 );
	packet->putByte ( numBlades );

	for ( int i=0; i<numBlades; i++ )
		packet->putLong ( targetServID );

	target->takeDamage ( _AFF_DAMAGE_POISON, caster, damage, output, packet, 1 );

	return NULL;
}

//
// Cleave
//

SPELL ( cast109 )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target ) 
		return NULL;
	
	int distance;
	int skill = calcPhysicalMod ( caster );
	
	caster = caster->getBaseOwner();

	if ( !caster || !caster->combatGroup )
		return NULL;

	// set up special effect
	targetX = target->combatX;
	targetY = target->combatY;

	// do some damage
	LinkedElement *element = caster->combatGroup->combatants.head();

	while ( element )
	{
		WorldObject *object = (WorldObject *)element->ptr();
		element = element->next();

		// see if this guy is near the impact
		distance = getDistance ( targetX, targetY, object->combatX, object->combatY );

		if ( distance < 5 ) {

			// this guy should take damage
			int damage = random ( 8*skill, 19*skill );
			if(object->player->isNPC) object->takeDamage ( _AFF_DAMAGE_NORMAL, caster, damage, output, packet, 1 );
			
		}
	}

	return NULL;
}

//
// castDRAIN_LIFE
//

SPELL ( castDRAIN_LIFE )
{
	WorldObject *target = roomMgr->findObject ( targetServID );
	
	if ( !target || !target->player ) {
		strcat ( output, "Nothing happens. " );
		return NULL;
	}

	int skill = calcSpellSkill ( caster, _SKILL_NECROMANCY );
	int damage = random ( 21 * skill, 34 * skill );		// 21 , 32

	caster = caster->getBaseOwner();
	int distance = getDistance ( caster->combatX, caster->combatY, target->combatX, target->combatY );

	if ( distance < gTouchDistance )	{
		packet->putByte ( _MOVIE_SPECIAL_EFFECT );
		packet->putLong ( caster->servID );
		packet->putByte ( _SE_DRAIN_LIFE );
		packet->putByte ( 1 );
		packet->putLong ( target->servID );

		target->takeDamage ( _AFF_DAMAGE_STEAL_LIFE, caster, damage, output, packet, 1 );
	} else {
		char buf[1024];
		sprintf ( sizeof ( buf ), buf, "The spell fails because %s is not close enough to touch! ", target->getName() );
		strcat ( output, buf );
	}

	return NULL;
}

//
// castACID_CLOUD
//
// Used in this fashion as an AoE Poison spell 
//
//

SPELL ( castACID_CLOUD )
{
	char buf[100];

	//heres a list of player targets
	LinkedList targetList;

	for(short combatX = targetX - 4; combatX <= targetX + 4; ++combatX)
	{
		if( combatX < 0 ) continue;
		else if( combatX >= _COMBAT_GRID_WIDTH ) continue;

		for(short combatY = targetY - 4; combatY <= targetY + 4; ++combatY)
		{
			if (combatY < 0) continue;
			else if( combatY >= _COMBAT_GRID_HEIGHT ) continue;
            
			//get the servID of the object in this square
			unsigned int servID = caster->combatGroup->grid[combatX][combatY];

			if( servID <= 1 ) continue;
			
			//add it to our target list if there is a valid servID
			//and they are not a minotaur (resistant)
			if( servID ) {
				WorldObject* obj = roomMgr->getObject( servID );
				
				if( obj && obj->character )
					targetList.add( obj );
				else logDisplay( "%s:%d castAREA_POISON - target error. servid %d, obj %d  obj->character %d", __FILE__, __LINE__, servID, (int)obj, obj?(int)obj->character:0 );
			}
		}
	}

	char numTargets = targetList.size();

	char skill = caster->getSkill( _SKILL_NECROMANCY );
	char intel = caster->intelligence;
	unsigned short totalDamage = (unsigned short)(skill * intel * 8 * 1.2); // 15 , 2.5
	unsigned short damageEach = totalDamage / (numTargets?numTargets:1);
	LinkedElement* element = targetList.head();

	while( element ) {
		WorldObject* obj = (WorldObject*)element->ptr();
		element = element->next();

		unsigned short damage = random((int)(damageEach * 2), (int)(damageEach * 5)); // 8 , 14

		if( obj ) {
			if( strcmp("minotaur", obj->basicName ) ) {
				packet->putByte ( _MOVIE_SPECIAL_EFFECT );
				packet->putLong ( caster->servID );
				packet->putByte ( _SE_POISONCLOUD_TARGET );
				packet->putByte ( 0 );
				packet->putLong ( obj->servID );

				obj->takeDamage( _AFF_DAMAGE_POISON, caster, damage, output, packet, 1 );

				short numRounds = static_cast<short>(((caster->intelligence - obj->endurance) / 2.5));
				if( numRounds < 0 ) numRounds = 0;
				
				// get the target's player state...
				CPlayerState *pPlayerState = obj->character;
				int nMystImmunity = 0;

				if ( pPlayerState ) {
					nMystImmunity = pPlayerState->GetMystImmunityCount();
				}
				
				if( nMystImmunity ) {
					//sprintf ( sizeof ( buf ), buf, "|c22|%s is unaffected, immune. ", obj->getName() );
					sprintf ( sizeof ( buf ), buf, "|c22|%s is not stunned, immune. ", obj->getName() );
					strcat ( output, buf );
				} else if( !obj->hasAffect( _AFF_STUN ) ) {
					
					if( numRounds ) {
						if ( pPlayerState ) {
							pPlayerState->ChangeMystImmunityCount ( numRounds * 2 );
						}

						obj->addAffect( _AFF_STUN, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, numRounds, -1 );
						sprintf( sizeof( buf ), buf, "|c22|%s is stunned for %d round%s. ", obj->getName(), numRounds, (numRounds == 1)?"":"s" );
						strcat ( output, buf );
					} else {
						sprintf( sizeof( buf ), buf, "|c22| is not stunned. ", obj->getName() );
						strcat ( output, buf );
					}
				}

			} else {
				//target resists
				strcat ( output, obj->getName() );
				//strcat ( output, "|c22| is unaffected. " );
				strcat ( output, "|c22| is not stunned. " );
			}
		}
	}

	short numEffects = 5;
	numEffects -= numTargets;
	
	targetList.release(); //this protects from deleting the objects pointed to by the list!

	char effectX;
	char effectY;

	while( numEffects > 0 ) {
		effectX = random( std::max(0, targetX -3), std::min(targetX +3, _COMBAT_GRID_WIDTH) );
		effectY = random( std::max(0, targetY -3), std::min(targetY +3, _COMBAT_GRID_HEIGHT) );
		//effectX = random( 0 >? targetX - 4, targetX + 4 <? _COMBAT_GRID_WIDTH );
		//effectY = random( 0 >? targetY - 4, targetY + 4 <? _COMBAT_GRID_HEIGHT );

		unsigned int servID = caster->combatGroup->grid[effectX][effectY];
			
		if( servID == 0) {
			packet->putByte ( _MOVIE_SPECIAL_EFFECT );
			packet->putLong ( caster->servID );
			packet->putByte ( _SE_POISONCLOUD_AREA );
			packet->putByte ( 0x00 );
			packet->putByte ( effectX );
			packet->putByte ( effectY );

			numEffects--;
		}
	}

	return NULL;
}

//
// castSUMMON_ZOMBIE: create a zombie at the given target location to fight for
// the caster
//

SPELL ( castSUMMON_ZOMBIE )
{
	/* this spell will summon 1 to N zombies where N = caster's proficiency */
	int numZombies = caster->getSkill ( _SKILL_NECROMANCY ) - 1;
	int level = ( calcSpellSkill ( caster, _SKILL_NECROMANCY ) - numZombies + 1) ;

	caster = caster->getBaseOwner();

	summonMonster ( numZombies, "Zombie", "zombie", _SE_SUMMON_ZOMBIE, targetX, targetY, caster, output, packet, level );

	return NULL;
}

//
// castVENOM
//

SPELL ( castVENOM )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target || target->player ) {
		strcat ( output, "Nothing happens. " );
		return NULL;
	}

	int skill = calcSpellSkill (caster, _SKILL_NECROMANCY );

	int duration = 0;

	duration = calcSpellDuration ( caster, 10 * skill, packet );

	caster = caster->getBaseOwner();

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_VENOM );
	packet->putByte ( 1 );
	packet->putLong ( target->getBaseOwner()->servID );

	affect_t *affect = NULL;

	if ( target->getBase ( _BWEAPON ) ) {

		affect = target->hasAffect ( _AFF_DAMAGE_POISON, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL  );

		if ( affect ) {
			affect->duration = duration;
		} else {
			affect = target->addAffect ( _AFF_DAMAGE_POISON, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, duration, 15, packet );
		}

		char buf[1024];
		sprintf ( sizeof ( buf ), buf, "A foul green mist surrounds the %s. ", target->basicName );
		strcat ( output, buf );

	} else {
		strcat ( output, "Nothing happens. " );
	}

	return affect;
}

//
// castSUMMON_UNDEAD
//

SPELL ( castSUMMON_UNDEAD )
{
	/* summon 1 to X Skeletons, Zombies, Ghouls, or Undead Necromancers */
	int skill = caster->getSkill ( _SKILL_NECROMANCY );
	int numCreatures = random ( 1, calcSpellSkill ( caster, _SKILL_NECROMANCY ) );
	LinkedList monsterList;

	caster = caster->getBaseOwner();

	for ( int i=0; i<numCreatures; i++ )
	{
		switch ( random ( 1, skill ) )
		{
			case 1:
				monsterList.add ( new StringObject ( "Zombie" ) );
			break;

			case 2:
				monsterList.add ( new StringObject ( "Skeleton" ) );
			break;

			case 3:
				monsterList.add ( new StringObject ( "LostSailor" ) );
			break;

			case 4:
				monsterList.add ( new StringObject ( "Ghoul" ) );
			break;

			case 5:
				monsterList.add ( new StringObject ( "UndeadNecromancer" ) );
			break;
		}
	}

	summonMonsters ( &monsterList, "undead monster", _SE_SUMMON_UNDEAD, targetX, targetY, caster, output, packet );

	return NULL;
}

//
// castACID_RAIN
//

SPELL ( castACID_RAIN )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target )
		return NULL;

	int distance;
	int skill = calcSpellSkill ( caster, _SKILL_NECROMANCY );
	caster = caster->getBaseOwner();

	if ( !caster || !caster->combatGroup )
		return NULL;

	// set up special effect
	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_ACID_RAIN );
	packet->putByte ( 1 );
	packet->putByte ( targetX );
	packet->putByte ( targetY );

	// do some damage
	LinkedElement *element = caster->combatGroup->combatants.head();

	while ( element )
	{
		WorldObject *object = (WorldObject *)element->ptr();
		element = element->next();

		// see if this guy is near the impact
		distance = getDistance ( targetX, targetY, object->combatX, object->combatY );

		if ( distance < 3 )
		{
			// this guy should take damage
			int damage = random ( 21*skill, 41*skill );		// 21 , 41
			object->takeDamage ( _AFF_DAMAGE_ACID, caster, damage, output, packet, 1 );
		}
	}
	return NULL;
}

//
// castNIGHT_FRIENDS
//

SPELL ( castNIGHT_FRIENDS )
{
	/* summon 1 to X Bats, Rats, Wolves */
	int skill = caster->getSkill ( _SKILL_NECROMANCY );
	int numCreatures = random ( 1, calcSpellSkill ( caster, _SKILL_NECROMANCY ) );
	LinkedList monsterList;

	caster = caster->getBaseOwner();

	for ( int i=0; i<numCreatures; i++ )
	{
		switch ( random ( 1, skill ) )
		{
			case 1:
				/* summon a Wood Rat */
				monsterList.add ( new StringObject ( "WoodRat" ) );

			break;

			case 2:
				/* summon a Flying Rat */
				monsterList.add ( new StringObject ( "FlyingRat" ) );
			break;

			case 3:
				/* summon a Swamp Rat */
				monsterList.add ( new StringObject ( "SwampRat" ) );
			break;

			case 4:
				/* summon a Crypt Bat */
				monsterList.add ( new StringObject ( "CryptBat" ) );
			break;

			case 5:
				/* summon a Rabid Wolf */
				monsterList.add ( new StringObject ( "RabidWolf" ) );
			break;
		}
	}

	summonMonsters ( &monsterList, "night friend", _SE_SUMMON_NIGHT_FRIENDS, targetX, targetY, caster, output, packet );

	return NULL;
}

//
// castSTEAL_LIFE
//

SPELL ( castSTEAL_LIFE )
{
	WorldObject *target = roomMgr->findObject ( targetServID );
	
	if ( !target || !target->player ) {
		strcat ( output, "Nothing happens. " );
		return NULL;
	}

	int skill = calcSpellSkill ( caster, _SKILL_NECROMANCY );
	//int damage = 40 * skill; 
	int damage = random ( 26*skill, 48*skill );

	
	caster = caster->getBaseOwner();
	int distance = getDistance ( caster->combatX, caster->combatY, target->combatX, target->combatY );

	if ( distance < gTouchDistance )
	{
		packet->putByte ( _MOVIE_SPECIAL_EFFECT );
		packet->putLong ( caster->servID );
		packet->putByte ( _SE_STEAL_LIFE );
		packet->putByte ( 1 );
		packet->putLong ( target->servID );

		target->takeDamage ( _AFF_DAMAGE_STEAL_LIFE, caster, damage, output, packet, 1 );

		caster->takeDamage ( WorldObject::_DAMAGE_HEAL, caster, -damage, output, packet );
	}
	else
	{
		char buf[1024];
		sprintf ( sizeof ( buf ), buf, "The spell fails because %s is not close enough to touch! ", target->getName() );
		strcat ( output, buf );
	}

	return NULL;
}

//
// cast118 (Crawling Charge)
//

SPELL ( cast118 )
{
	//This spell will affect the area surrounding the caster, but will not harm the caster.
	//It affects an area from 5 left to 5 right of the caster, and from 4 above to 4 below.
	//It does damage approximately equally to every combatant (friend or foe) in that area.
	//
	//If there are not at least 5 combatants that it hurts, it generates special effects
	//in random unoccupied squares, for effect.
	//

	//heres a list of player targets
	LinkedList targetList;

	short casterX = caster->combatX;
	short casterY = caster->combatY;

	caster = caster->getBaseOwner();
	char skill = caster->getSkill( _SKILL_ELEMENTALISM );
	char intel = caster->intelligence;

	for(short combatX = casterX - 5; combatX <= casterX + 5; ++combatX)
	{
		if( combatX < 0 ) continue;
		else if( combatX >= _COMBAT_GRID_WIDTH ) continue;

		for(short combatY = casterY - 4; combatY <= casterY + 4; ++combatY)
		{
			if (combatY < 0) continue;
			else if( combatY >= _COMBAT_GRID_HEIGHT ) continue;
			if( combatX == casterX && combatY == casterY ) continue;
            
			//get the servID of the object in this square
			unsigned int servID = caster->combatGroup->grid[combatX][combatY];

			if( servID <= 1 ) continue;
			if( servID == caster->servID ) continue;

			//add it to our target list if there is a valid servID
			if( servID ) {
				WorldObject* obj = roomMgr->getObject( servID );
				if( obj && obj->character )
					targetList.add( obj );
				else logDisplay( "%s:%d castAREA_LIGHTNING - target error. servid %d, obj %d  obj->character %d", __FILE__, __LINE__, servID, (int)obj, obj?(int)obj->character:0 );
			} 
		}
	}

	char numTargets = targetList.size();

	unsigned short totalDamage = skill * intel * 14;	// 15
	unsigned short damageEach = totalDamage / (numTargets?numTargets:1);
	LinkedElement* element = targetList.head();

	while( element ) {
		WorldObject* obj = (WorldObject*)element->ptr();
		element = element->next();

		unsigned short damage = random((int)(damageEach * .9), (int)(damageEach * 1.1));
		if( obj ) {
			packet->putByte ( _MOVIE_SPECIAL_EFFECT );
			packet->putLong ( caster->servID );
			packet->putByte ( _SE_LIGHTNING );
			packet->putByte ( 0 );
			packet->putLong ( obj->servID );

			obj->takeDamage( _AFF_DAMAGE_LIGHTNING, caster, damage, output, packet, 1 );
		}
	}

	short numEffects = 5;
	numEffects -= numTargets;
	
	targetList.release(); //this protects from deleting the objects pointed to by the list!

	char effectX;
	char effectY;

	while( numEffects > 0 ) {
		effectX = random( std::max(0, targetX -4), std::min(targetX +4, _COMBAT_GRID_WIDTH) );
		effectY = random( std::max(0, targetY -4), std::min(targetY +4, _COMBAT_GRID_HEIGHT) );
		//effectX = random( 0 >? casterX - 5, casterX + 5 <? _COMBAT_GRID_WIDTH );
		//effectY = random( 0 >? casterY - 4, casterY + 4 <? _COMBAT_GRID_HEIGHT );

		unsigned int servID = caster->combatGroup->grid[effectX][effectY];
			
		if( servID == 0) {
			packet->putByte ( _MOVIE_SPECIAL_EFFECT );
			packet->putLong ( caster->servID );
			packet->putByte ( _SE_LIGHTNING_BOLT );
			packet->putByte ( 0x00 );
			packet->putByte ( effectX );
			packet->putByte ( effectY );

			numEffects--;
		}
	}

	return NULL;
}

//
// castSUMMON_DOPPELGANGER
//

SPELL(castSUMMON_DOPPELGANGER)
{
  if (!caster || !caster->opposition)
    return NULL;

  LinkedList items, monsters;
  LinkedElement *element = caster->opposition->head();
  int summonCount = 0;
  int skill = calcSpellSkill(caster, _SKILL_NECROMANCY);

  if (element)
  {
    for (element = ((WorldObject *)element->ptr())->opposition->head(); element; element = element->next())
    {
      WorldObject *object = (WorldObject *)element->ptr();
      if (object->summoned)
        ++summonCount;
    }
  }
  if (summonCount >= 5)
  {
    char buf[1024];
    sprintf(sizeof(buf), buf, "|c60|The spell fails because there are already five summoned monsters on %s's side.|c43| ", caster->getName());
    strcat(output, buf);
    return(NULL);
  }

  if (targetX < 0)
    targetX = 0;
  else if (targetX > 23)
    targetX = 23;

  if (targetY < 0)
    targetY = 0;
  else if (targetY > 17)
    targetY = 17;

  int lastX = targetX, lastY = targetY;

  //  make summoned monster an exact copy of original
  WorldObject *monster = new WorldObject(roomMgr->findClass("NPC"));
  monster->copy(caster);
  monster->player->isNPC;
  monster->minLevel = caster->level;
  monster->maxLevel = caster->level;

  //  destroy non equipped items in inventory
  BContainer *container = (BContainer *)monster->getBase(_BCONTAIN);
  if (container)
    for (element = container->contents.head(); element; )
    {
      WorldObject *item = (WorldObject *)element->ptr();
      element = element->next();
      if (!item->isWornBy(monster) && !item->getBase(_BHEAD))
        roomMgr->destroyObj(item, 0, __FILE__, __LINE__);
    }

  //  remove non permanent and other unwanted effects
  if (monster->activeAffects)
    for (element = monster->activeAffects->head(); element; )
    {
      affect_t *affect = (affect_t *)element->ptr();
      element = element->next();
      if (affect->source == _AFF_SOURCE_SPELL || affect->source == _AFF_SOURCE_POTION || (affect->id >= _AFF_FREEZE && affect->id <= _AFF_INVULNERABLE) ||
          affect->id == _AFF_MAGIC_RESISTANCE || affect->id == _AFF_MAGIC_IMMUNITY || (affect->id >= _AFF_POS_DEX_MOD && affect->id <= _AFF_RETENTION))
        monster->delAffect(affect, packet);
    }
  monster->addAffect(_AFF_RETENTION, _AFF_TYPE_NORMAL, _AFF_SOURCE_ARTIFACT, -1, 0, NULL, 0);


  
  double ratio = ((double)skill * log10((skill > 10) ? (double)skill : 10)) / 1200;
  if (ratio > .5)
    ratio = .5;

  //  reduce attributes
  monster->strength = caster->calcStrength();
  //monster->strength = (int)ceil((double)monster->strength * ratio);
  monster->dexterity = caster->calcDexterity();
  //monster->dexterity = (int)ceil((double)monster->dexterity * ratio);
  monster->intelligence = caster->calcIntelligence();
  //monster->intelligence = (int)ceil((double)monster->intelligence * ratio);
  monster->endurance = caster->calcEndurance();
  //if (monster->endurance > 30)
  //  monster->endurance = 30;
  //monster->endurance = (int)ceil((double)monster->endurance * ratio);
  monster->quickness = caster->calcDexterity();

  //  reduce unarmed damage
  //if (monster->hands)
  //{
  //  monster->hands->minDamage = caster->hands->minDamage;
  //  if (monster->hands->minDamage > 2000)
  //    monster->hands->minDamage = 2000;
  //  monster->hands->minDamage = (int)ceil((double)monster->hands->minDamage * ratio);
  //  monster->hands->maxDamage = caster->hands->maxDamage;
  //  if (monster->hands->maxDamage > 4000)
  //    monster->hands->maxDamage = 4000;
  //  monster->hands->maxDamage = (int)ceil((double)monster->hands->maxDamage * ratio);
  //}

  if (monster->character)
  {
    //  reduce spell resistance
    monster->character->ChangeSpellResistance(0, (int)-(monster->character->m_anSpellResistance[0] * (1 - ratio)));
    monster->character->ChangeSpellResistance(1, (int)-(monster->character->m_anSpellResistance[1] * (1 - ratio)));
    monster->character->ChangeSpellResistance(2, (int)-(monster->character->m_anSpellResistance[2] * (1 - ratio)));
    monster->character->ChangeSpellResistance(3, (int)-(monster->character->m_anSpellResistance[3] * (1 - ratio)));
    monster->character->ChangeSpellResistance(4, (int)-(monster->character->m_anSpellResistance[4] * (1 - ratio)));
  }

  monster->addToDatabase();
  monster->summoned = caster->servID;
  NPC *npc = makeNPC(monster);
  roomMgr->addObject(monster);

  CombatGrid grid;
  int theX = targetX, theY = targetY;

  grid.mapAccessible(caster->combatGroup, targetX, targetY);
  grid.findClosestPoint(lastX, lastY, caster->combatX, caster->combatY, &theX, &theY);

  monster->combatX = theX;
  monster->combatY = theY;
  lastX = theX;
  lastY = theY;
  monster->x = (monster->combatX * 26) + 13 + 5;
  monster->y = (monster->combatY * 11) + 5 + 110;

  caster->combatGroup->addCharacter(monster, caster->combatGroup->attackers.contains(caster) ? _COMBAT_ATTACKER : _COMBAT_DEFENDER);
  npc->room = caster->room;
  npc->zone = npc->room->zone;
  monster->room = caster->room;
  monster->hidden++;
  caster->room->addPlayer(npc, packet);
  monster->hidden--;
  monsters.add(monster);

  // enable NPC AI
  npc->aiReady = 1;

  packet->putByte(_MOVIE_SPECIAL_EFFECT);
  packet->putLong(caster->servID);
  packet->putByte(_SE_SUMMON_ZOMBIE);
  packet->putByte(1);
  packet->putByte(1);

  for (element = monsters.head(); element; element = element->next())
  {
    WorldObject *obj = (WorldObject *)element->ptr();
    packet->putLong(obj->servID);
  }

  //int amount = (int)((double)monster->healthMax * ((double)caster->health / (double)caster->healthMax));
  //amount = monster->healthMax - amount;
  //monster->changeHealth(-amount, monster, 1, 1, packet);

  char buf[1024];
  sprintf(sizeof(buf), buf, "|c22|A mirror image of %s appears. ", caster->getName());
  strcat(output, buf);

  //  debugging
  /*sprintf(sizeof(buf), buf, "\nstr:  %d, dex:  %d, int:  %d,  end:  %d, qui:  %d, lev:  %d, hp:  %d / %d\n",
        monster->calcStrength(), monster->calcDexterity(), monster->calcIntelligence(), monster->calcEndurance(),
        monster->quickness, monster->level, monster->health, monster->healthMax);
    strcat(output, buf);*/

  monsters.release();
  return NULL;
}

//
// castDEATH_TOUCH
//

SPELL ( castDEATH_TOUCH )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target || !target->player ) {
		strcat ( output, "Nothing happens. " );
		return NULL;
	}

	int skill = calcSpellSkill ( caster, _SKILL_NECROMANCY );

	caster = caster->getBaseOwner();

	if ( !target->hasAffect ( _AFF_MARK_ENID ) ) {
		//int damage = 40 * skill;
		int damage = random ( 42*skill, 46*skill );

		if ( target->coarseAlignment() == _ALIGN_GOOD ) {
			damage *= 2.5;
		}

		target->takeDamage ( _AFF_DAMAGE_STEAL_LIFE, caster, damage, output, packet, 1 );
  	} else {
		strcat ( output, target->getName() );
		strcat ( output, " is unaffected by the death touch! " );
	}

	return NULL;
}

//
// castBANISH
//

SPELL ( castBANISH )
{
	WorldObject *target = roomMgr->findObject ( targetServID );
	caster = caster->getBaseOwner();

	if ( !target || !caster || !caster->opposition )
		return NULL;

	if ( target->getBase ( _BNPC ) && target->summoned ) {
		/* banish this monster */
		packet->putByte ( _MOVIE_SPECIAL_EFFECT );
		packet->putLong ( caster->servID );
		packet->putByte ( _SE_BANISHMENT );
		packet->putByte ( 1 );
		packet->putByte ( 1 );
		packet->putLong ( target->servID );
		target->takeDamage ( _AFF_DAMAGE_STEAL_LIFE, caster, target->health, output, packet, 1 );
	} else {
		char buf[1024];
		sprintf ( sizeof ( buf ), buf,"Nothing happens because %s is not a summoned monster. ", target->getName() );
		strcat ( output, buf );
	}
	return NULL;
}

//
// castSHADOW_WARRIOR
//

SPELL ( castSHADOW_WARRIOR )
{
	caster = caster->getBaseOwner();

	summonMonster ( 1, "Wraith", "wraith", _SE_SUMMON_WRAITH, targetX, targetY, caster, output, packet );

	return NULL;
}

//
// castSUMMON_DAEMON
//

SPELL ( castSUMMON_DAEMON )
{
	caster = caster->getBaseOwner();

	summonMonster ( 1, "Daemon", "daemon", _SE_SUMMON_DAEMON, targetX, targetY, caster, output, packet );

	return NULL;
}

//
// castDUACHS_VENGEANCE
//

SPELL ( castDUACHS_VENGEANCE )
{
	WorldObject *target = roomMgr->findObject ( targetServID );
	
	if ( !target || !target->player ) {
		strcat ( output, "Nothing happens." );
		return NULL;
	}

	
	int skill = calcSpellSkill ( caster, _SKILL_NECROMANCY );

	char buf[1024];
	sprintf ( sizeof ( buf ), buf, "|c14|Duach wreaks his vengeance on %s!! ", target->getName());
	strcat ( output, buf );

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_DUACHS_VENGEANCE );
	packet->putByte ( 1 );
	packet->putLong ( target->servID );

	//int damage = random ( skill * 45, skill * 65 );
	int damage = random ( skill * 85, skill * 125 ); // 90 , 152
	target->takeDamage ( _AFF_DAMAGE_FIRE, caster, damage, output, packet, 1 );

	return NULL;
}

//
// castDEATH_WISH
//

SPELL ( castDEATH_WISH )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target || !target->player ) {
		strcat ( output, "Nothing happens. " );
		return NULL;
	}

	char buf[1024];
	int skill = calcSpellSkill ( caster, _SKILL_NECROMANCY );

	caster = caster->getBaseOwner();

  	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
  	packet->putLong ( caster->servID );
  	packet->putByte ( _SE_DEATH_WISH );
  	packet->putByte ( 1 );
  	packet->putLong ( target->servID );

	if ( !target->hasAffect ( _AFF_MARK_ENID ) ) {
		//int damage = 35 * skill;
		int damage = random ( skill * 42, skill * 48 );

		if ( target->coarseAlignment() == _ALIGN_GOOD ) {
			damage *= 3;
		}

		target->takeDamage ( _AFF_DAMAGE_STEAL_LIFE, caster, damage, output, packet, 1 );
  	} else {
 		sprintf ( sizeof ( buf ), buf, "%s is unaffected by the death wish! ", target->getName() );
		strcat ( output, buf );
	}

	return NULL;
}

//
// castMASS_DRAIN
//

SPELL ( castMASS_DRAIN )
{
	caster = caster->getBaseOwner();

	if ( !caster || !caster->opposition || !caster->opposition->size() )
		return NULL;

	int drainTotal = 0;
	int skill = calcSpellSkill ( caster, _SKILL_NECROMANCY );

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_MASS_DRAIN );
	packet->putByte ( 1 );
	packet->putByte ( caster->opposition->size() );

	LinkedElement *element = caster->opposition->head();

	// build effect
	while ( element ) {

		WorldObject *target = (WorldObject *)element->ptr();
		element = element->next();
		packet->putLong ( target->servID );
	}

	element = caster->opposition->head();

	while ( element ) {
		WorldObject *target = (WorldObject *)element->ptr();
		element = element->next();

		CPlayerState *pPlayerState = target->character;
		int nResisted = 0;
	
		if ( pPlayerState ) {
			if ( pPlayerState->TestSpellResistance ( _SKILL_NECROMANCY - _SKILL_SORCERY ) ) {
				nResisted = 1;
	
				packet->putByte ( _MOVIE_SPECIAL_EFFECT );
				packet->putLong ( caster->servID );
				packet->putByte ( _SE_SPELL_BLAST );
				packet->putByte ( 0 );
				packet->putByte ( 1 );
				packet->putLong ( target->servID );
			}
		}
	
		if ( !nResisted ) {
			int damage = random ( skill * 21, skill * 32 );		// 21 , 32
			drainTotal += damage;
			target->takeDamage ( _AFF_DAMAGE_STEAL_LIFE, caster, damage, output, packet, 1 );
		}
	}

	// heal caster
	if ( drainTotal )
		caster->takeDamage ( WorldObject::_DAMAGE_HEAL, caster, -(std::max(1, (drainTotal /2))), output, packet );
//-(std::max(1, (drainTotal /2))
//-( 1 >? (drainTotal / 2) ), output, packet );

	return NULL;
}

//
// castPOISON_CURSE
//

SPELL ( castPOISON_CURSE )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target )
		return NULL;

	char buf[1024];
	int skill = calcSpellSkill ( caster, _SKILL_NECROMANCY );

	int duration = 0;

	duration = calcSpellDuration ( caster, 3 * skill, packet );

	caster = caster->getBaseOwner();

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_POISON_CURSE );
	packet->putByte ( 1 );
	packet->putLong ( target->servID );

	affect_t *affect = target->hasAffect ( _AFF_DAMAGE_POISON, _AFF_TYPE_WEAKNESS, _AFF_SOURCE_SPELL );

	if ( affect ) {
		affect->duration = duration;
	} else {
		affect = target->addAffect ( _AFF_DAMAGE_POISON, _AFF_TYPE_WEAKNESS, _AFF_SOURCE_SPELL, duration, 15, packet );
	}

	if ( !target->player ) {
		sprintf ( sizeof ( buf ), buf, "|c21|The %s is surrounded by a magical aura! ", target->getName() );
		strcat ( output, buf );
	} else {
		sprintf ( sizeof ( buf ), buf, "|c248|%s has been cursed with a weakness to poison!", target->getName() );
		strcat ( output, buf );
	}
	return affect;
}

//
// castACID_CURSE
//

SPELL ( castACID_CURSE )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target )
		return NULL;

	char buf[1024];
	int skill = calcSpellSkill ( caster, _SKILL_NECROMANCY );

	int duration = 0;

	duration = calcSpellDuration ( caster, 3 * skill, packet );

	caster = caster->getBaseOwner();

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_ACID_CURSE );
	packet->putByte ( 1 );
	packet->putLong ( target->servID );

	affect_t *affect = target->hasAffect ( _AFF_DAMAGE_ACID, _AFF_TYPE_WEAKNESS, _AFF_SOURCE_SPELL );

	if ( affect ) {
		affect->duration = duration;
	} else {
		affect = target->addAffect ( _AFF_DAMAGE_ACID, _AFF_TYPE_WEAKNESS, _AFF_SOURCE_SPELL, duration, 15, packet );
	}

	if ( !target->player ) {
		sprintf ( sizeof ( buf ), buf, "|c21|The %s is surrounded by a magical aura! ", target->getName() );
		strcat ( output, buf );
	} else {
		sprintf ( sizeof ( buf ), buf, "|c248|%s has been cursed with a weakness to acid!", target->getName() );
		strcat ( output, buf );
	}
	return affect;
}

//
// castPOISON_SHIELD
//

SPELL ( castPOISON_SHIELD )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target )
		return NULL;

	char buf[1024];
	int skill = calcSpellSkill ( caster, _SKILL_THAUMATURGY );

	int duration = 0;

	duration = calcSpellDuration ( caster, 3 * skill, packet );

	caster = caster->getBaseOwner();

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_POISON_SHIELD );
	packet->putByte ( 1 );
	packet->putLong ( target->servID );

	affect_t *affect = target->hasAffect ( _AFF_DAMAGE_POISON, _AFF_TYPE_RESISTANCE, _AFF_SOURCE_SPELL );

	if ( affect ) {
		affect->duration = duration;
	} else {
		affect = target->addAffect ( _AFF_DAMAGE_POISON, _AFF_TYPE_RESISTANCE, _AFF_SOURCE_SPELL, duration, 15, packet );
	}

	if ( !target->player ) {
		sprintf ( sizeof ( buf ), buf, "|c21|The %s is surrounded by a poison shield! ", target->getName() );
		strcat ( output, buf );
	} else {
		sprintf ( sizeof ( buf ), buf, "A poison shield surrounds %s! ", target->getName() );
		strcat ( output, buf );
	}

	return affect;
}

//
// castACID_SHIELD
//

SPELL ( castACID_SHIELD )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target )
		return NULL;

	char buf[1024];
	int skill = calcSpellSkill ( caster, _SKILL_THAUMATURGY );

	int duration = 0;

	duration = calcSpellDuration ( caster, 3 * skill, packet );

	caster = caster->getBaseOwner();

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_ACID_SHIELD );
	packet->putByte ( 1 );
	packet->putLong ( target->servID );

	affect_t *affect = target->hasAffect ( _AFF_DAMAGE_ACID, _AFF_TYPE_RESISTANCE, _AFF_SOURCE_SPELL );

	if ( affect ) {
		affect->duration = duration;
	} else {
		affect = target->addAffect ( _AFF_DAMAGE_ACID, _AFF_TYPE_RESISTANCE, _AFF_SOURCE_SPELL, duration, 15, packet );
	}

	if ( !target->player ) {
		sprintf ( sizeof ( buf ), buf, "An acid shield surrounds the %s! ", target->getName() );
		strcat ( output, buf );
	} else {
		sprintf ( sizeof ( buf ), buf, "An acid shield surrounds %s! ", target->getName() );
		strcat ( output, buf );
	}

	return affect;
}

//mike - repels everything away from the caster
SPELL ( castREPEL )
{
	char buf[300];

	caster = caster->getBaseOwner();

	short casterX = caster->combatX;
	short casterY = caster->combatY;

	int destX;
	int destY;

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_SHIELD_UP );
	packet->putByte ( 0 );
	packet->putLong ( caster->servID );

	LinkedElement* combatantElement = caster->combatGroup->combatants.head();

	while( combatantElement ) {
		WorldObject* combatant = (WorldObject*) combatantElement->ptr();
		combatantElement = combatantElement->next();

		if( combatant == caster ) continue;

		short combatantX = combatant->combatX;
		short combatantY = combatant->combatY;
		bool repelSuccess = false;

		short repelAffectDistance = 6; //we only affect combatants within this range of me
		short repelDistance = 12; //we push them back so they are this far away //10
		
		float dist = sqrt( ((float)(casterX - combatantX)*(casterX - combatantX)) + ((float)(casterY - combatantY)*(casterY - combatantY)) );
		if( dist > repelDistance ) {
			//if the combatant is outside our repel circle, we dont care about them.
			//sprintf ( sizeof ( buf ), buf, "Repel of %s from %s failed, too far away (dist = %f)! ", combatant->getName(), caster->getName(), dist );
			//strcat ( output, buf );
			continue; 
		}

		
		//calculate destination X,Y based on 8 repel directions
		if( combatantY > casterY ) {
			destY = casterY + repelDistance;
			if( combatantX > casterX ) {
				destX = casterX + repelDistance;
			} else if( combatantX < casterX ) {
				destX = casterX - repelDistance;
			} else {
				destX = casterX;
			}

		} else if( combatantY < casterY ) {
			destY = casterY - repelDistance;
			if( combatantX > casterX )
				destX = casterX + repelDistance;
			else if (combatantX < casterX )
				destX = casterX - repelDistance;
			else
				destX = casterX;

		} else { //combatantY == casterY
			destY = casterY;
			
			if( combatantX > casterX )
                destX = casterX + repelDistance;
			else if( combatantX < casterX )
				destX = casterX - repelDistance;
			else
				logInfo( _LOG_ALWAYS, "castREPEL() Error - Combatant %s(%d) occupies same square as caster %s(%d).", combatant->getName(), combatant->servID, caster->getName(), caster->servID );
		}

		if( destX < 0 ) destX = 0;
		else if( destX >= _COMBAT_GRID_WIDTH  ) destX = _COMBAT_GRID_WIDTH  - 1;
		if( destY < 0 ) destY = 0;
		else if( destY >= _COMBAT_GRID_HEIGHT ) destY = _COMBAT_GRID_HEIGHT - 1;

		if( combatant->combatGroup->positionCharacter( combatant, destX, destY ) == -1 ) {
			// the square at destX, destY is occupied. find a square near it to move to.
			combatant->combatGroup->findClosestPoint( destX, destY, casterX, casterY, &destX, &destY );
			
			if( combatant->combatGroup->positionCharacter( combatant, destX, destY ) == -1 ) {
				//we still failed. this is just not going to work for us.
				logInfo( _LOG_ALWAYS, "%s:%d SPELL(castRepel) - positionCharacter failed. Obj(%s of %s, servID %d) at (%d,%d), grid servID %d. Destination (%d, %d) with servID %d", __FILE__, __LINE__, combatant->getName(), combatant->classID, combatant->servID, combatant->combatX, combatant->combatY, combatant->combatGroup->grid[combatant->combatX][combatant->combatY], destX, destY, combatant->combatGroup->grid[destX][destY] );
				repelSuccess = false;
				//try to restore the position just in case.
				combatant->combatGroup->positionCharacter( combatant, combatantX, combatantY );
			} else repelSuccess = true;
		} else repelSuccess = true;

		if( repelSuccess ) {
			packet->putByte ( _MOVIE_SPECIAL_EFFECT );
			packet->putLong ( combatant->servID );
			packet->putByte ( _SE_SLIDE_COMBATANT );
			packet->putByte ( 0 ); //async
			packet->putLong ( combatant->servID ); //target
			packet->putWord ( destX ); //dest combat square
			packet->putWord ( destY );

			sprintf ( sizeof ( buf ), buf, "%s is repelled from %s! ", combatant->getName(), caster->getName() );
			strcat ( output, buf );
		} else {
			sprintf ( sizeof ( buf ), buf, "Repel of %s from %s failed, too crowded! ", combatant->getName(), caster->getName() );
			strcat ( output, buf );
		}
	}

	return NULL;
}

SPELL( castBURNING_MISTCLOUD )
{
	char buf[100];

	//heres a list of player targets
	LinkedList targetList;

	for(short combatX = targetX - 4; combatX <= targetX + 4; ++combatX)
	{
		if( combatX < 0 ) continue;
		else if( combatX >= _COMBAT_GRID_WIDTH ) continue;

		for(short combatY = targetY - 4; combatY <= targetY + 4; ++combatY)
		{
			if (combatY < 0) continue;
			else if( combatY >= _COMBAT_GRID_HEIGHT ) continue;
            
			//get the servID of the object in this square
			unsigned int servID = caster->combatGroup->grid[combatX][combatY];

			if( servID <= 1 ) continue;
			
			//add it to our target list if there is a valid servID
			//and they are not a minotaur (resistant)
			if( servID ) {
				WorldObject* obj = roomMgr->getObject( servID );
				
				if( obj && obj->character )
					targetList.add( obj );
				else logDisplay( "%s:%d castAREA_POISON - target error. servid %d, obj %d  obj->character %d", __FILE__, __LINE__, servID, (int)obj, obj?(int)obj->character:0 );
			}
		}
	}

	char numTargets = targetList.size();

	char skill = caster->getSkill( _SKILL_NECROMANCY );
	char intel = caster->intelligence;
	unsigned short totalDamage = skill * intel * 15;
	unsigned short damageEach = totalDamage / (numTargets?numTargets:1);
	LinkedElement* element = targetList.head();

	while( element ) {
		WorldObject* obj = (WorldObject*)element->ptr();
		element = element->next();

		unsigned short damage = random((int)(damageEach * .9), (int)(damageEach * 1.1));
		if( obj ) {
			if( strcmp("minotaur", obj->basicName ) ) {
				packet->putByte ( _MOVIE_SPECIAL_EFFECT );
				packet->putLong ( caster->servID );
				packet->putByte ( _SE_POISONCLOUD_TARGET );
				packet->putByte ( 0 );
				packet->putLong ( obj->servID );

				obj->takeDamage( _AFF_DAMAGE_POISON, caster, damage, output, packet, 1 );

				short numRounds = static_cast<short>((caster->intelligence * .12));
				if( numRounds < 0 ) numRounds = 0;

				if( numRounds ) { 
					if( !obj->hasAffect( _AFF_ENCUMBERANCE_CURSE ) ) {
						obj->addAffect( _AFF_ENCUMBERANCE_CURSE, _AFF_TYPE_WEAKNESS, _AFF_SOURCE_SPELL, numRounds, -1 );
						sprintf( sizeof( buf ), buf, "%s is cursed with an encumberance penalty for %d round%s. ", obj->getName(), numRounds, (numRounds == 1)?"":"s" );
						strcat ( output, buf );
					}
				} else {
					strcat ( output, obj->getName() );
					strcat ( output, " is unaffected. " );
				}

			} else {
				//target resists
				strcat ( output, obj->getName() );
				strcat ( output, " is unaffected. " );
			}
		}
	}

	short numEffects = 5;
	numEffects -= numTargets;
	
	targetList.release(); //this protects from deleting the objects pointed to by the list!

	char effectX;
	char effectY;

	while( numEffects > 0 ) {
		effectX = random( std::max(0, targetX -4), std::min(targetX +4, _COMBAT_GRID_WIDTH) );
		effectY = random( std::max(0, targetY -4), std::min(targetY +4, _COMBAT_GRID_HEIGHT) );
		//effectX = random( 0 >? targetX - 4, targetX + 4 <? _COMBAT_GRID_WIDTH );
		//effectY = random( 0 >? targetY - 4, targetY + 4 <? _COMBAT_GRID_HEIGHT );

		unsigned int servID = caster->combatGroup->grid[effectX][effectY];
			
		if( servID == 0) {
			packet->putByte ( _MOVIE_SPECIAL_EFFECT );
			packet->putLong ( caster->servID );
			packet->putByte ( _SE_POISONCLOUD_AREA );
			packet->putByte ( 0x00 );
			packet->putByte ( effectX );
			packet->putByte ( effectY );

			numEffects--;
		}
	}

	return NULL;
}

SPELL( castNAUSEATING_MISTCLOUD )
{
	char buf[100];

	//heres a list of player targets
	LinkedList targetList;

	for(short combatX = targetX - 4; combatX <= targetX + 4; ++combatX)
	{
		if( combatX < 0 ) continue;
		else if( combatX >= _COMBAT_GRID_WIDTH ) continue;

		for(short combatY = targetY - 4; combatY <= targetY + 4; ++combatY)
		{
			if (combatY < 0) continue;
			else if( combatY >= _COMBAT_GRID_HEIGHT ) continue;
            
			//get the servID of the object in this square
			unsigned int servID = caster->combatGroup->grid[combatX][combatY];

			if( servID <= 1 ) continue;
			
			//add it to our target list if there is a valid servID
			//and they are not a minotaur (resistant)
			if( servID ) {
				WorldObject* obj = roomMgr->getObject( servID );
				
				if( obj && obj->character )
					targetList.add( obj );
				else logDisplay( "%s:%d castAREA_POISON - target error. servid %d, obj %d  obj->character %d", __FILE__, __LINE__, servID, (int)obj, obj?(int)obj->character:0 );
			}
		}
	}

	char numTargets = targetList.size();

	char skill = caster->getSkill( _SKILL_NECROMANCY );
	char intel = caster->intelligence;
	unsigned short totalDamage = skill * intel * 15;
	unsigned short damageEach = totalDamage / (numTargets?numTargets:1);
	LinkedElement* element = targetList.head();

	while( element ) {
		WorldObject* obj = (WorldObject*)element->ptr();
		element = element->next();

		unsigned short damage = random((int)(damageEach * .9), (int)(damageEach * 1.1));

		if( obj ) {
			if( strcmp("minotaur", obj->basicName ) ) {
				packet->putByte ( _MOVIE_SPECIAL_EFFECT );
				packet->putLong ( caster->servID );
				packet->putByte ( _SE_POISONCLOUD_TARGET );
				packet->putByte ( 0 );
				packet->putLong ( obj->servID );

				obj->takeDamage( _AFF_DAMAGE_POISON, caster, damage, output, packet, 1 );

				short numRounds = static_cast<short>((caster->intelligence + (caster->endurance * .3)) * .07);
				if( numRounds < 0 ) numRounds = 0;

				// get the target's player state...
				CPlayerState *pPlayerState = obj->character;
				int nMystImmunity = 0;

				if ( pPlayerState ) {
					nMystImmunity = pPlayerState->GetMystImmunityCount();
				}
				
				if( !nMystImmunity && !obj->hasAffect( _AFF_STUN ) ) {

					if( numRounds ) {
						obj->addAffect( _AFF_STUN, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, numRounds, -1 );
						sprintf( sizeof( buf ), buf, "|c22|%s is stunned for %d round%s. ", obj->getName(), numRounds, (numRounds == 1)?"":"s" );
						strcat ( output, buf );
					} else {
						sprintf( sizeof( buf ), buf, "|c22|%s is unaffected. ", obj->getName() );
						strcat ( output, buf );
					}
				} else {
					sprintf( sizeof( buf ), buf, "|c22|%s is unaffected. ", obj->getName() );
					strcat ( output, buf );
				}

			} else {
				//target resists
				strcat ( output, obj->getName() );
				strcat ( output, "|c22| is unaffected. " );
			}
		}
	}

	short numEffects = 5;
	numEffects -= numTargets;
	
	targetList.release(); //this protects from deleting the objects pointed to by the list!

	char effectX;
	char effectY;

	while( numEffects > 0 ) {
		effectX = random( std::max(0, targetX -4), std::min(targetX +4, _COMBAT_GRID_WIDTH) );
		effectY = random( std::max(0, targetY -4), std::min(targetY +4, _COMBAT_GRID_HEIGHT) );
		//effectX = random( 0 >? targetX - 4, targetX + 4 <? _COMBAT_GRID_WIDTH );
		//effectY = random( 0 >? targetY - 4, targetY + 4 <? _COMBAT_GRID_HEIGHT );

		unsigned int servID = caster->combatGroup->grid[effectX][effectY];
			
		if( servID == 0) {
			packet->putByte ( _MOVIE_SPECIAL_EFFECT );
			packet->putLong ( caster->servID );
			packet->putByte ( _SE_POISONCLOUD_AREA );
			packet->putByte ( 0x00 );
			packet->putByte ( effectX );
			packet->putByte ( effectY );

			numEffects--;
		}
	}

	return NULL;
}

SPELL( castDEADLY_MISTCLOUD )
{
	char buf[100];

	//heres a list of player targets
	LinkedList targetList;

	for(short combatX = targetX - 4; combatX <= targetX + 4; ++combatX)
	{
		if( combatX < 0 ) continue;
		else if( combatX >= _COMBAT_GRID_WIDTH ) continue;

		for(short combatY = targetY - 4; combatY <= targetY + 4; ++combatY)
		{
			if (combatY < 0) continue;
			else if( combatY >= _COMBAT_GRID_HEIGHT ) continue;
            
			//get the servID of the object in this square
			unsigned int servID = caster->combatGroup->grid[combatX][combatY];

			if( servID <= 1 ) continue;
			
			//add it to our target list if there is a valid servID
			//and they are not a minotaur (resistant)
			if( servID ) {
				WorldObject* obj = roomMgr->getObject( servID );
				
				if( obj && obj->character )
					targetList.add( obj );
				else logDisplay( "%s:%d castAREA_POISON - target error. servid %d, obj %d  obj->character %d", __FILE__, __LINE__, servID, (int)obj, obj?(int)obj->character:0 );
			}
		}
	}

	char numTargets = targetList.size();

	char skill = caster->getSkill( _SKILL_NECROMANCY );
	char intel = caster->intelligence;
	unsigned short totalDamage = (unsigned short)(skill * intel * 15 * 1.75);
	unsigned short damageEach = totalDamage / (numTargets?numTargets:1);
	LinkedElement* element = targetList.head();

	while( element ) {
		WorldObject* obj = (WorldObject*)element->ptr();
		element = element->next();

		unsigned short damage = random((int)(damageEach * .9), (int)(damageEach * 1.1));

		if( obj ) {
			if( strcmp("minotaur", obj->basicName ) ) {
				packet->putByte ( _MOVIE_SPECIAL_EFFECT );
				packet->putLong ( caster->servID );
				packet->putByte ( _SE_POISONCLOUD_TARGET );
				packet->putByte ( 0 );
				packet->putLong ( obj->servID );

				obj->takeDamage( _AFF_DAMAGE_POISON, caster, damage, output, packet, 1 );

				short numRounds = static_cast<short>(((caster->intelligence - obj->endurance) / 2.5));
				if( numRounds < 0 ) numRounds = 0;
				
				// get the target's player state...
				CPlayerState *pPlayerState = obj->character;
				int nMystImmunity = 0;

				if ( pPlayerState ) {
					nMystImmunity = pPlayerState->GetMystImmunityCount();
				}
				
				if( nMystImmunity ) {
					sprintf ( sizeof ( buf ), buf, "|c22|%s is unaffected, immune. ", obj->getName() );
					strcat ( output, buf );
				} else if( !obj->hasAffect( _AFF_STUN ) ) {
					
					if( numRounds ) {
						if ( pPlayerState ) {
							pPlayerState->ChangeMystImmunityCount ( numRounds * 2 );
						}

						obj->addAffect( _AFF_STUN, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, numRounds, -1 );
						sprintf( sizeof( buf ), buf, "|c22|%s is stunned for %d round%s. ", obj->getName(), numRounds, (numRounds == 1)?"":"s" );
						strcat ( output, buf );
					} else {
						sprintf( sizeof( buf ), buf, "|c22|%s is unaffected. ", obj->getName() );
						strcat ( output, buf );
					}
				}

			} else {
				//target resists
				strcat ( output, obj->getName() );
				strcat ( output, "|c22| is unaffected. " );
			}
		}
	}

	short numEffects = 5;
	numEffects -= numTargets;
	
	targetList.release(); //this protects from deleting the objects pointed to by the list!

	char effectX;
	char effectY;

	while( numEffects > 0 ) {
		effectX = random( std::max(0, targetX -4), std::min(targetX +4, _COMBAT_GRID_WIDTH) );
		effectY = random( std::max(0, targetY -4), std::min(targetY +4, _COMBAT_GRID_HEIGHT) );
		//effectX = random( 0 >? targetX - 4, targetX + 4 <? _COMBAT_GRID_WIDTH );
		//effectY = random( 0 >? targetY - 4, targetY + 4 <? _COMBAT_GRID_HEIGHT );

		unsigned int servID = caster->combatGroup->grid[effectX][effectY];
			
		if( servID == 0) {
			packet->putByte ( _MOVIE_SPECIAL_EFFECT );
			packet->putLong ( caster->servID );
			packet->putByte ( _SE_POISONCLOUD_AREA );
			packet->putByte ( 0x00 );
			packet->putByte ( effectX );
			packet->putByte ( effectY );

			numEffects--;
		}
	}

	return NULL;
}

SPELL( castAREA_LIGHTNING )
{
	//This spell will affect the area surrounding the caster, but will not harm the caster.
	//It affects an area from 5 left to 5 right of the caster, and from 4 above to 4 below.
	//It does damage approximately equally to every combatant (friend or foe) in that area.
	//
	//If there are not at least 5 combatants that it hurts, it generates special effects
	//in random unoccupied squares, for effect.
	//

	//heres a list of player targets
	LinkedList targetList;

	short casterX = caster->combatX;
	short casterY = caster->combatY;

	caster = caster->getBaseOwner();
	char skill = caster->getSkill( _SKILL_ELEMENTALISM );
	char intel = caster->intelligence;

	for(short combatX = casterX - 5; combatX <= casterX + 5; ++combatX)
	{
		if( combatX < 0 ) continue;
		else if( combatX >= _COMBAT_GRID_WIDTH ) continue;

		for(short combatY = casterY - 4; combatY <= casterY + 4; ++combatY)
		{
			if (combatY < 0) continue;
			else if( combatY >= _COMBAT_GRID_HEIGHT ) continue;
			if( combatX == casterX && combatY == casterY ) continue;
            
			//get the servID of the object in this square
			unsigned int servID = caster->combatGroup->grid[combatX][combatY];

			if( servID <= 1 ) continue;
			if( servID == caster->servID ) continue;

			//add it to our target list if there is a valid servID
			if( servID ) {
				WorldObject* obj = roomMgr->getObject( servID );
				if( obj && obj->character )
					targetList.add( obj );
				else logDisplay( "%s:%d castAREA_LIGHTNING - target error. servid %d, obj %d  obj->character %d", __FILE__, __LINE__, servID, (int)obj, obj?(int)obj->character:0 );
			} 
		}
	}

	char numTargets = targetList.size();

	unsigned short totalDamage = skill * intel * 15;
	unsigned short damageEach = totalDamage / (numTargets?numTargets:1);
	LinkedElement* element = targetList.head();

	while( element ) {
		WorldObject* obj = (WorldObject*)element->ptr();
		element = element->next();

		unsigned short damage = random((int)(damageEach * .9), (int)(damageEach * 1.1));
		if( obj ) {
			packet->putByte ( _MOVIE_SPECIAL_EFFECT );
			packet->putLong ( caster->servID );
			packet->putByte ( _SE_LIGHTNING );
			packet->putByte ( 0 );
			packet->putLong ( obj->servID );

			obj->takeDamage( _AFF_DAMAGE_LIGHTNING, caster, damage, output, packet, 1 );
		}
	}

	short numEffects = 5;
	numEffects -= numTargets;
	
	targetList.release(); //this protects from deleting the objects pointed to by the list!

	char effectX;
	char effectY;

	while( numEffects > 0 ) {
		effectX = random( std::max(0, targetX -4), std::min(targetX +4, _COMBAT_GRID_WIDTH) );
		effectY = random( std::max(0, targetY -4), std::min(targetY +4, _COMBAT_GRID_HEIGHT) );
		//effectX = random( 0 >? casterX - 5, casterX + 5 <? _COMBAT_GRID_WIDTH );
		//effectY = random( 0 >? casterY - 4, casterY + 4 <? _COMBAT_GRID_HEIGHT );

		unsigned int servID = caster->combatGroup->grid[effectX][effectY];
			
		if( servID == 0) {
			packet->putByte ( _MOVIE_SPECIAL_EFFECT );
			packet->putLong ( caster->servID );
			packet->putByte ( _SE_LIGHTNING_BOLT );
			packet->putByte ( 0x00 );
			packet->putByte ( effectX );
			packet->putByte ( effectY );

			numEffects--;
		}
	}

	return NULL;
}

////////////////////////////////////////////////////////////
///                                                        /
/// Created New Spells for the different Immolation types. /
///                                                        /
////////////////////////////////////////////////////////////

//  duration = calcSpellDuration ( caster, 3 * skill, packet );
//                          < TO >
//  duration = calcSpellDuration ( caster, 5 * skill, packet );

//
// castIMMOLATION_COLD
//

SPELL ( castIMMOLATION_COLD )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target ) {
		strcat ( output, "Nothing happens." );
		return NULL;
	}

	char buf[1024];
	int skill = calcSpellSkill ( caster, _SKILL_ELEMENTALISM );

	int duration = 0;

	duration = calcSpellDuration ( caster, 6.5 * skill, packet );

	caster = caster->getBaseOwner();

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_ARTIC_GRASP );
	packet->putByte ( 1 );
	packet->putLong ( target->servID );

	affect_t *affect = target->hasAffect ( _AFF_IMMOLATION_COLD, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL );

	if ( affect ) {
		affect->duration = duration;
	} else {
		affect = target->addAffect ( _AFF_IMMOLATION_COLD, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, duration, skill, packet );
	}

	if ( !target->player ) {
		sprintf ( sizeof ( buf ), buf, "The %s is surrounded in magical ice for a moment. ", target->getName() );
		strcat ( output, buf );
	} else {
		sprintf ( sizeof ( buf ), buf, "%s ices over!", target->getName() );
		strcat ( output, buf );
	}

	return affect;
}

//
// castVITALITY
//

SPELL ( castVITALITY )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target ) {
		strcat ( output, "Nothing happens." );
		return NULL;
	}

	char buf[1024];
	int skill = calcSpellSkill ( caster, _SKILL_THAUMATURGY );

	int duration = 0;

	duration = calcSpellDuration ( caster, 3 * skill, packet );

	caster = caster->getBaseOwner();

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_INDESTRUCTION );
	packet->putByte ( 1 );
	packet->putLong ( target->servID );

	affect_t *affect = target->hasAffect ( _AFF_POS_END_MOD, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL );

	if ( affect ) {
		affect->duration = duration;
	} else {
		affect = target->addAffect ( _AFF_POS_END_MOD, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, duration, skill, packet );
	}

	if ( !target->player ) {
		sprintf ( sizeof ( buf ), buf, "|c21|The %s is surrounded by a mystical aura! ", target->getName() );
		strcat ( output, buf );
	} else {
		sprintf ( sizeof ( buf ), buf, "|c12|%s has been blessed with Endurance! ", target->getName() );
		strcat ( output, buf );
	}

	return affect;
}

//
// castENLIGHTENMENT
//

SPELL ( castENLIGHTENMENT )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target )
		return NULL;

	char buf[1024];
	int skill = calcSpellSkill ( caster, _SKILL_THAUMATURGY );

	int duration = 0;

	duration = calcSpellDuration ( caster, 3 * skill, packet );

	caster = caster->getBaseOwner();

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_NIMBILITY );
	packet->putByte ( 1 );
	packet->putLong ( target->servID );

	affect_t *affect = target->hasAffect ( _AFF_POS_INT_MOD, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL );

	if ( affect ) {
		affect->duration = duration;
	} else {
		affect = target->addAffect ( _AFF_POS_INT_MOD, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, duration, 15, packet );
	}

	if ( !target->player ) {
		sprintf ( sizeof ( buf ), buf, "|c21|The %s is surrounded by a mystical aura! ", target->getName() );
		strcat ( output, buf );
	} else {
		sprintf ( sizeof ( buf ), buf, "|c12|%s has been blessed with Intelligence! ", target->getName() );
		strcat ( output, buf );
	}	
	return affect;
}

//
// castIMMOLATION_LIGHTNING
//

SPELL ( castIMMOLATION_LIGHTNING )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target ) {
		strcat ( output, "Nothing happens." );
		return NULL;
	}

	char buf[1024];
	int skill = calcSpellSkill ( caster, _SKILL_ELEMENTALISM );

	int duration = 0;

	duration = calcSpellDuration ( caster, 6.5 * skill, packet );

	caster = caster->getBaseOwner();

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putLong ( caster->servID );
	packet->putByte ( _SE_ELECTRIC_CHARGE );
	packet->putByte ( 1 );
	packet->putLong ( target->servID );

	affect_t *affect = target->hasAffect ( _AFF_IMMOLATION_LIGHTNING, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL );

	if ( affect ) {
		affect->duration = duration;
	} else {
		affect = target->addAffect ( _AFF_IMMOLATION_LIGHTNING, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, duration, skill, packet );
	}

	if ( !target->player ) {
		sprintf ( sizeof ( buf ), buf, "|c21|The %s is surrounded by electricity for a moment. ", target->getName() );
		strcat ( output, buf );
	} else {
		sprintf ( sizeof ( buf ), buf, "%s crackles with electricity!", target->getName() );
		strcat ( output, buf );
	}

	return affect;
}

//
// castSUMMON_DRAGON
//

SPELL ( castSUMMON_DRAGON )
{
	caster = caster->getBaseOwner();

	switch ( random ( 0, 8 ) )
	{
		case 0:
			/* summon a Lost Bog Dragon */
            summonMonster ( 1, "DPDragWizN", "Lost Bog Dragon", _SE_SUMMON_DAEMON, targetX, targetY, caster, output, packet );
		break;

		case 1:
			/* summon a Mire Dragon */
            summonMonster ( 1, "DragonD", "Mire Dragon", _SE_SUMMON_DAEMON, targetX, targetY, caster, output, packet );
		break;

		case 2:
			/* summon a Bog Dragon */
            summonMonster ( 1, "DragonC", "Bog Dragon", _SE_SUMMON_DAEMON, targetX, targetY, caster, output, packet );
		break;

		case 3:
			/* summon an Elder Dragon */
            summonMonster ( 1, "DragonE", "Elder Dragon", _SE_SUMMON_DAEMON, targetX, targetY, caster, output, packet );
		break;

		case 4:
			/* summon a Legendary Lava Dragon */
            summonMonster ( 1, "DragonG", "Legendary Lava Dragon", _SE_SUMMON_DAEMON, targetX, targetY, caster, output, packet );
		break;

        case 5:
			/* summon a Legendary Wyrm */
            summonMonster ( 1, "DragonB", "Legendary Wyrm", _SE_SUMMON_DAEMON, targetX, targetY, caster, output, packet );
		break;

        case 6:
			/* summon a Legendary Obsidian Dragon */
            summonMonster ( 1, "DragonF", "Legendary Obsidian Dragon", _SE_SUMMON_DAEMON, targetX, targetY, caster, output, packet );
		break;

        case 7:
			/* summon a GOOD Dragon Wiz */
            summonMonster ( 1, "DPDragWizG", "Lost Wyrm Dragon", _SE_SUMMON_DAEMON, targetX, targetY, caster, output, packet );
		break;

        case 8:
			/* summon a EVIL Dragon Wiz */
            summonMonster ( 1, "DPDragWizE", "Lost Mire Dragon", _SE_SUMMON_DAEMON, targetX, targetY, caster, output, packet );
		break;
	}

	return NULL;
}

SPELL ( castSUMMON_MIST )
{
	caster = caster->getBaseOwner();

	switch ( random ( 0, 5 ) )
	{
		case 0:
			summonMonster ( 1, "EvilMinion", "evil minion", _SE_SUMMON_DAEMON, targetX, targetY, caster, output, packet );
		break;
		case 1:
			summonMonster ( 1, "Champion", "champion", _SE_SUMMON_PIXIE, targetX, targetY, caster, output, packet );
		break;
		case 2:
			summonMonster ( 1, "Protector", "protector", _SE_SUMMON_NIGHT_FRIENDS, targetX, targetY, caster, output, packet );
		break;
		case 3:
			summonMonster ( 1, "GoodWizard", "ancient one", _SE_SUMMON_PIXIE, targetX, targetY, caster, output, packet );
		break;
		case 4:
			summonMonster ( 1, "EvilWizard", "warlock", _SE_SUMMON_DAEMON, targetX, targetY, caster, output, packet );
		break;
		case 5:
			summonMonster ( 1, "NeutralWizard", "mist mage", _SE_SUMMON_NIGHT_FRIENDS, targetX, targetY, caster, output, packet );
		break;
	}

	return NULL;
}

SPELL ( castIMP_INVULNERABILITY )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target || !target->player ) {
		strcat ( output, "Nothing happens. " );
		return NULL;
	}

	caster = caster->getBaseOwner();

	if ( caster->character->profession == _PROF_WIZARD ) {

		int skill = calcSpellSkill ( caster, _SKILL_THAUMATURGY );
		int duration = 0;

		duration = calcSpellDuration ( caster, 15 * skill, packet );	// 10

		packet->putByte ( _MOVIE_SPECIAL_EFFECT );
		packet->putLong ( caster->servID );
		packet->putByte ( _SE_INVULNERABILITY );
		packet->putByte ( 1 );
		packet->putLong ( target->servID );

		affect_t *affect = target->hasAffect ( _AFF_INVULNERABLE );

		if ( affect ) {
			affect->value = 140;	// 70
			affect->duration = duration;
		} else {
			target->addAffect ( _AFF_INVULNERABLE, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, duration, 140, packet );	// 70
		}

		target->calcAC();

		char buf[1024];
		sprintf ( sizeof ( buf ), buf, "|c21|An improved invulnerability aura surrounds %s! ", target->getName() );
		strcat ( output, buf );

	}else{ 
		char buf[1024];
		sprintf ( sizeof ( buf ), buf, "|c60|Your class cannot use this spell!" );
   		strcat ( output, buf );
	}
	return NULL;
}

spell_info gSpellTable[_SPELL_MAX] = {
	{
		// _SPELL_HOME
		castHome,
		_SKILL_SORCERY,
		_SKILL_LVL_FAMILIAR,
		_TARGET_NONE,
		"Halome sadar!",
		1,
		_SPELL_FAST,
		_NON_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_KILL_STAR
		castKillStar,
		_SKILL_SORCERY,
		_SKILL_LVL_FAMILIAR,
		_TARGET_OBJ,
		"Projen bladir!",
		1,
		_SPELL_FAST,
		_COMBAT_SPELL,
		0,
		FALSE,
		TRUE
	},

	{
		// _SPELL_UNLOCK
		castUnlock,
		_SKILL_SORCERY,
		_SKILL_LVL_FAMILIAR,
		_TARGET_OBJ,
		"Portan porbin!",
		1,
		_SPELL_FAST,
		_NON_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_DISPEL_MAGIC
		castDispelMagic,
		_SKILL_SORCERY,
		_SKILL_LVL_FAMILIAR,
		_TARGET_OBJ,
		"Negatos katir!",
		1,
		_SPELL_MEDIUM,
		_BOTH_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_ENGRAVE
		castEngrave,
		_SKILL_SORCERY,
		_SKILL_LVL_FAMILIAR,
		_TARGET_OBJ,
		"Engravus!",
		20,
		_SPELL_FAST,
		_NON_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_MULTI_BLADE
		castMultiBlade,
		_SKILL_SORCERY,
		_SKILL_LVL_PROFICIENT,
		_TARGET_OBJ,
		"Projen bladiros!",
		3,
		_SPELL_MEDIUM,
		_COMBAT_SPELL,
		0,
		FALSE,
		TRUE
	},

	{
		// _SPELL_GATHER_THE_FELLOWSHIP
		castGatherTheFellowship,
		_SKILL_SORCERY,
		_SKILL_LVL_PROFICIENT,
		_TARGET_NONE,
		"Elanon sadar yokanion!",
		3,
		_SPELL_FAST,
		_NON_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_CORNUCOPIA
		castCornucopia,
		_SKILL_SORCERY,
		_SKILL_LVL_PROFICIENT,
		_TARGET_NONE,
		"Elanon consuma!",
		3,
		_SPELL_FAST,
		_NON_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_CLOUD_OF_FOG
		castCloudOfFog,
		_SKILL_SORCERY,
		_SKILL_LVL_GRAND_MASTER,
		_TARGET_OBJ,
		"Ylis enlightama!",
		150,
		_SPELL_FAST,
		_NON_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_IMPROVE_ARMOR
		castImproveArmor,
		_SKILL_SORCERY,
		_SKILL_LVL_PROFICIENT,
		_TARGET_OBJ,
		"Impovrin seld!",
		3,
		_SPELL_MEDIUM,
		_BOTH_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_TELEPORT
		castTeleport,
		_SKILL_SORCERY,
		_SKILL_LVL_EXPERT,
		_TARGET_NONE,
		"Elanon sadar!",
		5,
		_SPELL_FAST,
		_NON_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_EXTENSION
		castExtension,
		_SKILL_SORCERY,
		_SKILL_LVL_EXPERT,
		_TARGET_NONE,
		"Durata xetan!",
		5,
		_SPELL_FAST,
		_BOTH_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_SEE_INVISIBILITY
		castSeeInvisibility,
		_SKILL_SORCERY,
		_SKILL_LVL_EXPERT,
		_TARGET_NONE,
		"Revatos unaseem akanos!",
		5,
		_SPELL_FAST,
		_BOTH_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_SHIFT
		castShift,
		_SKILL_SORCERY,
		_SKILL_LVL_EXPERT,
		_TARGET_NONE,
		"Atersis nelahar!",
		5,
		_SPELL_FAST,
		_BOTH_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_INVISIBILITY
		castInvisibility,
		_SKILL_SORCERY,
		_SKILL_LVL_EXPERT,
		_TARGET_NONE,
		"Dasguusi jehar!",
		5,
		_SPELL_MEDIUM,
		_BOTH_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_COMBAT_TELEPORT
		castCombatTeleport,
		_SKILL_SORCERY,
		_SKILL_LVL_MASTER,
		_TARGET_LOCALE,
		"Respusis fulakahn!",
		7,
		_SPELL_SLOW,
		_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_TELEPORT_GROUP
		castTeleportGroup,
		_SKILL_SORCERY,
		_SKILL_LVL_MASTER,
		_TARGET_NONE,
		"Guhtanik tul eahrak!",
		7,
		_SPELL_FAST,
		_NON_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_PERMANENCY
		castPermanency,
		_SKILL_SORCERY,
		_SKILL_LVL_MASTER,
		_TARGET_OBJ,
		"Huek tul kerasuhg hadrion!",
		500,
		_SPELL_SLOW,
		_NON_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_RUST
		castRust,
		_SKILL_SORCERY,
		_SKILL_LVL_MASTER,
		_TARGET_OBJ,
		"Deserionriak!",
		7,
		_SPELL_MEDIUM,
		_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_DEFENSELESSNESS
		castDefenselessness,
		_SKILL_SORCERY,
		_SKILL_LVL_MASTER,
		_TARGET_NONE,
		"Negata arman!",
		7,
		_SPELL_SLOW,
		_BOTH_SPELL,
		0,
		TRUE,
		FALSE
	},

	{
		// _SPELL_IMPROVED_INVISIBILITY
		castImprovedInvisibility,
		_SKILL_SORCERY,
		_SKILL_LVL_GRAND_MASTER,
		_TARGET_NONE,
		"Kusar dasguusi arkriohn!",
		9,
		_SPELL_SLOW,
		_BOTH_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_ENCHANT_ITEM
		castEnchantItem,
		_SKILL_SORCERY,
		_SKILL_LVL_GRAND_MASTER,
		_TARGET_OBJ,
		"Yoyhab tul kestrorgh kerasuhg diam!",
		1000,
		_SPELL_SLOW,
		_NON_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_MASS_RUST
		castMassRust,
		_SKILL_SORCERY,
		_SKILL_LVL_GRAND_MASTER,
		_TARGET_NONE,
		"Kusar deserionriak arkrioal!",
		14,
		_SPELL_SLOWEST,
		_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_ELPHAMES_JUSTICE
		castElphamesJustice,
		_SKILL_SORCERY,
		_SKILL_LVL_GRAND_MASTER,
		_TARGET_NONE,
		"Karkath tuus elamen tos!",
		1000,
		_SPELL_SLOW,
		_COMBAT_SPELL,
		0,
		FALSE,
		TRUE
	},

	{
		// _SPELL_FIRE_GRASP
		castFIRE_GRASP,
		_SKILL_ELEMENTALISM,
		_SKILL_FAMILIAR,
		_TARGET_NONE,
		"Kahtir flamir!",
		1,
		_SPELL_FAST,
		_BOTH_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_FLAME_ORB
		castFLAME_ORB,
		_SKILL_ELEMENTALISM,
		_SKILL_FAMILIAR,
		_TARGET_NONE,
		"Projen flamin!",
		1,
		_SPELL_FAST,
		_COMBAT_SPELL,
		0,
		FALSE,
		TRUE
	},

	{
		// _SPELL_ARTIC_GRASP
		castARTIC_GRASP,
		_SKILL_ELEMENTALISM,
		_SKILL_FAMILIAR,
		_TARGET_NONE,
		"Kahtir cryonous!",
		1,
		_SPELL_FAST,
		_BOTH_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_ICE_ORB
		castICE_ORB,
		_SKILL_ELEMENTALISM,
		_SKILL_FAMILIAR,
		_TARGET_NONE,
		"Projen cryonous!",
		1,
		_SPELL_FAST,
		_COMBAT_SPELL,
		0,
		FALSE,
		TRUE
	},

	{
		// _SPELL_EARTH_SPIKE
		castEARTH_SPIKE,
		_SKILL_ELEMENTALISM,
		_SKILL_GRAND_MASTER,
		_TARGET_NONE,
		"Earth Spike",
		50,
		_SPELL_FAST,
		_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_INCINERATE
		castINCINERATE,
		_SKILL_ELEMENTALISM,
		_SKILL_FAMILIAR,
		_TARGET_NONE,
		"Flamin ark karsersis!",
		5,
		_SPELL_FAST,
		_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_GUST_OF_WIND
		castGUST_OF_WIND,
		_SKILL_ELEMENTALISM,
		_SKILL_PROFICIENT,
		_TARGET_NONE,
		"Dalterahk!",
		3,
		_SPELL_MEDIUM,
		_COMBAT_SPELL,
		0,
		FALSE,
		TRUE
	},

	{
		// _SPELL_IMMOLATION
		castIMMOLATION,
		_SKILL_ELEMENTALISM,
		_SKILL_PROFICIENT,
		_TARGET_NONE,
		"Flamin tul ershealkn!",
		3,
		_SPELL_MEDIUM,
		_BOTH_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_DANCING_FLAME
		castDANCING_FLAME,
		_SKILL_ELEMENTALISM,
		_SKILL_GRAND_MASTER,
		_TARGET_NONE,
		"Dancing Flame",
		50,
		_SPELL_MEDIUM,
		_COMBAT_SPELL,
		0,
		FALSE,
		TRUE
	},

	{
		// _SPELL_FLAME_BLADE
		castFLAME_BLADE,
		_SKILL_ELEMENTALISM,
		_SKILL_PROFICIENT,
		_TARGET_NONE,
		"Kultahk flamin por!",
		3,
		_SPELL_MEDIUM,
		_BOTH_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_ARCTIC_CHARGE
		castARTIC_CHARGE,
		_SKILL_ELEMENTALISM,
		_SKILL_PROFICIENT,
		_TARGET_NONE,
		"Kultahk ahmwa por!",
		3,
		_SPELL_MEDIUM,
		_BOTH_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_COLD_STEEL
		castCOLD_STEEL,
		_SKILL_ELEMENTALISM,
		_SKILL_PROFICIENT,
		_TARGET_NONE,
		"Kultahk cryonous por!",
		3,
		_SPELL_MEDIUM,
		_BOTH_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_SAND_STORM
		castSAND_STORM,
		_SKILL_ELEMENTALISM,
		_SKILL_PROFICIENT,
		_TARGET_NONE,
		"Dalterahk tohs granul!",
		3,
		_SPELL_MEDIUM,
		_COMBAT_SPELL,
		0,
		FALSE,
		TRUE
	},

	{
		// _SPELL_SPARK
		castSPARK,
		_SKILL_ELEMENTALISM,
		_SKILL_PROFICIENT,
		_TARGET_NONE,
		"Ahmwa tos tersak!",
		3,
		_SPELL_MEDIUM,
		_COMBAT_SPELL,
		0,
		FALSE,
		TRUE
	},

	{
		// _SPELL_ICE_STORM
		castICE_STORM,
		_SKILL_ELEMENTALISM,
		_SKILL_EXPERT,
		_TARGET_NONE,
		"Dalterahk tohs cryonous xeeskak!",
		7,
		_SPELL_MEDIUM,
		_COMBAT_SPELL,
		0,
		FALSE,
		TRUE
	},

	{
		// _SPELL_FREEZING_WIND
		castFREEZING_WIND,
		_SKILL_ELEMENTALISM,
		_SKILL_GRAND_MASTER,
		_TARGET_NONE,
		"Dalterahk tohs coolar!",
		50,
		_SPELL_SLOW,
		_COMBAT_SPELL,
		0,
		FALSE,
		TRUE
	},

	{
		// _SPELL_HURRICANE
		castHURRICANE,
		_SKILL_ELEMENTALISM,
		_SKILL_GRAND_MASTER,
		_TARGET_NONE,
		"Dalterahk tohs dalterahk!",
		50,
		_SPELL_SLOW,
		_COMBAT_SPELL,
		0,
		FALSE,
		TRUE
	},

	{
		// _SPELL_CRAWLING_CHARGE
		castCRAWLING_CHARGE,
		_SKILL_ELEMENTALISM,
		_SKILL_GRAND_MASTER,
		_TARGET_NONE,
		"Lighteru Arenos Trixmut",
		150,
		_SPELL_SLOW,
		_COMBAT_SPELL,
		0,
		FALSE,
		TRUE
	},

	{
		// _SPELL_STONING
		castSTONING,
		_SKILL_ELEMENTALISM,
		_SKILL_EXPERT,
		_TARGET_NONE,
		"Tak eros nothrahk!",
		7,
		_SPELL_MEDIUM,
		_COMBAT_SPELL,
		0,
		FALSE,
		TRUE
	},

	{
		// _SPELL_FIREBALL
		castFIREBALL,
		_SKILL_ELEMENTALISM,
		_SKILL_MASTER,
		_TARGET_NONE,
		"Flamir tos ehrikahr!",
		9,
		_SPELL_MEDIUM,
		_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_LIGHTNING_BOLT
		castLIGHTNING_BOLT,
		_SKILL_ELEMENTALISM,
		_SKILL_MASTER,
		_TARGET_NONE,
		"Tohra ahmi ahmwa surak!",
		9,
		_SPELL_SLOW,
		_COMBAT_SPELL,
		0,
		FALSE,
		TRUE
	},

	{
		// _SPELL_FREEZE
		castFREEZE,
		_SKILL_ELEMENTALISM,
		_SKILL_MASTER,
		_TARGET_NONE,
		"Coolar sahn akirsos!",
		7,
		_SPELL_SLOWEST,
		_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_CRUSHING_BOULDER
		castCRUSHING_BOULDER,
		_SKILL_ELEMENTALISM,
		_SKILL_MASTER,
		_TARGET_NONE,
		"Kosha eros zuukar!",
		7,
		_SPELL_MEDIUM,
		_COMBAT_SPELL,
		0,
		FALSE,
		TRUE
	},

	{
		// _SPELL_ELECTRIC_FURY
		castELECTRIC_FURY,
		_SKILL_ELEMENTALISM,
		_SKILL_GRAND_MASTER,
		_TARGET_NONE,
		"Ahmwa tak noeb dukuuth!",
		75,
		_SPELL_SLOW,
		_COMBAT_SPELL,
		0,
		FALSE,
		TRUE
	},

	{
		// _SPELL_COLD_SNAP
		castCOLD_SNAP,
		_SKILL_ELEMENTALISM,
		_SKILL_GRAND_MASTER,
		_TARGET_NONE,
		"Hekahn unoeb cryonous!",
		15,
		_SPELL_SLOW,
		_COMBAT_SPELL,
		0,
		FALSE,
		TRUE
	},

	{
		// _SPELL_EARTHQUAKE
		castEARTHQUAKE,
		_SKILL_ELEMENTALISM,
		_SKILL_GRAND_MASTER,
		_TARGET_NONE,
		"Eros akar noeb tygrikaeth!",
		75,
		_SPELL_SLOW,
		_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_DESPOTHES_WRATH
		castDESPOTHES_WRATH,
		_SKILL_ELEMENTALISM,
		_SKILL_GRAND_MASTER,
		_TARGET_NONE,
		"Tahkar ohnerak dekorak tuhl vermihr!",
		650,
		_SPELL_SLOW,
		_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_HOLD_MONSTER
		castHOLD_MONSTER,
		_SKILL_MYSTICISM,
		_SKILL_FAMILIAR,
		_TARGET_NONE,
		"Trapel monsen!",
		3,
		_SPELL_FAST,
		_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_FUMBLE
		castFUMBLE,
		_SKILL_MYSTICISM,
		_SKILL_FAMILIAR,
		_TARGET_NONE,
		"Kahnir axuah!",
		3,
		_SPELL_SLOW,
		_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_PSYCHIC_ORB
		castPSYCHIC_ORB,
		_SKILL_MYSTICISM,
		_SKILL_FAMILIAR,
		_TARGET_NONE,
		"Projen mentali!",
		1,
		_SPELL_FAST,
		_COMBAT_SPELL,
		0,
		FALSE,
		TRUE
	},

	{
		// _SPELL_CONFUSION
		castCONFUSION,
		_SKILL_MYSTICISM,
		_SKILL_PROFICIENT,
		_TARGET_NONE,
		"Mentun ahnawar!",
		5,
		_SPELL_SLOWEST,
		_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_FORGET
		castFORGET,
		_SKILL_MYSTICISM,
		_SKILL_GRAND_MASTER,
		_TARGET_NONE,
		"Ylis inhel!",
		200,
		_SPELL_SLOW,
		_BOTH_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_MIND_SHACKLE
		castMIND_SHACKLE,
		_SKILL_MYSTICISM,
		_SKILL_PROFICIENT,
		_TARGET_NONE,
		"Usbin xuk!",
		3,
		_SPELL_FAST,
		_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_IDENTIFY
		castIDENTIFY,
		_SKILL_MYSTICISM,
		_SKILL_PROFICIENT,
		_TARGET_NONE,
		"Elphux teskhi revalus!",
		15,
		_SPELL_MEDIUM,
		_BOTH_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_BERSERK
		castBERSERK,
		_SKILL_MYSTICISM,
		_SKILL_EXPERT,
		_TARGET_NONE,
		"Furyus atrankin!",
		7,
		_SPELL_FAST,
		_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_STUN
		castSTUN,
		_SKILL_MYSTICISM,
		_SKILL_EXPERT,
		_TARGET_NONE,
		"Diasu petralt!",
		7,
		_SPELL_FAST,
		_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_LOYALTY_SHIFT
		castLOYALTY_SHIFT,
		_SKILL_MYSTICISM,
		_SKILL_EXPERT,
		_TARGET_NONE,
		"Vlos unem!",
		7,
		_SPELL_FAST,
		_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_WARP_MIND
		castWARP_MIND,
		_SKILL_MYSTICISM,
		_SKILL_EXPERT,
		_TARGET_NONE,
		"Mentus xuax!",
		5,
		_SPELL_MEDIUM,
		_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_ETHEREALIZE
		castETHEREALIZE,
		_SKILL_MYSTICISM,
		_SKILL_EXPERT,
		_TARGET_NONE,
		"Etherealize",
		10,
		_SPELL_MEDIUM,
		_BOTH_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_SPELL_BLAST
		castSPELL_BLAST,
		_SKILL_MYSTICISM,
		_SKILL_EXPERT,
		_TARGET_NONE,
		"Zratos destulu!",
		10,
		_SPELL_MEDIUM,
		_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_MASS_HOLD
		castMASS_HOLD,
		_SKILL_MYSTICISM,
		_SKILL_MASTER,
		_TARGET_NONE,
		"Trapel monsenati!",
		 20,
		_SPELL_SLOWEST,
		_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_65
		cast65,
		_SKILL_NECROMANCY,
		_SKILL_GRAND_MASTER,
		_TARGET_NONE,
		"Dragon! Kill this filth!",
		0,
		_SPELL_FAST,
		_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_MASS_FUMBLE
		castMASS_FUMBLE,
		_SKILL_MYSTICISM,
		_SKILL_MASTER,
		_TARGET_NONE,
		"Kahnira setanu axuah ust!",
		30,
		_SPELL_SLOWEST,
		_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_FEAR
		castFEAR,
		_SKILL_MYSTICISM,
		_SKILL_MASTER,
		_TARGET_NONE,
		"Mortusu noknahr tulaak!",
		14,
		_SPELL_SLOWEST,
		_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_IRON_CHAINS
		castIRON_CHAINS,
		_SKILL_MYSTICISM,
		_SKILL_MASTER,
		_TARGET_NONE,
		"Usbin xukuus drak terenoh!",
		7,
		_SPELL_SLOW,
		_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_MASS_BERSERK
		castMASS_BERSERK,
		_SKILL_MYSTICISM,
		_SKILL_GRAND_MASTER,
		_TARGET_NONE,
		"Furyus xuku atrankin kuerto!",
		30,
		_SPELL_SLOWEST,
		_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_MASS_STUN
		castMASS_STUN,
		_SKILL_MYSTICISM,
		_SKILL_GRAND_MASTER,
		_TARGET_NONE,
		"Diasu xuku petraltus terenoh!",
		30,
		_SPELL_SLOWEST,
		_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_MASS_LOYALTY_SHIFT
		castMASS_LOYALTY_SHIFT,
		_SKILL_MYSTICISM,
		_SKILL_GRAND_MASTER,
		_TARGET_NONE,
		"Vlos xuku unemos enok!",
		18,
		_SPELL_SLOWEST,
		_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_MABONS_FORCED_FLEE
		castMABONS_FORCED_FLEE,
		_SKILL_MYSTICISM,
		_SKILL_GRAND_MASTER,
		_TARGET_NONE,
		"Katrakos xuknut tosrithos vehk enok!",
		10000,
		_SPELL_SLOWEST,
		_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_ILLUSIONARY_FOE
		castILLUSIONARY_FOE,
		_SKILL_MYSTICISM,
		_SKILL_GRAND_MASTER,
		_TARGET_NONE,
		"Mirkos extos ilusior!",
		9,
		_SPELL_SLOW,
		_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_ANTI_MAGIC_AURA
		castANTI_MAGIC_AURA,
		_SKILL_MYSTICISM,
		_SKILL_GRAND_MASTER,
		_TARGET_NONE,
		"Torsuuk extos atukt estu!",
		25,
		_SPELL_SLOWEST,
		_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
//		_BOTH_SPELL
	},

	{
		// _SPELL_LIGHT_DART
		castLIGHT_DART,
		_SKILL_THAUMATURGY,
		_SKILL_FAMILIAR,
		_TARGET_NONE,
		"Ahn Ilani!",
		1,
		_SPELL_FAST,
		_COMBAT_SPELL,
		0,
		FALSE,
		TRUE
	},

	{
		// _SPELL_CURSE_OF_CLUMSINESS
		castCURSE_OF_CLUMSINESS,
		_SKILL_NECROMANCY,
		_SKILL_FAMILIAR,
		_TARGET_NONE,
		"Dahtuuk exuhn!",
		1,
		_SPELL_FAST,
		_BOTH_SPELL,
		0,
		TRUE,
		FALSE
	},

	{
		// _SPELL_NIMBILITY
		castNIMBILITY,
		_SKILL_THAUMATURGY,
		_SKILL_FAMILIAR,
		_TARGET_NONE,
		"Ylis uhalmi!",
		1,
		_SPELL_FAST,
		_BOTH_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_EMPOWER
		castEMPOWER,
		_SKILL_THAUMATURGY,
		_SKILL_PROFICIENT,
		_TARGET_NONE,
		"Ylis numahtra!",
		3,
		_SPELL_FAST,
		_BOTH_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_ENFEEBLE
		castENFEEBLE,
		_SKILL_NECROMANCY,
		_SKILL_PROFICIENT,
		_TARGET_NONE,
		"Makrualk exuhn!",
		3,
		_SPELL_FAST,
		_BOTH_SPELL,
		0,
		TRUE,
		FALSE
	},

	{
		// _SPELL_MISSILE_RESISTANCE
		castMISSILE_RESISTANCE,
		_SKILL_THAUMATURGY,
		_SKILL_PROFICIENT,
		_TARGET_NONE,
		"Mehwiis refectus!",
		3,
		_SPELL_FAST,
		_BOTH_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_HEAL
		castHEAL,
		_SKILL_THAUMATURGY,
		_SKILL_PROFICIENT,
		_TARGET_NONE,
		"Domni vescance!",
		3,
		_SPELL_FAST,
		_BOTH_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_SUMMON_PIXIE
		castSUMMON_PIXIE,
		_SKILL_THAUMATURGY,
		_SKILL_PROFICIENT,
		_TARGET_NONE,
		"Clactous pixay",
		5,
		_SPELL_SLOW,
		_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_PURIFY
		castPURIFY,
		_SKILL_THAUMATURGY,
		_SKILL_PROFICIENT,
		_TARGET_NONE,
		"Novinus epur!",
		3,
		_SPELL_FAST,
		_BOTH_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_CURE_POISON
		castCURE_POISON,
		_SKILL_THAUMATURGY,
		_SKILL_EXPERT,
		_TARGET_NONE,
		"Novinus venah epurlahn!",
		9,
		_SPELL_MEDIUM,
		_BOTH_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_ELEMENTAL
		castSUMMON_ELEMENTAL,
		_SKILL_THAUMATURGY,
		_SKILL_GRAND_MASTER,
		_TARGET_NONE,
		"Clactous flamin tak oceaus dalter eleman",
		5,
		_SPELL_SLOW,
		_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_GREATER_HEAL
		castGREATER_HEAL,
		_SKILL_THAUMATURGY,
		_SKILL_EXPERT,
		_TARGET_NONE,
		"Domnixali vescance ahrwihn!",
		10,
		_SPELL_SLOW,
		_BOTH_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_REMOVE_CURSE
		castREMOVE_CURSE,
		_SKILL_THAUMATURGY,
		_SKILL_EXPERT,
		_TARGET_NONE,
		"Volhari tovinius ain lohnesti!",
		10,
		_SPELL_SLOW,
		_BOTH_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_SUMMON_FAERY
		castSUMMON_FAERY,
		_SKILL_THAUMATURGY,
		_SKILL_EXPERT,
		_TARGET_NONE,
		"Clactous faery",
		10,
		_SPELL_SLOW,
		_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_FIRE_CURSE
		castFIRE_CURSE,
		_SKILL_NECROMANCY,
		_SKILL_EXPERT,
		_TARGET_NONE,
		"Morbitus xek flamin!",
		5,
		_SPELL_FAST,
		_BOTH_SPELL,
		0,
		TRUE,
		FALSE
	},

	{
		// _SPELL_COLD_CURSE
		castCOLD_CURSE,
		_SKILL_NECROMANCY,
		_SKILL_EXPERT,
		_TARGET_NONE,
		"Morbitus xek cryonous!",
		5,
		_SPELL_FAST,
		_BOTH_SPELL,
		0,
		TRUE,
		FALSE
	},

	{
		// _SPELL_ELECTRIC_CURSE
		castELECTRIC_CURSE,
		_SKILL_NECROMANCY,
		_SKILL_EXPERT,
		_TARGET_NONE,
		"Morbitus xek amweh!",
		5,
		_SPELL_FAST,
		_BOTH_SPELL,
		0,
		TRUE,
		FALSE
	},

	{
		// _SPELL_SHIELD
		castSHIELD,
		_SKILL_THAUMATURGY,
		_SKILL_EXPERT,
		_TARGET_NONE,
		"Domnius etrini!",
		5,
		_SPELL_FAST,
		_BOTH_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_GREATER_SHIELD
		castGREATER_SHIELD,
		_SKILL_THAUMATURGY,
		_SKILL_MASTER,
		_TARGET_NONE,
		"Domnius maxu etrini!",
		7,
		_SPELL_MEDIUM,
		_BOTH_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_WRATH_OF_THE_GODS
		castWRATH_OF_THE_GODS,
		_SKILL_THAUMATURGY,
		_SKILL_MASTER,
		_TARGET_NONE,
		"Alini ehm ahnious palon!",
		15,
		_SPELL_SLOW,
		_COMBAT_SPELL,
		0,
		FALSE,
		TRUE
	},

	{
		// _SPELL_FIRE_SHIELD
		castFIRE_SHIELD,
		_SKILL_THAUMATURGY,
		_SKILL_MASTER,
		_TARGET_NONE,
		"Domnius flamin!",
		7,
		_SPELL_FAST,
		_BOTH_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_COLD_SHIELD
		castCOLD_SHIELD,
		_SKILL_THAUMATURGY,
		_SKILL_MASTER,
		_TARGET_NONE,
		"Domnius cryonous!",
		7,
		_SPELL_FAST,
		_BOTH_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_LIGHTNING_SHIELD
		castLIGHTNING_SHIELD,
		_SKILL_THAUMATURGY,
		_SKILL_MASTER,
		_TARGET_NONE,
		"Domnius amwah!",
		7,
		_SPELL_FAST,
		_BOTH_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_SUMMON_NYMPH
		castSUMMON_NYMPH,
		_SKILL_THAUMATURGY,
		_SKILL_MASTER,
		_TARGET_NONE,
		"Clactous nymphous",
		15,
		_SPELL_SLOW,
		_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_REGENERATION
		castREGENERATION,
		_SKILL_THAUMATURGY,
		_SKILL_MASTER,
		_TARGET_NONE,
		"Structos solamni eskel!",
		7,
		_SPELL_FAST,
		_BOTH_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_INDESTRUCTION
		castINDESTRUCTION,
		_SKILL_THAUMATURGY,
		_SKILL_GRAND_MASTER,
		_TARGET_NONE,
		"Estructos domnios kelesni!",
		9,
		_SPELL_MEDIUM,
		_NON_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_INVULNERABILITY
		castINVULNERABILITY,
		_SKILL_THAUMATURGY,
		_SKILL_GRAND_MASTER,
		_TARGET_NONE,
		"Estructos odomniu maxun!",
		20,
		_SPELL_SLOW,
		_BOTH_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_ENIDS_BLESSING
		castENIDS_BLESSING,
		_SKILL_THAUMATURGY,
		_SKILL_GRAND_MASTER,
		_TARGET_NONE,
		"Enid' Blessing",
		1500,
		_SPELL_SLOWEST,
		_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_BANISHMENT
		castBANISHMENT,
		_SKILL_THAUMATURGY,
		_SKILL_GRAND_MASTER,
		_TARGET_NONE,
		"Tespris esgon vesteport!",
		18,
		_SPELL_SLOW,
		_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_SUMMON_FAERY_QUEEN
		castSUMMON_FAERY_QUEEN,
		_SKILL_THAUMATURGY,
		_SKILL_GRAND_MASTER,
		_TARGET_NONE,
		"Clactous superios faery",
		20,
		_SPELL_SLOW,
		_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_ACID_SPHERE
		castACID_SPHERE,
		_SKILL_NECROMANCY,
		_SKILL_FAMILIAR,
		_TARGET_NONE,
		"Projen murax!",
		1,
		_SPELL_FAST,
		_COMBAT_SPELL,
		0,
		FALSE,
		TRUE
	},

	{
		// _SPELL_VENOMOUS_TOUCH
		castVENOMOUS_TOUCH,
		_SKILL_NECROMANCY,
		_SKILL_FAMILIAR,
		_TARGET_NONE,
		"Emantos venah!",
		1,
		_SPELL_FAST,
		_BOTH_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_107
		cast107,
		_SKILL_NECROMANCY,
		_SKILL_GRAND_MASTER,
		_TARGET_NONE,
		"Arise! Bring forth destruction!",
		0,
		_SPELL_FAST,
		_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_POISON_BOLT
		castPOISON_BOLT,
		_SKILL_NECROMANCY,
		_SKILL_PROFICIENT,
		_TARGET_NONE,
		"Projen venah!",
		2,
		_SPELL_FAST,
		_COMBAT_SPELL,
		0,
		FALSE,
		TRUE
	},

	{
		// _SPELL_109
		cast109,
		_SKILL_SORCERY,
		_SKILL_GRAND_MASTER,
		_TARGET_NONE,
		"109",
		0,
		_SPELL_FAST,
		_BOTH_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_DRAIN_LIFE
		castDRAIN_LIFE,
		_SKILL_NECROMANCY,
		_SKILL_PROFICIENT,
		_TARGET_NONE,
		"Amven torak!",
		3,
		_SPELL_FAST,
		_COMBAT_SPELL,
		0,
		FALSE,
		TRUE
	},

	{
		// _SPELL_ACID_CLOUD
		castACID_CLOUD,
		_SKILL_NECROMANCY,
		_SKILL_PROFICIENT,
		_TARGET_NONE,
		"Setos eian murax!",
		3,
		_SPELL_MEDIUM,
		_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_SUMMON_ZOMBIE
		castSUMMON_ZOMBIE,
		_SKILL_NECROMANCY,
		_SKILL_PROFICIENT,
		_TARGET_NONE,
		"Clactous usdemos!",
		5,
		_SPELL_SLOW,
		_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_VENOM
		castVENOM,
		_SKILL_NECROMANCY,
		_SKILL_EXPERT,
		_TARGET_NONE,
		"Emantos binduus venah!",
		7,
		_SPELL_FAST,
		_BOTH_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_SUMMON_UNDEAD
		castSUMMON_UNDEAD,
		_SKILL_NECROMANCY,
		_SKILL_EXPERT,
		_TARGET_NONE,
		"Clactous necrata!",
		10,
		_SPELL_SLOW,
		_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_ACID_RAIN
		castACID_RAIN,
		_SKILL_NECROMANCY,
		_SKILL_EXPERT,
		_TARGET_NONE,
		"Murax venhen rasuthk!",
		7,
		_SPELL_MEDIUM,
		_COMBAT_SPELL,
		0,
		FALSE,
		TRUE
	},

	{
		// _SPELL_NIGHT_FRIENDS
		castNIGHT_FRIENDS,
		_SKILL_NECROMANCY,
		_SKILL_EXPERT,
		_TARGET_NONE,
		"Clactous effendi an neh!",
		15,
		_SPELL_SLOW,
		_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_STEAL_LIFE
		castSTEAL_LIFE,
		_SKILL_NECROMANCY,
		_SKILL_EXPERT,
		_TARGET_NONE,
		"Sukt kohn amven xek torak!",
		9,
		_SPELL_MEDIUM,
		_COMBAT_SPELL,
		0,
		FALSE,
		TRUE
	},

	{
		// _SPELL_118 
		cast118,
		_SKILL_NECROMANCY,
		_SKILL_GRAND_MASTER,
		_TARGET_NONE,
		"Lighteru Arenos",
		0,
		_SPELL_FAST,
		_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_SUMMON_DOPPELGANGER
		castSUMMON_DOPPELGANGER,
		_SKILL_NECROMANCY,
		_SKILL_MASTER,
		_TARGET_NONE,
		"Clactous mirsos estion!",
		18,
		_SPELL_SLOW,
		_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_DEATH_TOUCH
		castDEATH_TOUCH,
		_SKILL_NECROMANCY,
		_SKILL_MASTER,
		_TARGET_NONE,
		"Kektos kohn xektah gulak!",
		20,
		_SPELL_SLOW,
		_COMBAT_SPELL,
		0,
		FALSE,
		TRUE
	},

	{
		// _SPELL_BANISH
		castBANISH,
		_SKILL_NECROMANCY,
		_SKILL_MASTER,
		_TARGET_NONE,
		"Esgon vestex!",
		5,
		_SPELL_SLOW,
		_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_SHADOW_WARRIOR
		castSHADOW_WARRIOR,
		_SKILL_NECROMANCY,
		_SKILL_MASTER,
		_TARGET_NONE,
		"Clactos vektous krai!",
		20,
		_SPELL_SLOW,
		_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_SUMMON_DAEMON
		castSUMMON_DAEMON,
		_SKILL_NECROMANCY,
		_SKILL_GRAND_MASTER,
		_TARGET_NONE,
		"Clactous demonis nethrahk!",
		30,
		_SPELL_SLOWEST,
		_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_DUACHS_VENGEANCE
		castDUACHS_VENGEANCE,
		_SKILL_NECROMANCY,
		_SKILL_GRAND_MASTER,
		_TARGET_NONE,
		"Demix tuus nekriith veh duach!",
		450,
		_SPELL_SLOW,
		_COMBAT_SPELL,
		0,
		FALSE,
		TRUE
	},

	{
		// _SPELL_DEATH_WISH
		castDEATH_WISH,
		_SKILL_NECROMANCY,
		_SKILL_GRAND_MASTER,
		_TARGET_NONE,
		"Tuus morbidious tek narcissi!",
		55,
		_SPELL_SLOWEST,
		_COMBAT_SPELL,
		0,
		FALSE,
		TRUE
	},

	{
		// _SPELL_MASS_DRAIN
		castMASS_DRAIN,
		_SKILL_NECROMANCY,
		_SKILL_GRAND_MASTER,
		_TARGET_NONE,
		"Amven xux torak keptih!",
		15,
		_SPELL_SLOWEST,
		_COMBAT_SPELL,
		0,
		FALSE,
		TRUE
	},

	{
		// _SPELL_POISON_CURSE
		castPOISON_CURSE,
		_SKILL_NECROMANCY,
		_SKILL_EXPERT,
		_TARGET_NONE,
		"Morbitus xek venah!",
		5,
		_SPELL_FAST,
		_BOTH_SPELL,
		0,
		TRUE,
		FALSE
	},

	{
		// _SPELL_ACID_CURSE
		castACID_CURSE,
		_SKILL_NECROMANCY,
		_SKILL_EXPERT,
		_TARGET_NONE,
		"Morbitus xek murax!",
		5,
		_SPELL_FAST,
		_BOTH_SPELL,
		0,
		TRUE,
		FALSE
	},

	{
		// _SPELL_POISON_SHIELD
		castPOISON_SHIELD,
		_SKILL_THAUMATURGY,
		_SKILL_MASTER,
		_TARGET_NONE,
		"Domnius venah!",
		7,
		_SPELL_FAST,
		_BOTH_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_ACID_SHIELD
		castACID_SHIELD,
		_SKILL_THAUMATURGY,
		_SKILL_MASTER,
		_TARGET_NONE,
		"Domnius murax!",
		7,
		_SPELL_FAST,
		_BOTH_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_HEAD_OF_DEATH
		castHEAD_OF_DEATH,
		_SKILL_ELEMENTALISM,
		_SKILL_MASTER,
		_TARGET_NONE,
		"Head of painful, instantaneous, and unstoppable death.",
		1,
		_SPELL_FAST,
		_BOTH_SPELL,
		0,
		FALSE,
		TRUE
	},

	{
		// _SPELL_GREATER_IDENTIFY 132
		castGREATER_IDENTIFY,
		_SKILL_SORCERY,
		_SKILL_GRAND_MASTER,
		_TARGET_NONE,
		"Kekris elphux teskhimi revalus!",
		30,
		_SPELL_FAST,
		_BOTH_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_MONSTER_SUMMONING_I
		castMONSTER_SUMMONING_I,
		_SKILL_ELEMENTALISM,
		_SKILL_MASTER,
		_TARGET_NONE,
		"Monster Summoning I",
		10,
		_SPELL_FAST,
		_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_MONSTER_SUMMONING_II
		castMONSTER_SUMMONING_II,
		_SKILL_ELEMENTALISM,
		_SKILL_MASTER,
		_TARGET_NONE,
		"Monster Summoning II",
		20,
		_SPELL_FAST,
		_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_MONSTER_SUMMONING_III
		castMONSTER_SUMMONING_III,
		_SKILL_ELEMENTALISM,
		_SKILL_MASTER,
		_TARGET_NONE,
		"Monster Summoning III",
		30,
		_SPELL_FAST,
		_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_SUMMON_RATLING
		castSUMMON_RATLING,
		_SKILL_ELEMENTALISM,
		_SKILL_GRAND_MASTER,
		_TARGET_NONE,
		"Summon Ratling",
		10,
		_SPELL_FAST,
		_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_SUMMON_BAT
		castSUMMON_BAT,
		_SKILL_ELEMENTALISM,
		_SKILL_GRAND_MASTER,
		_TARGET_NONE,
		"Summon Bat",
		20,
		_SPELL_FAST,
		_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_SUMMON_FENRIS
		castSUMMON_FENRIS,
		_SKILL_ELEMENTALISM,
		_SKILL_GRAND_MASTER,
		_TARGET_NONE,
		"Summon Fenris",
		50,
		_SPELL_FAST,
		_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_SUMMON_IMP
		castSUMMON_IMP,
		_SKILL_ELEMENTALISM,
		_SKILL_GRAND_MASTER,
		_TARGET_NONE,
		"Summon Imp",
		25,
		_SPELL_FAST,
		_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_SUMMON_OGRE
		castSUMMON_OGRE,
		_SKILL_ELEMENTALISM,
		_SKILL_GRAND_MASTER,
		_TARGET_NONE,
		"Summon Ogre",
		100,
		_SPELL_FAST,
		_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_SUMMON_TROLL
		castSUMMON_TROLL,
		_SKILL_ELEMENTALISM,
		_SKILL_GRAND_MASTER,
		_TARGET_NONE,
		"Summon Troll",
		100,
		_SPELL_FAST,
		_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},

	{
		// _SPELL_SUMMON_SERAPH
		castSUMMON_SERAPH,
		_SKILL_ELEMENTALISM,
		_SKILL_GRAND_MASTER,
		_TARGET_NONE,
		"Summon Seraph",
		100,
		_SPELL_FAST,
		_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},
	{
		// _SPELL_REPEL
		castREPEL,
		_SKILL_MYSTICISM,
		_SKILL_GRAND_MASTER,
		_TARGET_NONE,
		"Repello hostilis",
		200,
		_SPELL_FAST,
		_COMBAT_SPELL,
		0,
		TRUE,
		FALSE
	},
	{
		// _SPELL_BURNING_MISTCLOUD
		castBURNING_MISTCLOUD,
		_SKILL_NECROMANCY,
		_SKILL_EXPERT,
		_TARGET_NONE,
		"Nebulosus Ardens",
		100,
		_SPELL_FAST,
		_COMBAT_SPELL,
		0,
		TRUE,
		TRUE
	},
	{
		// _SPELL_NAUSEATING_MISTCLOUD
		castNAUSEATING_MISTCLOUD,
		_SKILL_NECROMANCY,
		_SKILL_MASTER,
		_TARGET_NONE,
		"Nebulosus evomo",
		100,
		_SPELL_FAST,
		_COMBAT_SPELL,
		0,
		TRUE,
		TRUE
	},
	{
		// _SPELL_DEADLY_MISTCLOUD
		castDEADLY_MISTCLOUD,
		_SKILL_NECROMANCY,
		_SKILL_GRAND_MASTER,
		_TARGET_NONE,
		"Nebulosus fatalis",
		100,
		_SPELL_FAST,
		_COMBAT_SPELL,
		0,
		TRUE,
		TRUE
	},
	{
		// _SPELL_SUMMON_DRAGON
		castSUMMON_DRAGON,
		_SKILL_ELEMENTALISM,
		_SKILL_GRAND_MASTER,
		_TARGET_NONE,
		"Summon Dragon",
		400,
		_SPELL_FAST,
		_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	},
	{
		// _SPELL_IMMOLATION_COLD
		castIMMOLATION_COLD,
		_SKILL_ELEMENTALISM,
		_SKILL_GRAND_MASTER,
		_TARGET_NONE,
		"Shivvir tul ershealkn!",
		250,
		_SPELL_MEDIUM,
		_BOTH_SPELL,
		0,
		FALSE,
		FALSE
	},
	{
		// _SPELL_VITALITY
		castVITALITY,
		_SKILL_THAUMATURGY,
		_SKILL_GRAND_MASTER,
		_TARGET_NONE,
		"Ylis vitl yuhma!",
		1500,
		_SPELL_MEDIUM,
		_BOTH_SPELL,
		0,
		FALSE,
		FALSE
	},
	{
		// _SPELL_ENLIGHTENMENT
		castENLIGHTENMENT,
		_SKILL_THAUMATURGY,
		_SKILL_GRAND_MASTER,
		_TARGET_NONE,
		"Ylis speln yuhma!",
		1500,
		_SPELL_MEDIUM,
		_BOTH_SPELL,
		0,
		FALSE,
		FALSE
	},
	{
		// _SPELL_IMMOLATION_LIGHTNING
		castIMMOLATION_LIGHTNING,
		_SKILL_ELEMENTALISM,
		_SKILL_GRAND_MASTER,
		_TARGET_NONE,
		"Elektr tul ershealkn!",
		250,
		_SPELL_MEDIUM,
		_BOTH_SPELL,
		0,
		FALSE,
		FALSE
	},
	{
		// _SPELL_IMP_INVULNERABILITY
		castIMP_INVULNERABILITY,
		_SKILL_THAUMATURGY,
		_SKILL_GRAND_MASTER,
		_TARGET_NONE,
		"Estructos odomniu improvn!",
		115,
		_SPELL_FAST,	// SLOW
		_BOTH_SPELL,
		0,
		FALSE,
		FALSE
	},
	{
		// _SPELL_SUMMON_MIST
		castSUMMON_MIST,
		_SKILL_NECROMANCY,
		_SKILL_GRAND_MASTER,
		_TARGET_NONE,
		"Arise, servant of the mists..",
		500,
		_SPELL_FAST,
		_COMBAT_SPELL,
		0,
		FALSE,
		FALSE
	}
};

//
// *****************************************************************
// miscellaneous utility functions
// *****************************************************************
//

int createProjectile ( int special, WorldObject *source, int targetServID, PackedData *packet, int minDamage, int maxDamage, int projectileCount, char *hitText, char *name )
{
	WorldObject *target = roomMgr->findObject ( targetServID );

	if ( !target )
		return _SPELL_NO_FAILURE;

	int damage = random ( minDamage * projectileCount, maxDamage * projectileCount );
	projectileCount--;

	putMovieText ( source, packet, "%s %s %s with %s for %d damage!\n", source->getName(), hitText, target->getName(), name, damage );

	for ( int i=0; i<projectileCount; i++ ) {
		packet->putByte ( _MOVIE_SPECIAL_EFFECT );
		packet->putByte ( special );
		packet->putByte ( 0 );
		packet->putLong ( target->servID );
	}

	packet->putByte ( _MOVIE_SPECIAL_EFFECT );
	packet->putByte ( special );
	packet->putByte ( 1 );
	packet->putLong ( target->servID );

	target->changeHealth ( -damage, source, 1, 0, packet );

	return _SPELL_NO_FAILURE;
}

int calcSpellCost ( spell_info *spell, WorldObject *caster )
{
	return 1;
}

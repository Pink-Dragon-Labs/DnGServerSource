//
// mlfaery.cpp
//
// Control logic for faeries.
//
// Author: Stephen Nichols
//

#include "roommgr.hpp"

int gEggTimer = 5;
int gDoDropEggs = 1;

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

GoodFaery::GoodFaery()
{
}

GoodFaery::~GoodFaery()
{
}


void GoodFaery::init( void )
{
	new MWActionChill ( 0, this );
	new MWActionWander ( 0, this );
	new MWActionChangeRooms ( 0, this );
	new MWActionGroup ( 0, this );
	new MWActionHeal ( 0, this );
	new MWActionCure ( 0, this );
	new MWActionBless ( 0, this, _SPELL_EMPOWER, _AFF_EMPOWER, _AFF_TYPE_NORMAL );
	new MWActionBless ( 0, this, _SPELL_NIMBILITY, _AFF_POS_DEX_MOD, _AFF_TYPE_NORMAL );

	MWActionRandomEmote* emote = new MWActionRandomEmote ( 0, this );
	emote->add ( "blows you a soft kiss." );
	emote->add ( "dances a happy jig in the air." );
	emote->add ( "daydreams of gentle breezes." );
	emote->add ( "giggles happily." );
	emote->add ( "smiles beatifically." );
}

int GoodFaery::iWantToJump ( WorldObject *thisCharacter )
{
	return ( FALSE );
}

int GoodFaery::spellAttack ( void )
{
	// if we're not shifting, shift...
	if ( !character->hasAffect ( _AFF_SHIFT ) ) {
		setAction ( new CombatCastSpell ( character, _SPELL_SHIFT, character->servID, 0, 0 ) );
		return 1;	
	}

	/* good faery code */

	/* heal myself or test to see if my party members need healing */
	int didHeal = partyHeal();
	if (didHeal) return 1;

	/* if I'm a grand-master thaumaturgist, summon another faery if possible */
	if ( ( character->getSkill ( _SKILL_THAUMATURGY ) == 5 ) &&
		 ( !random ( 0, 3 ) ) )
	{
		if ( getSummonedCount ( _FRIENDLY_SUMMONED_MONSTERS ) <  _MAX_SUMMONED_MONSTERS )
		{
			/* summon another faery */
			setAction ( new CombatCastSpell ( character, _SPELL_SUMMON_FAERY, combatTarget->servID, combatTarget->combatX, combatTarget->combatY ) );
			return 1;
		}
	}

	/* just do a light dart at my skill level */
	setAction ( new CombatCastSpell ( character, _SPELL_LIGHT_DART, combatTarget->servID, 0, 0 ) );
	return 1;
}

// evil faery stuff
EvilFaery::EvilFaery()
{
}

EvilFaery::~EvilFaery()
{
}

void EvilFaery::init ( void )
{
	new MWActionChill ( 0, this );
	new MWActionWander ( 0, this );
	new MWActionChangeRooms ( 0, this );
	new MWActionGroup ( 0, this );
	new MWActionCurse ( 0, this, _SPELL_POISON_CURSE, _AFF_DAMAGE_POISON, _AFF_TYPE_WEAKNESS );
	new MWActionCurse ( 0, this, _SPELL_ACID_CURSE, _AFF_DAMAGE_ACID, _AFF_TYPE_WEAKNESS );
	new MWActionCurse ( 0, this, _SPELL_CURSE_OF_CLUMSINESS, _AFF_NEG_DEX_MOD, _AFF_TYPE_NORMAL );
	new MWActionCurse ( 0, this, _SPELL_ENFEEBLE, _AFF_ENFEEBLE, _AFF_TYPE_NORMAL );

	MWActionRandomEmote* emote = new MWActionRandomEmote ( 0, this );
	emote->add ( "chants a mystical rune." );
	emote->add ( "giggles evilly." );
	emote->add ( "hisses at you." );
	emote->add ( "cackles wickedly." );
	emote->add ( "looks closely at your aura." );
	emote->add ( "sticks her tongue out at you." );
	emote->add ( "thinks about cursing you." );
}

int EvilFaery::spellAttack ( void )
{
	// if we're not shifting, shift...
	if ( !character->hasAffect ( _AFF_SHIFT ) ) {
		setAction ( new CombatCastSpell ( character, _SPELL_SHIFT, character->servID, 0, 0 ) );
		return 1;	
	}

	/* evil faery code */
	/* just cast a random evil spell */
	switch ( random ( 0, 6 ) )
	{
		 case 0:
		 case 1:
		 case 2:
			 setAction ( new CombatCastSpell ( character, _SPELL_ACID_SPHERE, combatTarget->servID, 0, 0 ) );
			 break;

		 case 3:
		 case 4:
		 case 5:
		 	setAction ( new CombatCastSpell ( character, _SPELL_POISON_BOLT, combatTarget->servID, 0, 0 ) );
			 break;

		 case 6:
			if ( !combatTarget->hasAffect ( _AFF_BERSERK ) ) {
				setAction ( new CombatCastSpell ( character, _SPELL_BERSERK, combatTarget->servID, 0, 0 ) );
			} else {
		 		setAction ( new CombatCastSpell ( character, _SPELL_POISON_BOLT, combatTarget->servID, 0, 0 ) );
			}

			break;
	 }

	 return 1;
}


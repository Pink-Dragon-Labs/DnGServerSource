//
// mlmistwiz.cpp
//
// Control logic for Mist Wizards.
//
// Author: Bryan Waters (with Scott Wochholz)
//

#include "roommgr.hpp"
#include "globals.hpp"

//---------- Spells for All -----------
//_SPELL_MASS_FUMBLE (Myst)
//_SPELL_MASS_BERSERK (Myst)
//_SPELL_FEAR (Myst)
//_SPELL_MASS_STUN (Myst)
//_SPELL_DISPEL_MAGIC (Sorc)
//_SPELL_IMPROVED_INVISIBILITY (Sorc)
//_SPELL_DEFENSELESSNESS (Sorc)
//_SPELL_ENIDS_BLESSING (Thau)
//_SPELL_BANISHMENT (Thau)
//_SPELL_CONFUSION
//_SPELL_IRON_CHAINS

//---------- Spells for Good -----------
//_SPELL_CRUSHING_BOULDER (Elem)
//_SPELL_WRATH_OF_THE_GODS (Thau)
//_SPELL_LIGHT_DART (Thau)
//_SPELL_PSYCHIC_ORB (Myst)
//_SPELL_WARP_MIND (Myst)

//---------- Spells for Neutral -----------
//_SPELL_DESPOTHES_WRATH (Elem)
//_SPELL_ELECTRIC_FURY (Elem)
//_SPELL_LIGHTNING_BOLT (Elem)
//_SPELL_FLAME_ORB (Elem)
//_SPELL_ICE_ORB (Elem)

//---------- Spells for Evil -----------
//_SPELL_DUACHS_VENGEANCE (Necro)
//_SPELL_MASS_DRAIN (Necro)
//_SPELL_DEATH_WISH (Necro)
//_SPELL_POISON_BOLT (Necro)


// ------------------------ Good Mist Wizard -----------------------------
GoodWizard::GoodWizard() {
}

GoodWizard::~GoodWizard() {
}

void GoodWizard::init( void ) {
	new MWActionChill ( 0, this );
	new MWActionWander ( 0, this );
	new MWActionChangeRooms ( 0, this );
	new MWActionGroup ( 0, this );
	new MWActionCurse ( 0, this, _SPELL_DEFENSELESSNESS, _AFF_DEFENSELESS, _AFF_TYPE_NORMAL );
	new MWActionCurse ( 0, this, _SPELL_ENFEEBLE, _AFF_ENFEEBLE, _AFF_TYPE_NORMAL );
	new MWActionCurse ( 0, this, _SPELL_CURSE_OF_CLUMSINESS, _AFF_NEG_DEX_MOD, _AFF_TYPE_NORMAL );

	MWActionRandomEmote* emote = new MWActionRandomEmote ( 0, this );
	emote->add ( "scorns the devotee of the current gods." );
	emote->add ( "turns his eyes upon you." );
	emote->add ( "implores the ancient gods to enlighten the disbelievers." );
	emote->add ( "chortles derisively." );
	emote->add ( "gazes at you with disapproval." );
}

int GoodWizard::iWantToJump ( WorldObject *thisCharacter ) {
	return ( FALSE );
}

int GoodWizard::spellAttack ( void ) {
	/* Scott: Would prefer the healing done here be by _SPELL_ENIDS_BLESSING */
	/* ----------- Heal myself or test if my group needs healing -----------*/
	int didHeal = partyHeal();
	if (didHeal) return 1;


	/* ----------- Cast Banishment if there are enemy summoned creatures -----------*/
	if ( (!random ( 0, 1 )) &&
		 ( getSummonedCount ( _ENEMY_SUMMONED_MONSTERS ) ) )
	{
		setAction ( new CombatCastSpell ( character, _SPELL_BANISHMENT, character->servID, 0, 0 ) );
		return 1;
	}

	/* ----------- If we're not Invisible, become so -----------*/
	if ( !character->hasAffect ( _AFF_IMPROVED_INVISIBILITY ) ) {
		setAction ( new CombatCastSpell ( character, _SPELL_IMPROVED_INVISIBILITY, character->servID, 0, 0 ) );
		return 1;	
	}

	/* ----------- Pick an Attack -----------*/
	switch ( random ( 0, 16 ) )
	{
		case 0:
		case 1:
		  setAction ( new CombatCastSpell ( character, _SPELL_CRUSHING_BOULDER, combatTarget->servID, 0, 0 ) );
		  break;
		case 2:
		case 3:
		case 4:
		case 5: 
		  setAction ( new CombatCastSpell ( character, _SPELL_WRATH_OF_THE_GODS, combatTarget->servID, 0, 0 ) );
		  break;
		case 6:
		case 7:
		  setAction ( new CombatCastSpell ( character, _SPELL_WARP_MIND, combatTarget->servID, 0, 0 ) );
		  break;
		case 8: 
		  setAction ( new CombatCastSpell ( character, _SPELL_PSYCHIC_ORB, combatTarget->servID, 0, 0 ) );
		  break;
		case 9:
		  setAction ( new CombatCastSpell ( character, _SPELL_MASS_FUMBLE, combatTarget->servID, 0, 0 ) );
		  break;
		case 10:
		  setAction ( new CombatCastSpell ( character, _SPELL_IRON_CHAINS, combatTarget->servID, 0, 0 ) );
		  break;
		case 11:
		  setAction ( new CombatCastSpell ( character, _SPELL_FEAR, combatTarget->servID, 0, 0 ) );
		  break;
		case 12:
		  setAction ( new CombatCastSpell ( character, _SPELL_MASS_STUN, combatTarget->servID, 0, 0 ) );
		  break;
		case 13:
		  setAction ( new CombatCastSpell ( character, _SPELL_CONFUSION, combatTarget->servID, 0, 0 ) );
		  break;
		case 14: {
		// ------ Check if Target is Defensless, if not then do it
		  if ( !combatTarget->hasAffect ( _AFF_DEFENSELESS ) ) {
		   setAction ( new CombatCastSpell ( character, _SPELL_DEFENSELESSNESS, combatTarget->servID, 0, 0 ) );
			} 
		// ------ If Target already Defenseless...
	  	  else {
		  setAction ( new CombatCastSpell ( character, _SPELL_LIGHT_DART, combatTarget->servID, 0, 0 ) );
		  }
		  break;
		}			// ~~~~~ End case 14
		case 15: {
		// ------ Check if Target is Beserked, if not then do it
	  	  if ( !combatTarget->hasAffect ( _AFF_BERSERK ) ) {
		   setAction ( new CombatCastSpell ( character, _SPELL_MASS_BERSERK, combatTarget->servID, 0, 0 ) );
			} 
		// ------ If already Beserked...
	  	  else {
		  setAction ( new CombatCastSpell ( character, _SPELL_LIGHT_DART, combatTarget->servID, 0, 0 ) );
		  }
	  	  break;
		}			// ~~~~~ End case 15
		case 16: {
		// ------ Check if Target has Invul Shield, if so Dispel it
	  	  if ( combatTarget->hasAffect ( _AFF_INVULNERABLE ) ) {
		   setAction ( new CombatCastSpell ( character, _SPELL_DISPEL_MAGIC, combatTarget->servID, 0, 0 ) );
		  } 
		// ------ Check if Target can See Invis, if so Dispel it
	  	  else if ( combatTarget->hasAffect ( _AFF_SEE_INVISIBLE ) ) {
		   setAction ( new CombatCastSpell ( character, _SPELL_DISPEL_MAGIC, combatTarget->servID, 0, 0 ) );
		  }
		// ------ Check if Target is Invis, if so Dispel it
	  	  else if ( combatTarget->hasAffect ( _AFF_IMPROVED_INVISIBILITY ) ) {
		   setAction ( new CombatCastSpell ( character, _SPELL_DISPEL_MAGIC, combatTarget->servID, 0, 0 ) );
		  }
		// ------ If no need to Dispel...
	  	  else {
		  setAction ( new CombatCastSpell ( character, _SPELL_LIGHT_DART, combatTarget->servID, 0, 0 ) );
		  }
	  	  break;
		}			// ~~~~~ End case 16
	}				// ~~~~~ End switch
	
	return 1;
}					// ~~~~~ End GoodWizard::spellAttack

// ------------------------ Neutral Mist Wizard -----------------------------
NeutralWizard::NeutralWizard() {
}

NeutralWizard::~NeutralWizard() {
}

void NeutralWizard::init( void ) {
	new MWActionChill ( 0, this );
	new MWActionWander ( 0, this );
	new MWActionChangeRooms ( 0, this );
	new MWActionGroup ( 0, this );
	new MWActionCurse ( 0, this, _SPELL_FIRE_CURSE, _AFF_DAMAGE_FIRE, _AFF_TYPE_WEAKNESS );
	new MWActionCurse ( 0, this, _SPELL_COLD_CURSE, _AFF_DAMAGE_COLD, _AFF_TYPE_WEAKNESS );
	new MWActionCurse ( 0, this, _SPELL_ELECTRIC_CURSE, _AFF_DAMAGE_LIGHTNING, _AFF_TYPE_WEAKNESS );
	new MWActionCurse ( 0, this, _SPELL_DEFENSELESSNESS, _AFF_DEFENSELESS, _AFF_TYPE_NORMAL );
	new MWActionCurse ( 0, this, _SPELL_ENFEEBLE, _AFF_ENFEEBLE, _AFF_TYPE_NORMAL );
	new MWActionCurse ( 0, this, _SPELL_CURSE_OF_CLUMSINESS, _AFF_NEG_DEX_MOD, _AFF_TYPE_NORMAL );

	MWActionRandomEmote* emote = new MWActionRandomEmote ( 0, this );
	emote->add ( "scoffs at the believer of the new gods." );
	emote->add ( "turns his eyes upon you." );
	emote->add ( "prays to the ancient gods for their return." );
	emote->add ( "snickers sardonically." );
	emote->add ( "stares at you with disdain." );
}

int NeutralWizard::iWantToJump ( WorldObject *thisCharacter ) {
	return ( FALSE );
}

int NeutralWizard::spellAttack ( void ) {
	/* Scott: Would prefer the healing done here be by _SPELL_ENIDS_BLESSING */
	/* ----------- Heal myself or test if my group needs healing -----------*/
	int didHeal = partyHeal();
	if (didHeal) return 1;


	/* ----------- Cast Banishment if there are enemy summoned creatures -----------*/
	if ( (!random ( 0, 1 )) && ( getSummonedCount ( _ENEMY_SUMMONED_MONSTERS ) ) ) {
		setAction ( new CombatCastSpell ( character, _SPELL_BANISHMENT, character->servID, 0, 0 ) );
		return 1;
	}

	/* ----------- If we're not Invisible, become so -----------*/
	if ( !character->hasAffect ( _AFF_IMPROVED_INVISIBILITY ) ) {
		setAction ( new CombatCastSpell ( character, _SPELL_IMPROVED_INVISIBILITY, character->servID, 0, 0 ) );
		return 1;	
	}

	/* ----------- Pick an Attack -----------*/
	switch ( random ( 0, 16 ) ) {
		case 0:
		case 1:
			setAction ( new CombatCastSpell ( character, _SPELL_DESPOTHES_WRATH, combatTarget->servID, 0, 0 ) );
		break;
		case 2:
		case 3:
		case 4:
		case 5: {
				// ------ Check if Target is Electric Cursed, if not then do it
				if ( !combatTarget->hasAffect ( _AFF_DAMAGE_LIGHTNING, _AFF_TYPE_WEAKNESS ) ) {
					setAction ( new CombatCastSpell ( character, _SPELL_ELECTRIC_CURSE, combatTarget->servID, 0, 0 ) );
				} else { // ------ If already Electric Cursed, zap 'em
					setAction ( new CombatCastSpell ( character, _SPELL_ELECTRIC_FURY, character->servID, 0, 0 ) );
				}
			}
			break;
		case 6:
		case 7:
		case 8: {
				// ------ Check if Target is Electric Cursed, if not then do it
  				if ( !combatTarget->hasAffect ( _AFF_DAMAGE_LIGHTNING, _AFF_TYPE_WEAKNESS ) ) {
					setAction ( new CombatCastSpell ( character, _SPELL_ELECTRIC_CURSE, combatTarget->servID, 0, 0 ) );
				} else { // ------ If already Electric Cursed, zap 'em
					setAction ( new CombatCastSpell ( character, _SPELL_LIGHTNING_BOLT, character->servID, 0, 0 ) );
				}
			}
			break;
		case 9:
			setAction ( new CombatCastSpell ( character, _SPELL_MASS_FUMBLE, combatTarget->servID, 0, 0 ) );
		break;
		case 10:
			setAction ( new CombatCastSpell ( character, _SPELL_IRON_CHAINS, combatTarget->servID, 0, 0 ) );
		break;
		case 11:
			setAction ( new CombatCastSpell ( character, _SPELL_FEAR, combatTarget->servID, 0, 0 ) );
		break;
		case 12:
			setAction ( new CombatCastSpell ( character, _SPELL_MASS_STUN, combatTarget->servID, 0, 0 ) );
		break;
		case 13:
			setAction ( new CombatCastSpell ( character, _SPELL_CONFUSION, combatTarget->servID, 0, 0 ) );
		break;
		case 14: {
				// ------ Check if Target is Defensless, if not then do it
				if ( !combatTarget->hasAffect ( _AFF_DEFENSELESS ) ) {
					setAction ( new CombatCastSpell ( character, _SPELL_DEFENSELESSNESS, combatTarget->servID, 0, 0 ) );
				} else { // ------ If Target already Defenseless...
					// ------ Check if Target is Fire Cursed, if not then do it
					if ( !combatTarget->hasAffect ( _AFF_DAMAGE_FIRE, _AFF_TYPE_WEAKNESS ) ) {
						setAction ( new CombatCastSpell ( character, _SPELL_FIRE_CURSE, combatTarget->servID, 0, 0 ) );
					} else { // ------ If already Fire Cursed, burn 'em
						setAction ( new CombatCastSpell ( character, _SPELL_FLAME_ORB, character->servID, 0, 0 ) );
					}
				}
			}	// ~~~~~ End case 14

			break;
		case 15: {
				// ------ Check if Target is Beserked, if not then do it
				if ( !combatTarget->hasAffect ( _AFF_BERSERK ) ) {
					setAction ( new CombatCastSpell ( character, _SPELL_MASS_BERSERK, combatTarget->servID, 0, 0 ) );
				} else { // ------ If already Beserked...
					// ------ Check if Target is Fire Cursed, if not then do it
					if ( !combatTarget->hasAffect ( _AFF_DAMAGE_FIRE, _AFF_TYPE_WEAKNESS ) ) {
						setAction ( new CombatCastSpell ( character, _SPELL_FIRE_CURSE, combatTarget->servID, 0, 0 ) );
					} else { // ------ If already Fire Cursed, burn 'em
						setAction ( new CombatCastSpell ( character, _SPELL_FLAME_ORB, character->servID, 0, 0 ) );
					}
				}
			}

			break;
		case 16: {
				// ------ Check if Target has Invul Shield, if so Dispel it
				if ( combatTarget->hasAffect ( _AFF_INVULNERABLE ) ) {
					setAction ( new CombatCastSpell ( character, _SPELL_DISPEL_MAGIC, combatTarget->servID, 0, 0 ) );
				} else if ( combatTarget->hasAffect ( _AFF_SEE_INVISIBLE ) ) {
					// ------ Check if Target can See Invis, if so Dispel it
					setAction ( new CombatCastSpell ( character, _SPELL_DISPEL_MAGIC, combatTarget->servID, 0, 0 ) );
				} else if ( combatTarget->hasAffect ( _AFF_IMPROVED_INVISIBILITY ) ) {
					// ------ Check if Target is Invis, if so Dispel it
					setAction ( new CombatCastSpell ( character, _SPELL_DISPEL_MAGIC, combatTarget->servID, 0, 0 ) );
				} else {
					// ------ If no need to Dispel...
					// ------ Check if Target is Cold Cursed, if not then do it
					if ( !combatTarget->hasAffect ( _AFF_DAMAGE_COLD, _AFF_TYPE_WEAKNESS ) ) {
						setAction ( new CombatCastSpell ( character, _SPELL_COLD_CURSE, combatTarget->servID, 0, 0 ) );
					} else { // ------ If already Poison Cold , ice 'em
						setAction ( new CombatCastSpell ( character, _SPELL_ICE_ORB, character->servID, 0, 0 ) );
					}
				}			// ~~~~~ End outer else
			}

			break;
	}				// ~~~~~ End switch

	return 1;
}					// ~~~~~ End NeutralWizard::spellAttack

// ------------------------ Evil Mist Wizard -----------------------------
EvilWizard::EvilWizard() {
}

EvilWizard::~EvilWizard() {
}

void EvilWizard::init( void ) {
	new MWActionChill ( 0, this );
	new MWActionWander ( 0, this );
	new MWActionChangeRooms ( 0, this );
	new MWActionGroup ( 0, this );
	new MWActionCurse ( 0, this, _SPELL_POISON_CURSE, _AFF_DAMAGE_POISON, _AFF_TYPE_WEAKNESS );
	new MWActionCurse ( 0, this, _SPELL_DEFENSELESSNESS, _AFF_DEFENSELESS, _AFF_TYPE_NORMAL );
	new MWActionCurse ( 0, this, _SPELL_ENFEEBLE, _AFF_ENFEEBLE, _AFF_TYPE_NORMAL );
	new MWActionCurse ( 0, this, _SPELL_CURSE_OF_CLUMSINESS, _AFF_NEG_DEX_MOD, _AFF_TYPE_NORMAL );

	MWActionRandomEmote* emote = new MWActionRandomEmote ( 0, this );
	emote->add ( "sneers at the zealot of the false gods." );
	emote->add ( "turns his eyes upon you." );
	emote->add ( "invokes the ancient gods to rain destruction upon the unbelievers." );
	emote->add ( "chuckles disdainfully." );
	emote->add ( "glares at you with contempt." );
}

int EvilWizard::iWantToJump ( WorldObject *thisCharacter ) {
	return ( FALSE );
}

int EvilWizard::spellAttack ( void ) {
	/* Scott: Would prefer the healing done here be by _SPELL_ENIDS_BLESSING */
	/* ----------- Heal myself or test if my group needs healing -----------*/
	if ( partyHeal() )
		return 1;

	/* ----------- Cast Banishment if there are enemy summoned creatures -----------*/
	if ( (!random ( 0, 1 )) && ( getSummonedCount ( _ENEMY_SUMMONED_MONSTERS ) ) ) {
		setAction ( new CombatCastSpell ( character, _SPELL_BANISHMENT, character->servID, 0, 0 ) );
		return 1;
	}

	/* ----------- If we're not Invisible, become so -----------*/
	if ( !character->hasAffect ( _AFF_IMPROVED_INVISIBILITY ) ) {
		setAction ( new CombatCastSpell ( character, _SPELL_IMPROVED_INVISIBILITY, character->servID, 0, 0 ) );
		return 1;	
	}
 
	/* ----------- Pick an Attack -----------*/
	switch ( random ( 0, 16 ) ) {
		case 0:
		case 1:
		case 2:
			setAction ( new CombatCastSpell ( character, _SPELL_MASS_DRAIN, combatTarget->servID, 0, 0 ) );
			break;
		case 3:
		case 4:
		case 5:
			setAction ( new CombatCastSpell ( character, _SPELL_DUACHS_VENGEANCE, combatTarget->servID, 0, 0 ) );
			break;
		case 6:
		case 7:
		case 8:
			setAction ( new CombatCastSpell ( character, _SPELL_DEATH_WISH, combatTarget->servID, 0, 0 ) );
			break;
		case 9:
			setAction ( new CombatCastSpell ( character, _SPELL_MASS_FUMBLE, combatTarget->servID, 0, 0 ) );
			break;
		case 10:
			setAction ( new CombatCastSpell ( character, _SPELL_IRON_CHAINS, combatTarget->servID, 0, 0 ) );
			break;
		case 11:
			setAction ( new CombatCastSpell ( character, _SPELL_FEAR, combatTarget->servID, 0, 0 ) );
			break;
		case 12:
			setAction ( new CombatCastSpell ( character, _SPELL_MASS_STUN, combatTarget->servID, 0, 0 ) );
			break;
		case 13:
			setAction ( new CombatCastSpell ( character, _SPELL_CONFUSION, combatTarget->servID, 0, 0 ) );
			break;
		case 14: {
				// ------ Check if Target is Defensless, if not then do it
				if ( !combatTarget->hasAffect ( _AFF_DEFENSELESS ) ) {
					setAction ( new CombatCastSpell ( character, _SPELL_DEFENSELESSNESS, combatTarget->servID, 0, 0 ) );
				} else {	// ------ If Target already Defenseless...
					// ------ Check if Target is Poisoned Cursed, if not then do it
					if ( !combatTarget->hasAffect ( _AFF_DAMAGE_POISON, _AFF_TYPE_WEAKNESS ) ) {
						setAction ( new CombatCastSpell ( character, _SPELL_POISON_CURSE, combatTarget->servID, 0, 0 ) );
					} else {	// ------ If already Poison Cursed, hit 'em with Poison
						setAction ( new CombatCastSpell ( character, _SPELL_POISON_BOLT, combatTarget->servID, 0, 0 ) );
					}			// ~~~~~ End inner else
				}
			}

			break;
		case 15: {
				// ------ Check if Target is Beserked, if not then do it
				if ( !combatTarget->hasAffect ( _AFF_BERSERK ) ) {
					setAction ( new CombatCastSpell ( character, _SPELL_MASS_BERSERK, combatTarget->servID, 0, 0 ) );
				} else {	// ------ If already Beserked...
					// ------ Check if Target is Poisoned Cursed, if not then do it
					if ( !combatTarget->hasAffect ( _AFF_DAMAGE_POISON, _AFF_TYPE_WEAKNESS ) ) {
						setAction ( new CombatCastSpell ( character, _SPELL_POISON_CURSE, combatTarget->servID, 0, 0 ) );
					} else {
						// ------ If already Poison Cursed, hit 'em with Poison
						setAction ( new CombatCastSpell ( character, _SPELL_POISON_BOLT, combatTarget->servID, 0, 0 ) );
					}			// ~~~~~ End inner else
				}
			}

			break;
		case 16: {
				// ------ Check if Target has Invul Shield, if so Dispel it
				if ( combatTarget->hasAffect ( _AFF_INVULNERABLE ) ) {
					setAction ( new CombatCastSpell ( character, _SPELL_DISPEL_MAGIC, combatTarget->servID, 0, 0 ) );
				} else if ( combatTarget->hasAffect ( _AFF_SEE_INVISIBLE ) ) {	// ------ Check if Target can See Invis, if so Dispel it
					setAction ( new CombatCastSpell ( character, _SPELL_DISPEL_MAGIC, combatTarget->servID, 0, 0 ) );
				} else if ( combatTarget->hasAffect ( _AFF_IMPROVED_INVISIBILITY ) ) {	// ------ Check if Target is Invis, if so Dispel it
					setAction ( new CombatCastSpell ( character, _SPELL_DISPEL_MAGIC, combatTarget->servID, 0, 0 ) );
				} else {	// ------ If no need to Dispel...
					// ------ Check if Target is Poisoned Cursed, if not then do it
					if ( !combatTarget->hasAffect ( _AFF_DAMAGE_POISON, _AFF_TYPE_WEAKNESS ) ) {
						setAction ( new CombatCastSpell ( character, _SPELL_POISON_CURSE, combatTarget->servID, 0, 0 ) );
					} else {	// ------ If already Poison Cursed, hit 'em with Poison
						setAction ( new CombatCastSpell ( character, _SPELL_POISON_BOLT, combatTarget->servID, 0, 0 ) );
					}			// ~~~~~ End inner else
				}			// ~~~~~ End outer else
			}

			break;
	}				// ~~~~~ End switch

	return 1;
}					// ~~~~~ End EvilWizard::spellAttack

/*
	ACTIONS.HPP
	Global action functions

	Author: Stephen Nichols
*/

#ifndef _ACTIONS_HPP_
#define _ACTIONS_HPP_

class WorldObject;
class PackedData;

// define the action function pointer
typedef int(*action_t)(WorldObject *object, PackedData *packet, int argc, char **argv);

#define ACTION_FN(x) int x ( WorldObject *object, PackedData *packet, int argc, char ** argv )

// teleport someone somewhere
ACTION_FN ( actTeleport );

// damage some object
ACTION_FN ( actDamage );

// heal some object
ACTION_FN ( actHeal );

// open some object
ACTION_FN ( actOpen );

// close some object
ACTION_FN ( actClose );

// lock some object
ACTION_FN ( actLock );

// unlock some object
ACTION_FN ( actUnlock );

// activate a switch 
ACTION_FN ( actActivate );

// deactivate a switch
ACTION_FN ( actDeactivate );

// enable a switch
ACTION_FN ( actEnable );

// disable a switch
ACTION_FN ( actDisable );

// create an object somewhere
ACTION_FN ( actCreateObj );

// summon a creature into the room
ACTION_FN ( actSummon );

// place text over object
ACTION_FN ( actFloatingText );

// generate a zone "chat" message
ACTION_FN ( actZoneMsg );

// generate a room "chat" message
ACTION_FN ( actRoomMsg );

// play a sound effect
ACTION_FN ( actPlaySound );

// play a midi file
ACTION_FN ( actPlayMusic );

// cast a spell on an object
ACTION_FN ( actCastSpell );

// cast a spell on everyone in the room
ACTION_FN ( actCastSpellRoom );

// destroy an object by name
ACTION_FN ( actDestroyObj );

// open an object for a number of minutes and force it closed again
ACTION_FN ( actTimedOpen );

// clear all actions on a particular object that use the passed verb
ACTION_FN ( actClearActions );

// add a new action function to a particular object
ACTION_FN ( actAddAction );

// set an affect on an object
ACTION_FN ( actSetAffect );

// set an affect on an object
ACTION_FN ( actClearAffect );

// kill an object
ACTION_FN ( actKill );

// force owner to drop object
ACTION_FN ( actForceDrop );

// damn an object
ACTION_FN ( actDamn );

// inspire an object
ACTION_FN ( actInspire );

// change experience
ACTION_FN ( actChangeExperience );

// give the mark (or curse) of a God to someone...
ACTION_FN ( actEnidMark );
ACTION_FN ( actDuachMark );
ACTION_FN ( actDespothesMark );	//MIKE

// player state management...
ACTION_FN ( actChangeManaDrain );
ACTION_FN ( actChangeMeleePhase );
ACTION_FN ( actChangeEvilMDMMod );
ACTION_FN ( actChangeGoodMDMMod );
ACTION_FN ( actChangeMeleeArmorPiercing );
ACTION_FN ( actChangeMystImmunityCount );
ACTION_FN ( actChangeCastResistance );
ACTION_FN ( actChangeSpellResistance );
ACTION_FN ( actChangeSDM );

ACTION_FN ( actMinotaurSoulConsume );

ACTION_FN ( actAddForwardProc );
ACTION_FN ( actAddReverseProc );
ACTION_FN ( actAddForwardProcChance );
ACTION_FN ( actAddReverseProcChance );

ACTION_FN ( actGemChest_vClose );
ACTION_FN ( actGemChest_vLock );

#endif

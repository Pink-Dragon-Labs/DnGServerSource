//
// ambushgroup
//
// This file contains the CAmbushGroup class.
//
// author: Stephen Nichols
//

#include "roommgr.hpp"
#include "ambushgroup.hpp"

//
// CAmbushGroup: This class represents a group of monsters that can be
// spawned to attack a player.
//

CAmbushGroup::CAmbushGroup()
{
	m_nChance = 1;
	m_nLevel = 0;
}

CAmbushGroup::~CAmbushGroup()
{
	m_MonsterList.release();
}

// call to set the chance that this group will appear
void CAmbushGroup::SetChance ( int nChance )
{
	m_nChance = nChance;
}

// call to get the current chance value
int CAmbushGroup::GetChance ( void )
{
	return m_nChance;
}

// call to get the current level value
int CAmbushGroup::GetLevel ( void )
{
	return m_nLevel;
}

// call to add a new monster to this grouping
void CAmbushGroup::AddMonster ( char *pName )
{
	WorldObject *pClass = roomMgr->findClass ( pName );

	if ( pClass ) {
		m_MonsterList.add ( pClass );

		// count the level of this monster...
		if ( (pClass->maxLevel > 1) ) {
			m_nLevel += pClass->maxLevel;
		} else {
			m_nLevel += pClass->level;
		}
	}
}

// call to spawn this group of monsters to attack the given player
void CAmbushGroup::SpawnAndAttack ( WorldObject *pCharacter )
{
	if ( !m_MonsterList.size() ) 
		return;

	RMPlayer *pPlayer = pCharacter->player;
	RMRoom *pRoom = pCharacter->room;

	// step through the monsters and create a group of them
	LinkedElement *pElement = m_MonsterList.head();
	NPC *pFirstNPC = NULL;

	// SNTODO: remove this story-line specific code...
	// always put a hell soul at the front of an ambush group...
	{
		WorldObject *pObj = pRoom->addObject ( "HellSoul", pCharacter->x, pCharacter->y, pCharacter->loop, 0 );

		// make the HellSoul invisible...
		pObj->addAffect ( _AFF_IMPROVED_INVISIBILITY, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, 100, 5000, NULL, 0 );

		// create the NPC and add it to the room...
		pFirstNPC = makeNPC ( pObj );
		pFirstNPC->newRoom ( pRoom );

		pFirstNPC->aiReady = 1;
	}

	// step through the monsters and create a group of them
	while ( pElement ) {
		WorldObject *pClass = (WorldObject *)pElement->ptr();
		pElement = pElement->next();

		WorldObject *pObj = pRoom->addObject ( pClass->classID, pCharacter->x, pCharacter->y, pCharacter->loop, 0 );

		NPC *pNPC = makeNPC ( pObj );
		pNPC->newRoom ( pRoom );

		if ( pFirstNPC ) {
			pNPC->joinGroup ( pFirstNPC );
		} else {
			pFirstNPC = pNPC;
		}

		pNPC->aiReady = 1;
	}

	// make a movie packet...
	PackedMsg packet;
	packet.putLong ( pFirstNPC->servID );
	packet.putLong ( pRoom->number );

	pFirstNPC->engage ( pCharacter, &packet, pCharacter->x, pCharacter->y );
	pFirstNPC->character->combatGroup->ambushAttack = 1;

	putMovieText ( pFirstNPC->character, &packet, "|c60|You've been ambushed!|c43|" );

	packet.putByte ( _MOVIE_END );
	roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, packet.data(), packet.size(), pRoom );
}

//
// zonescripts
//
// This module contains various CZoneScripts.
//
// author: Stephen Nichols
//

#include "zone.hpp"
#include "zonescripts.hpp"
#include "rmroom.hpp"
#include "wobject.hpp"
#include "roommgr.hpp"

//
// CEnidDngZoneScript: This script handles special behavior for the 
// enid dungeon zone.
//

CEnidDngZoneScript::CEnidDngZoneScript()
{
	m_nCycles = 2;
}

CEnidDngZoneScript::~CEnidDngZoneScript()
{
}

// handle changing states...
void CEnidDngZoneScript::ChangeState ( int nNewState )
{
	m_nState = nNewState;

	logDisplay ( "enidScript ChangeState %d", nNewState );

	switch ( m_nState ) {
		// send an earthquake to everyone in our zone...
		case 0: {
			Zone *pZone = GetZone();

			// step through each room and send an EQ special effect to any
			// with players in it...
			LinkedElement *pElement = pZone->rooms.head();

			while ( pElement ) {
				RMRoom *pRoom = (RMRoom *)pElement->ptr();
				pElement = pElement->next();

				if ( pRoom && pRoom->size() ) {
					// get the first player in the room for owning the command
					RMPlayer *pPlayer = (RMPlayer *)pRoom->head()->ptr();

					PackedMsg packet;
					packet.putLong ( pPlayer->servID );
					packet.putLong ( pRoom->number );

					packet.putByte ( _MOVIE_SPECIAL_EFFECT );
					packet.putLong ( pPlayer->servID );
					packet.putByte ( _SE_EARTHQUAKE );
					packet.putByte ( 0 );
					packet.putLong ( pPlayer->servID );

					packet.putByte ( _MOVIE_END );

					roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, packet.data(), packet.size(), pRoom );
				}
			}

			m_nState--;
			m_nCycles = random ( 5, 10 );
		}

		break;
	}
}

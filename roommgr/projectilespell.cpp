//
// projectilespell
//
// This file contains the CProjectileSpell class.
//
// author: Stephen Nichols
//

#include "projectilespell.hpp"
#include "roommgr.hpp"

//
// CProjectileSpell: This class represents a spell that generates projectiles
// and hurls them toward a target.  These projectiles cause damage.
//

CProjectileSpell::CProjectileSpell()
{
	m_nMinDamage = 3;
	m_nMaxDamage = 7;
	m_nDamageType = WorldObject::_DAMAGE_CUT;
	m_nSpecialEffect = _SE_MULTI_BLADE;
	m_bMultiEffect = 1;
	m_bSpreadDamage = 0;
	m_bCombatAllowed = 1;
	m_bNonCombatAllowed = 0;
}

CProjectileSpell::~CProjectileSpell()
{
}

// call to cast this spell
affect_t *CProjectileSpell::Cast ( WorldObject *pCaster, int nTargetID, int nTargetX, int nTargetY, char *pOutputString, PackedData *pPacket )
{
	WorldObject *pTarget = roomMgr->findObject ( nTargetID );

	if ( !pTarget || !pTarget->player ) {
		strcat ( pOutputString, "No target. " );
		return NULL;
	}

	// point to the real caster...
	pCaster = pCaster->getBaseOwner();

	// get the SDM of the caster...
	int nSDM = calcSpellMod ( pCaster );

	// calculate the damage to do...
	int nDamage = random ( m_nMinDamage, m_nMaxDamage );

	// adjust the damage by the SDM
	nDamage = (nDamage * nSDM) / 100;

	// calculate the number of projectiles to make based on the damage done
	int nProjectileCount = ( 7 <? ( 1 >? (nDamage / 75) ) );

	// put the special effect information out...
	if ( m_bMultiEffect ) {
		pPacket->putByte ( _MOVIE_SPECIAL_EFFECT );
		pPacket->putLong ( pCaster->servID );
		pPacket->putByte ( m_nSpecialEffect );
		pPacket->putByte ( 1 );

		if ( m_bSpreadDamage ) {
			pPacket->putByte ( nProjectileCount * pCaster->opposition->size() );

			// create a set of projectiles for each valid target...
			LinkedElement *pElement = pCaster->opposition->head();

			while ( pElement ) {
				WorldObject *pCurTarget = (WorldObject *)pElement->ptr();
				pElement = pElement->next();

				for ( int i=0; i<nProjectileCount; i++ ) {
					pPacket->putLong ( pCurTarget->servID );
				}
			}
		} else {
			// we're not spreading damage, so the target gets them all
			pPacket->putByte ( nProjectileCount );

			for ( int i=0; i<nProjectileCount; i++ ) {
				pPacket->putLong ( nTargetID );
			}
		}
	}

	// deal the damage that was done...
	if ( m_bSpreadDamage ) {
		// create a set of projectiles for each valid target...
		LinkedElement *pElement = pCaster->opposition->head();

		while ( pElement ) {
			WorldObject *pCurTarget = (WorldObject *)pElement->ptr();
			pElement = pElement->next();

			pCurTarget->takeDamage ( m_nDamageType, pCaster, nDamage, pOutputString, pPacket, 1 );
		}
	} else {
		pTarget->takeDamage ( m_nDamageType, pCaster, nDamage, pOutputString, pPacket, 1 );
	}

	return NULL;
}

/*
	BContainer class	
	author: Stephen Nichols
*/

#include "bcontain.hpp"
#include "roommgr.hpp"

BContainer::BContainer ( WorldObject *obj ) : WorldObjectBase ( obj ) {
	weight = bulk = 0;
	weightReduction = bulkReduction = 0;
	weightCapacity = bulkCapacity = 1;
	type = _BCONTAIN;

	m_pAccept = NULL;
	m_nAccept = 0;
}

BContainer::~BContainer() {
	// step through and validate the contents of this container
	LinkedElement *element = contents.head();

	while ( element ) {
		WorldObject *obj = (WorldObject *)element->ptr();
		LinkedElement *next = element->next();

		if ( !isValidPtr ( obj ) ) {
			logInfo ( _LOG_ALWAYS, "Invalid pointer (0x%x) in BContainer destructor.", obj );
			contents.delElement ( element );
		}

		else if ( getPtrType ( obj ) != _MEM_WOBJECT ) {
			logInfo ( _LOG_ALWAYS, "Valid pointer (0x%x) in BContainer destructor is not a WorldObject (type = %d).", obj, getPtrType ( obj ) );
			contents.delElement ( element );
		}

		else if ( !obj->isOwnedBy ( self ) ) {
			logInfo ( _LOG_ALWAYS, "Valid WorldObject (0x%x) in BContainer destructor does not belong to self.", obj );
			contents.delElement ( element );
		}

		element = next;
	}

	if ( m_pAccept ) 
		delete m_pAccept;
}

void BContainer::copy ( WorldObjectBase *theBase ) {
	BContainer *base = (BContainer *)theBase;

	weight = base->weight;
	bulk = base->bulk;
	weightCapacity = base->weightCapacity;
	bulkCapacity = base->bulkCapacity;
	weightReduction = base->weightReduction;
	bulkReduction = base->bulkReduction;

	if ( 1 ) {
		LinkedElement *element = base->contents.head();

		while ( element ) {
			WorldObject *object = (WorldObject *)element->ptr();
			element = element->next();

			WorldObject *clone = NULL;

			if ( roomMgr->initted ) {
				if ( object->randomChance ) {
					if ( random ( 1, 100 ) > object->randomChance ) {
						continue;
					}
				}

				BTreasure *btreasure = (BTreasure *)object->getBase ( _BTREASURE );

				if ( btreasure ) {
					clone = btreasure->makeObj();

					if ( !clone ) {
						continue;
					}
				}
			}

			if ( !clone )
				clone = object->clone();;

			clone->addToDatabase();
			clone->forceIn ( self );

			BWearable *bwear = (BWearable *)object->getBase ( _BWEAR );
			BWeapon *bweapon = (BWeapon *)object->getBase ( _BWEAPON );
			BHead *bhead = (BHead *)object->getBase ( _BHEAD );

			if ( bhead )
				clone->isVisible = 1;

			if ( (bwear && bwear->owner) || (bweapon && bweapon->owner) ) 
				clone->forceOn ( self );
		}
	}
}

void BContainer::buildPacket ( PackedData *packet, int override ) {
	// validate each object in this container before building a packet
	LinkedElement *element = contents.head();
	
	while ( element ) {
		LinkedElement *next = element->next();
		WorldObject *object = (WorldObject *)element->ptr();
		element = next;
	}

	packet->putWord ( (unsigned short)contents.size() );

	element = contents.head();

	while ( element ) {
		LinkedElement *next = element->next();
		WorldObject *object = (WorldObject *)element->ptr();

		// if the object in this container owns us, we're gonna blow
		if ( object->owns ( self ) ) {
			logInfo ( _LOG_ALWAYS, "%s(%d): circular reference in BContainer::buildPacket", __FILE__, __LINE__ );
			contents.del ( object );
		} else {
			object->buildPacket ( packet, override );
		}

		element = next;
	}
}

void BContainer::writeSCIData ( FILE *file ) {
	fprintf ( file, "\t\t((aWhatObj addBase: BContainer)\n" );
	fprintf ( file, "\t\t\tpWeightCap: %d,\n", weightCapacity );
	fprintf ( file, "\t\t\tpBulkCap: %d,\n", bulkCapacity );
	fprintf ( file, "\t\t)\n" );
}

int BContainer::calculateWeight ( void ) {
	weight = 0;

	BCarryable *bcarry = (BCarryable *)self->getBase ( _BCARRY );

	if ( bcarry )
		weight = bcarry->weight;

	LinkedElement *element = contents.head();

	while ( element ) {
		WorldObject *obj = (WorldObject *)element->ptr();
		BCarryable *base = (BCarryable *)obj->getBase ( _BCARRY ); 
		
		BContainer *bcontain = (BContainer *)obj->getBase ( _BCONTAIN );

		// must not add my weight to calc'd weight when processing 
		// a container in a container.

		if ( bcontain ) 
			weight += bcontain->calculateWeight();
		else 
			if ( base )
				weight += base->weight;

		element = element->next();
	}

	return weight;
}

// calculate the bulk of the container's contents

int BContainer::calculateBulk ( void ) {
	bulk = 0;

	BCarryable *bcarry = (BCarryable *)self->getBase ( _BCARRY );

	if ( bcarry )
		bulk = bcarry->bulk;

	LinkedElement *element = contents.head();

	if ( element ) {

		while ( element ) {
	
			WorldObject *obj = (WorldObject *)element->ptr();
			BCarryable *base = (BCarryable *)obj->getBase ( _BCARRY ); 
			BContainer *bcontain = (BContainer *)obj->getBase ( _BCONTAIN );

			// must add my bulk to calc'd bulk when processing 
			// a container in a container. 

			if ( bcontain )
				bulk += bcontain->calculateBulk();
			else 
	  			if ( base )
  					bulk += base->bulk;

			element = element->next();
		}
	}

	return bulk;
}

void BContainer::changeWeight ( int value, int reduce )
{
	WorldObject *owner = self->getBaseOwner();
	BContainer *bcontain = (BContainer *)owner->getBase ( _BCONTAIN );

	if ( bcontain )
		bcontain->calculateWeight();
	else
		calculateWeight();
}

void BContainer::changeBulk ( int value, int reduce ) {
	WorldObject *owner = self->getBaseOwner();
	BContainer *bcontain = (BContainer *)owner->getBase ( _BCONTAIN );

	if ( bcontain )
		bcontain->calculateBulk();
	else
		bulk += value;
//		calculateBulk();

}

int BContainer::tooHeavy ( int value, int reduce ) {
	if ( bulkCapacity == 0 && contents.size() )
		return 1;
 	else if ( bulkCapacity == 0 )
		return 0;

	BCarryable *bcarry = (BCarryable *)self->getBase ( _BCARRY );

	if ( ( weight + value ) > weightCapacity ) 
		return 1;

	WorldObject *owner = self->getOwner();

	if ( owner != self ) {
		BContainer *bcontain = (BContainer *)owner->getBase ( _BCONTAIN );
		return bcontain->tooHeavy ( value );
	}

	return 0;
}

int BContainer::tooBulky ( int value, int reduce ) {
	if ( weightCapacity == 0 && contents.size() )
		return 1;
	else if ( bulkCapacity == 0 )
		return 0;

	BCarryable *bcarry = (BCarryable *)self->getBase ( _BCARRY );

	if ( ( bulk + value ) > bulkCapacity ) 
		return 1;

	WorldObject *owner = self->getOwner();

	if ( owner != self ) {
		BContainer *bcontain = (BContainer *)owner->getBase ( _BCONTAIN );
		return bcontain->tooBulky ( value );
	}

	return 0;
}

int BContainer::perform ( int action, va_list *args ) {
	int retVal = _WO_ACTION_ALLOWED;

	switch ( action ) {
		// intercept put on requests for objects that we don't own
		case vPutOn: {
			WorldObject *object = (WorldObject *)va_arg ( *args, WorldObject * );

			if ( !object->isOwnedBy ( self ) )
				retVal = _WO_ACTION_PROHIBITED;
		}

		break;
	}

	return retVal;
}

void BContainer::submitServIDs ( PackedData *packet ) {
	LinkedElement *element = contents.head();

	while ( element ) {
		WorldObject *object = (WorldObject *)element->ptr();
		object->submitServIDs ( packet );

		element = element->next();
	}
}

WorldObject *BContainer::goldPile ( void ) {
	WorldObject *retVal = NULL;

	LinkedElement *element = contents.head();

	while ( element ) {
		WorldObject *object = (WorldObject *)element->ptr();

		if ( object->physicalState & _STATE_MONEY ) {
			retVal = object;
			break;
		}

		element = element->next();
	}

	return retVal;
}

// visibility control
void BContainer::makeVisible ( int state ) {
	LinkedElement *element = contents.head();

	while ( element ) {
		WorldObject *obj = (WorldObject *)element->ptr();
		obj->makeVisible ( state );

		BContainer *base = (BContainer *)obj->getBase ( _BCONTAIN );

		if ( base )
			base->makeVisible ( state );

		element = element->next();
	}
}

int BContainer::accepts( WorldObject* obj ) {
	int nCount = 0;

	if ( m_pAccept && m_nAccept ) {
		for (int nRecord = 0;nRecord < m_nAccept;nRecord++) {
			if ( obj->classNumber == m_pAccept[ nRecord ].acceptItem->classNumber ) {
				// increment values
				gAcceptCounts[ m_pAccept[nRecord].acceptCounter ] += m_pAccept[nRecord].acceptAmount;

				FILE* acceptFile = fopen ( "../logs/counts.bin", "wb" );

				if ( acceptFile ) {
					fwrite( &gAcceptCounts, sizeof( int ), _MAX_ACCEPT_COUNTS, acceptFile );
					fclose( acceptFile );
				}

				roomMgr->destroyObj ( obj, 1, __FILE__, __LINE__ );

				return 1;
			}
		}


		BContainer* bContain = (BContainer *) obj->getBase ( _BCONTAIN );

		if ( bContain ) {
			LinkedElement *element = bContain->contents.head();

			while ( element ) {
				WorldObject* pObj = (WorldObject *) element->ptr();
				element = element->next();

				nCount += accepts( pObj );
			}
		}
	}

	return nCount;
}

acceptRecord* BContainer::addAccept( const char* pItem ) {
	if ( !m_pAccept ) {
		m_pAccept = new acceptRecord[10];
		m_nAccept = 0;
	}

	if ( m_nAccept >= 10 ) {
		return NULL;
	}

	m_pAccept[ m_nAccept ].acceptItem = roomMgr->findClass ( pItem );

	m_nAccept++;

	return &m_pAccept[ ( m_nAccept - 1 ) ];
}

int BContainer::doesAccept() {
	return m_pAccept ? 1 : 0;
}

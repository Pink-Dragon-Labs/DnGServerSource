/*
	BContainer class
	author: Stephen Nichols
*/

#ifndef _BCONTAIN_HPP_
#define _BCONTAIN_HPP_

#include "wobjectbase.hpp"

#define _MAX_CONTAINER_SIZE	32

typedef struct {
	int size;
	int contents[_MAX_CONTAINER_SIZE];
} DBContents;

typedef struct {
	WorldObject*	acceptItem;
	unsigned char	acceptCounter;
	char			acceptAmount;
} acceptRecord;

class BContainer : public WorldObjectBase {
public:
	int weight, bulk, weightCapacity, bulkCapacity, weightReduction, bulkReduction;
	LinkedList contents;

	acceptRecord*	m_pAccept;
	int				m_nAccept;

	int accepts( WorldObject* obj );
	int doesAccept();

	acceptRecord* addAccept( const char* pItem );
	
	BContainer ( WorldObject *obj );
	virtual ~BContainer();

	void copy ( WorldObjectBase *base );

	/* calculate the weight and bulk of this object */
	int calculateWeight ( void );
	int calculateBulk ( void );

	void changeWeight ( int val, int reduce = 1 );
	void changeBulk ( int val, int reduce = 1 );

	/* is this object too bulky or too heavy? */
	int tooBulky ( int value = 0, int reduce = 1 ); 
	int tooHeavy ( int value = 0, int reduce = 1 );

	/* packet manipulation */
	virtual void buildPacket ( PackedData *packet, int override = 0 );
	virtual void writeSCIData ( FILE *file );

	/* submit my contents' servIDs into a packed data struct */
	virtual void submitServIDs ( PackedData *packet );

	/* return the first money object in my contents */
	WorldObject *goldPile ( void );

	/* verb manipulation */
	virtual int perform ( int action, va_list *args );

	// visibility control
	void makeVisible ( int state );

};

#endif

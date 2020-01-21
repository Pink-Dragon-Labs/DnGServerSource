/*
	BDescribed class
	author: Stephen Nichols
*/

#ifndef _BDESCRIBE_HPP_
#define _BDESCRIBE_HPP_

#include "wobjectbase.hpp"
#include "globals.hpp"

#define _MAX_ACCEPT_COUNTS	10
extern int gAcceptCounts[_MAX_ACCEPT_COUNTS];

class BDescribed : public WorldObjectBase {

public:
	const char* text;
	const char* shopText;
	const char* riddleText;
	const char* riddleAnswer;
	const char* idText;
	const char* acceptText;

	bool isBook;

	char acceptDisplay[ _MAX_ACCEPT_COUNTS ];

	BDescribed ( WorldObject* obj );
	virtual ~BDescribed();

	void copy ( WorldObjectBase* base );

	void setText ( const char* str )
	{ text = gStringCache.submitString( str ); }

	void setShopText ( const char* str )
	{ shopText = gStringCache.submitString( str ); }

	void setRiddleText ( const char* str )
	{ riddleText = gStringCache.submitString( str ); }
	
	void setRiddleAnswer ( const char* str )
	{ riddleAnswer = gStringCache.submitString( str ); }
	
	void setIDText ( const char* str )
	{ idText = gStringCache.submitString( str ); }
	
	void setAcceptText ( const char* str )
	{ acceptText = gStringCache.submitString( str ); }

	// do not put my string in the packet, the client must use a special request
	// message to get the description.  this saves bandwidth. 
	virtual void buildPacket ( PackedData* packet, int override = 0 ) {
		//packet->putString( text );
	}

	virtual void writeSCIData ( FILE* file ) {
		fprintf ( file, "\t\t(aWhatObj addBase: BDescribed)\n" );
	}
};

#endif

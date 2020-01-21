/*
	BDescribed class	
	author: Stephen Nichols
*/

#include "bdescribed.hpp"
#include "roommgr.hpp"

BDescribed::BDescribed ( WorldObject *obj ) : WorldObjectBase ( obj ),
	text( gStringCache.submitString( "You see nothing out of the ordinary." ) ),
	shopText ( gStringCache.submitString( "The shop keeper just whistles as though you said nothing." ) ),
	idText ( gStringCache.submitString( "" ) ),
	acceptText ( 0 ),
	riddleText( 0 ),
	riddleAnswer( 0 ),
	isBook( false )
{
	type = _BDESCRIBED;

	for (int nRecord = 0;nRecord < _MAX_ACCEPT_COUNTS;nRecord++)
		acceptDisplay[ nRecord ] = 0;
}

BDescribed::~BDescribed() {
	setText ( 0 );
	setShopText ( 0 );
	setIDText ( 0 );
	setAcceptText ( 0 );
	setRiddleText ( 0 );
	setRiddleAnswer ( 0 );
}

//consider a proper copy contructor here - a compiler-generated one would
//handle the below initialization
void BDescribed::copy ( WorldObjectBase *theBase ) {
	type = _BDESCRIBED;

	BDescribed *base = (BDescribed *)theBase;

	text = base->text;
	isBook = base->isBook;
	shopText = base->shopText;
	riddleText = base->riddleText;
	riddleAnswer = base->riddleAnswer;
	idText = base->idText;
	acceptText = base->acceptText;

	for (int nRecord = 0;nRecord < _MAX_ACCEPT_COUNTS;nRecord++)
		acceptDisplay[ nRecord ] = base->acceptDisplay[ nRecord ];
}

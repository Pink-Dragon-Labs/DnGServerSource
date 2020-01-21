//
// csinvitem
//
// This file contains the CSInvItem class.
//
// author: Doug Oldfield 
//

#include "system.hpp"
#include "cstools.hpp"
#include "html.hpp"
#include "csinvitem.hpp"

CSInvItem::CSInvItem ( char **ptr, int *size, int theLevel, int &theID )
{
	level = theLevel;
	id = theID;
	createData ( ptr, size, theID );
}

CSInvItem::~CSInvItem()
{
	if ( name ) {
		free ( name );
		name = NULL;
	}
	if ( specialName ) {
		free ( specialName );
		specialName = NULL;
	}
	if ( password ) {
		free ( password ); 
		password = NULL;
	}
	if ( spellBag ) {
		free ( spellBag );
		spellBag = NULL;
	}

	while ( affectList.size () ) {
		CSAffect *af = (CSAffect *)affectList.at ( 0 );
		affectList.del ( af );
		delete af;
	}

	while ( invItemList.size () ) {
		CSInvItem *inv = (CSInvItem *)invItemList.at ( 0 );
		invItemList.del ( inv );
		delete inv;
	}
}

// this member reads the passed-in data into member variables
int CSInvItem::createData ( char **thePtr, int *size, int &theID )
{
	char line[1024];

	char *ptr = *thePtr;

	bufgets ( line, &ptr, size );
	name = strdup ( line ? line : (char*) " " );
	bufgets ( line, &ptr, size );
	specialName = strdup ( line ? line : (char*) " " );
	bufgets ( line, &ptr, size );
	color = atoi ( line );
	bufgets ( line, &ptr, size );
	physicalState = atoi ( line );
	bufgets ( line, &ptr, size );
	value = atoi ( line );
	bufgets ( line, &ptr, size );
	mana = atoi ( line );
	bufgets ( line, &ptr, size );
	health = atoi ( line );
	bufgets ( line, &ptr, size );
	x = atoi ( line );
	bufgets ( line, &ptr, size );
	y = atoi ( line );
	bufgets ( line, &ptr, size );
	loop = atoi ( line );

	// get any affects and add them to the list
	bufgets ( line, &ptr, size );
	int affectCount = atoi ( line );
																					 
	while ( affectCount > 0 ) {
		bufgets ( line, &ptr, size );
		CSAffect *af = new CSAffect ( line );
		affectList.add ( af );
		--affectCount;
	}

	bufgets ( line, &ptr, size );
	wear = atoi ( line );
	bufgets ( line, &ptr, size );
	weapon = atoi ( line );
	bufgets ( line, &ptr, size );
	open = atoi ( line );
	bufgets ( line, &ptr, size );
	lock = atoi ( line );
	bufgets ( line, &ptr, size );
	entry = atoi ( line );
	bufgets ( line, &ptr, size );
	password = strdup ( line ? line : (char*) " " );
	bufgets ( line, &ptr, size );
	use = atoi ( line );
	bufgets ( line, &ptr, size );
	spellBag = strdup	 ( line ? line : (char*) " " );

	// get all the inv items and add them to the list
	bufgets ( line, &ptr, size );
	int invCount = atoi ( line );

	containedItems = invCount;

	if ( invCount > 1000 ) {
		return 0;
	}

	if ( *size < 1 )
		return 0;

	while ( invCount > 0 ) {
		CSInvItem *inv = new CSInvItem ( &ptr, size, level + 1, ++theID );
		invItemList.add ( inv );
		--invCount;

		if ( *size < 1 )
			return 0;
	}

	*thePtr = ptr;

	return 0;
}

// this member collects all the member vars into a single string that can
// be easily written back to the db
void CSInvItem::packData ( PackedData *p )
{
	p->printf ( "%s\n", name );
	p->printf ( "%s\n", specialName );
	p->printf ( "%d\n", color );
	p->printf ( "%d\n", physicalState );
	p->printf ( "%d\n", value );
	p->printf ( "%d\n", mana );
	p->printf ( "%d\n", health );
	p->printf ( "%d\n", x );
	p->printf ( "%d\n", y );
	p->printf ( "%d\n", loop );

	// get any affects and process them
	int size = affectList.size ();
	p->printf ( "%d\n", size );

	int i;

	for (i = 0; i < size; ++i ) {
		CSAffect *af = (CSAffect *)affectList.at ( i );
		p->printf ( "%d %d %d %d %d\n", af->duration, af->id, af->source, af->type, af->value );
	}

	p->printf ( "%d\n", wear );
	p->printf ( "%d\n", weapon );
	p->printf ( "%d\n", open );
	p->printf ( "%d\n", lock );
	p->printf ( "%d\n", entry );
	p->printf ( "%s\n", password );
	p->printf ( "%d\n", use );
	p->printf ( "%s\n", spellBag );

	size = invItemList.size ();
	p->printf ( "%d\n", size );

	// get all the inv items and add them to the list
	for ( i = 0; i < size; ++i ) {
		CSInvItem *inv = (CSInvItem*)invItemList.at ( i );
		inv->packData ( p );
	}
}

// this member displays just the name of this item.
void CSInvItem::displayName ()
{
	htmlIndentLine ( level );
	htmlPrint ( "%s", name );
	htmlNewLine ();

	// show any inv items for this item 
	if ( invItemList.size () ) {
			for ( int i = 0; i < invItemList.size (); ++i ) {
				CSInvItem *inv = (CSInvItem *)invItemList.at ( i );
				inv->displayName ();
			}
	}
}

void CSInvItem::makeTableRow ( char *title, char *text )
{
	htmlNewTableRow ();
	htmlNewTableHeadingLeft ( "%s", title );
	htmlNewTableData ( "%s", text );
	htmlEndTableRow ();
}

void CSInvItem::makeTableRow ( char *title, int value )
{
	htmlNewTableRow ();
	htmlNewTableHeadingLeft ( "%s", title );
	htmlNewTableData ( "%d", value );
	htmlEndTableRow ();
}

// this displays this inv item 
void CSInvItem::display ( int editAllowed )
{

	htmlNewTable ( 1 );
		makeTableRow ( "Name", name );
		makeTableRow ( "Special Name", specialName );
		makeTableRow ( "Color", color );
		makeTableRow ( "Physical State", physicalState );
		makeTableRow ( "Value", value );
		makeTableRow ( "Mana", mana );
		makeTableRow ( "Health", health );
		makeTableRow ( "X", x );
		makeTableRow ( "Y", y );
		makeTableRow ( "Loop", loop );
		makeTableRow ( "Wear", wear );
		makeTableRow ( "Weapon", weapon );
		makeTableRow ( "Open", open );
		makeTableRow ( "Lock", lock );
		makeTableRow ( "Entry", entry );
		if ( !editAllowed ) {
			if ( password )
				free ( password );
			password = strdup ( "********" );
		}

		makeTableRow ( "Password", password );
		makeTableRow ( "Use", use );
	htmlEndTable();


	// show any affects for this item
	if ( affectList.size () ) {
		htmlPrint ( "Affects" );
		htmlNewLine ();

		htmlNewTable ( 1 );
			htmlNewTableRow ();
				CSAffect x;
				x.tableHeading ();
			htmlEndTableRow ();

			for (int i = 0; i < affectList.size (); ++i ) {
				CSAffect *af = (CSAffect *)affectList.at ( i );
//				af->display ();
				htmlNewTableRow ();
					af->tableData ();
				htmlEndTableRow ();
			}
		htmlEndTable ();
	}

	// show any inv items for this character
	if ( invItemList.size () ) {
		htmlNewTable ( 1 );
			makeTableRow ( "Inventory Items", " " );

			for ( int i = 0; i < invItemList.size (); ++i ) {
				CSInvItem *inv = (CSInvItem *)invItemList.at ( i );
				inv->display ( editAllowed );
			}
			htmlEndTable ();
	} else if ( containedItems ) {
		htmlNewTable ( 1 );
			makeTableRow ( "Inventory Items", containedItems );

			htmlEndTable ();
	}
}

// displays some of the data on a single line
void CSInvItem::displaySmall ( int editAllowed )
{
	if ( !editAllowed ) {
		if ( password )
			free ( password );
		password = strdup ( "********" );
	}

	htmlIndentLine ( level );
	htmlPrint ( "%d. %s %s %s %d", id, name, specialName, password, containedItems );
	htmlNewLine ();

	// show any inv items for this item
	if ( invItemList.size () ) {
		for ( int i = 0; i < invItemList.size (); ++i ) {
			CSInvItem *inv = (CSInvItem *)invItemList.at ( i );
			inv->displaySmall ( editAllowed );
		}
	}
}

// displays some of the data on a single line and links to a details page
void CSInvItem::displaySmallLink ( int editAllowed )
{
	if ( !editAllowed ) {
		if ( password )
			free ( password );
		password = strdup ( "********" );
	}

	htmlIndentLine ( level );
	htmlPrint ( "%d. %s %s %s %d - [ htmlLink ( %, %s ) ]", id, name, specialName, password, containedItems, id );
	htmlNewLine ();

	// show any inv items for this item
	if ( invItemList.size () ) {
		for ( int i = 0; i < invItemList.size (); ++i ) {
			CSInvItem *inv = (CSInvItem *)invItemList.at ( i );
			inv->displaySmall ( editAllowed );
		}
	}
}

// returns this item if id=index
CSInvItem *CSInvItem::getInvItem ( int index )
{
	for ( int i = 0; i < invItemList.size (); ++i ) {
		CSInvItem *inv = (CSInvItem*)invItemList.at ( i );
		if ( inv->id == index )
			return inv;

		CSInvItem *otherInv = inv->getInvItem ( index );
		if ( otherInv ) {
			return otherInv;
		}
	}

	return NULL;
}

// removes the item from the list
int CSInvItem::delInvItem ( CSInvItem *item )
{
	for ( int i = 0; i < invItemList.size (); ++i ) {
		CSInvItem *inv = (CSInvItem*)invItemList.at ( i );
		if ( inv == item ) {
			invItemList.del ( inv );
			return 1;
		}

		if ( inv->delInvItem ( item ) )
			return 1;
	}

	return 0;
}

// add an item to the list
void CSInvItem::addInvItem ( CSInvItem *item )
{
	invItemList.add ( item );
}

// returns my specialName if it matches the passed-in str.
// or if any of my inv items match
CSInvItem *CSInvItem::findSpecialName ( char *srch )
{
	if ( strstr ( specialName, srch ) ) {
		return this;
	}

	for ( int i = 0; i < invItemList.size (); ++i ) {
		CSInvItem *inv = (CSInvItem*)invItemList.at ( i );
		if ( inv->findSpecialName ( srch ) ) {
			return inv;
		}
	}

	return NULL;
}


//
// csinvitem
//
// This file contains the CSInvItem class.
//
// author: Doug Oldfield 
//

#ifndef _CSINVITEM_HPP_
#define _CSINVITEM_HPP_

#include "list.hpp"

//
// This class represents a single inventory item.  
//

class CSInvItem : public ListObject
{
public:
	CSInvItem ( char **ptr, int *size, int level, int &theID );
	virtual ~CSInvItem ();
	int createData ( char **ptr, int *size, int &theID );
	void display ( int editAllowed = 0 );
	void displayName ( void );

	// displays some of the data on a single line
	void displaySmall ( int editAllowed = 0 );

	// displays some of the data on a single line and links to a details page
	void displaySmallLink ( int editAllowed = 0 );

   // this member creates a table row based on the string passed to it
	void makeTableRow ( char *title, char *text );

	// this member creates a table row based on the int passed to it
	void makeTableRow ( char *title, int value );

	// this member packs the member vars into a single string
	// that can be easily written to the db
	void packData ( PackedData *p );

	// this member returns this item if it's id = index
	CSInvItem *getInvItem ( int index );

	// this member removes the item and returns 1 to indicate it did so 
	int delInvItem ( CSInvItem *item );

	// this member adds the item to the list
	void addInvItem ( CSInvItem *item );

	// this member returns this item if its specialName matches the passed-in string
	CSInvItem *findSpecialName ( char *str );

	// a unique identifier 
	int id;

	// my indent level 
	int level;

	char *name;
	char *specialName;
	int color;
	int physicalState;
	int value;
	int mana;
	int health;
	int x;
	int y;
	int loop;
	LinkedList affectList;
	int wear;
	int weapon;
	int open;
	int lock;
	int entry;
	char *password;
	int use;
	char *spellBag;
	int containedItems;
	LinkedList invItemList;
};

#endif

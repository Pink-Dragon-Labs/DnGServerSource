//
// cscharacter
//
// This file contains the CSCharacter class.
//
// author: Doug Oldfield
//

#ifndef _CSCHARACTER_HPP_
#define _CSCHARACTER_HPP_

#include "cstools.hpp"
#include "list.hpp"
#include "packdata.hpp"

class CSInvItem;

//
// This class represents a single character.  
//

class CSCharacter : public ListObject
{
public:
	CSCharacter();
	virtual ~CSCharacter();
	SQLDatabase *db;

	// constructor that takes an id
	CSCharacter ( SQLDatabase *db, int id, int serverNum );

	// constructor that takes a char name
	CSCharacter ( SQLDatabase *db, char *name, int serverNum );

	// this member sets initial values
	void init ( void );

	// this member takes the server number and returns
	// the name of the table to use for that server
	char *getTableName ( void );

	// this member loads this character from the given database
	int load ( char *name );

	// this member loads this character from the given id
	int load ( int id );

	// this member loads this character from the given account id
	int loadFromAccountID ( int accountID );

	// this member is called by all the load functions to read the data into this character
	int createData ( SQLResponse *result );

	// this member creates a new character
	int create ( void );

	// this member updates a character
	int update ( void );

	// this member deletes a character
	int destroy ( void );

	// this member packs all the member vars onto a single string so data can be written back
	// to the db
	PackedData *packData ( void );

	// this member creates a table row based on the string passed to it
	void makeTableRow ( char *title, char *text );

	// this member creates a table row based on the int passed to it
	void makeTableRow ( char *title, int value );

	// this member displays a character
	void display ( int editAllowed = 0 );

	// this member diplays this character as a link
	void displayLink ( char *linkStr, char *sessionID );

	// this member changes the name of this character
	void changeName ( int nameID );
	void changeName ( int nameID, char* sName );

	// this member clears PVP
	int resetPVP ( void );

	// this member resets the title
	int resetTitle ( void );

	// this member resets the bio
	int resetBio ( void );

	// returns the item if it's specialName matches srch
	CSInvItem *findSpecialName ( char *srch );

	// these correspond to the sql table fields

	int characterNameID, accountID, id;

	// the server number where this character resides
	int server;

	// these correspond to the realm's character class
	char *loginName;
	char *creationTime;
	char *classID;
	char *characterName;
	char *title;
	int level;
	int maxLevel;
	int experiencePoints;
	int buildPoints;
	int hometown;
	int strength;
	int dexterity;
	int intelligence;
	int quickness;
	int endurance;
	int health;
	int maxHealth;
	int hunger;
	int thirst;
	int alignment;
	int value;
	int mana;
	int physicalState;
	char *skills;
	char *spells;
	int stealCount;
	int stealUnserved;
	int killCount;
	int killUnserved;
	int peaceful;
	int warnCount;
	int race;
	int sex;
	int skinColor;
	int headNumber;
	int hairNumber;
	int hairColor;
	int browNumber;
	int faceHairNumber;
	int eyeNumber;
	int eyeColor;
	int noseNumber;
	int mouthNumber;
	int earNumber;
	int lastRoom;
	int lastDungeon;
	int x;
	int y;
	int loop;
	char *bio;
	int girth;
	int height;
	int one;
	char *lastCrimeTime;

	// list that holds affects
	LinkedList affectList;

	// list that holds inventory items;
	LinkedList invItemList;

	int playerKills;
	int NPCKills;
	int oldLevel;
	int stamina;
	char *writeTime;
};

#endif

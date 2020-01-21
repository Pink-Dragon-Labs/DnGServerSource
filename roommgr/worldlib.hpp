/*
	WORLDLIB.HPP
	classes for processing the world library

	author: Stephen Nichols
*/

#ifndef _WORLDLIB_HPP_
#define _WORLDLIB_HPP_

enum { 
	_OIP_START,
	_OIP_MAIN_BLOCK,
	_OIP_PROPERTIES,
	_OIP_ACTIONS,
	_OIP_COMPONENTS,
	_OIP_BASE,
	_OIP_EFFECT,
	_OIP_STORE_INVENTORY,
	_OIP_STORE_CATEGORY
};

enum {
	_ZIP_START
};

enum {
	_WIP_START,
	_WIP_ROOM,
	_WIP_PROPERTIES,
	_WIP_ATPINFOBLOCK,
	_WIP_ATPINFO,
	_WIP_OBJECTS,
	_WIP_OBJECT
};

enum {
	_TTP_START,
};

enum {
	_QIP_START,
};

class TalkTree;
class TalkTreeTopic;
class Quest;

// this is the class that all InfoParsers are based on
class InfoParser
{
	char _error[1024];

protected:
	int state;

public:
	InfoParser();
	virtual ~InfoParser();

	// return the currently registered error
	inline char *error ( void ) { return _error; };

	// register an error
	void registerError ( const char *format, ... );

	// rewind the token pointer by one
	void prevToken ( void );

	// return the current token
	char *getToken ( void );

	// look at the current token and return it's string
	char *getTokenText ( void );

	// scan the current list of tokens for a quoted string
	char *getString ( void ); 

	// return the current token's string with error checking
	char *getIdentifier ( char *str );

	// return the current token's string with error checking
	int getChar( char* nValue );
	int getChar( unsigned char* nValue );

	// return whether the current token matches the passed text
	int expect ( char *str );

	// look at the current token at return it as a converted integer
	int getInteger ( int * );

	// look at the current token at return it as a converted unsigned long long
	int getLongNumber ( long * );

	// parse a token list
	int parseLine ( void );

	// process a list of tokens
	virtual int processTokenList ( void );

	// instance number of this object
	int instanceNum;

	// this is the PackedData to use for reading 
	char *input;
	int index;
	int numTokens;
	int lastGetIdx;
};

// this class handles parsing object specification information
class ObjectInfoParser : public InfoParser
{
public:
	ObjectInfoParser();
	virtual ~ObjectInfoParser();

	// process a list of tokens
	virtual int processTokenList ( void );

	// define the object that this parser is creating
	WorldObject *object;
	WorldObjectBase *base;
	ShopCategory *category;
	int saveObjects;
};

// this class handles parsing zone specification information
class ZoneInfoParser : public InfoParser
{
public:
	ZoneInfoParser();
	virtual ~ZoneInfoParser();

	// process a list of tokens
	virtual int processTokenList ( void );

	Zone *zone;
};

// this class handles parsing world specification information
class WorldInfoParser : public InfoParser
{
public:
	WorldInfoParser();
	virtual ~WorldInfoParser();

	// process a list of tokens
	virtual int processTokenList ( void );

	RMRoom *room;
	ObjectInfoParser parser;
	Zone *zone;
};

// this class handles parsing talk-trees
class TalkTreeParser : public InfoParser
{
public:
	TalkTreeParser();
	virtual ~TalkTreeParser();

	// process a list of tokens
	virtual int processTokenList ( void );

	TalkTree *tree;
	TalkTreeTopic *topic;
};

// this class handles parsing quest definitions
class QuestParser : public InfoParser
{
public:
	QuestParser();
	virtual ~QuestParser();

	// process a token list (duh!)
	virtual int processTokenList ( void );
	
	Quest *quest;
};

#endif

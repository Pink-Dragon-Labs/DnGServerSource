//
// cscharacter
//
// This file contains the CSCharacter class.
//
// author: Doug Oldfield 
//

#include "system.hpp"
#include "cstools.hpp"

CSCharacter::CSCharacter()
{
	db = NULL;
	id = -1;
	accountID = -1;
	characterNameID = -1;
	characterName = NULL;
	loginName = NULL;
	server = -1;
}

void CSCharacter::init()
{
	db = NULL;
	characterName = NULL;
	loginName = NULL;
	id = -1;
	accountID = -1;
	characterNameID = -1;
	server = -1;
	creationTime = NULL;
	classID = NULL;
	title = NULL;
	skills = NULL;
	spells = NULL;
	bio = NULL;
	lastCrimeTime = NULL;
	writeTime = NULL;
	level = -1;
	maxLevel = -1;
	experiencePoints = -1;
	buildPoints = -1;
	hometown = -1;
	strength = -1;
	dexterity = -1;
	intelligence = -1;
	quickness = -1;
	endurance = -1;
	health = -1;
	maxHealth = -1;
	hunger = -1;
	thirst = -1;
	alignment = -1;
	value = -1;
	mana = -1;
	physicalState = -1;
	stealCount = -1;
	stealUnserved = -1;
	killCount = -1;
	killUnserved = -1;
	peaceful = -1;
	warnCount = -1;
	race = -1;
	sex = -1;
	skinColor = -1;
	headNumber = -1;
	hairNumber = -1;
	hairColor = -1;
	browNumber = -1;
	faceHairNumber = -1;
	eyeNumber = -1;
	eyeColor = -1;
	noseNumber = -1;
	mouthNumber = -1;
	earNumber = -1;
	lastRoom = -1;
	lastDungeon = -1;
	x = -1;
	y = -1;
	loop = -1;
	girth = -1;
	height = -1;
	one = -1;
}

CSCharacter::CSCharacter ( SQLDatabase *db, int theID, int servNum )
{
	init ();
	this->db = db;
	server = servNum;
	load ( theID );
}

CSCharacter::CSCharacter ( SQLDatabase *db, char *name, int servNum )
{
	init ();
	this->db = db;
	server = servNum;
	load ( name );
}

CSCharacter::~CSCharacter()
{
	accountID = -1;
	id = -1;
	characterNameID = -1;
	db = NULL;
	server = -1;

	if ( characterName ) {
		free ( characterName );
		characterName = NULL;
	}

	if ( loginName ) {
		free ( loginName );
		loginName = NULL;
	}

	if ( creationTime ) {
		free ( creationTime ); 
		creationTime = NULL;
	}
	if ( classID ) {
		free ( classID ); 
		classID = NULL;
	}
	if ( title ) {
		free ( title ); 
		title = NULL;
	}
	if ( skills ) {
		free ( skills ); 
		skills = NULL;
	}
	if ( spells ) {
		free ( spells ); 
		spells = NULL;
	}
	if ( bio ) {
		free ( bio ); 
		bio = NULL;
	}
	if ( lastCrimeTime ) {
		delete lastCrimeTime; 
		lastCrimeTime = NULL;
	}
	if ( writeTime ) {
		delete writeTime; 
		writeTime = NULL;
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

// this member reads the result into the character variables
int CSCharacter::createData ( SQLResponse *result )
{
	// this guy gets passed by reference to the inv items so they can
	// all have a unique id
	int invID = 0;

	id = atoi ( result->table ( 0, 0 ) );
	accountID = atoi ( result->table ( 0, 1 ) );
	characterNameID = atoi ( result->table ( 0, 2 ) );

	// result->table ( 0, 3 ) is a blob that must be split into parts

	char *tmpStr;
	char line[1024];
	int size = result->length ( 0, 3 );
	char *ptr = result->table ( 0, 3 );

	bufgets ( line, &ptr, &size );
	loginName = strdup ( line ? line : (char*) " " );
	bufgets ( line, &ptr, &size );
	creationTime = strdup ( line ? line : (char*) " " );
	bufgets ( line, &ptr, &size );
	classID = strdup ( line ? line : (char*) " " );
	bufgets ( line, &ptr, &size );
	characterName = strdup ( line ? line : (char*) " " );
	bufgets ( line, &ptr, &size );
	title = strdup ( line ? line : (char*) " " );

	// the level and maxLevel are together on a line like so: 1000:1000
	// but, not all characters are like that. some just have a level like so: 1000

	bufgets ( line, &ptr, &size );
	tmpStr = strchr ( line, ':' );
	if ( !tmpStr ) {
		level = atoi ( line );
		maxLevel = level;
	} else {
		level = atoi ( strtok ( line, ":" ) );
		maxLevel = atoi ( strtok ( NULL, ":" ) );
	}

	bufgets ( line, &ptr, &size );
	experiencePoints = atoi ( line );
	bufgets ( line, &ptr, &size );
	buildPoints = atoi ( line );
	bufgets ( line, &ptr, &size );
	hometown = atoi ( line );
	bufgets ( line, &ptr, &size );
	strength = atoi ( line );
	bufgets ( line, &ptr, &size );
	dexterity = atoi ( line );
	bufgets ( line, &ptr, &size );
	intelligence = atoi ( line );
	bufgets ( line, &ptr, &size );
	quickness = atoi ( line );
	bufgets ( line, &ptr, &size );
	endurance = atoi ( line );
	bufgets ( line, &ptr, &size );
	health = atoi ( line );
	bufgets ( line, &ptr, &size );
	maxHealth = atoi ( line );
	bufgets ( line, &ptr, &size );
	hunger = atoi ( line );
	bufgets ( line, &ptr, &size );
	thirst = atoi ( line );
	bufgets ( line, &ptr, &size );
	alignment = atoi ( line );
	bufgets ( line, &ptr, &size );
	value = atoi ( line );
	bufgets ( line, &ptr, &size );
	mana = atoi ( line );
	bufgets ( line, &ptr, &size );
	physicalState = atoi ( line );
	bufgets ( line, &ptr, &size );
	skills = strdup ( line ? line : (char*) " " );
	bufgets ( line, &ptr, &size );
	spells = strdup ( line ? line : (char*) " " );
	bufgets ( line, &ptr, &size );
	stealCount = atoi ( line );
	bufgets ( line, &ptr, &size );
	stealUnserved = atoi ( line );
	bufgets ( line, &ptr, &size );
	killCount = atoi ( line );
	bufgets ( line, &ptr, &size );
	killUnserved = atoi ( line );
	bufgets ( line, &ptr, &size );
	peaceful = atoi ( line );
	bufgets ( line, &ptr, &size );
	warnCount = atoi ( line );
	bufgets ( line, &ptr, &size );
	race = atoi ( line );
	bufgets ( line, &ptr, &size );
	sex = atoi ( line );
	bufgets ( line, &ptr, &size );
	skinColor = atoi ( line );
	bufgets ( line, &ptr, &size );
	headNumber = atoi ( line );

	if ( headNumber == -1 ) {
		hairNumber = -1;
		hairColor = -1;
		browNumber = -1;
		faceHairNumber = -1;
		eyeNumber = -1;
		eyeColor = -1;
		noseNumber = -1;
		mouthNumber = -1;
		earNumber = -1;
	} else {
		bufgets ( line, &ptr, &size );
		hairNumber = atoi ( line );
		bufgets ( line, &ptr, &size );
		hairColor = atoi ( line );
		bufgets ( line, &ptr, &size );
		browNumber = atoi ( line );
		bufgets ( line, &ptr, &size );
		faceHairNumber = atoi ( line );
		bufgets ( line, &ptr, &size );
		eyeNumber = atoi ( line );
		bufgets ( line, &ptr, &size );
		eyeColor = atoi ( line );
		bufgets ( line, &ptr, &size );
		noseNumber = atoi ( line );
		bufgets ( line, &ptr, &size );
		mouthNumber = atoi ( line );
		bufgets ( line, &ptr, &size );
		earNumber = atoi ( line );
	}

	// last room and last dungeon are on a line together like so: 150000219:0

	bufgets ( line, &ptr, &size );
	tmpStr = strchr ( line, ':' );
	if ( !tmpStr ) {
		lastRoom = atoi ( line );
		lastDungeon = 0;
	} else {
		lastRoom = atoi ( strtok ( line, ":" ) );
		lastDungeon = atoi ( strtok ( NULL, ":" ) );
	}

	bufgets ( line, &ptr, &size );
	x = atoi ( line );
	bufgets ( line, &ptr, &size );
	y = atoi ( line );
	bufgets ( line, &ptr, &size );
	loop = atoi ( line );
	bufgets ( line, &ptr, &size );
	bio = strdup ( line ? line : (char*) " " );
	bufgets ( line, &ptr, &size );
	girth = atoi ( line );
	bufgets ( line, &ptr, &size );
	height = atoi ( line );
	bufgets ( line, &ptr, &size );
	one = atoi ( line );
	bufgets ( line, &ptr, &size );
	lastCrimeTime = strdup ( line ? line : (char*) " " );

	// get any affects and add them to the list
	bufgets ( line, &ptr, &size );
	int affectCount = atoi ( line );

	while ( affectCount > 0 ) {
		if ( size < 1 )
			return 0;

		bufgets ( line, &ptr, &size );
		CSAffect *af = new CSAffect ( line );
		affectList.add ( af );
		--affectCount;
	}

	// get any inv items and add them to the list
	bufgets ( line, &ptr, &size );
	int invCount = atoi ( line );
	while ( invCount > 0 ) {
		if ( size < 1 )
			return 0;

		CSInvItem *inv = new CSInvItem ( &ptr, &size, 0, ++invID );
		invItemList.add ( inv );
		--invCount;
	}

	// these guys are after the inventory items
	bufgets ( line, &ptr, &size );
	playerKills = atoi ( line );
	bufgets ( line, &ptr, &size );
	NPCKills = atoi ( line );
	bufgets ( line, &ptr, &size );
	oldLevel = atoi ( line );
	bufgets ( line, &ptr, &size );
	stamina = atoi ( line );
	bufgets ( line, &ptr, &size );
	writeTime = strdup ( line ? line : (char*) " " );

	return 0;
}

// this member returns the table name based on the passed-in server number
char *CSCharacter::getTableName ( void )
{
	char *sName = new char[64];

	db->query ( "lock tables serverList read" );
	SQLResponse *s = db->query ( "select characterTable from serverList where id=%d", server ); 
	csSQLFatal ( db );

	if ( s ) {
		strcpy ( sName, s->table ( 0, 0 ) );
		delete s;
	}

	else {
		strcpy ( sName, "INVALID" );
	}

	db->query ( "unlock tables" );
	return sName;
}

// this member loads this character from the given database
int CSCharacter::load ( char *name )
{
	CSCharacterName cName ( db, name );

	if ( cName.id != -1 ) {

		char *t = getTableName ();

		db->query ( "lock tables %s read", t );

		SQLResponse *result = db->query ( "select id, accountID, characterNameID, data from %s where characterNameID=%d", t, cName.id );

		csSQLFatal ( db );

		delete t;

		if ( result ) {
			createData ( result );
			delete result;
		}
	}
	db->query ( "unlock tables" );
	return 0;
}

// this member loads this character based on the given ID 
int CSCharacter::load ( int theID )
{

	char *t = getTableName ();

	db->query ( "lock tables %s read", t );
	SQLResponse *result = db->query ( "select id, accountID, characterNameID, data from %s where id=%d", t, theID );
	csSQLFatal ( db );

	delete t;

	if ( result ) {
		createData ( result );
		delete result;
	}

	db->query ( "unlock tables" );
	return 0;
}

// this member loads this character based on the given account ID
int CSCharacter::loadFromAccountID ( int accountID )
{
	char *t = getTableName ();

	db->query ( "lock tables %s read", t );
	SQLResponse *result = db->query ( "select id, accountID, characterNameID, data from %s where accountID = %d", t, accountID );
	csSQLFatal ( db );

	delete t;

	if ( result ) {
		createData ( result );
		delete result;
	}

	db->query ( "unlock tables" );
	return 0;
}

// this member creates a new character record based on this data
int CSCharacter::create ( )
{
	PackedData *p = packData ();

	char *t = getTableName ();
	db->query ( "lock tables %s write", t );
	SQLResponse *result = db->query ( "insert into %s set id=0, accountID=%d, characterNameID=%d, data='%b'", t, accountID, characterNameID, p->size (), p->data () );
	csSQLFatal ( db );

	delete t;
	delete p;

	// get id of record just created
	id = db->lastInsertID ();

	if ( result )
		delete result;

	db->query ( "unlock tables" );
	return 0;
}

// this member updates an existing character record
int CSCharacter::update ( )
{
	PackedData *p = packData ();

	char *t = getTableName ();

	db->query ( "lock tables %s write", t );
	SQLResponse *result = db->query ( "update %s set accountID=%d, characterNameID=%d, data='%b' where id = %d", t, accountID, characterNameID, p->size (), p->data (), id );
	csSQLFatal ( db );

	delete t;
	delete p;

	if ( result )
		delete result;

	db->query ( "unlock tables" );
	return 0;
}

// this member deletes a character 
int CSCharacter::destroy ( )
{
	char *t = getTableName ();

	db->query ( "lock tables %s write", t );
	SQLResponse *result = db->query ( "delete from %s where id=%d", t, id );
	csSQLFatal ( db );

	delete t;

	if ( result )
		delete result;

	db->query ( "unlock tables" );
	return 0;
}

// this member writes all the member vars to a single string so the 
// record can be written back to the db
PackedData *CSCharacter::packData ( void )
{
	PackedData *p = new PackedData;

	p->printf ( "%s\n", loginName );
	p->printf ( "%s\n", creationTime );
	p->printf ( "%s\n", classID );
	p->printf ( "%s\n", characterName );
	p->printf ( "%s\n", title );
	p->printf ( "%d:%d\n", level, maxLevel );
	p->printf ( "%d\n", experiencePoints );
	p->printf ( "%d\n", buildPoints );
	p->printf ( "%d\n", hometown );
	p->printf ( "%d\n", strength );
	p->printf ( "%d\n", dexterity );
	p->printf ( "%d\n", intelligence );
	p->printf ( "%d\n", quickness );
	p->printf ( "%d\n", endurance );
	p->printf ( "%d\n", health );
	p->printf ( "%d\n", maxHealth );
	p->printf ( "%d\n", hunger );
	p->printf ( "%d\n", thirst );
	p->printf ( "%d\n", alignment );
	p->printf ( "%d\n", value );
	p->printf ( "%d\n", mana );
	p->printf ( "%d\n", physicalState );
	p->printf ( "%s\n", skills );
	p->printf ( "%s\n", spells );
	p->printf ( "%d\n", stealCount );
	p->printf ( "%d\n", stealUnserved );
	p->printf ( "%d\n", killCount );
	p->printf ( "%d\n", killUnserved );
	p->printf ( "%d\n", peaceful );
	p->printf ( "%d\n", warnCount );
	p->printf ( "%d\n", race );
	p->printf ( "%d\n", sex );
	p->printf ( "%d\n", skinColor );
	p->printf ( "%d\n", headNumber );
	p->printf ( "%d\n", hairNumber );
	p->printf ( "%d\n", hairColor );
	p->printf ( "%d\n", browNumber );
	p->printf ( "%d\n", faceHairNumber );
	p->printf ( "%d\n", eyeNumber );
	p->printf ( "%d\n", eyeColor );
	p->printf ( "%d\n", noseNumber );
	p->printf ( "%d\n", mouthNumber );
	p->printf ( "%d\n", earNumber );
	p->printf ( "%d:%d\n", lastRoom, lastDungeon );
	p->printf ( "%d\n", x );
	p->printf ( "%d\n", y );
	p->printf ( "%d\n", loop );
	p->printf ( "%s\n", bio );
	p->printf ( "%d\n", girth );
	p->printf ( "%d\n", height );
	p->printf ( "%d\n", one );
	p->printf ( "%s\n", lastCrimeTime );

	int size = affectList.size ();
	p->printf ( "%d\n", size );

	int i;

	for (i = 0; i < size; ++i ) {
		CSAffect *af = (CSAffect *)affectList.at ( i );
		p->printf ( "%d %d %d %d %d\n", af->duration, af->id, af->source, af->type, af->value );
	}

	size = invItemList.size ();
	p->printf ( "%d\n", size );

	for ( i = 0; i < size; ++i ) {
		CSInvItem *inv = (CSInvItem *)invItemList.at ( i );
		inv->packData ( p );
	}

	p->printf ( "%d\n", playerKills );
	p->printf ( "%d\n", NPCKills );
	p->printf ( "%d\n", oldLevel );
	p->printf ( "%d\n", stamina );
	p->printf ( "%s\n", writeTime );

	return p;
}

void CSCharacter::makeTableRow ( char *title, char *text )
{
	htmlNewTableRow ();
		htmlNewTableHeadingLeft ( "%s", title );
		htmlNewTableData ( "%s", text );
	htmlEndTableRow ();
}

void CSCharacter::makeTableRow ( char *title, int value )
{
	htmlNewTableRow ();
		htmlNewTableHeadingLeft ( "%s", title );
		htmlNewTableData ( "%d", value );
	htmlEndTableRow ();
}

// this displays this character
void CSCharacter::display ( int editAllowed )
{
	char *tmp;

	htmlNewTable ( 0, "50%" );
		makeTableRow ( "ID", id );
		makeTableRow ( "Account ID", accountID );
		makeTableRow ( "Name", characterName );
		tmp = csFromUnixTime ( db, creationTime );
		makeTableRow ( "Creation Time", tmp );
		free ( tmp );
		makeTableRow ( "Class ID", classID );
		tmp = csStripTags ( title );	
		makeTableRow ( "Title", tmp );
		free ( tmp );
		makeTableRow ( "Level", level );
		makeTableRow ( "Max Level", maxLevel );
		makeTableRow ( "Experience Points", experiencePoints );
		makeTableRow ( "Build Points", buildPoints );
		makeTableRow ( "Hometown", hometown );
		makeTableRow ( "Strength", strength );
		makeTableRow ( "Dexterity", dexterity );
		makeTableRow ( "Intelligence", intelligence );
		makeTableRow ( "Quickness", quickness );
		makeTableRow ( "Endurance", endurance );
		makeTableRow ( "Health", health );
		makeTableRow ( "Max Health", maxHealth );
		makeTableRow ( "Hunger", hunger );
		makeTableRow ( "Thirst", thirst );
		makeTableRow ( "Alignment", alignment );
		makeTableRow ( "Value", value );
		makeTableRow ( "Mana", mana );
		makeTableRow ( "PhysicalState", physicalState );
		makeTableRow ( "Skills", skills );
		makeTableRow ( "Spells", spells );
		makeTableRow ( "Steal Count", stealCount );
		makeTableRow ( "Steal Unserved", stealUnserved );
		makeTableRow ( "Kill Count", killCount );
		makeTableRow ( "Kill Unserved", killUnserved );
		makeTableRow ( "Peaceful", peaceful );
		makeTableRow ( "Warn Count", warnCount );
		makeTableRow ( "Race", race );
		makeTableRow ( "Sex", sex );
		makeTableRow ( "Skin Color", skinColor );
		makeTableRow ( "Head Number", headNumber );
		makeTableRow ( "Hair Number", hairNumber );
		makeTableRow ( "Hair Color", hairColor );
		makeTableRow ( "Brow Number", browNumber );
		makeTableRow ( "Face Hair Number", faceHairNumber );
		makeTableRow ( "Eye Number", eyeNumber );
		makeTableRow ( "Eye Color", eyeColor );
		makeTableRow ( "Nose Number", noseNumber );
		makeTableRow ( "Mouth Number", mouthNumber );
		makeTableRow ( "Ear Number", earNumber );
		makeTableRow ( "Last Room", lastRoom );
		makeTableRow ( "Last Dungeon", lastDungeon );
		makeTableRow ( "X", x );
		makeTableRow ( "Y", y );
		makeTableRow ( "Loop", loop );
		tmp = csStripTags ( title );
		makeTableRow ( "Bio", tmp );
		free ( tmp );
		makeTableRow ( "Girth", girth );
		makeTableRow ( "Height", height );

		// convert lastCrimeTime from seconds since 1970 into a readable time string
		struct tm *tmLastCrimeTime;
		time_t tlastCrimeTime;
		tlastCrimeTime = atol( lastCrimeTime );

		if ( tlastCrimeTime ) {
			
			tmLastCrimeTime = localtime( &tlastCrimeTime );

			if ( tmLastCrimeTime ) {
				makeTableRow ( "Last Crime Time", asctime( tmLastCrimeTime ) );
			} else {
				makeTableRow ( "Last Crime Time", "none or bad data" );
			}
		} else {
			makeTableRow ( "Last Crime Time", "none" );
		}

		makeTableRow ( "Player Kills", playerKills );
		makeTableRow ( "NPC Kills", NPCKills );
		makeTableRow ( "Old Level", oldLevel );
		makeTableRow ( "Stamina", stamina );
		tmp = csFromUnixTime ( db, writeTime );
		makeTableRow ( "Write Time", tmp );
		free ( tmp );
	htmlEndTable ();

/*
	// show any affects for this character
	if ( affectList.size () ) {
		htmlNewLine (); htmlPrint ( "Affects:" ); htmlNewLine ();
		htmlNewTable ( 1 );
			htmlNewTableRow ();
				CSAffect x;
				x.tableHeading ();
			htmlEndTableRow ();

			for (int i = 0; i < affectList.size (); ++i ) {
				CSAffect *af = (CSAffect *)affectList.at ( i );
				htmlNewTableRow ();
					af->tableData ();
				htmlEndTableRow ();
//				af->display ();
			}
		htmlEndTable ();
	}
*/

	// show any inv items for this character
	if ( invItemList.size () ) {
		htmlNewLine (); htmlPrint ( "Inventory Items:" ); htmlNewLine ();

		for ( int i = 0; i < invItemList.size (); ++i ) {
			CSInvItem *inv = (CSInvItem *)invItemList.at ( i );
			inv->displaySmallLink ( editAllowed );
		}
	}

}

// displays this character's name as a link
void CSCharacter::displayLink ( char *linkStr, char *sessionID )
{
	htmlLinkNoBreak ( characterName, linkStr, sessionID, id );
	htmlIndentLine ( 2 );
}

// changes the name of this character
void CSCharacter::changeName ( int nameID )
{
	CSCharacterName name ( db , nameID );

	if ( name.id == -1 ) {
		htmlPrint ( "WARNING: bad character name id passed to CSCharacter::changeName()!!" );
		htmlNewLine ();
		return;
	}

	changeName ( nameID , name.name );
}

// changes the name of this character
void CSCharacter::changeName ( int nameID, char* sName )
{
	CSCharacterName name ( db , nameID );

	if ( name.id == -1 ) {
		htmlPrint ( "WARNING: bad character name id passed to CSCharacter::changeName()!!" );
		htmlNewLine ();
		return;
	}

	CSCharacterName oldName ( db, characterNameID );

	characterNameID = nameID;
	characterName = strdup ( sName );

	// have to take care of files with the old name in 
	// data/crimes, data/levels, data/messages, data/quests
	// at least until these things get into the db

	if ( oldName.id != -1 ) {
		char renameStr[1024];

		char *lowerOld = strdup ( oldName.name );
		strlower ( lowerOld );
		char *lowerNew = strdup ( sName );
		strlower ( lowerNew );

		SQLResponse* qRootDir = db->query( "select rootDir from serverList where id = %d", server );

		if ( qRootDir ) {
			sprintf ( sizeof ( renameStr ), renameStr, "mv %sdata/crimes/%s /realm/3.0/data/crimes/%s", qRootDir->table( 0, 0 ), lowerOld, lowerNew );
			system ( renameStr );

			sprintf ( sizeof ( renameStr ), renameStr, "mv %sdata/levels/%s /realm/3.0/data/levels/%s", qRootDir->table( 0, 0 ), lowerOld, lowerNew );
			system ( renameStr );

			sprintf ( sizeof ( renameStr ), renameStr, "mv %sdata/messages/%s /realm/3.0/data/messages/%s", qRootDir->table( 0, 0 ), lowerOld, lowerNew );
			system ( renameStr );

			// files in the messages directory have the character name in them on the 2nd line
			File file;
			file.open ( "/realm/3.0/data/messages/%s", lowerNew );
			int bufferSize = file.size ();
			char *buffer = (char *)malloc ( bufferSize );
			file.read ( buffer, bufferSize );

			char str[1024];
			char *ptr = buffer;

			file.truncate ();

			// get 1st line
			bufgets ( str, &ptr, &bufferSize, sizeof ( str ) );
			file.printf ( "%s\n", str );
			// get 2nd line and change it to the new name
			bufgets ( str, &ptr, &bufferSize, sizeof ( str ) );
			file.printf ( "%s\n", sName );

			// get rest of lines and write them unchanged
			while ( bufferSize ) {
				bufgets ( str, &ptr, &bufferSize, sizeof ( str ) );
				file.printf ( "%s\n", str );
			}
			file.close ();

			sprintf ( sizeof ( renameStr ), renameStr, "mv %sdata/quests/%s /realm/3.0/data/quests/%s", qRootDir->table( 0, 0 ), lowerOld, lowerNew );
			system ( renameStr );

			delete qRootDir;
		}

		free ( lowerOld );
		free ( lowerNew );
	}

	else
	{
		htmlPrint ( "old name bad!!!" ); htmlNewLine ();
	}	
}

// clears the PVP flag
int CSCharacter::resetPVP ( void )
{
	int ret = 1;

	if ( peaceful )
		ret = 0;
	else
		peaceful = 1;

/*
	PackedData p;

	char line[1024];
	int size = strlen ( data ), skip, changed = 0;
	char *ptr = data;

	// skip the 1st 28 lines
	for ( skip = 0; skip < 28; skip++ ) {
		bufgets ( line, &ptr, &size );
		p.printf ( "%s\n", line );
	}

	// get current pvp
	bufgets ( line, &ptr, &size );

	if ( !strcmp ( line, "0" ) ) 
		ret = 1;

	// put a positive one
	p.printf ( "1\n" );

	// copy the rest of the data
	while ( size ) {
		bufgets ( line, &ptr, &size );
		p.printf ( "%s\n", line );
	}

	if ( ret )
		data = strdup ( (char*)p.data () );
*/

	return ret;
}

// clears the title
int CSCharacter::resetTitle ( void )
{
	int ret = 0;

	// if it's already been cleared, don't do it again
	if ( strcmp ( title, "<cleared by CS>" ) ) {
		ret = 1;
		if ( title )
			free ( title );

		title = strdup ( "<cleared by CS>" );
	}

/*
	PackedData p;

	char line[1024];
	int size = strlen ( data ), skip, changed = 0;
	char *ptr = data;

	// skip the 1st 4 lines
	for ( skip = 0; skip < 4; skip++ ) {
		bufgets ( line, &ptr, &size );
		p.printf ( "%s\n", line );
	}

	// get current title
	bufgets ( line, &ptr, &size );

	if ( !strcmp ( line, "<cleared by CS>" ) ) {
		p.printf ( "%s\n", line );
	} else {
		// write new title
		p.printf ( "<cleared by CS>\n" );
		ret = 1;
	}

	// copy the rest of the data
	while ( size ) {
		bufgets ( line, &ptr, &size );
		p.printf ( "%s\n", line );
	}

	if ( ret )
		data = strdup ( (char*)p.data () );
*/

	return ret;
}

// clears the bio
int CSCharacter::resetBio ( void )
{
	int ret = 0;

	// if it's already been cleared, don't do it again
	if ( strcmp ( bio, "<cleared by CS>" ) ) {
		ret = 1;
		if ( bio )
			free ( bio );

		bio = strdup ( "<cleared by CS>" );
	}

/*
	PackedData p;

	char line[1024];
	int size = strlen ( data ), skip, changed = 0;
	char *ptr = data;

	// skip the 1st 47 lines
	for ( skip = 0; skip < 47; skip++ ) {
		bufgets ( line, &ptr, &size );
		p.printf ( "%s\n", line );
	}

	// get current bio
	bufgets ( line, &ptr, &size );

	if ( !strcmp ( line, "<cleared by CS>" ) ) {
		p.printf ( "%s\n", line );
		ret = 0;
	} else {
		// write new bio
		p.printf ( "<cleared by CS>\n" );
	}

	// copy the rest of the data
	while ( size ) {
		bufgets ( line, &ptr, &size );
		p.printf ( "%s\n", line );
	}

	if ( ret )
		data = strdup ( (char*)p.data () );
*/

	return ret;
}

// returns the inv item if it's specialName matches srch
CSInvItem *CSCharacter::findSpecialName ( char *srch )
{
	for ( int i = 0; i < invItemList.size (); ++i ) {
		CSInvItem *inv = (CSInvItem*)invItemList.at ( i );
		CSInvItem *inv2 = inv->findSpecialName ( srch );

		if ( inv2 ) {
			return inv2;
		}
	}

	return NULL;
}


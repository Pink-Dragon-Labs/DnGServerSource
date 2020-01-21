//
// login
//
// This file contains a series of callbacks that handle the login process.
//
// author: Stephen Nichols
//

#include "roommgr.hpp"
#include "globals.hpp"
#include "callbacks.hpp"

// this is the list of LoginContexts pending processing
LinkedList gPendingLogins;

// this is the global login counter (used for context in callback)
int gLoginCounter = 0;

// this is a pointer to the active login context
LoginContext *gActiveLoginContext = NULL;

// this tracks the current number of characters to load
int gLoginCharCount = 0;

// this tracks the current loaded character
int gLoginCharNum = 0;

//
// LoginContext: This class holds the data necessary to process a login.
//
LoginContext::LoginContext()
{
	loginName = NULL;
	password = NULL;
	nOSVersion = 0;
	pID = NULL;
	player = NULL;
	id = -1;
}

LoginContext::LoginContext ( char *theLogin, char *thePass, int nOSID, int nCount, char* pThird, RMPlayer *thePlayer )
{
	if ( !theLogin )
		theLogin = strdup ( "<garbage>" );

	if ( !thePass )
		thePass = strdup ( "<garbage>" );

	stripWhitespace ( theLogin );
	stripWhitespace ( thePass );

	id = gLoginCounter++;
	loginName = theLogin;
	password = thePass;
	nOSVersion = nOSID;
	m_nCount = nCount;
	pID = pThird;

	player = thePlayer;

	// attach this context to the player
	player->loginContext = this;

	// add ourself to the pending login list
	gPendingLogins.add ( this );

	PackedMsg update;
	update.putWord ( gPendingLogins.indexOf ( this ) + 1 );

	// update the player on his position
	roomMgr->send ( _IPC_LOGIN_UPDATE, (char *)update.data(), update.size(), player->owner );

	// if we're the active context, start it up
	if ( gPendingLogins.size() == 1 )
		start();
}

LoginContext::~LoginContext()
{
	if ( loginName ) {
		free ( loginName );
		loginName = NULL;
	}

	if ( password ) {
		free ( password );
		password = NULL;
	}

	if ( pID ) {
		free ( pID );
		pID = NULL;
	}

	if ( player ) {
		player->loginContext = NULL;
		player = NULL;
	}

	id = -1;

	gPendingLogins.del ( this );

	// step through and update each login context position
	PackedMsg update;
	update.putWord ( 0 );

	LinkedElement *element = gPendingLogins.head();
	short idx = 1;

	while ( element ) {
		LoginContext *context = (LoginContext *)element->ptr();
		element = element->next();

		update.setWord ( 8, idx );
		roomMgr->send ( _IPC_LOGIN_UPDATE, (char *)update.data(), update.size(), context->player->owner );

		idx++;
	}

	// pass the buck if we're the active login
	if ( gActiveLoginContext == this ) {
		gActiveLoginContext = NULL;

		// is there anyone to pass the buck to?
		LoginContext *lc = (LoginContext *)gPendingLogins.at ( 0 );

		if ( lc )
			lc->start();
	}
}

// this member starts the process of logging in a context
void LoginContext::start ( void )
{
	gDataMgr->login ( id, loginName, password, nOSVersion, m_nCount, pID, player->owner->IP() );
	gActiveLoginContext = this;
}

// this member sends a response message to the player
void LoginContext::sendResponse ( int servID, int nRights, char *errorStr )
{
	PackedData ack;
	ack.putLong ( servID );
	ack.putLong ( 0 );
	ack.putLong ( servID );

	ack.putByte ( 1 ); //player->checkRegistration() );
	ack.putLong( nRights );
	ack.putString ( errorStr );

	roomMgr->send ( _IPC_PLAYER_CHECK_LOGIN, (char *)ack.data(), ack.size(), player->owner );
}

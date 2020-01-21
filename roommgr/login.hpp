//
// login
//
// This file contains a series of callbacks that handle the login process.
//
// author: Stephen Nichols
//

#ifndef _LOGIN_HPP_
#define _LOGIN_HPP_

#include "../global/list.hpp"

// define these classes to avoid compiler error
class RMPlayer;
class FMRequest;

//
// LoginContext: This class holds the data necessary to process a login.
//
class LoginContext : public ListObject
{
public:
	LoginContext();
	LoginContext ( char *theLogin, char *thePass, int nOSID, int nCount, char* pThird, RMPlayer *player );
	virtual ~LoginContext();

	// this member starts the login process for this context
	void start ( void );

	// this member sends a response message to the player
	void sendResponse ( int servID, int nRights, char *errorString="" );

	// this member is the login name provided
	char *loginName;

	// this member is the password provided
	char *password;

	// this member is the os version number
	int	nOSVersion;

	// this member is the number of id code(s) from this client
	int	m_nCount;

	// this member is the id code from this client
	char* pID;

	// this is the RMPlayer that response messages should be sent to
	RMPlayer *player;

	// this is the context ID
	int id;
};

// this is the list of LoginContexts pending processing
extern LinkedList gPendingLogins;

// this is the global login counter (used for context in callback)
extern int gLoginCounter;

// this is a pointer to the active login context
extern LoginContext *gActiveLoginContext;

#endif

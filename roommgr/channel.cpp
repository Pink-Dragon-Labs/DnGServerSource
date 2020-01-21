//
// CHANNEL.CPP
// Author: Stephen Nichols
//

#include "roommgr.hpp"

Channel *gChannels[_MAX_CHANNEL];

Channel::Channel()
{
	isReadOnly = 0;
	isSystem = 0;
	name = NULL;
	topic = NULL;
	number = 0;
	m_pPassword = NULL;

	setName ( "Gossip" );
	setTopic ( "General discussions." );
}

Channel::~Channel()
{
	members.release();
	gms.release();
	m_ModeratorList.release();

	setName ( NULL );
	setTopic ( NULL );
	setPassword ( NULL );
}

void Channel::addPlayer ( RMPlayer *player )
{
	player->channel = this;

	if ( !player->checkAccess ( _ACCESS_GUIDE ) ) {
		if ( isModerator ( player ) ) {
			if ( !(player->player->physicalState & _STATE_BUSY) )
				sendInfo ( "'%s' has just has come online.\n", player->getName() );
		} else {
			sendInfo ( "'%s' has come online.\n", player->getName() );
		}
	}

	if ( player->checkAccess ( _ACCESS_GUIDE ) ) 
		gms.add ( player );

	// make the first player a moderator...
	if ( !isSystem && !members.size() ) {
		makeModerator ( player );	
	} else {
		if ( isModerator ( player ) && !player->checkAccess ( _ACCESS_GUIDE ) ) 
			m_ModeratorList.add ( player );
	}

	members.add ( player );
}

void Channel::delPlayer ( RMPlayer *player, bool bLeft )
{
	gms.del ( player );
	members.del ( player );
	m_ModeratorList.del ( player );

	// handle the last player leaving...
	if ( !members.size() ) {
		// clean up the special lists...
		m_ModeratorNameList.dispose();
		m_BannedNameList.dispose();

		// reset the channel info...
		if ( !isSystem ) {
			setName ( "Gossip" );
			setTopic ( "General discussions." );
			setPassword ( NULL );
		}
	} else {
		if ( !player->checkAccess ( _ACCESS_GUIDE ) )
			if ( bLeft )
				sendInfo ( "'%s' has gone offline.\n", player->getName() );
			else
				sendInfo ( "'%s' was banned.\n", player->getName() );
	}

	player->channel = NULL;
}

void Channel::sendText ( const char *format, ... )
{
	if ( members.size() ) {
		char output[1024];
		va_list args;

		va_start ( args, format );
		vsprintf ( sizeof ( output ), output, format, args );
		va_end ( args );

		roomMgr->sendListText ( &members, output );
	}
}

void Channel::sendInfo ( const char *format, ... )
{
	if ( members.size() ) {
		char output[1024];
		va_list args;

		va_start ( args, format );
		vsprintf ( sizeof ( output ), output, format, args );
		va_end ( args );

		roomMgr->sendListInfo ( &members, output );
	}
}

void Channel::setName ( char *str )
{
	if ( name ) {
		free ( name );
		name = NULL;
	}

	if ( str ) 
		name = strdup ( str );
}

void Channel::setTopic ( char *str )
{
	if ( topic ) {
		free ( topic );
		topic = NULL;
	}

	if ( str )
		topic = strdup ( str );
}

// set the password of this channel...
void Channel::setPassword ( char *pPassword ) 
{
	if ( m_pPassword ) {
		free ( m_pPassword );
		m_pPassword = NULL;
	}

	if ( pPassword ) {
		m_pPassword = strdup ( pPassword );
	}
}

// get the password of this channel
char *Channel::getPassword ( void )
{
	return m_pPassword;
}

// check the password provided to see if it matches this channel password
int Channel::checkPassword ( char *pPassword )
{
	if ( NULL == m_pPassword )
		return 1;

	if ( pPassword ) {
		if ( !strcasecmp ( pPassword, m_pPassword ) )
			return 1;
	}

	return 0;
}

// is the given character a moderator of this channel?
int Channel::isModerator ( RMPlayer *pPlayer )
{
	if ( isSystem )
		return 0;

	if ( pPlayer->checkAccess ( _ACCESS_GUIDE ) ) 
		return 1;

	return isModerator ( pPlayer->getName() );
}

// is the given character name a moderator of this channel?
int Channel::isModerator ( char *pCharName )
{
	if ( isSystem )
		return 0;

	LinkedElement *pElement = m_ModeratorNameList.head();

	while ( pElement ) {
		StringObject *pStringObj = (StringObject *)pElement->ptr();
		pElement = pElement->next();

		if ( !strcasecmp ( pCharName, pStringObj->data ) )
			return 1;
	}

	return 0;
}

// make the given player a moderator of this channel...
void Channel::makeModerator ( RMPlayer *pPlayer )
{
	if ( !isModerator ( pPlayer ) ) {
		m_ModeratorList.add ( pPlayer );
		m_ModeratorNameList.add ( new StringObject ( pPlayer->getName() ) );
	}
}

// make the given player a moderator of this channel by name...
void Channel::makeModerator ( char *pName )
{
	if ( !isModerator ( pName ) ) {
		m_ModeratorNameList.add ( new StringObject ( pName ) );
	}
}

// remove the given player from the moderator list (revoke moderator rights)
void Channel::removeModerator ( RMPlayer *pPlayer )
{
	m_ModeratorList.del ( pPlayer );
	removeModerator ( pPlayer->getName() );
}

// remove the given player from the moderator list by name
void Channel::removeModerator ( char *pName )
{
	LinkedElement *pElement = m_ModeratorNameList.head();

	while ( pElement ) {
		StringObject *pStringObj = (StringObject *)pElement->ptr();

		// if the name matches, delete the element and the string...
		if ( !strcasecmp ( pName, pStringObj->data ) ) {
			m_ModeratorNameList.delElement ( pElement );	
			delete pStringObj;

			break;
		}

		pElement = pElement->next();
	}
}

// get the top moderator name and return it (N/A if none)
char *Channel::getTopModeratorName ( void )
{
	// return "The Realm" if is a system channel
	if ( isSystem ) {
		return "The Realm";
	}

	// step through the list of moderators and return the first non-imp name
	LinkedElement *pElement = m_ModeratorList.head();

	while ( pElement ) {
		RMPlayer *pPlayer = (RMPlayer *)pElement->ptr();
		pElement = pElement->next();

		if ( pPlayer && !(pPlayer->player->physicalState & _STATE_BUSY) ) {
			return pPlayer->getName();
		}
	}

	return "N/A";
}

// ban a logged in player...
void Channel::banPlayer ( RMPlayer *pPlayer )
{
	banPlayer ( pPlayer->getName() );
}

// ban a player by name...
void Channel::banPlayer ( char *pPlayerName )
{
	// only ban players that are not banned
	if ( !isBanned ( pPlayerName ) ) {
		m_BannedNameList.add ( new StringObject ( pPlayerName ) );

		//
		// if the size of the list exceeds what's allowed, toss the top banned
		// player
		//
		const int nMaxBannedSize = 100;

		while ( m_BannedNameList.size() > nMaxBannedSize ) {
			StringObject *pObj = (StringObject *)m_BannedNameList.head()->ptr();
			m_BannedNameList.delElement ( m_BannedNameList.head() );

			delete pObj;
		}
	}
}

// unban a logged in player...
void Channel::unbanPlayer ( RMPlayer *pPlayer )
{
	unbanPlayer ( pPlayer->getName() );
}

// unban a player by name...
void Channel::unbanPlayer ( char *pPlayerName )
{
	LinkedElement *pElement = m_BannedNameList.head();

	while ( pElement ) {
		StringObject *pObj = (StringObject *)pElement->ptr();

		if ( !strcasecmp ( pPlayerName, pObj->data ) ) {
			m_BannedNameList.delElement ( pElement );
			delete pObj;

			return;
		}

		pElement = pElement->next();
	}
}

// is a logged in player banned?
int Channel::isBanned ( RMPlayer *pPlayer )
{
	return isBanned ( pPlayer->getName() );
}

// is a player banned by name?
int Channel::isBanned ( char *pPlayerName )
{
	LinkedElement *pElement = m_BannedNameList.head();

	while ( pElement ) {
		StringObject *pObj = (StringObject *)pElement->ptr();
		pElement = pElement->next();

		if ( !strcasecmp ( pPlayerName, pObj->data ) ) {
			return 1;
		}
	}

	return 0;
}

// clear the banned lsit
void Channel::clearBanList ( void )
{
	m_BannedNameList.dispose();
}

int Channel::isEmpty()
{
	return ! members.size();
}

// put our membership list into a packet format...
void Channel::listMembers ( PackedData *pPacket )
{
	pPacket->putLong( members.size() - gms.size() );

	// step through the list of members
	LinkedElement *pElement = members.head();

	while ( pElement ) {
		RMPlayer *pPlayer = (RMPlayer *)pElement->ptr();
		pElement = pElement->next();

		if ( !pPlayer->checkAccess ( _ACCESS_GUIDE ) ) {
			if ( isModerator( pPlayer ) ) {
				if ( ( pPlayer->player->physicalState & _STATE_BUSY ) ) {
					pPacket->putString ( "None" );
					pPacket->putByte ( 2 );
				} else {
					pPacket->putString ( pPlayer->getName() );
					pPacket->putByte ( 1 );
				}
			} else {
				pPacket->putString ( pPlayer->getName() );
				pPacket->putByte ( 0 );
			}
		}
	}
}

// put our banned players into packet format...
void Channel::listBanned ( PackedData *pPacket )
{
	pPacket->putLong( m_BannedNameList.size() );

	// step through the list of banned
	LinkedElement *pElement = m_BannedNameList.head();

	while ( pElement ) {
		StringObject *pObj = (StringObject *) pElement->ptr();
		pElement = pElement->next();

		pPacket->putString ( pObj->data );
	}
}



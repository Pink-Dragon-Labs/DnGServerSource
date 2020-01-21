//
// CHANNEL.HPP
//	Author: Stephen Nichols
//

#ifndef _CHANNEL_HPP_
#define _CHANNEL_HPP_

#include "../global/system.hpp"

#define _MAX_CHANNEL 1000

class RMPlayer;

class Channel
{
protected:
	// holds the RMPlayer object pointers of on-line player moderators
	LinkedList m_ModeratorList;

	// holds StringObject pointers to names of player moderators
	LinkedList m_ModeratorNameList;

	// holds StringObject pointers to names of banned players...
	LinkedList m_BannedNameList;

	// current password for the channel
	char *m_pPassword;

public:
	Channel();
	virtual ~Channel();

	void addPlayer ( RMPlayer *player );
	void delPlayer ( RMPlayer *player, bool bLeft = TRUE );

	void sendText ( const char *format, ... );
	void sendInfo ( const char *format, ... );

	void makeModerator ( RMPlayer *pPlayer );
	void makeModerator ( char *pPlayerName );

	void removeModerator ( RMPlayer *pPlayer );
	void removeModerator ( char *pPlayerName );

	int isModerator ( RMPlayer *pPlayer );
	int isModerator ( char *pPlayerName );

	void makePrivate ( void );
	void makePublic ( void );

	void banPlayer ( RMPlayer *pPlayer );
	void banPlayer ( char *pPlayerName );

	void unbanPlayer ( RMPlayer *pPlayer );
	void unbanPlayer ( char *pPlayerName );

	int isBanned ( RMPlayer *pPlayer );
	int isBanned ( char *pPlayerName );

	int isEmpty();

	void clearBanList ( void );

	void setName ( char *str );
	void setTopic ( char *str );

	// get the top moderator name and return it (N/A if none)
	char *getTopModeratorName ( void );

	// put our membership list into a packet format...
	void listMembers ( PackedData *pPacket );

	// put our moderator list into packet format...
	void listModerators ( PackedData *pPacket );

	// put our banned players into packet format...
	void listBanned ( PackedData *pPacket );

	void setPassword ( char *pPassword );
	int checkPassword ( char *pPassword );
	char *getPassword ( void );

	// holds RMPlayer pointers to normal members and GMs...
	LinkedList members, gms;

	char *name, *topic;
	int number;
	int isReadOnly : 1;
	int isSystem : 1;
};

extern Channel *gChannels[_MAX_CHANNEL];

#endif

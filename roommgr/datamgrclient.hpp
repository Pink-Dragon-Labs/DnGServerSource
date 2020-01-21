//
// datamgrclient
//
// This file contains the DataMgrClient class and supporting structures.
//
// author: Stephen Nichols
//

#ifndef _DATAMGR_CLIENT_HPP_
#define _DATAMGR_CLIENT_HPP_

class Building;
class RMPlayer;
class WorldObject;

struct CrimeData;

// enumerate the permanent log type (this must match the SQL type field in 
// accountLog)
enum {
	_DMGR_PLOG_UNKNOWN = 1,
	_DMGR_PLOG_GMCMD,
	_DMGR_PLOG_IMPCMD,
	_DMGR_PLOG_SETPASS,
	_DMGR_PLOG_SETTITLE,
	_DMGR_PLOG_ERASECHAR,
	_DMGR_PLOG_CREATECHAR,
	_DMGR_PLOG_DISABLED,
	_DMGR_PLOG_SUSPENDED,
	_DMGR_PLOG_ENABLED,
	_DMGR_PLOG_LOSTCHAR,
	_DMGR_PLOG_RENEWAL,
	_DMGR_PLOG_REMOVED,
	_DMGR_PLOG_CANCELLED,
	_DMGR_PLOG_UPD_REG,
	_DMGR_PLOG_UPD_BILL,
	_DMGR_PLOG_SYSCMD,
	_DMGR_PLOG_BANNED,
	_DMGR_PLOG_PWD_GUESS,
	_DMGR_PLOG_EVENTS,
	_DMGR_PLOG_MAX
};

// define the datamgr callback function pointer
typedef void (*hlcallback_t) ( int result, int context, int houseID, int accountID, char *name, int size, char *data );

//
// This class serves as an interface between this process and the remote
// DataMgr process.
//

class DataMgrClient : public IPCClient {
public:
	DataMgrClient();
	virtual ~DataMgrClient();

	// this member handles incoming messages
	virtual void handleMessage ( IPCMessage *msg );

	// this member is called to log something permanently
	void logPermanent ( char *login, char *charName, int type, const char *format, ... );

	// this member is called to log engraves
	void logEngrave( const char* pLogin, const char* charName, const char* pName );
	
	// this member is called to send the hello message to the server
	void hello ( void );

	// this member is called to log an account in
	void login ( int id, char *login, char *password, int nOSVersion, int nCount, char *pID, int nIP );

	// get config information for this character
	void config( WorldObject* character );

	// write out the quests data
	void writeQuests( WorldObject* character, int nSize, int* pData );

	// write out the crimes data
	void writeCrimes( WorldObject* character, int nSize, CrimeData* pData );

	// this member is called to log an account out of the system
	void logout ( RMPlayer *target );

	// this member is called to log an account out of the system
	void logout ( int id );

	// this member is called to revoke the gossip rights of an account
	void revoke ( RMPlayer *target, int state = 1 );

	// this member is called to gag an account
	void gag ( RMPlayer *target, int state = 1 );

	// this member is called to suspend an account
	void suspend ( RMPlayer *target );

	// this member is called to disable an account
	void disable ( RMPlayer *target );

	// this member sets or clears the login message
	void LoginMessage ( char* pMessage );

	// this member sets or clears the downtime message
	void DowntimeMessage ( char* pMessage );
	
	// this member is called to update the password of an account
	void setpass ( RMPlayer *target, char *password );

	// this member is called to write an existing character to the database
	void writeCharacter ( WorldObject *character, PackedData *data );

	// this member is called to create a new character
	void newCharacter ( WorldObject *character, PackedData *data );

	// this member is called to retrieve the whats new information
	void whatsNew ( WorldObject *character );
	
	// this member is called to delete a character
	void delCharacter ( WorldObject *character );

	// this member is called to create a new house
	void newHouse ( Building *house, PackedData *data );

	// this member is called to write an existing house
	void writeHouse ( Building *house, PackedData *data );

	// this member loads a house from the database
	void loadHouse ( char *name, hlcallback_t callback, int context );

	// this member loads a house from the database
	void loadAllHouses ();

	// this member is called to set rights of an account
	void rights ( RMPlayer *target );

	// this member is called to get the list of mail messages
	void getMailList( WorldObject* character );

	// this member is called to get the list of mail messages
	void getMailMessage( WorldObject* character, int nMsgID );

	// this member is called to get the list of mail messages
	void deleteMail( WorldObject* character, int nMsgID );

	// this member is called to get the list of mail messages
	void sendMail ( WorldObject* character, char* pPtr, int nSize );
	void sendMail( WorldObject* character, char* pTo, char* pBody, int nCount );

	// Give coppers to a player
	void copper( RMPlayer* target, int nAmount );

	// Give credit days to a player
	void credit( RMPlayer* target, int nAmount );

	// handle adding a bounty on a player not online
	void placeBounty( WorldObject* character, char* pName, long nBounty );

	// Handle adding a friend
	void AddFriend( RMPlayer* player, char* pName );

	// Handle removing a friend
	void DelFriend( RMPlayer* player, char* pName );

	// Handle adding a foe
	void AddFoe( RMPlayer* player, char* pName );

	// Handle removing a foe
	void DelFoe( RMPlayer* player, char* pName );

	// handle checking for a valid player to ignore
	void CheckFoe( RMPlayer* player, char* pName );

	// handle saving the spells list for a player
	void WriteSpells( RMPlayer* player, IPCMessage* message );

	// handle documenting the crasher
	void crasher( int nID );
};

// this is the global datamgr client
extern DataMgrClient *gDataMgr;

#endif

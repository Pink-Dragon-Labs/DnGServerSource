#ifndef _RMPLAYER_HPP_
#define _RMPLAYER_HPP_

//#define _MAX_GROUP_SIZE 10
#define _MAX_GROUP_SIZE 6
#define _MAX_NPC_GROUP_SIZE 12
#define _MAX_CHAR_NAME 17
#define _SECURITY_BUF_SIZE 32

// define access types
// change also those in data manager.
#define	_ACCESS_NORMAL			0x00000000
#define	_ACCESS_IMPLEMENTOR		0x00000001
#define _ACCESS_MODERATOR		0x00000043
#define _ACCESS_GUIDE			0x00000047
#define _ACCESS_PROPHET			0x00000009
#define _ACCESS_CS			0x00000013
#define _ACCESS_EVENT			0x00000021
#define _ACCESS_PUBLICRELATIONS		0x00000041

#define _ACCESS_PRIVILEGED		0x0000007f
#define _ACCESS_ONLY_EVENT		0x00000020
#define _ACCESS_ONLY_PROPHET		0x00000008

#define _ACCESS_TELEPORT		0x00001053
#define _ACCESS_BASIC_TELEPORT	0x00002017
#define _ACCESS_SHUTDOWN		0x00004001
#define _ACCESS_LOG_CHAT		0x00008001
#define _ACCESS_TOON_MODIFY		0x00010001
#define _ACCESS_CONJURE			0x00100001
#define _ACCESS_DISABLE			0x00200011
#define _ACCESS_AMBUSH			0x0040001f
#define _ACCESS_HEAL			0x00800001
#define _ACCESS_BUY_STORE		0x04000003
#define _ACCESS_LOG_ACCOUNT		0x08000000

#define	_ACCESS_DISABLED		0x10000000
#define	_ACCESS_SUSPENDED		0x20000000
#define	_ACCESS_NO_GOSSIP		0x40000000
#define	_ACCESS_GAGGED			0x80000000


#define _PRONOUN_HIM		0
#define _PRONOUN_HIS		1
#define _PRONOUN_HE			2

// how many seconds in 1 year?
#define _EXPIRATION_TIME (365 * 24 * 60 * 60)
#define _90DAY_TIME (90 * 24 * 60 * 60)
#define _TRIAL_TIME (30 * 24 * 60 * 60)

// define package info
#define _PKG_BASIC			1
#define _PKG_COMBAT			2

#define _TALK_GO_ON 		1
#define _TALK_GO_BACK		2
#define _TALK_LAST_MENU		4

#define _TRIAL				0
#define _90DAY				1
#define _12MONTH			2

#define ROOM_LIMIT_SIZE 30

#include "../global/system.hpp"
#include "playerstate.hpp"

// define external classes...
class RMRoom;
class WorldObject;
class Zone;
class Channel;
class TalkTree;
class TalkTreeTopic;
class QuestInfo;
class TrackingInfo;
class LoginContext;
class CFriend;

struct chatLog {
        char   sText[ 360 ];
        char   sToon[ 36 ];
        char   sAccount[ 36 ];
        int    nRoom;
        int    nChannel;
		bool   isPrivateChannel;
};

// Any changes to this struct need to be made also in the datamgr/datamgr.hpp file!!!!
// Any changes to this struct need to be made also in the datamgr/datamgr.hpp file!!!!
struct CrimeData {
        int criminal, loadTime;
        unsigned murders;
        unsigned pickedPockets;
        unsigned tourneyKills, tourneyDeaths;
        unsigned arenaKills, arenaDeaths;
        unsigned criminalsKilled;
        unsigned bountyCollected;
        unsigned bountyOnHead;
        int timeLeft;
};
// Any changes to the above struct need to be made also in the datamgr/datamgr.hpp file!!!!
// Any changes to the above struct need to be made also in the datamgr/datamgr.hpp file!!!!

class RMPlayer : public Player {
protected:
	// pointer to the friend object that represents this player...
	CFriend *m_pFriend;

public:
	int rights, pkgInfo, allowJoin, creationTime, expirationTime, serial, gossipBanTime, isTeleporting, validated;
	int lastAmbushTime, lastJumpTime;
	int ping, pingTime, pingClientTime, badPingCount, lastWriteTime;
		
	RMRoom *room;
	IPCMessage *loginMsg;
	WorldObject *player, *character;
	int teleportRoomNum;
	char *teleportDestBuildingOwner;
	Zone *zone;
	int isNPC, tossing, delay, secured;
	WorldObject *dungeonEntrance;
	int badGossipTime, badGossipCount, seed;
	WorldObject *_lockedObj, *lastTalkTarget;
	char *roomTitle;
	int lastTalkSequence;
	Channel *channel;
	LinkedList *monsterList;
	
	//mike-groupspawn - this is here so when the group is dissolved, we can
	//remove the group from this list.
	//-This list is a member of a MonsterGroup object, which is tied to some
	// zone out there. If this groupList member is NULL, that means we are
	// not the leader of a group that was spawned as a group by a zone.
	//-This works just like monsterList above,
	// except at the group level, rather than individual monsters.
	LinkedList *groupList;

	CrimeData* m_pCrimes;

	void writeCrimes();
	CrimeData* getCrimeData();

	int accountID;
	CombatAction *combatAction, *nextAction;
	char *engraveName, *accountTypeStr;
	int billingDate;
	int nCredits, nCoppers;

	LoginContext *loginContext;

	LinkedList *ignoreList;

	// talk tree info
	TalkTree *talkTree;
	TalkTreeTopic *talkTreeTopic;
	int talkTreePage;

	// message log
	chatLog*	textLogs;
	int		textCount;
	char*		m_pLastComplain;

	int firstLogin; //, trialAccount;

	// quests being done
	LinkedList *quests;

	// declare who is the leader of my group
	bool isGroupLeader ( void );
	RMPlayer *groupLeader;
	RMPlayer* waitingMember;
	LinkedList group;

	RMPlayer();
	virtual ~RMPlayer();
	
	// what type of account?
	int accountType;

	// get the type of player account, trial, 90Day, 12Month etc.
	char * getAccountType( void );

	// set my combat action
	void setAction ( CombatAction *action );

	// set my teleport room num
	void setTeleportRoomNum ( int num );

	// set my engrave name
	char *setEngraveName ( char *str );

	// go to a new room
	void newRoom ( RMRoom *room );

	// set the room title
	void setRoomTitle ( char *str );

	// add text to the text log
	void addText ( RMPlayer *thePlayer, char *text );

	// report the text log
	void reportText ( char* name, char* str, char* pReason );

	// cast a spell
	void castSpell ( spell_info *spell, WorldObject *caster, long targetServID, int targetX, int targetY, PackedData *packet );

	// perform a proc
    void castProc ( spell_info *spell, WorldObject *caster, long targetServID, int targetX, int targetY, PackedData *packet );

	void rob ( WorldObject *target, PackedData *packet );

	// process requests
	int processRequest ( IPCMessage *message );
	int process_IPC_GET_REPAIR_PRICE ( IPCMessage *message );
	int process_IPC_REPAIR ( IPCMessage *message );
	int process_IPC_TERMINATED ( IPCMessage *message );
	int process_IPC_CHANGE_PASSWORD ( IPCMessage *message );
	int process_IPC_GET_POSN ( IPCMessage *message );
	int process_IPC_GROUP_KICK ( IPCMessage *message );
	int process_IPC_GROUP_LEAVE ( IPCMessage *message );
	int process_IPC_GROUP_JOIN ( IPCMessage *message );
	int process_IPC_GROUP_QUESTION ( IPCMessage *message );
	int process_IPC_UNLOCK_OBJ ( IPCMessage *message );
	int process_IPC_LOCK_OBJ ( IPCMessage *message );
	int process_IPC_FATAL_DATA ( IPCMessage *message );
	int process_IPC_QUEST_DECLINE ( IPCMessage *message );
	int process_IPC_QUEST_ACCEPT ( IPCMessage *message );
	int process_IPC_TREE_BACK ( IPCMessage *message );
	int process_IPC_TREE_GET_TEXT ( IPCMessage *message );
	int process_IPC_TREE_CHOOSE_TOPIC ( IPCMessage *message );
	int process_IPC_TREE_GET ( IPCMessage *message );
	int process_IPC_GET_BOOK_PAGE ( IPCMessage *message );
	int process_IPC_GET_BOOK_INFO ( IPCMessage *message );
	int process_IPC_CAST_SPELL ( IPCMessage *message );
	int process_IPC_MONEY_GIVE ( IPCMessage *message );
	int process_IPC_MONEY_TAKE ( IPCMessage *message );
	int process_IPC_MONEY_PUT ( IPCMessage *message );
	int process_IPC_MONEY_DROP ( IPCMessage *message );
	int process_IPC_SHOP_EXAMINE ( IPCMessage *message );
	int process_IPC_SHOP_BUY ( IPCMessage *message );
	int process_IPC_SHOP_RECHARGE ( IPCMessage *message );
	int process_IPC_SHOP_GET_RECHARGE_PRICE ( IPCMessage *message );
	int process_IPC_SHOP_GET_PRICE ( IPCMessage *message );
	int process_IPC_SHOP_SELL ( IPCMessage *message );
	int process_IPC_SET_TITLE ( IPCMessage *message );
	int process_IPC_GET_SHOP_INFO ( IPCMessage *message );
	int process_IPC_COMBAT_EXIT ( IPCMessage *message );
	int process_IPC_COMBAT_FLEE ( IPCMessage *message );
	int process_IPC_VERB_DYE ( IPCMessage *message );
	int process_IPC_VERB_STAND ( IPCMessage *message );
	int process_IPC_VERB_ROB ( IPCMessage *message );
	int process_IPC_VERB_MEMORIZE ( IPCMessage *message );
	int process_IPC_VERB_SIT ( IPCMessage *message );
	int process_IPC_VERB_CONSUME ( IPCMessage *message );
	int process_IPC_VERB_ENGAGE ( IPCMessage *message );
	int process_IPC_COMBAT_ACTION ( IPCMessage *message );
	int process_IPC_VERB_UNLOCK ( IPCMessage *message );
	int process_IPC_VERB_LOCK ( IPCMessage *message );
	int process_IPC_VERB_CLOSE ( IPCMessage *message );
	int process_IPC_VERB_OPEN ( IPCMessage *message );
	int process_IPC_MAIL_MESSAGE_SEND ( IPCMessage *message );
	int process_IPC_MAIL_MESSAGE_DELETE ( IPCMessage *message );
	int process_IPC_MAIL_MESSAGE_GET ( IPCMessage *message );
	int process_IPC_MAIL_MESSAGE_ARCHIVE ( IPCMessage *message );
	int process_IPC_MAIL_MESSAGE_COMPLAIN ( IPCMessage *message );
	int process_IPC_MAIL_LIST_GET ( IPCMessage *message );
	int process_IPC_SEND_REG ( IPCMessage *message );
	int process_IPC_MIX_OBJECT ( IPCMessage *message );
	int process_IPC_VERB_PUT_IN ( IPCMessage *message );
	int process_IPC_VERB_TAKE_OFF ( IPCMessage *message );
	int process_IPC_VERB_PUT_ON ( IPCMessage *message );
	int process_IPC_VERB_DROP ( IPCMessage *message );
	int process_IPC_GET_QUEST_LIST ( IPCMessage *message );
	int process_IPC_VERB_GIVE ( IPCMessage *message );
	int process_IPC_VERB_USE ( IPCMessage *message );
	int process_IPC_VERB_GET ( IPCMessage *message );
	int process_IPC_PLAYER_SET_BIOGRAPHY ( IPCMessage *message );
	int process_IPC_PLAYER_GET_BIOGRAPHY ( IPCMessage *message );
	int process_IPC_PLAYER_GET_DESCRIPTION ( IPCMessage *message );
	int process_IPC_PLAYER_SET_HEAD_DATA ( IPCMessage *message );
	int process_IPC_PLAYER_GET_EXTENDED_PROPS ( IPCMessage *message );
	int process_IPC_PLAYER_QUERY_CHARACTERS ( IPCMessage *message );
	int process_IPC_PLAYER_DESTROY_CHARACTER ( IPCMessage *message );
	int process_IPC_PLAYER_CREATE_CHARACTER ( IPCMessage *message );
	int process_IPC_PLAYER_CHAT ( IPCMessage *message );
	int process_IPC_PLAYER_SYSTEM_MSG ( IPCMessage *message );
	int process_IPC_PLAYER_ROOM_CHAT ( IPCMessage *message );
	int process_IPC_PLAYER_MOVIE ( IPCMessage *message );
	int process_IPC_PLAYER_SHIFT_ROOM ( IPCMessage *message );
	int process_IPC_TALK ( IPCMessage *message );
	int process_IPC_GET_ENTRY_INFO ( IPCMessage *message );
	int process_IPC_GET_HOUSE ( IPCMessage *message );
	int process_IPC_PLAYER_CHANGE_ROOM ( IPCMessage *message );
	int process_IPC_CLIENT_HUNG_UP ( IPCMessage *message );
	int process_IPC_PLAYER_OLD_CHARACTER_LOGIN ( IPCMessage *message );
	int process_IPC_PLAYER_CHARACTER_LOGIN ( IPCMessage *message );
	int process_IPC_LOGIN_DONE ( IPCMessage *message );
	int process_IPC_INN_LOGIN ( IPCMessage *message );
	int process_IPC_CREATE_OBJECT ( IPCMessage *message );
	int process_IPC_WHATS_NEW ( IPCMessage *message );
	int process_IPC_SET_ENGRAVE_NAME ( IPCMessage *message );
	int process_IPC_GET_LOOK_INFO ( IPCMessage *message );
	int process_IPC_SELL_CRYSTALS ( IPCMessage *message );
	int process_IPC_UPDATE_ATTRIBUTES ( IPCMessage *message );
	int process_IPC_MASS_BUY ( IPCMessage *message );
	int process_IPC_MASS_SELL ( IPCMessage *message );
	int process_IPC_GET_SELL_PRICES ( IPCMessage *message );
	int process_IPC_CREATE_CHANNEL ( IPCMessage *message );
	int process_IPC_GET_REPAIR_PRICES ( IPCMessage *message );
	int process_IPC_MASS_REPAIR ( IPCMessage *message );

	int process_IPC_TRADE( IPCMessage *message );


	// put a logout request on my request queue
	void forceLogout ( void );

	// process a chat string, handle special commands
	void processRoomChat ( char *text );

	// verb processing methods
	void preVerbMessage ( PackedData *message, PackedData *movie, WorldObject **directObject, WorldObject **indirectObject );

	void postVerbMessage ( int result, int verbMsg, PackedData *movie, WorldObject *directObject, WorldObject *indirectObject );

	// return the title for this player's character
	char *getName ( void );

	// return the login name for this player
	char *getLogin ( void );
	char *getPassword ( void );

	// return the pronoun for addressing this player's character
	char *getPronoun ( int plural = 0 );

	// send a text message to my room
	void sendRoomText ( const char *format, ... );

	// send a chat message to my room (chat messages are logged)
	void sendRoomChat ( const char *format, ... );

	// ignore commands - ignore another player
	void ignore( char* pName );
	void unignore( char* pName );
	int isIgnoring( char* pName );

	int autoGive;
	int invite;

	// exit my combat group
	void exitCombat ( PackedData *movie = NULL );

	// quest stuff
	void loadQuestData ( int* pData );
	void writeQuestData ( void );
	QuestInfo *findQuest ( long number );
	void addQuest ( QuestInfo *info );
	void delQuest ( QuestInfo *info );

	// attack a player
	virtual int attack ( WorldObject *obj, PackedData *packet, int retaliate = 1, int canDodge = 1, int canBlock = 1 );
	virtual int engage ( WorldObject *obj, PackedData *packet, int theX = -1, int theY = -1 );

	// die, please
	virtual void die ( PackedData *packet, WorldObject *killer );

	// group management
	int canAddMember();
	int addGroupMember ( RMPlayer *member );
	int delGroupMember ( RMPlayer *member );
	int isGroupMember ( RMPlayer *member );
	int joinGroup ( RMPlayer *player );
	void leaveGroup ( void );
	void kickGroupMember ( RMPlayer *member );
	void disbandGroup ( void );
	void sendGroupText ( const char *format, ... );
	void sendGroupChat ( const char *format, ... );
	void sendToGroup ( int type, unsigned char *buffer, int size, RMPlayer *excusion = NULL );

	int nWantingToJoin[ _MAX_GROUP_SIZE ];
	int nCountWantingToJoin;
	
	// turn hands on
	void handsOn ( void );

	// access junk
	void initAccess ( int access ) { rights = access; }
	void setAccess ( int access );
	void clearAccess ( int access );
	int checkTimedAccess ( int access );
	int checkAccess ( int access ) { return ( rights & access ); }
	int canControl ( RMPlayer *player );

	// access control
	void disable ( const char *format, ... );
	void suspend ( const char *format, ... );
	void writeSerialNumber( void );
	int allowedToLogin ( void );

	int checkRegistration( void );

	// get the pointer to our friend object...
	CFriend *GetFriendEntry ( void );

	// add a new friend reference to this player
	void AddFriend ( char *pPlayerName );

	// delete an existing friend reference from this player
	void DelFriend ( char *pPlayerName );

	// check to see if they are a friend already
	bool IsFriend( char* pPlayerName );

	// Race check ---
	int CheckRace(WorldObject *object, int theRace);

	// Class check ---
	int CheckClass(WorldObject *object, int theClass);
};

extern void *playerThread ( void *arg );
extern void *loginThread ( void *arg );
extern void *findServIDThread ( void *arg );
extern void *dumpThread ( void *arg );
extern void *validateLoginThread ( void *arg );
extern void stripTokens ( LinkedList * );
extern int isProfane ( const char *str );

extern int isValidName ( const char *theName );

extern int gLimboCount;
extern int gCurMsgType;

extern BinaryTree gCharacterTree;
extern BinaryTree gLoginTree;
extern BinaryTree gLoginTreeNotInGame;

// validates player
int loadValidatedPlayer ( RMPlayer *player, char *login, char *password, char *buffer, int bufferSize, int doEverything = 1 );

// sets the account days for account type
int getDays ( RMPlayer *player );



extern char gLoadingChar[2048];

void logMoney ( const char *format, ... );

RMPlayer *findPlayer ( char *name, RMPlayer *player );

#endif

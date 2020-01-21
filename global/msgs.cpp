#include "msgs.hpp"
//#include "string.hpp"

char gMsgNames[256][60] = {
	"CLIENT_SEND",					//	0
	"CLIENT_CONNECTED",				//	1
	"CLIENT_HUNG_UP",				//	2
	"SERVER_SEND",					//	3
	"SERVER_CONNECTED",				//	4
	"SERVER_HUNG_UP",				//	5
	"OLD_PATCH_REQUEST",			//	6
	"OLD_PATCH_BLOCK",				//	7
	"OLD_PATCH_INFO",				//	8
	"",								//	9
	"ROUTE_INFO",					//	10
	"PATCH_REQUEST",				//	11
	"PATCH_BLOCK",					//	12
	"PATCH_INFO",					//	13
	"PLAYER_CHECK_LOGIN",			//	14
	"SECURITY",						//	15
	"INN_LOGIN",					//	16
	"SERVER_PING",					//	17
	"PLAYER_QUERY_CHARACTERS",		//	18
	"",								//	19
	"OLD_INN_LOGIN",				//	20
	"PLAYER_LOGIN",					//	21
	"PLAYER_LOGOUT",				//	22
	"PLAYER_PAGE",					//	23
	"PLAYER_SYSTEM_MSG",			//	24
	"PLAYER_ACK",					//	25
	"PLAYER_NAK",					//	26
	"PLAYER_ADD_ROOM",				//	27
	"PLAYER_READ_ROOM",				//	28
	"PLAYER_UPDATE_ROOM",			//	29
	"PLAYER_DELETE_ROOM",			//	30
	"PLAYER_UNLINK_ROOM",			//	31
	"PLAYER_CHANGE_ROOM",			//	32
	"PLAYER_NEW_ROOM",				//	33
	"PLAYER_INVALIDATE_ROOMS",		//	34
	"PLAYER_ENTERED_GAME",			//	35
	"PLAYER_EXITED_GAME",			//	36
	"PLAYER_ROOM_CHAT",				//	37
	"PLAYER_TEXT",					//	38
	"PLAYER_CREATE_OBJ",			//	39
	"PLAYER_OBJ_INFO",				//	40
	"PLAYER_MOVIE",					//	41
	"PLAYER_MOVIE_DATA",			//	42
	"PLAYER_CHAT",					//	43
	"PLAYER_SERVID",				//	44
	"PLAYER_DESTROY_OBJECT",		//	45
	"OLD_SERVER_PING",				//	46
	"CLIENT_PING",					//	47
	"PLAYER_CREATE_CHARACTER",		//	48
	"PLAYER_DESTROY_CHARACTER",		//	49
	"PLAYER_QUERY_CHARACTERS",		//	50
	"PLAYER_REQUEST_SERVID",		//	51
	"PLAYER_SHIFT_ROOM",			//	52
	"PLAYER_CHARACTER_INFO",		//	53
	"PLAYER_OLD_CHARACTER_LOGIN",	//	54
	"PLAYER_GET_BIOGRAPHY",			//	55
	"PLAYER_SET_BIOGRAPHY",			//	56
	"PLAYER_GET_DESCRIPTION",		//	57
	"PLAYER_GET_EXTENDED_PROPS",	//	58
	"PLAYER_SET_HEAD_DATA",			//	59
	"PLAYER_GET_CHAR_INFO",			//	60
	"PLAYER_WHO",					//	61
	"OLD_PLAYER_CHECK_LOGIN",		//	62
	"ATTACH_EFFECT",				//	63
	"REMOVE_EFFECT",				//	64
	"PLAYER_EFFECT_DATA",			//	65
	"SET_PROP",						//	66
	"COMBAT_BEGIN",					//	67
	"COMBAT_MOVE",					//	68
	"COMBAT_EXIT",					//	69
	"COMBAT_FLEE",					//	70
	"GET_SHOP_INFO",				//	71
	"SHOP_BUY",						//	72
	"SHOP_EXAMINE",					//	73
	"SHOP_SELL",					//	74
	"SHOP_GET_PRICE",				//	75
	"MONEY_DROP",					//	76
	"MONEY_PUT",					//	77
	"MONEY_GIVE",					//	78
	"MONEY_TAKE",					//	79
	"CAST_SPELL",					//	80
	"PLAYER_INFO",					//	81
	"GET_BOOK_INFO",				//	82
	"GET_BOOK_PAGE",				//	83
	"FATAL_DATA",					//	84
	"LOCK_OBJ",						//	85
	"UNLOCK_OBJ",					//	86
	"GROUP_JOIN",					//	87
	"GROUP_LEAVE",					//	88
	"GROUP_KICK",					//	89
	"OLD_SECURITY",					//	90
	"SECURITY_REQUEST",				//	91
	"GET_POSN",						//	92
	"CHANGE_PASSWORD",				//	93
	"GET_HOUSE",					//	94
	"GET_ENTRY_INFO",				//	95
	"TALK",							//	96
	"LOGIN_DONE",					//	97
	"LOGIN_UPDATE",					//	98
	"FOREFIT_TURN",					//	99
	"OLD_ROUTE_INFO",				//	100
	"SHOP_RECHARGE",				//	101
	"SHOP_GET_RECHARGE_PRICE",		//	102
	"SET_TITLE",					//	103
	"TREE_GET",						//	104
	"TREE_CHOOSE_TOPIC",			//	105
	"TREE_GET_TEXT",				//	106
	"TREE_BACK",					//	107
	"QUEST_ACCEPT",					//	108
	"QUEST_DECLINE",				//	109
	"GET_QUEST_LIST",				//	110
	"MIX_OBJECT",					//	111
	"COMBAT_ACTION",				//	112
	"CREATE_OBJECT",				//	113
	"WHATS_NEW",					//	114
	"SET_ENGRAVE_NAME",				//	115
	"GET_LOOK_INFO",				//	116
	"SELL_CRYSTALS",				//	117
	"BULK_TAKE",					//	118
	"BULK_DROP",					//	119
	"BULK_PUT",						//	120
	"BULK_GIVE",					//	121
	"REPAIR",						//	122
	"GET_REPAIR_PRICE",				//	123
	"UPDATE_ATTRIBUTES",			//	124
	"SEND_REG",						//	125
	"MASS_SELL",					//	126
	"MASS_BUY",						//	127
	"GET_SELL_PRICES",				//	128
	"CREATE_CHANNEL",				//	129
	"PLAYER_CHARACTER_LOGIN",		//	130
	"GET_REPAIR_PRICES",			//	131
	"MASS_REPAIR",					//	132
	"ROCKING",						//	133
	"",								//	134
	"MAIL_LIST_GET",				//	135
	"MAIL_MESSAGE_GET",				//	136
	"MAIL_MESSAGE_DELETE",			//	137
	"MAIL_MESSAGE_SEND",			//	138
	"MAIL_MESSAGE_ARCHIVE",			//	139
	"MAIL_MESSAGE_COMPLAIN",		//	140
	"",								//	141
	"",								//	142
	"",								//	143
	"",								//	144
	"",								//	145
	"",								//	146
	"",								//	147
	"",								//	148
	"",								//	149
	"VERB_GET",						//	150
	"VERB_DROP",					//	151
	"VERB_PUT_IN",					//	152
	"VERB_PUT_ON",					//	153
	"VERB_TAKE_OFF",				//	154
	"VERB_OPEN",					//	155
	"VERB_CLOSE",					//	156
	"VERB_LOCK",					//	157
	"VERB_UNLOCK",					//	158
	"VERB_ATTACK",					//	159
	"VERB_ENGAGE",					//	160
	"VERB_CONSUME",					//	161
	"VERB_SIT",						//	162
	"VERB_STAND",					//	163
	"VERB_GIVE",					//	164
	"VERB_MEMORIZE",				//	165
	"VERB_ROB",						//	166
	"VERB_USE",						//	167
	"VERB_PUSH",					//	168
	"VERB_DYE",						//	169
	"VERB_COMBINE",					//	170
	"",								//	171
	"",								//	172
	"",								//	173
	"",								//	174
	"",								//	175
	"",								//	176
	"",								//	177
	"",								//	178
	"",								//	179
	"NPC_PULSE",					//	180
	"TIMER_PULSE",					//	181
	"COMBAT_PULSE",					//	182
	"ZONE_RESET_PULSE",				//	183
	"PROCESS_AFFECT_PULSE",			//	184
	"CHAR_DOIT_PULSE",				//	185
	"HEAL_PULSE",					//	186
	"PING_PULSE",					//	187
	"MONSTER_PULSE",				//	188
	"MAINTENANCE_PULSE",			//	189
	"RESET_PULSE",					//	190
	"ZONE_RESET_CMD",				//	191
	"ADD_TO_ROOM",					//	192
	"TOSS_DEAD_HOUSES",				//	193
	"DUNGEON_QUEUE_PULSE",			//	194
	"TOSS_DEAD_MAIL",				//	195
	"AMBUSH_PULSE",					//	196
	"",								//	197
	"",								//	198
	"",								//	199
	"FILEMGR_HELLO",				//	200
	"FILEMGR_GET",					//	201
	"FILEMGR_PUT",					//	202
	"FILEMGR_EXISTS",				//	203
	"FILEMGR_ERASE",				//	204
	"FILEMGR_APPEND",				//	205
	"FILEMGR_EXCLUSIVE_CREATE",		//	206
	"",								//	207
	"",								//	208
	"",								//	209
	"DATAMGR_LOG_PERMANENT",		//	210
	"DATAMGR_LOGIN",				//	211
	"DATAMGR_LOGOUT",				//	212
	"DATAMGR_NEW_CHAR",				//	213
	"DATAMGR_SAVE_CHAR",			//	214
	"DATAMGR_DEL_CHAR",				//	215
	"DATAMGR_SUSPEND",				//	216
	"DATAMGR_DISABLE",				//	217
	"DATAMGR_REVOKE",				//	218
	"DATAMGR_GAG",					//	219
	"DATAMGR_SETPASS",				//	220
	"DATAMGR_NEW_HOUSE",			//	221
	"DATAMGR_WRITE_HOUSE",			//	222
	"DATAMGR_DEL_HOUSE",			//	223
	"DATAMGR_GET_HOUSE",			//	224
	"DATAMGR_HELLO",				//	225
	"DATAMGR_GET_ALL_HOUSES",		//	226
	"DATAMGR_GET_NEXT_HOUSE",		//	227
	"DATAMGR_LOGINMESSAGE",			//	228
	"DATAMGR_DOWNTIMEMESSAGE",		//	229
	"DATAMGR_SET_RIGHTS",			//	230
	"",								//	231
	"",								//	232
	"",								//	233
	"",								//	234
	"",								//	235
	"",								//	236
	"",								//	237
	"",								//	238
	"",								//	239
	"",								//	240
	"",								//	241
	"",								//	242
	"",								//	243
	"",								//	244
	"",								//	245
	"",								//	246
	"",								//	247
	"",								//	248
	"",								//	249
	"",								//	250
	"",								//	251
	"",								//	252
	"",								//	253
	"CLIENT_HACKED_MSG",			//	254
};

// Message class
int			IPCStats::m_nSlowestMsg = -1;
clock_t		IPCStats::m_nSlowestTime = 0;
time_t		IPCStats::m_nStartTime = 0;
IPCStats	IPCStats::Msgs[ _IPC_MAX_MESSAGES ];

IPCStats::IPCStats() {
	m_nExecTime = 0;

	m_nInCount = 0;
	m_nInbound = 0;

	m_nOutCount = 0;
	m_nOutbound = 0;
	m_nAcked = 0;
	m_nNaked = 0;
}

void IPCStats::addExecTime( clock_t nTime, IPCMessage* pMsg ) {
	m_nExecTime += nTime;

	m_nInCount++;
	m_nInbound += pMsg->size();

	if ( nTime > m_nSlowestTime ) {
		m_nSlowestMsg = m_nType;
		m_nSlowestTime = nTime;
	}
}

void IPCStats::addOutbound( int nSize, int* pData ) {
	if ( nSize ) {
		if ( m_nType == _IPC_PLAYER_ACK ) {
			if ( pData[2] < _IPC_MAX_MESSAGES ) {
				Msgs[ pData[2] ].m_nAcked += nSize;
				Msgs[ pData[2] ].m_nOutCount++;
			}
		} else if ( m_nType == _IPC_PLAYER_NAK ) {
			if ( pData[2] < _IPC_MAX_MESSAGES ) {
				Msgs[ pData[2] ].m_nNaked += nSize;
				Msgs[ pData[2] ].m_nOutCount++;
			}
		}

		m_nOutbound += nSize;
		m_nOutCount++;
	}
}

char* IPCStats::displayStats( int nCount ) {
	static char buf[1024];

	buf[0] = 0;

	if ( m_nInCount ) {
		sprintf( buf, "#%d\t%s\t%lld\t%lld\t%f\t%f\t%lld\t%lld\t%lld\t%lld\n", nCount, gMsgNames[ m_nType], m_nInCount, m_nInbound, ( m_nExecTime / 1000000000.0 ), ( ( m_nExecTime / 1000000000.0 ) / m_nInCount ), m_nOutCount, m_nOutbound, m_nAcked, m_nNaked );
	} else if ( m_nOutbound || m_nAcked || m_nNaked ) {
		sprintf( buf, "#%d\t%s\t0\t0\t0\t0\t%lld\t%lld\t%lld\t%lld\n", nCount, gMsgNames[ m_nType], m_nOutCount, m_nOutbound, m_nAcked, m_nNaked );
	}

	return buf;
}

char* IPCStats::display() {
	static char MsgStr[ 512 * _IPC_MAX_MESSAGES ];

	double msgTime = 0;

	unsigned long long   nInCount = 0;
	unsigned long long   nInbound = 0;
	unsigned long long   nOutCount = 0;
	unsigned long long   nOutbound = 0;
	unsigned long long   nAcked = 0;
	unsigned long long   nNaked = 0;

	sprintf( MsgStr, "Msg#\tMessage\tCount\tInbound\tTotal_Time\tAverage_Time\tCount\tOutbound\tAcked\tNaked\n" );

	for (int count=0; count < _IPC_MAX_MESSAGES; count++ ) {
		strcat ( MsgStr, Msgs[ count ].displayStats( count ) );

		msgTime += Msgs[ count ].m_nExecTime;

		nInCount	+= Msgs[ count ].m_nInCount;
		nInbound	+= Msgs[ count ].m_nInbound;
		nOutCount	+= Msgs[ count ].m_nOutCount;
		nOutbound	+= Msgs[ count ].m_nOutbound;
		nAcked		+= Msgs[ count ].m_nAcked;
		nNaked		+= Msgs[ count ].m_nNaked;
	}

	msgTime /= 1000000000.0;

	char buf[2048];

	time_t	endTime = time( NULL );

	sprintf( buf, "\n\tTotal)\t%lld\t%lld\t%f\t%f\t%lld\t%lld\t%lld\t%lld\n\nSlowest Msg #%d took % 8.6f seconds.\nTotal process time = % 8.6f\nTotal run time = % 14d", nInCount, nInbound, msgTime, msgTime / nInCount, nOutCount, nOutbound, nAcked, nNaked, m_nSlowestMsg, ( m_nSlowestTime / 1000000000.0 ), (double) ( (double) clock() / CLOCKS_PER_SEC ), ( endTime - m_nStartTime ) );
	strcat ( MsgStr, buf );

	return (char*) & MsgStr;
}

void IPCStats::init() {
	m_nStartTime = time( NULL );

	for (int i = 0;i < _IPC_MAX_MESSAGES;i++) {
		Msgs[ i ].m_nType = i;
	}
}

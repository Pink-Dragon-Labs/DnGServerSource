//
// datamgr
//
// This file contains the DataMgr class and supporting structures.
//
// author: Stephen Nichols
//

#include "../global/string.hpp"
#include "../global/packdata.hpp"
#include "datamgr.hpp"
#include "../global/fatal.hpp"
#include "../global/logmgr.hpp"
#include "../global/tree.hpp"
#include "../global/file.hpp"

// this is defined in wobject.hpp in the roommgr -- this is ugly, fix it later
#define _ERR_REDUNDANT 10001

// define access types
// Change also in rmplayer.hpp
#define _ACCESS_NORMAL                  0x00000000
#define _ACCESS_IMPLEMENTOR             0x00000001
#define _ACCESS_MODERATOR               0x00000043
#define _ACCESS_GUIDE                   0x00000047
#define _ACCESS_PROPHET                 0x00000009
#define _ACCESS_CS                      0x00000013
#define _ACCESS_EVENT                   0x00000021
#define _ACCESS_PUBLICRELATIONS         0x00000041

#define _ACCESS_PRIVILEGED              0x0000007f
#define _ACCESS_ONLY_EVENT              0x00000020
#define _ACCESS_ONLY_PROPHET            0x00000008

#define _ACCESS_TELEPORT		0x00001013
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

#define _ACCESS_SET_IGNORE_STATS	0x00020000

// this is the binary tree that holds the character names
BinaryTree characterTree;
BinaryTree loginTree;

// this is the global table of character tables
char *gCharacterTables[_MAX_SERVER];

// this is the global table of house tables
char *gHouseTables[_MAX_SERVER];

// this is the global table of house tables
char *gEngraveTables[_MAX_SERVER];

// this is the global table of mailData tables
char *gMailDataTables[_MAX_SERVER];

// this is the global table of mailLink tables
char *gMailLinkTables[_MAX_SERVER];

// this is the global table of friend tables
char *gFriendsTables[_MAX_SERVER];

// this is the global table of foe tables
char *gFoesTables[_MAX_SERVER];

// this is the global last crasher of this server
int  gCrasher[_MAX_SERVER];

SQLResponse *g_qHouseID;
int g_nRecord;

// this is the global datamgr
DataMgr gDataMgr;

DataMgr::DataMgr() {
	sql = NULL;
}

DataMgr::~DataMgr() {
	if ( sql ) {
		delete sql;
		sql = NULL;
	}
}

// this member is called to init the SQL database
void DataMgr::initSQL ( char *server, char *user, char *pass, char *db ) {
	if ( sql ) {
		delete sql;
		sql = NULL;
	}

	sql = new SQLDatabase ( server, user, pass, db );

	if ( sql->error )
		fatal ( "SQL: '%s'", sql->error );
}

// this member is called to handle incoming messages
void DataMgr::handleMessage ( IPCMessage *msg ) {
	IPCClient *client = (IPCClient *)msg->from();

	// determine the character and house table to use for this server ID
	char *charTable = NULL, *houseTable = NULL, *engraveTable = NULL, *mailData = NULL, *mailLink = NULL, *friendsTable = NULL, *foesTable = NULL;

	if ( client->uid() != -1 ) {
		charTable = gCharacterTables[client->uid()];
		houseTable = gHouseTables[client->uid()];
		engraveTable = gEngraveTables[client->uid()];
		mailData = gMailDataTables[client->uid()];
		mailLink = gMailLinkTables[client->uid()];
		friendsTable = gFriendsTables[client->uid()];
		foesTable = gFoesTables[client->uid()];
	}

	switch ( msg->type() ) {
		// handle the hello message from a client
		case _IPC_DATAMGR_HELLO: {
			PackedData packet ( msg->data(), msg->size() );

			// set the serverID value for this client
			int serverID = packet.getWord();
			client->uid() = serverID;

			SQLResponse *info = sql->query ( "select characterTable, houseTable, engraveTable, mailData, mailLink, friendTable, foeTable, crasher from serverList where id = %d", serverID );

			if ( info ) {
				gCharacterTables[serverID] = strdup ( info->table ( 0, 0 ) );
				gHouseTables[serverID] = strdup ( info->table ( 0, 1 ) );
				gEngraveTables[serverID] = strdup ( info->table ( 0, 2 ) );
				gMailDataTables[serverID] = strdup( info->table( 0, 3 ) );
				gMailLinkTables[serverID] = strdup( info->table( 0, 4 ) );
				gFriendsTables[serverID] = strdup( info->table( 0, 5 ) );
				gFoesTables[serverID] = strdup( info->table( 0, 6 ) );
				gCrasher[serverID] = atoi( info->table( 0, 7 ) );

				delete info;
			}

			// update the database to show that this server is up
			sql->query ( "update serverList set isUp='up' where id=%d", serverID );
		}

		break;

		// handle logging in an account 
		case _IPC_DATAMGR_LOGIN: {
			PackedData packet ( msg->data(), msg->size() );
			SQLResponse *characters = NULL;

			// this is the response data
			PackedData response;
			char resultMsg[1024] = "";
			int resultCode = _DATAMGR_OKAY;

			// get data from the packet 
			int id = packet.getLong();
			char *loginName = packet.getString();
			char *password = packet.getString();
			int serverID = packet.getByte();
			int nIP = packet.getLong();
			int nOSVersion = packet.getLong();
			int nCount = packet.getByte();
			char* pID = NULL;
			
			if ( nCount )
				pID = packet.getString();

			// give a default login name and password if none specified
			if ( !loginName )
				loginName = strdup ( "!!!!!!!!!!!!" );

			if ( !password )
				password = strdup ( "!!!!!!!!!!!!" );

			// get the account ID
			SQLResponse *account = sql->query ( "select accounts.id, accounts.status, accounts.billingStatus, accounts.rights+0, date_format(accounts.disciplineExpiration, '%s'), unix_timestamp(accounts.disciplineExpiration), accounts.type, UNIX_TIMESTAMP(accounts.billingDate), accounts.nCredits, accounts.nCoppers, accounts.reason from accounts, loginNames where loginNames.name = '%s' and accounts.loginNameID=loginNames.id and accounts.password = '%s'", "%M %e, %Y %h:%i:%s %p", loginName, password  );

			
			int accountID = -1, actionTimer = 0, rights = 0, nReason = 0;
			char *status = NULL, *billingStatus = NULL, *disciplineExpiration = NULL, *accountType = NULL;
			int billingDate, nCredits, nCoppers;

			if ( account ) {
				// the account exists, extract the information
				accountID = atoi ( account->table ( 0, 0 ) );
				status = account->table ( 0, 1 );
				billingStatus = account->table ( 0, 2 );
				rights = atoi( account->table ( 0, 3 ) );
				disciplineExpiration = account->table ( 0, 4 );
				actionTimer = atoi ( account->table ( 0, 5 ) );
				accountType = account->table ( 0, 6 );
				billingDate = atoi( account->table ( 0, 7 ) );
				nCredits = atoi( account->table ( 0, 8 ) );
				nCoppers = atoi( account->table ( 0, 9 ) );
				nReason = atoi( account->table ( 0, 10 ) );

				char pSerial[21];
				char pModel[41];

				if ( nCount ) {
					for (char nBay = 0;nBay < nCount;nBay++) {
						strncpy( pSerial, &pID [ ( nBay * 60 ) ], 20 );
						strncpy( pModel, &pID [ ( nBay * 60 ) + 20 ], 40 );

						SQLResponse* sqlID = sql->query( "select action+0 from findID where serial like '%s' and model like '%s'", pSerial, pModel );

						if ( sqlID ) {
							if ( atoi ( sqlID->table( 0, 0 ) ) == 2 ) {
								SQLResponse *account = sql->query ( "select loginNameID from accounts where id = %d", accountID );
						
								if ( account ) {
									sql->query ( "insert into accountLog set id=0, loginNameID=%s, characterNameID=-1, date=curdate(), time=curtime(), type='banned', text='IP %s ID = %s', server=%d", account->table( 0, 0 ), csIPtoChar( nIP ), pSerial, client->uid() );

									delete account;
								}

								sprintf ( sizeof ( resultMsg ), resultMsg, "Invalid login name or password.  Please re-enter your login name and password to continue." );
								resultCode = _DATAMGR_ERROR;

								nBay = nCount;
							}

							delete sqlID;
						} else {
							// insert this client's id into the database
							sql->query( "insert into findID set accountID = %d, serial = '%s', model = '%s', bay = %d", accountID, pSerial, pModel, nBay );
						}
					}
//				} else {
//					SQLResponse* sqlID = sql->query( "select action+0 from findID where accountID = %d and serial = '_UNKNOWN____UNKNOWN_' and model = '_UNKNOWN___UNKNOWN____UNKNOWN___UNKNOWN_'", accountID );
//
//					if ( sqlID ) {
//						delete sqlID;
//					} else {
//						sql->query( "insert into findID set accountID = %d, serial = '_UNKNOWN____UNKNOWN_', model = '_UNKNOWN___UNKNOWN____UNKNOWN___UNKNOWN_', bay = 0", accountID );
//					}
				}

				// **** SPECIAL VALUE LINE HACK ****
				// if this is a inactive account, make it active
				if ( !strcmp ( accountType, "inactive" ) ) {
					sql->query ( "update accounts set type='yearly', billingDate=date_add(now(), interval 90 day) where id=%d", accountID );
					accountType = "yearly";
				}

				// check the rights of the account
				if ( !strcmp ( billingStatus, "unpaid" ) ) {
					sprintf ( sizeof ( resultMsg ), resultMsg, "This account can not be played until it is paid for.  Please visit www.realmserver.com to pay for this account." );
					resultCode = _DATAMGR_ERROR;
				} 

				// handle a revoked account
				else if ( !strcmp ( status, "revoked" ) ) {
					if ( getseconds() >= actionTimer ) {
						// remove the revoke
						sql->query ( "update accounts set status='normal' where id=%d", accountID );
					} else {
						// add the revoke to the rights
						rights |= _ACCESS_NO_GOSSIP;
					}
				}

				// handle a gagged account
				else if ( !strcmp ( status, "gagged" ) ) {
					if ( getseconds() >= actionTimer ) {
						// remove the revoke
						sql->query ( "update accounts set status='normal' where id=%d", accountID );
					} else {
						// add the revoke to the rights
						rights |= _ACCESS_GAGGED;
					}
				}

				// handle a suspended account
				else if ( !strcmp ( status, "suspended" ) ) {
					// check to see if the expiration time has expired
					if ( getseconds() < actionTimer ) {
						SQLResponse* reason = sql->query( "select text from reasons where id = %d", nReason );
						if ( reason ) {	
							sprintf ( sizeof ( resultMsg ), resultMsg, "This account is suspended until %s.\n\n%s", disciplineExpiration, reason->table( 0, 0 ) );

							delete reason;
						} else {
							sprintf ( sizeof ( resultMsg ), resultMsg, "This account is currently suspended and can not be played until %s.", disciplineExpiration );
						}
						resultCode = _DATAMGR_ERROR;
					} else {
						// remove the suspension
						sql->query ( "update accounts set status='normal' where id=%d", accountID );
					}
				}

				// handle a disabled account
				else if ( !strcmp ( status, "disabled" ) ) {
					SQLResponse* reason = sql->query( "select text from reasons where id = %d", nReason );
					if ( reason ) {
						sprintf ( sizeof ( resultMsg ), resultMsg, "This account is currently disabled.\n\n%s", reason->table( 0, 0 ) );
						delete reason;
					} else {
						sprintf ( sizeof ( resultMsg ), resultMsg, "This account is currently disabled." );
					}																
					resultCode = _DATAMGR_ERROR;
				}

                                // handle a disabled account
                                else if ( !strcmp ( status, "purged" ) ) {
                                        sprintf ( sizeof ( resultMsg ), resultMsg, "The subscription to this account has gone unpaid for more than 90 days and the account's contents have been purged. If you would like information on how to reactivate the account, including its characters, please contact our Customer Service Department through our website, www.realmserver.com." );
                                        resultCode = _DATAMGR_ERROR;
                                }

				// check for a downtime message
				if ( resultCode == _DATAMGR_OKAY ) {
					SQLResponse *downtime = sql->query ( "select message from messages where serverID = %d and type = 'downtime'", client->uid() );

					if ( downtime ) { 
						if ( (!( rights & _ACCESS_IMPLEMENTOR ) ) && (!( rights & _ACCESS_PUBLICRELATIONS )) ) {
							sprintf ( sizeof ( resultMsg ), resultMsg, "%s", downtime->table ( 0, 0 ) );
							resultCode = _DATAMGR_ERROR;
						} else {
							SQLResponse* sqlOS = sql->query( "select accountID from osVersion where accountID = %d and major = %d and minor = %d and type = %d", accountID, ( ( nOSVersion >> 24 ) & 0x0000000f ), ( ( nOSVersion >> 16 ) & 0x000000ff ), ( ( nOSVersion >> 28 ) & 0x0000000f ) );

							if ( sqlOS ) {
								// update this client's os version build# in the database
								sql->query( "update osVersion set build = %d where accountID = %d, major = %d, minor = %d, type = %d", ( nOSVersion & 0x0000ffff ), accountID, ( ( nOSVersion >> 24 ) & 0x0000000f ), ( ( nOSVersion >> 16 ) & 0x000000ff ), ( ( nOSVersion >> 28 ) & 0x0000000f ) );

								delete sqlOS;
							} else {
								// insert this client's os version into the database
								sql->query( "insert into osVersion set accountID = %d, major = %d, minor = %d, build = %d, type = %d", accountID, ( ( nOSVersion >> 24 ) & 0x0000000f ), ( ( nOSVersion >> 16 ) & 0x000000ff ), ( nOSVersion & 0x0000ffff ), ( ( nOSVersion >> 28 ) & 0x0000000f ) );
							}

							// update the serverList so that the number of users is known
							sql->query ( "update serverList set userCount=userCount+1 where id=%d", serverID );

							characters = sql->query ( "select characterNames.name, %s.data, %s.id, characterNameID from characterNames, %s where %s.accountID = %d and characterNames.id = %s.characterNameID order by characterNames.name", charTable, charTable, charTable, charTable, accountID, charTable );
						}

						delete downtime;
					} else {
						// check to see if the account is already logged in
						sql->query ( "insert into accountLocks set accountID=%d, server=%d", accountID, serverID );

						if ( sql->error ) {
							// account is already logged in
							sprintf ( sizeof ( resultMsg ), resultMsg, "This account is currently in use.  You are only allowed to use the account on one machine at a time." );
							resultCode = _DATAMGR_ERROR;
						} else {
							SQLResponse* sqlOS = sql->query( "select accountID from osVersion where accountID = %d and major = %d and minor = %d and type = %d", accountID, ( ( nOSVersion >> 24 ) & 0x0000000f ), ( ( nOSVersion >> 16 ) & 0x000000ff ), ( ( nOSVersion >> 28 ) & 0x0000000f ) );

							if ( sqlOS ) {
								// update this client's os version build# in the database
								sql->query( "update osVersion set build = %d where accountID = %d, major = %d, minor = %d, type = %d", ( nOSVersion & 0x0000ffff ), accountID, ( ( nOSVersion >> 24 ) & 0x0000000f ), ( ( nOSVersion >> 16 ) & 0x000000ff ), ( ( nOSVersion >> 28 ) & 0x0000000f ) );

								delete sqlOS;
							} else {
								// insert this client's os version into the database
								sql->query( "insert into osVersion set accountID = %d, major = %d, minor = %d, build = %d, type = %d", accountID, ( ( nOSVersion >> 24 ) & 0x0000000f ), ( ( nOSVersion >> 16 ) & 0x000000ff ), ( nOSVersion & 0x0000ffff ), ( ( nOSVersion >> 28 ) & 0x0000000f ) );
							}

							// update the serverList so that the number of users is known
							sql->query ( "update serverList set userCount=userCount+1 where id=%d", serverID );

							characters = sql->query ( "select characterNames.name, %s.data, %s.id, characterNameID from characterNames, %s where %s.accountID = %d and characterNames.id = %s.characterNameID order by characterNames.name", charTable, charTable, charTable, charTable, accountID, charTable );
						}
					}
				}
			} else {
				SQLResponse *account = sql->query ( "select accounts.rights+0, loginNames.id from accounts, loginNames where loginNames.name = '%s' and accounts.loginNameID=loginNames.id", loginName );

				if ( account ) {
					int nRights = atoi( account->table( 0, 0 ) );

					if ( nRights & 0x0000001f ) {
						SQLResponse* result = NULL;
						
						// pass this on to the SQL database for logging of a password guess.
						if ( nCount )
							result = sql->query ( "insert into accountLog set id=0, loginNameID=%s, characterNameID=-1, date=curdate(), time=curtime(), type='pwd-guess', text='IP %s ID = %s', server=%d", account->table( 0, 1 ), csIPtoChar( nIP ), pID, client->uid() );
						else
							result = sql->query ( "insert into accountLog set id=0, loginNameID=%s, characterNameID=-1, date=curdate(), time=curtime(), type='pwd-guess', text='IP %s', server=%d", account->table( 0, 1 ), csIPtoChar( nIP ), client->uid() );

						if ( result )
							delete result;
					}
				}

				// there is no account by this name and password
				sprintf ( sizeof ( resultMsg ), resultMsg, "Invalid login name or password.  Please re-enter your login name and password to continue." );
				resultCode = _DATAMGR_ERROR;
			}

			// build the response packet
			response.putLong ( id );
			response.putByte ( resultCode );

			if ( resultCode == _DATAMGR_ERROR ) {
				// put the error message
				response.putString ( resultMsg );
			} else {
				// insert a login record into the access log
				sql->query( "insert into access set serverID=%d, accountID=%d, login=UNIX_TIMESTAMP( now() ), logout=0, IP=%d", serverID, accountID, nIP );
                logDisplay("Account ID %d", accountID);
				// put the rights string first
				response.putLong ( accountID );
				response.putLong ( rights );
				response.putLong ( actionTimer );
				response.putString ( accountType );
				response.putLong ( billingDate );
				response.putLong ( nCredits );
				response.putLong ( nCoppers );

				SQLResponse* pLoginMessage = sql->query ( "select message from messages where serverID = %d and type = 'login'", serverID );

				if ( pLoginMessage ) {
					response.putString( pLoginMessage->table( 0, 0 ) );

					delete pLoginMessage;
				} else {
					response.putString( "" );
				}

				// put the characters in the response packet
				if ( characters ) {
					// put the number of characters
					response.putByte ( characters->rows );

					for ( int c=0; c<characters->rows; c++ ) {
						response.putString ( characters->table ( c, 0 ) );
						response.putLong ( atoi ( characters->table ( c, 2 ) ) );
						response.putLong ( atoi ( characters->table ( c, 3 ) ) );
						response.putLong ( characters->length ( c, 1 ) );
						response.putArray ( characters->table ( c, 1 ), characters->length ( c, 1 ) );
					}

					// toss the character data
					delete characters;
				} else {
					// put zero character count
					response.putByte ( 0 );
				}
			}

			// toss the account response
			if ( account )
				delete account;

			// send the response packet on to the client
			send ( _IPC_DATAMGR_LOGIN, &response, client );

			if ( pID )
				free ( pID );

			free ( loginName );
			free ( password );
		}

		break;

		// handle getting configuration information for the selected character to play
		case _IPC_DATAMGR_CONFIG_INFO: {
			PackedData packet ( msg->data(), msg->size() );

			char nSelection = packet.getByte();

			switch ( nSelection ) {
				case _DATAMGR_CONFIG_INFO_GET: {
					int nServID = packet.getLong();
                		        int nCharacterID = packet.getLong();

					PackedData response;

					response.putByte( _DATAMGR_CONFIG_INFO_GET );
					response.putLong( nServID );

					SQLResponse* qFriends = sql->query ( "select name from characterNames, %s where %s.characterNameID = %d and characterNames.id = %s.FriendNameID order by characterNames.name", friendsTable, friendsTable, nCharacterID, friendsTable );

					if ( qFriends ) {
						response.putWord( qFriends->rows );

						for (int nRecord = 0;nRecord < qFriends->rows;nRecord++) {
							response.putString( qFriends->table( nRecord, 0 ) );
						}

						delete qFriends;
					} else {
						response.putWord( 0 );
					}

                		        SQLResponse* qFoes = sql->query ( "select name from characterNames, %s where %s.characterNameID = %d and characterNames.id = %s.FoeNameID order by characterNames.name", foesTable, foesTable, nCharacterID, foesTable );

		                        if ( qFoes ) {
                		                response.putWord( qFoes->rows );

                                		for (int nRecord = 0;nRecord < qFoes->rows;nRecord++) {
		                                        response.putString( qFoes->table( nRecord, 0 ) );
                		                }

                                		delete qFoes;
		                        } else {
                		                response.putWord( 0 );
		                        }

					SQLResponse* qConfig = sql->query( "select spells, quests, crimes from %s where characterNameID = %d", charTable, nCharacterID );

					if ( qConfig ) {
						// Put the spells list
						response.putLong ( qConfig->length( 0, 0 ) );

						if ( qConfig->length( 0, 0 ) )
							response.putArray ( qConfig->table( 0, 0 ), qConfig->length( 0, 0 ) );

		                                // Put the quest list
                		                response.putLong ( qConfig->length( 0, 1 ) );

		                                if ( qConfig->length( 0, 1 ) )
                		                        response.putArray ( qConfig->table( 0, 1 ), qConfig->length( 0, 1 ) );

                                		// Put the crime
		                                response.putLong ( qConfig->length( 0, 2 ) );

                		                if ( qConfig->length( 0, 2 ) )
                                		        response.putArray ( qConfig->table( 0, 2 ), qConfig->length( 0, 2 ) );

						delete qConfig;
					} else {
						response.putLong( 0 );
						response.putLong( 0 );
						response.putLong( 0 );
					}

                		        // send the response packet on to the client
		                        send ( _IPC_DATAMGR_CONFIG_INFO, &response, client );
				}

				break;

                		// handle saving the quests data
                                case _DATAMGR_CONFIG_INFO_WRITE_QUESTS: {
		                        int nCharacterID = packet.getLong();

                		        int nSize = packet.getLong();

		                        char* pData = (char*) malloc( nSize );

                		        packet.getArray( pData, nSize );

		                        SQLResponse* qWriteData = sql->query( "update %s set quests=\"%b\" where characterNameID=%d", charTable, nSize, pData, nCharacterID );

                		        free( pData );
                                }

                                break;

				// handle saving the crimes data
                                case _DATAMGR_CONFIG_INFO_WRITE_CRIMES: {
		                        int nCharacterID = packet.getLong();

		                        int nSize = packet.getLong();

                		        char* pData = (char*) malloc( nSize );

		                        packet.getArray( pData, nSize );

                		        SQLResponse* qWriteData = sql->query( "update %s set crimes=\"%b\" where characterNameID=%d", charTable, nSize, pData, nCharacterID );

		                        free( pData );
			        }

                                break;

                                case _DATAMGR_CONFIG_INFO_ADD_FRIEND: {
					int nServID = packet.getLong();
					int nCharacterID = packet.getLong();
		                        char* pName = packet.getString();

		                        PackedData response;

					response.putByte( _DATAMGR_CONFIG_INFO_ADD_FRIEND );
		                        response.putLong( nServID );
					response.putString( pName );

					SQLResponse* qCharacterName = sql->query( "select id from characterNames where name = '%s'", pName );

					if ( qCharacterName ) {
						int nTargetID = atoi( qCharacterName->table( 0, 0 ) );

						delete qCharacterName;

						SQLResponse* qCharacter = sql->query( "select id from %s where characterNameID = %d", charTable, nTargetID );

						if ( qCharacter ) {
							delete qCharacter;

							sql->query( "insert into %s set characterNameID = %d, FriendNameID = %d", friendsTable, nCharacterID, nTargetID );

							response.putByte( 1 );
						} else {
	                                                response.putByte( 0 );
						}
					} else {
						response.putByte( 0 );
					}

					free( pName );

					send ( _IPC_DATAMGR_CONFIG_INFO, &response, client );
                                }

                                break;

                                case _DATAMGR_CONFIG_INFO_DEL_FRIEND: {
                                        int nCharacterID = packet.getLong();
                                        char* pName = packet.getString();

                                        SQLResponse* qCharacterName = sql->query( "select id from characterNames where name = '%s'", pName );

                                        if ( qCharacterName ) {
                                                int nTargetID = atoi( qCharacterName->table( 0, 0 ) );

                                                delete qCharacterName;

                                                sql->query( "delete from %s where characterNameID = %d and FriendNameID = %d", friendsTable, nCharacterID, nTargetID );
						if ( sql->error )
							logDisplay( "SQL ERROR: %s", sql->error );
                                        }

                                        free( pName );
                                }

				break;

                                case _DATAMGR_CONFIG_INFO_ADD_FOE: {

				   	int nServID = packet.getLong();
				   	int nCharacterID = packet.getLong();             
				   	char* pName = packet.getString();              
				   	PackedData response;              
				   	response.putByte( _DATAMGR_CONFIG_INFO_ADD_FOE );             
				   	response.putLong( nServID );             
				   	response.putString( pName );              
				   	SQLResponse* qCharacterName = sql->query( "select id from characterNames where name = '%s'", pName );
				   	if ( qCharacterName ) {
					   	int nTargetID = atoi( qCharacterName->table( 0, 0 ) );
					   	delete qCharacterName;
					   	//Don't allow setting yourself as a foe
					   	if ( nCharacterID != nTargetID ) {
						   	SQLResponse* qRights = sql->query( "select rights+0 from %s, accounts where characterNameID = %d and %s.accountID = accounts.id", charTable, nTargetID, charTable);
						   	//Ensure implementors and moderators cannot be ignored
							if ( qRights ) {
								int nRights = ( atoi( qRights->table( 0, 0 ) ) & _ACCESS_PRIVILEGED );
								if ( nRights ) {
									response.putByte( 0 );
								} else {
									SQLResponse* qCharacter = sql->query( "select id from %s where characterNameID = %d", charTable, nTargetID );
									if ( qCharacter ) {
										sql->query( "insert into %s set characterNameID = %d, FoeNameID = %d", foesTable, nCharacterID, nTargetID );
										response.putByte( 1 );
									} else {
										response.putByte( 0 );
									}
								}
							} else {
								response.putByte( 0 );
							}
						}
				   	} else {
						response.putByte( 0 );
				   	}
				   	free( pName );
				   	send ( _IPC_DATAMGR_CONFIG_INFO, &response, client );
				}

                                break;



                                case _DATAMGR_CONFIG_INFO_CHECK_FOE: {
                                        int nServID = packet.getLong();
                                        int nCharacterID = packet.getLong();
                                        char* pName = packet.getString();

                                        PackedData response;

					response.putByte( _DATAMGR_CONFIG_INFO_CHECK_FOE );
                                        response.putLong( nServID );
                                        response.putString( pName );

                                        SQLResponse* qCharacterName = sql->query( "select id from characterNames where name = '%s'", pName );

                                        if ( qCharacterName ) {
                                                int nTargetID = atoi( qCharacterName->table( 0, 0 ) );

                                                delete qCharacterName;

                                                SQLResponse* qCharacter = sql->query( "select rights+0 from %s, accounts where characterNameID = %d and %s.accountID = accounts.id", charTable, nTargetID, charTable);

                                                if ( qCharacter && ( nCharacterID != nTargetID ) ) {
							int nRights = ( atoi( qCharacter->table( 0, 0 ) ) & _ACCESS_PRIVILEGED );
                                                        delete qCharacter;

							if ( nRights ) {
                                                        	response.putByte( 0 );
							}else{
								response.putByte( 1 );
							}
                                                } else {
                                                        response.putByte( 0 );
                                                }
                                        } else {
                                                response.putByte( 0 );
                                        }

                                        free( pName );

					send ( _IPC_DATAMGR_CONFIG_INFO, &response, client );
                                }

                                break;

                                case _DATAMGR_CONFIG_INFO_DEL_FOE: {
                                        int nCharacterID = packet.getLong();
                                        char* pName = packet.getString();

                                        SQLResponse* qCharacterName = sql->query( "select id from characterNames where name = '%s'", pName );

                                        if ( qCharacterName ) {
                                                int nTargetID = atoi( qCharacterName->table( 0, 0 ) );

                                                delete qCharacterName;

                                                sql->query( "delete from %s where characterNameID = %d and FoeNameID = %d", foesTable, nCharacterID, nTargetID );
                                        }

                                        free( pName );
                                }

                                break;

                                case _DATAMGR_CONFIG_INFO_WRITE_SPELLS: {
                                        int nCharacterID = packet.getLong();
					int nSize = packet.getLong();

					char* pData = (char*) malloc( nSize );
					packet.getArray( pData, nSize );

                                        sql->query( "update %s set spells = '%b' where characterNameID = %d", charTable, nSize, pData, nCharacterID );
                                }

                                break;
			}
		}

		break;

                // handle adding a bounty to this player
                case _IPC_DATAMGR_PLACE_BOUNTY: {
                        PackedData packet ( msg->data(), msg->size() );

                        int nServID = packet.getLong();
			char* pName = packet.getString();
			long nBounty = packet.getLong();

			PackedData response;

			response.putLong( nServID );

			SQLResponse* qCharacterName = sql->query( "select id from characterNames where name = '%s'", pName );

			if ( qCharacterName ) {
				SQLResponse* qCharacters = sql->query( "select crimes from %s where characterNameID = %s", charTable, qCharacterName->table( 0, 0 ) );

				if ( qCharacters ) {
					CrimeData pData;

					if ( qCharacters->length( 0, 0 ) ) {
						memcpy( &pData, qCharacters->table( 0, 0 ), sizeof( CrimeData ) );
					} else {
						memset( &pData, 0, sizeof( CrimeData ) );
					}

					pData.bountyOnHead += nBounty;

					sql->query( "update %s set crimes=\"%b\" where characterNameID = %s", charTable, sizeof( CrimeData ), &pData, qCharacterName->table( 0, 0 ) );

                		        response.putLong( pData.bountyOnHead );
		                        response.putString( pName );
					response.putByte( 1 );
					delete qCharacters;
				} else {
		                        response.putLong( nBounty );
                		        response.putString( pName );
        	                        response.putByte( 0 );
				}

				delete qCharacterName;
			} else {
        	                response.putLong( nBounty );
	                        response.putString( pName );
				response.putByte( 0 );
			}

			send ( _IPC_DATAMGR_PLACE_BOUNTY, &response, client );

			free( pName );
                }

                break;

		// handle adding data to the permanent log
		case _IPC_DATAMGR_LOG_ENGRAVE: {
			PackedData packet ( msg->data(), msg->size() );
			SQLResponse* result = NULL;

			// get the login name
			char* loginName = packet.getString();
			strlower ( loginName );

			// get the character name
			char *charName = packet.getString();
			strlower ( charName );

			// get the character name
			char* pName = packet.getString();

			// get the loginID from the login name
			int loginID = -1;

			TreeNode *node = loginTree.find ( loginName );

			if ( node ) {
				loginID = (int)node->data;	
			} else {
				result = sql->query ( "insert into loginNames set id=0, name='%s'", loginName );

				if ( result ) {
					loginID = sql->lastInsertID();
					delete result;
				} else {
					result = sql->query ( "select id from loginNames where name = '%s'", loginName );

					loginID = atoi ( result->table ( 0, 0 ) );
					delete result;
				}

				loginTree.add ( loginName, (void *)loginID );
			}

			// get the charID from the character name
			int charID = -1;

			node = characterTree.find ( charName );

			if ( node ) {
				charID = (int)node->data;	
			} else {
				result = sql->query ( "insert into characterNames set id=0, name='%s'", charName );

				if ( result ) {
					charID = sql->lastInsertID();
					delete result;
				} else {
					result = sql->query ( "select id from characterNames where name = '%s'", charName );

					charID = atoi ( result->table ( 0, 0 ) );
					delete result;
				}

				characterTree.add ( charName, (void *)charID );
			}

			// pass this on to the SQL database
			result = sql->query ( "insert into %s set loginNameID=%d, characterNameID=%d, engraved='%s'", engraveTable, loginID, charID, pName );

			if ( result )
				delete result;

			if ( loginName )
				free( loginName );

			if ( charName )
				free( charName );

			if ( pName )
				free( pName );

			}

			break;

		// handle adding data to the permanent log
		case _IPC_DATAMGR_LOG_PERMANENT: {
			PackedData packet ( msg->data(), msg->size() );
			SQLResponse *result = NULL;

			// get the login name
			char *loginName = packet.getString();
			strlower ( loginName );

			// get the character name
			char *charName = packet.getString();
			strlower ( charName );

			// get the type of log message
			int type = packet.getByte();

			// get the text of the log message
			char *text = packet.getString();

			// set some defaults, if necessary
			if ( !loginName )
				loginName = strdup ( "!!!!!!!!!!!!" );

			if ( !charName )
				charName = strdup ( "!!!!!!!!!!!!" );

			if ( !text )
				text = strdup ( "" );

			// get the loginID from the login name
			int loginID = -1;

			TreeNode *node = loginTree.find ( loginName );

			if ( node ) {
				loginID = (int)node->data;	
			} else {
				result = sql->query ( "insert into loginNames set id=0, name='%s'", loginName );

				if ( result ) {
					loginID = sql->lastInsertID();
					delete result;
				} else {
					result = sql->query ( "select id from loginNames where name = '%s'", loginName );

					loginID = atoi ( result->table ( 0, 0 ) );
					delete result;
				}

				loginTree.add ( loginName, (void *)loginID );
			}

			// get the charID from the character name
			int charID = -1;

			node = characterTree.find ( charName );

			if ( node ) {
				charID = (int)node->data;	
			} else {
				result = sql->query ( "insert into characterNames set id=0, name='%s'", charName );

				if ( result ) {
					charID = sql->lastInsertID();
					delete result;
				} else {
					result = sql->query ( "select id from characterNames where name = '%s'", charName );

					charID = atoi ( result->table ( 0, 0 ) );
					delete result;
				}

				characterTree.add ( charName, (void *)charID );
			}

			// pass this on to the SQL database
			result = sql->query ( "insert into accountLog set id=0, loginNameID=%d, characterNameID=%d, date=curdate(), time=curtime(), type=%d, text='%s', server=%d", loginID, charID, type, text, client->uid() );

			delete result;

			// toss the strings
			free ( text );
			free ( loginName );
			free ( charName );
		}

		break;

		// this message removes the account lock for an account
		case _IPC_DATAMGR_LOGOUT: {
			PackedData packet ( msg->data(), msg->size() );
			int accountID = packet.getLong();

			sql->query ( "delete from accountLocks where accountID = %d and server = %d", accountID, client->uid() );

			// insert a logout record into the access log
			sql->query( "update access set logout=UNIX_TIMESTAMP( now() ) where serverID=%d and logout=0 and accountID=%d", client->uid(), accountID );

			// update the serverList so that the number of users is known
			sql->query ( "update serverList set userCount=userCount-1 where id=%d", client->uid() );
		}

		break;

		// this message revokes the gossip rights of an account
		case _IPC_DATAMGR_REVOKE: {
			PackedData packet ( msg->data(), msg->size() );
			int accountID = packet.getLong();
			int state = packet.getByte();

			if ( state )
				sql->query ( "update accounts set status='revoked', disciplineExpiration=date_add(now(), interval 1 hour) where id=%d", accountID );
			else
				sql->query ( "update accounts set status='normal' where id = %d", accountID );
		}

		break;

		// this message gags an account 
		case _IPC_DATAMGR_GAG: {
			PackedData packet ( msg->data(), msg->size() );
			int accountID = packet.getLong();
			int state = packet.getByte();

			if ( state )
				sql->query ( "update accounts set status='gagged', disciplineExpiration=date_add(now(), interval 12 hour) where id=%d", accountID );
			else
				sql->query ( "update accounts set status='normal' where id = %d", accountID );
		}

		break;

		// this message suspends an account 
		case _IPC_DATAMGR_SUSPEND: {
			PackedData packet ( msg->data(), msg->size() );
			int accountID = packet.getLong();
			int nReason = packet.getLong();

			sql->query ( "update accounts set status='suspended', reason=%d, disciplineExpiration=date_add(now(), interval 14 day) where id=%d", nReason, accountID );
			
			if ( sql->error )
				logDisplay ( "SQL Error: '%s'", sql->error );
		}

		break;

		// this message disables an account 
		case _IPC_DATAMGR_DISABLE: {
			PackedData packet ( msg->data(), msg->size() );
			int accountID = packet.getLong();
			int nReason = packet.getLong();

			sql->query ( "update accounts set status='disabled', reason=%d where id=%d", nReason, accountID );

			if ( sql->error )
				logDisplay ( "SQL Error: '%s'", sql->error );
		}

		break;

		// this message sets the password on an account 
		case _IPC_DATAMGR_SETPASS: {
			PackedData packet ( msg->data(), msg->size() );
			int accountID = packet.getLong();
			char *password = packet.getString();

			if ( !password )
				password = strdup ( "" );

			sql->query ( "update accounts set password='%s' where id=%d", password, accountID );

			free ( password );
		}

		break;

		// this message updates a character already in the database
		case _IPC_DATAMGR_SAVE_CHAR: {
			PackedData packet ( msg->data(), msg->size() );
			int characterID = packet.getLong();
			int accountID = packet.getLong();

			int size = packet.getLong();
			char *data = (char *)malloc ( size );
			packet.getArray ( data, size );

			sql->query ( "update %s set data='%b' where id = %d and accountID = %d", charTable, size, data, characterID, accountID );

			free ( data );
		}

		break;

				// this message handles creating a new character
		case _IPC_DATAMGR_WHATS_NEW: {
			PackedData packet ( msg->data(), msg->size() );
			int servID = packet.getLong();
			int serverID = client->uid();

			SQLResponse *qWhatsNew = sql->query ( "select posted, message from whatsnew where serverID = %d ORDER BY posted DESC", serverID );

			if ( qWhatsNew ) {
				// build the response packet
				PackedData response;
				response.putLong( servID );
				response.putWord( qWhatsNew->rows );

				for (int i = 0;i < qWhatsNew->rows;i++) {
					char sTime[10240];

					sprintf( sizeof( sTime ), sTime, "\n|c86|* * * * * * * * * * * * %s * * * * * * * * * * * *|c43|\n\n%s", qWhatsNew->table( i, 0 ), qWhatsNew->table( i, 1 ) );
					response.putString( sTime );
				}

				// send the response back
				send ( _IPC_DATAMGR_WHATS_NEW, &response, client );

				delete qWhatsNew;
			} else {
				// build the response packet
				PackedData response;
				response.putLong( servID );
				response.putString( "Sorry there is not what's new information at this time!" );

				// send the response back
				send ( _IPC_DATAMGR_WHATS_NEW, &response, client );
			}
		}

		break;

		// this message handles creating a new character
		case _IPC_DATAMGR_NEW_CHAR: {
			PackedData packet ( msg->data(), msg->size() );
			int servID = packet.getLong();
			char *name = packet.getString();
			int accountID = packet.getLong();
			int charNameID = -1, resultCode = -1, sqlID = -1;

			// freak out if there is no name for this character
			if ( !name )
				break;

			// make sure this name is in the database...
			sql->query ( "insert into characterNames set name='%s'", name );

			if ( sql->error ) {
				SQLResponse *id = sql->query ( "select id from characterNames where name = '%s'", name );

				if ( id ) {
					charNameID = atoi ( id->table ( 0, 0 ) );
					delete id;
				}
			} else {
				charNameID = sql->lastInsertID();
			}

			// see if the name is in the reserved name list
			SQLResponse *reserved = sql->query ( "select id from reservedCharacterNames where characterNameID = %d", charNameID );

			if ( reserved ) {
				resultCode = _ERR_REDUNDANT;
				delete reserved;
			} else {
				// see if the name is attached to an account already
				SQLResponse *owner = sql->query ( "select id from %s where characterNameID = %d", charTable, charNameID );

				if ( owner ) {
					resultCode = _ERR_REDUNDANT;
					delete owner;
				} else {
					// get the character data from the packet
					int size = packet.getLong();

					char *data = (char *)malloc ( size );
					packet.getArray ( data, size );

					// put it in the database
					sql->query ( "insert into %s set id=0, accountID=%d, characterNameID=%d, data='%b'", charTable, accountID, charNameID, size, data );

					sqlID = sql->lastInsertID();

					// Update the bExists
					sql->query( "update %s set bExists = 1 where accountID=%d and characterID=%d", mailData, accountID, charNameID );
					sql->query( "update %s set bExists = 1 where accountID=%d and characterID=%d", mailLink, accountID, charNameID );

					free ( data );
				}
			}

			free ( name );

			// send the response back...
			PackedData response;
			response.putLong ( servID );
			response.putLong ( resultCode );
			response.putLong ( sqlID );
			response.putLong ( charNameID );

			send ( _IPC_DATAMGR_NEW_CHAR, &response, client );
		}

		break;

		// this message is sent to delete a character
		case _IPC_DATAMGR_DEL_CHAR: {
			PackedData packet ( msg->data(), msg->size() );
			int charID = packet.getLong();
			int accountID = packet.getLong();
			int charNameID = 0;

			// get the character's name id
			SQLResponse *nameID = sql->query ( "select characterNameID from %s where id=%d", charTable, charID );

			if ( nameID ) {
				charNameID = atoi ( nameID->table ( 0, 0 ) );

				// toss the house first
				sql->query ( "delete from %s where characterNameID=%d and accountID=%d", houseTable, charNameID, accountID ); 

				delete nameID;
			}

			sql->query ( "delete from %s where id=%d and accountID=%d", charTable, charID, accountID );

			if ( sql->error )
				logDisplay ( "SQL Error: '%s'", sql->error );

			// Update the bExists
			sql->query( "update %s set bExists = 0 where accountID=%d and characterID=%d", mailData, accountID, charNameID );
			sql->query( "update %s set bExists = 0, bDeleted = 1 where accountID=%d and characterID=%d", mailLink, accountID, charNameID );
	
			// Clean up the friends list for this toon
			sql->query( "delete from %s where FriendNameID = %d or CharacterNameID = %d", friendsTable, charNameID, charNameID );

			// Clean up the foe list for this toon
			sql->query( "delete from %s where FriendNameID = %d or CharacterNameID = %d", foesTable, charNameID, charNameID );
		}

		break;

		// handle a load house request
		case _IPC_DATAMGR_GET_HOUSE: {
		
			PackedData packet ( msg->data(), msg->size() );
			int context = packet.getLong();
			int callback = packet.getLong();
			char *name = packet.getString();

			// build the response packet
			PackedData response;
			response.putLong ( context );
			response.putLong ( callback );

			// load the house
			SQLResponse *house = sql->query ( "select %s.id, %s.accountID, %s.data from %s, accounts, characterNames where characterNames.name = '%s' and %s.characterNameID = characterNames.id and %s.accountID = accounts.id and accounts.status != 'disabled' and accounts.status != 'suspended' and accounts.billingStatus != 'unpaid'", houseTable, houseTable, houseTable, houseTable, name? name : "", houseTable, houseTable );

			if ( sql->error )
				logDisplay ( "SQL Error: '%s'", sql->error );

			if ( house ) {
				response.putByte ( _DATAMGR_OKAY );
				// put the special IDs for the house
				response.putLong ( atoi ( house->table ( 0, 0 ) ) );
				response.putLong ( atoi ( house->table ( 0, 1 ) ) );
				response.putString ( name );

				int size = house->length ( 0, 2 );
				response.putLong ( size );
				response.putArray ( house->table ( 0, 2 ), size );

				delete house;
			} else {
				
				response.putByte ( _DATAMGR_ERROR );
			}

			free ( name );

			// send the response back
			send ( _IPC_DATAMGR_GET_HOUSE, &response, client );
		}

		break;

		// handle a house update request
		case _IPC_DATAMGR_WRITE_HOUSE: {
			PackedData packet ( msg->data(), msg->size() );
			int id = packet.getLong();
			int accountID = packet.getLong();

			int size = packet.getLong();
			char *data = (char *)malloc ( size );
			packet.getArray ( data, size );

			// update the house...
			sql->query ( "update %s set data='%b' where id = %d and accountID = %d", houseTable, size, data, id, accountID );

			free ( data );
		}

		break;

		// handle a house creation request
		case _IPC_DATAMGR_NEW_HOUSE: {
			PackedData packet ( msg->data(), msg->size() );
			char *name = packet.getString();
			int accountID = packet.getLong();

			if ( !name )
				break;

			int size = packet.getLong();
			char *data = (char *)malloc ( size );
			packet.getArray ( data, size );

			// get the character name id
			SQLResponse *nameID = sql->query ( "select id from characterNames where name = '%s'", name );
			int characterNameID = -1;

			if ( nameID ) {
				characterNameID = atoi ( nameID->table ( 0, 0 ) );
				delete nameID;
			}

			// add the house...
			sql->query ( "insert into %s set id=0, accountID=%d, characterNameID=%d, data='%b'", houseTable, accountID, characterNameID, size, data );

			free ( data );
			free ( name );
		}

		break;

		// handle an incoming connection
		case _IPC_CLIENT_CONNECTED: {
			int nIP = client->IP();

			client->maxMsgSize = 1000000000;
			client->uid() = -1;
			IPCServer::handleMessage ( msg );
		}

		break;

		// handle a disconnect
		case _IPC_CLIENT_HUNG_UP: {
			if ( client->uid() != -1 ) {
				sql->query( "update access set logout=UNIX_TIMESTAMP( now() ) where serverID=%d and logout=0", client->uid() );
				sql->query ( "delete from accountLocks where server=%d", client->uid() );

				// update the serverList so that the number of users is known
				sql->query ( "update serverList set userCount=0, isUp='down' where id=%d", client->uid() );

				// toss the character and house table entries
				if ( gCharacterTables[client->uid()] ) {
					free ( gCharacterTables[client->uid()] );
					gCharacterTables[client->uid()] = NULL;
				}
	
				if ( gHouseTables[client->uid()] ) {
					free ( gHouseTables[client->uid()] ); 
					gHouseTables[client->uid()] = NULL;
				}

                                if ( gEngraveTables[ client->uid() ] ) {
					free( gEngraveTables[ client->uid() ] );
					gEngraveTables[ client->uid() ] = NULL;
				}

                                if ( gMailDataTables[ client->uid() ] ) {
                                        free( gMailDataTables[ client->uid() ] );
                                        gMailDataTables[ client->uid() ] = NULL;
				}

                                if ( gMailLinkTables[ client->uid() ] ) {
                                        free( gMailLinkTables[ client->uid() ] );
                                        gMailLinkTables[ client->uid() ] = NULL;
				}

                                if ( gFriendsTables[ client->uid() ] ) {
                                        free( gFriendsTables[ client->uid() ] );
                                        gFriendsTables[ client->uid() ] = NULL;
				}

                                if ( gFoesTables[ client->uid() ] ) {
                                        free( gFoesTables[ client->uid() ] );
                                        gFoesTables[ client->uid() ] = NULL;
				}
			}
	
			IPCServer::handleMessage ( msg );
		}

		break;

				// handle a load house request
		case _IPC_DATAMGR_GET_ALL_HOUSES: {
			PackedData packet ( msg->data(), msg->size() );

			g_qHouseID = sql->query ( "select id from %s where characterNameID != 0 ORDER BY id", houseTable );

			if ( sql->error )
				logDisplay ( "SQL Error: '%s'", sql->error );

			if ( g_qHouseID ) {
				g_nRecord = 0;
				{
					// load the house
					SQLResponse *house = sql->query ( "select %s.id, %s.accountID, %s.data, characterNames.name from %s, characterNames where %s.id = %s and %s.characterNameID = characterNames.id", houseTable, houseTable, houseTable, houseTable, houseTable, g_qHouseID->table( g_nRecord++, 0), houseTable );

					if ( sql->error )
						logDisplay ( "SQL Error: '%s'", sql->error );
				
					if ( house ) {
						logDisplay( "Loaded house %s.", house->table( 0, 3 ) );
						// build the response packet
						PackedData response;

						response.putByte ( _DATAMGR_OKAY );

						// put the special IDs for the house
						response.putLong ( atoi ( house->table ( 0, 0 ) ) );
						response.putLong ( atoi ( house->table ( 0, 1 ) ) );
						response.putString ( house->table( 0, 3) );

						int size = house->length ( 0, 2 );
						response.putLong ( size );
						response.putArray ( house->table ( 0, 2 ), size );

						// send the response back
						send ( _IPC_DATAMGR_GET_ALL_HOUSES, &response, client );

						delete house;
					} else {
						logDisplay( "Failed to get the data" );

						// build the response packet
						PackedData response;

						response.putByte ( _DATAMGR_ERROR );

						// send the response back
						send ( _IPC_DATAMGR_GET_ALL_HOUSES, &response, client );
					}
				}
			}
		}

		break;

		case _IPC_DATAMGR_GET_NEXT_HOUSE: {
			PackedData packet ( msg->data(), msg->size() );

			if ( g_qHouseID ) {
				if ( g_nRecord == g_qHouseID->rows ) {
					delete g_qHouseID;
					g_qHouseID = NULL;
				} else {
					// load the house
					SQLResponse *house = sql->query ( "select %s.id, %s.accountID, %s.data, characterNames.name from %s, characterNames where %s.id = %s and %s.characterNameID = characterNames.id", houseTable, houseTable, houseTable, houseTable, houseTable, g_qHouseID->table( g_nRecord++, 0), houseTable );

					if ( sql->error )
						logDisplay ( "SQL Error: '%s'", sql->error );
				
					if ( house ) {
						logDisplay( "Loaded house %s.", house->table( 0, 3 ) );
						// build the response packet
						PackedData response;

						response.putByte ( _DATAMGR_OKAY );

						// put the special IDs for the house
						response.putLong ( atoi ( house->table ( 0, 0 ) ) );
						response.putLong ( atoi ( house->table ( 0, 1 ) ) );
						response.putString ( house->table( 0, 3) );

						int size = house->length ( 0, 2 );
						response.putLong ( size );
						response.putArray ( house->table ( 0, 2 ), size );

						// send the response back
						send ( _IPC_DATAMGR_GET_ALL_HOUSES, &response, client );

						delete house;
					} else {
						logDisplay( "Failed to get the data" );

						// build the response packet
						PackedData response;

						response.putByte ( _DATAMGR_ERROR );

						// send the response back
						send ( _IPC_DATAMGR_GET_ALL_HOUSES, &response, client );
					}
				}
			}
		}

		break;

		case _IPC_DATAMGR_LOGINMESSAGE: {

			PackedData packet ( msg->data(), msg->size() );

			char* pMessage = packet.getString();

			if ( pMessage ) {
				// add the new login message
				sql->query ( "insert into messages set serverID=%d, type = 'login', message = '%s'", client->uid(), pMessage );

				if ( sql->error ) {
					sql->query ( "update messages set message = '%s' where serverID=%d and type = 'login'", pMessage, client->uid() );
				}

				free( pMessage );
			} else {
				sql->query ( "delete from messages where serverID=%d and type = 'login'", client->uid() );
			}
		}

		break;

		case _IPC_DATAMGR_DOWNTIMEMESSAGE: {
			PackedData packet ( msg->data(), msg->size() );

			char* pMessage = packet.getString();

			if ( pMessage ) {
				// add the new downtime message
				sql->query ( "insert into messages set serverID=%d, type = 'downtime', message = '%s'", client->uid(), pMessage );

				if ( sql->error ) {
					sql->query ( "update messages set message = '%s' where serverID=%d and type = 'downtime'", pMessage, client->uid() );
				}

				free( pMessage );
			} else {
				sql->query ( "delete from messages where serverID=%d and type = 'downtime'", client->uid() );
			}
		}

		break;

		case _IPC_DATAMGR_SET_RIGHTS: {
			PackedData packet ( msg->data(), msg->size() );
			int accountID = packet.getLong();

			SQLResponse *account = sql->query ( "select rights+0 from accounts where id=%d", accountID );

			if ( account ) {
				int nRights = atoi( account->table ( 0, 0 ) );

				nRights |= _ACCESS_SET_IGNORE_STATS;

				sql->query( "update accounts set rights = %d where id = %d", nRights, accountID );
				delete account;
			}
		}

		break;

		case _IPC_DATAMGR_MAIL: {
			PackedData packet ( msg->data(), msg->size() );
			int result = packet.getByte();

			switch ( result ) {
				case _DATAMGR_MAIL_GET_LIST: {
					int nServID = packet.getLong();
					int nToonID = packet.getLong();

					// build the response packet
					PackedData response;

					response.putByte( _DATAMGR_MAIL_GET_LIST );
					response.putLong( nServID );

					SQLResponse* qMMs = sql->query( "select %s.id, date, subject, name, (%s.viewerID = %d) from %s, %s, characterNames where ( %s.characterID = %d or %s.viewerID = %d ) and %s.bDeleted = 0 and %s.id = %s.msgID and %s.characterID = characterNames.id",
											mailData, mailLink, nToonID, mailData, mailLink, mailLink, nToonID, mailLink, nToonID, mailLink, mailData, mailLink, mailData );

					if ( sql->error )
						logDisplay( "SQL: %s", sql->error );

					if ( qMMs ) {
						response.putWord( qMMs->rows );

						for (int nRecord = 0;nRecord < qMMs->rows;nRecord++) {
							response.putLong( atoi( qMMs->table( nRecord, 0 ) ) );	//	Unique Message ID
							response.putByte( atoi( qMMs->table( nRecord, 4 ) ) );	//	View Only
							response.putLong( atoi( qMMs->table( nRecord, 1 ) ) );	//	Time             
							response.putString( qMMs->table( nRecord, 2 ) );		//	Subject          
							response.putString( qMMs->table( nRecord, 3 ) );		//	From             
						}

						delete qMMs;
					} else {
						response.putWord( 0 );		//	Count of MMs
					}

					// send the response back
					send ( _IPC_DATAMGR_MAIL, &response, client );
				}

				break;

				case _DATAMGR_MAIL_GET: {
					int nServID = packet.getLong();
					int nToonID = packet.getLong();
					int nMsgID = packet.getLong();

					// build the response packet
					PackedData response;

					response.putByte( _DATAMGR_MAIL_GET );
					response.putLong( nServID );

					SQLResponse* qMMs = sql->query( "select characterID from %s where msgID = %d and characterID = %d", mailLink, nMsgID, nToonID );

					if ( qMMs ) {
						delete qMMs;

						qMMs = sql->query( "select body, bExists from %s where id = %d", mailData, nMsgID );

						if ( sql->error )
							logDisplay( "SQL: %s", sql->error );

						if ( qMMs ) {
							response.putString( qMMs->table( 0, 0 ) );
							response.putByte( atoi( qMMs->table( 0, 1 ) ) );

							delete qMMs;
						} else {
							response.putString( "" );
							response.putByte( 0 );
						}

						qMMs = sql->query( "select name from %s, characterNames where %s.msgID = %d and %s.characterID = characterNames.id", mailLink, mailLink, nMsgID, mailLink );

						if ( sql->error )
							logDisplay( "SQL: %s", sql->error );

						if ( qMMs ) {
							int nSize = qMMs->rows;

							response.putByte( nSize );

							for (int nRecord = 0;nRecord < nSize;nRecord++) {
								response.putString( qMMs->table( nRecord, 0 ) );
							}

							delete qMMs;
						} else {
							response.putByte( 0 );
						}
					} else {
						response.putString( "" );
						response.putByte( 0 );
						response.putByte( 0 );
					}

					// send the response back
					send ( _IPC_DATAMGR_MAIL, &response, client );
				}

				break;

				case _DATAMGR_MAIL_DELETE: {
					int nServID = packet.getLong();
					int nMsgID = packet.getLong();
					int nToonID = packet.getLong();

					sql->query( "update %s set bDeleted=1 where msgID=%d and ( characterID=%d or viewerID=%d )", mailLink, nMsgID, nToonID, nToonID );

					// build the response packet
					PackedData response;

					response.putByte( _DATAMGR_MAIL_DELETE );
					response.putLong( nServID );
					response.putLong( nMsgID );

					// send the response back
					send ( _IPC_DATAMGR_MAIL, &response, client );
				}

				break;

				case _DATAMGR_MAIL_SEND: {
					char pError[10240];

					pError[0] = 0;

					int		nToCount = 0;
					char*	pToToon[128];
					int		nToToon[128];
					int		nToAccount[128];

					int nServID = packet.getLong();
					int nToonID = packet.getLong();
					int nAccountID = packet.getLong();
					int nCount = packet.getWord();

					int nType = packet.getByte();
					int nReferralID = packet.getLong();
					char* pSubject = packet.getString();
					char* pBody = packet.getString();

					if ( !pSubject && !pBody ) {
						// build the response packet
						PackedData response;

						response.putByte( _DATAMGR_MAIL_SEND );
						response.putLong( nServID );
						response.putByte( 0 );
						response.putString( "Blank messages are not allowed." );

						// send the response back
						send ( _IPC_DATAMGR_MAIL, &response, client );

						return;
					}

					if ( !pSubject )
						pSubject = strdup( "" );

					if ( !pBody )
						pBody = strdup( "" );

					pToToon[ nToCount ] = packet.getString();

					while ( pToToon[ nToCount ] ) {
						SQLResponse* qToonID = sql->query( "select characterNameID, accountID from characterNames, %s where name = '%s' and %s.characterNameID = characterNames.id", charTable, pToToon[ nToCount ], charTable );

						if ( qToonID ) {
							int nToonID = atoi( qToonID->table( 0, 0 ) );
							int nDuplicate = 0;

							for (int nRecord = 0;nRecord < nToCount;nRecord++) {
								if ( nToToon[ nRecord ] == nToonID ) {
									nDuplicate = 1;
									break;
								}
							}

							if ( !nDuplicate ) {
								nToToon[ nToCount ] = nToonID;
								nToAccount[ nToCount ] = atoi( qToonID->table( 0, 1 ) );
		
								nToCount++;
							} else {
								free( pToToon[ nToCount ] );
							}
							delete qToonID;
						} else {
							if ( pError[0] ) {
								strcat( pError, pToToon[ nToCount ] );
								strcat( pError, " " );
							} else {
								sprintf( sizeof( pError ), pError, "The following recipient(s) are unknown: %s ", pToToon[ nToCount ] );
							}
						}

						pToToon[ nToCount ] = packet.getString();
					}

					// build the response packet
					PackedData response;

					response.putByte( _DATAMGR_MAIL_SEND );
					response.putLong( nServID );

					if ( !nToCount )
						sprintf( sizeof( pError ), pError, "You must send this to someone!" );

					if ( !pError[0] ) {
						response.putByte( 1 );

						if ( nCount == 1 ) {
							sql->query( "insert into %s set accountID=%d, characterID=%d, date=UNIX_TIMESTAMP( now() ), subject='%s', body='%s', referralID=%d", 
								mailData, nAccountID, nToonID, pSubject, pBody, nReferralID );

							if ( sql->error )
								logDisplay( sql->error );

							int nMsgID = sql->lastInsertID();

							for (int nRecord = 0;nRecord < nToCount;nRecord++) {
								sql->query( "insert into %s set msgID=%d, accountID=%d, characterID=%d", mailLink, nMsgID, nToAccount[nRecord], nToToon[nRecord] );
							}
						} else {
							for (int nMail = 0;nMail < nCount;nMail++) {
								sql->query( "insert into %s set accountID=%d, characterID=%d, date=UNIX_TIMESTAMP( now() ), subject='%s %d of %d', body='%s', referralID=%d", 
									mailData, nAccountID, nToonID, pSubject, nMail, nCount, pBody, nReferralID );
			
								int nMsgID = sql->lastInsertID();

								for (int nRecord = 0;nRecord < nToCount;nRecord++) {
									sql->query( "insert into %s set msgID=%d, accountID=%d, characterID=%d", mailLink, nMsgID, nToAccount[nRecord], nToToon[nRecord] );
								}
							}
						}
					} else {
						response.putByte( 0 );
						response.putString( pError );
					}

					// send the response back
					send ( _IPC_DATAMGR_MAIL, &response, client );

					if ( pSubject )
						free( pSubject );

					if ( pBody )
						free( pBody );

					for (int nCleanUp = 0;nCleanUp < nToCount;nCleanUp++)
						if ( pToToon[ nCleanUp ] )
							free( pToToon[ nCleanUp ] );
				}

				break;
			};
		}

		break;

                case _IPC_DATAMGR_COPPER: {
			PackedData packet ( msg->data(), msg->size() );
			int accountID = packet.getLong();
			int nCopper = packet.getLong();

			sql->query ( "update accounts set nCoppers=%d where id=%d", nCopper, accountID );                                                                                              
			if ( sql->error )
				logDisplay ( "SQL Error: '%s'", sql->error );
		}

		break;

			case _IPC_DATAMGR_CREDIT: {
			PackedData packet ( msg->data(), msg->size() );
			int accountID = packet.getLong();
			int nCredit = packet.getLong();

			sql->query ( "update accounts set nCredits=%d where id=%d", nCredit, accountID );
			
			if ( sql->error )
				logDisplay ( "SQL Error: '%s'", sql->error );
		}

	        break;
			
		case _IPC_DATAMGR_GAME_CRASHER: {
			PackedData packet ( msg->data(), msg->size() );

			int nID = packet.getLong();

			if ( nID != -1 ) {
                sql->query ( "update accounts set status='disabled', reason=22 where id=%d", nID );

                File file;
                file.open ( "/tmp/disable" );
                file.truncate();
                file.printf ( "From: G@M.E\nDisabled the account that crashed the server %d\n", nID );
                file.close();
				sql->query( "update serverList set crasher=%d where id=%d", nID, client->uid() );
				logDisplay("Account %d disabled", nID);
			}
		}

		break;

		// pass all unhandled message up to the IPCServer
		default: {
			IPCServer::handleMessage ( msg );
		}

		break;
	}
}


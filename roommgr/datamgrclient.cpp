//
// datamgrclient
//
// This file contains the DataMgrClient class and supporting structures.
//
// author: Stephen Nichols
//
#include <algorithm>
#include "roommgr.hpp"
#include "datamgrclient.hpp"
#include "../global/datamgrdefs.hpp"
#include "globals.hpp"

#include "callbacks.hpp"

// external functions...
extern void createMailFile ( char *name );

// this callback completes the teleport to house process
void cdLoadHouseTest( int result, int houseID, int accountID, char *name, int size, char *data )
{
	Building *house = NULL;

	if ( result == _DATAMGR_OKAY ) {
		// process the loaded house
		house = processHouseData ( name, data, size, houseID, accountID );


		if ( house ) {
			delete house;
		} else {
			logDisplay( "Did not load house %s %d %d.", name, houseID, accountID );
		}
	}
}

// this is the global datamgr client
DataMgrClient *gDataMgr = new DataMgrClient;

DataMgrClient::DataMgrClient()
{
	maxMsgSize = 1000000000;
}

DataMgrClient::~DataMgrClient()
{
}

// this member handles incoming messages
void DataMgrClient::handleMessage ( IPCMessage *msg )
{
	clock_t nStartTime = clock();

	switch ( msg->type() ) {
		// handle a login response message...
		case _IPC_DATAMGR_LOGIN: {
			PackedData packet ( msg->data(), msg->size() );

			// get the information from the packet
			int id = packet.getLong();
			int resultCode = packet.getByte();

			// make sure this login response is for the current login context
			if ( !gActiveLoginContext || (gActiveLoginContext->id != id) ) {
				// force the data manager to log out this account
				if ( resultCode != _DATAMGR_ERROR ) {
					int accountID = packet.getLong();
					gDataMgr->logout ( accountID );
				}

				logDisplay ( "login context is not correct for _IPC_DATAMGR_LOGIN" );
				return;
			}

			if ( resultCode == _DATAMGR_ERROR ) {
				char *errorString = packet.getString();

				// send the check login response
				gActiveLoginContext->sendResponse ( -1, 0, errorString );
				delete gActiveLoginContext;

				free ( errorString );
			} else {
				int accountID = packet.getLong();
				int rights = packet.getLong();
				int gossipBanTime = packet.getLong();
				char *accountType = packet.getString();
				int billingDate = packet.getLong();
				int nCredits = packet.getLong();
				int nCoppers = packet.getLong();
				char *pLoginMessage = packet.getString();

				// get the player object
				RMPlayer *player = gActiveLoginContext->player;
				player->accountID = accountID;
				player->gossipBanTime = gossipBanTime;
				player->accountTypeStr = accountType;
				player->billingDate = billingDate;
				player->nCredits = nCredits;
				player->nCoppers = nCoppers;

				if ( player->character ) {
					delete player->character;

					player->character = NULL;
				}

				if ( !gLoginTree.find( gActiveLoginContext->loginName ) )
					gLoginTree.add ( gActiveLoginContext->loginName, player );

				// create the world object for this account
				WorldObject *obj = new WorldObject ( roomMgr->findClass ( "Player" ) );
				obj->addToDatabase();

				// link the player and the object
				player->player = obj;
				obj->player = player;

				// set the login name and password for the player object
				BPlayer *bplayer = (BPlayer *)obj->getBase ( _BPLAYER );
				bplayer->setLoginName ( gActiveLoginContext->loginName );
				bplayer->setPassword ( gActiveLoginContext->password );

				// set the rights of this player
				player->initAccess ( rights );

				int numChars = packet.getByte();

				for ( int i=0; i<numChars; i++ ) {
					char *name = packet.getString();
					int sqlID = packet.getLong();
					int characterID = packet.getLong();
					int size = packet.getLong();

					char *data = (char *)malloc ( size );
					packet.getArray ( data, size );

					name[0] = toupper( name[0] );

					// load this character and attach it to the player
					WorldObject *charObj = loadCharacterData ( player, data, size, gActiveLoginContext->loginName, name );

					if ( charObj ) {
						// see if we should finish loading the character
						int finishLoad = 1;					

						if ( charObj->physicalState & _STATE_HACKER ) {
							// don't let normal players have hacked characters
							if ( !player->checkAccess ( _ACCESS_GUIDE ) ) {
								delete charObj;
								finishLoad = 0;
							}
						}

						// finish the loading process 
						if ( finishLoad ) {
							// keep non-guides out of hideout 
							if ( !player->checkAccess ( _ACCESS_GUIDE ) && (charObj->roomNumber == 6066 || charObj->roomNumber == 6067) ) 
								charObj->roomNumber = 3012;
							
							charObj->sqlID = sqlID;
							charObj->characterID = characterID;
							charObj->player = player;

							if ( !charObj->hasAffect ( _AFF_RESET, _AFF_TYPE_NORMAL, _AFF_SOURCE_GIFT ) )
								charObj->addAffect ( _AFF_RESET, _AFF_TYPE_NORMAL, _AFF_SOURCE_GIFT, -1, 0, NULL );

							if ( !charObj->hasAffect ( _AFF_RESET_A, _AFF_TYPE_NORMAL, _AFF_SOURCE_GIFT ) )
								charObj->addAffect ( _AFF_RESET_A, _AFF_TYPE_NORMAL, _AFF_SOURCE_GIFT, -1, 0, NULL );

							// attach the character to the player
							bplayer->addCharacter ( charObj );
						} else {
							logInfo ( _LOG_ALWAYS, "character did not load %s, %s", gActiveLoginContext->loginName, name );
						}
					} else {
						logInfo ( _LOG_ALWAYS, "character NULL from loadCharacterData %s, %s", gActiveLoginContext->loginName, name );
					}

					free ( name );
					free ( data );
				}

				// finish the player preparation
				if ( player->checkAccess ( _ACCESS_MODERATOR ) ) {
					roomMgr->_gms.add ( player );
					roomMgr->_guides.add ( player );
					roomMgr->_hosts.add ( player );
				} else if ( player->checkAccess ( _ACCESS_GUIDE ) ) {
					roomMgr->_guides.add ( player );
					roomMgr->_hosts.add ( player );
				} else if ( player->checkAccess ( _ACCESS_PROPHET | _ACCESS_EVENT ) ) {
					roomMgr->_hosts.add ( player );
				}

				if ( bEventHappening && !gEventPlayers.contains( player ) && !gBadEventPlayers.contains( (ListObject*) player->accountID ) ) {
					gEventPlayers.add( player );
				}

				// set the closed physical state
				player->player->physicalState |= _STATE_CLOSED;

				// send the check login response
				gActiveLoginContext->sendResponse ( obj->servID, player->rights );

				// send the response to the player if the request has been received
				if ( player->validated )
					roomMgr->sendACK ( _IPC_PLAYER_LOGIN, player );

				// toss the login context
				delete gActiveLoginContext;

				if ( pLoginMessage ) {
					roomMgr->sendSystemMsg ( "Login Message", player, pLoginMessage );

					free ( pLoginMessage );
				}
			}
		}

		break;

		// handle getting the config information to the client
		case _IPC_DATAMGR_CONFIG_INFO: {
                        PackedData packet ( msg->data(), msg->size() );

			char nSelection = packet.getByte();

			switch ( nSelection ) {
				case _DATAMGR_CONFIG_INFO_GET: {
		                        int servID = packet.getLong();

                		        WorldObject *obj = roomMgr->findObject ( servID );

		                        // character is toast, exit...
                		        if ( !obj )
                                		return;

		                        // get the player
                		        RMPlayer* player = obj->player;

		                        if ( !player )
                		                return;

		                        PackedMsg response;

					response.putACKInfo ( _IPC_PLAYER_CHARACTER_LOGIN );

					// get the friends
					int nSize = packet.getWord();
					response.putWord( nSize );

					for (int nRecord = 0;nRecord < nSize;nRecord++) {
						char* pName = packet.getString();

						player->AddFriend( pName );
						response.putString( pName );

						delete pName;
					}

		                        // get the foes
                		        nSize = packet.getWord();
					response.putWord( nSize );

		                        for (int nRecord = 0;nRecord < nSize;nRecord++) {
                		                char* pName = packet.getString();
						player->ignore( pName );
                                		response.putString( pName );

		                                delete pName;
                		        }

					nSize = packet.getLong();
					response.putLong( nSize );

					// Handle the spells list data
					if ( nSize ) {
						char* data = (char*) malloc( nSize );
						packet.getArray( data, nSize );

						response.putArray( data, nSize );
						free( data );
					}

					nSize = packet.getLong();

					// handle the quest data
					if ( nSize ) {
						// nSize should be a multiple of 16 ( 4 ints per entry )  PLUS 1 int for the count
		                                int* data = (int*) malloc( nSize );
                		                packet.getArray( data, nSize );

						player->loadQuestData( data );

                		                free( data );
					}

					nSize = packet.getLong();

					// handle the crime data
					if ( nSize ) {
						if ( nSize != sizeof( CrimeData ) )
							logInfo( _LOG_ALWAYS, "%s(%d) Database and the game are not sync'd on the size of CrimeData", __FILE__, __LINE__ );

						CrimeData* data = player->getCrimeData();
		                                packet.getArray( data, nSize );
					}

	                                roomMgr->sendTo ( _IPC_PLAYER_ACK, &response, player );

					if ( bEventHappening && gEventTitle )
						roomMgr->sendPlayerText( player, "-8t%s", gEventTitle );
				}

				break;

                                case _DATAMGR_CONFIG_INFO_ADD_FRIEND: {
                		        int servID = packet.getLong();

		                        WorldObject *obj = roomMgr->findObject ( servID );

                		        // character is toast, exit...
		                        if ( !obj )
                		                return;

		                        // get the player
		                        RMPlayer* player = obj->player;

                		        if ( !player )
                                		return;

					char* pName = packet.getString();
					int nResult = packet.getByte();

					if ( nResult ) {
						player->AddFriend( pName );
						roomMgr->sendPlayerText ( player, "-f %s", pName );
					} else {
						roomMgr->sendPlayerText ( player, "|c43|Info> Could not find %s to add them to your friend's list.", pName );
					}

					free( pName );
                                }

                                break;

                                case _DATAMGR_CONFIG_INFO_ADD_FOE: {
                                        int servID = packet.getLong();

                                        WorldObject *obj = roomMgr->findObject ( servID );

                                        // character is toast, exit...
                                        if ( !obj )
                                                return;

                                        // get the player
                                        RMPlayer* player = obj->player;

                                        if ( !player )
                                                return;

                                        char* pName = packet.getString();

                                        int nResult = packet.getByte();

                                        if ( nResult ) {
                                                player->ignore( pName );
						roomMgr->sendPlayerText ( player, "-ep%s", pName );
                                        } else {
                                                roomMgr->sendPlayerText ( player, "|c43|Info> Could not find %s to add them to your foe's list.", pName );
                                        }

                                        free( pName );
                                }

                                break;

                                case _DATAMGR_CONFIG_INFO_CHECK_FOE: {
                                        int servID = packet.getLong();

                                        WorldObject *obj = roomMgr->findObject ( servID );

                                        // character is toast, exit...
                                        if ( !obj )
                                                return;

                                        // get the player
                                        RMPlayer* player = obj->player;

                                        if ( !player )
                                                return;

                                        char* pName = packet.getString();

                                        int nResult = packet.getByte();

                                        if ( nResult ) {
                                                player->ignore( pName );
						roomMgr->sendPlayerText ( player, "-e %s", pName );
                                        } else {
                                                roomMgr->sendPlayerText ( player, "|c43|Info> Could not find %s to add them to your foe's list.", pName );
                                        }

                                        free( pName );
                                }

                                break;

                                case _DATAMGR_CONFIG_INFO_WRITE_SPELLS: {
                                }

                                break;
			}
		}

		break;

                // handle getting the config information to the client
                case _IPC_DATAMGR_PLACE_BOUNTY: {
                        PackedData packet ( msg->data(), msg->size() );
                        int servID = packet.getLong();

                        WorldObject *obj = roomMgr->findObject ( servID );                                                    

                        // character is toast, exit...
                        if ( !obj )
                                return;

                        // get the player
                        RMPlayer* player = obj->player;

                        if ( !player )
                                return;

			long nBounty = packet.getLong();
			char* pName = packet.getString();
			char nResult = packet.getByte();

			if ( nResult ) {
				roomMgr->sendPlayerText ( player, "|c43|Info> You have placed a bounty on the head of %s. The total bounty is now %d gold\n", pName, nBounty );
			} else {
				player->character->value += nBounty;
				roomMgr->sendPlayerText ( player, "|c43|Info> There is nobody by the name of '%s' to place a bounty on.\n", pName );
			}	
			
			delete pName;
		}

		break;

		// handle getting the whats new information
		case _IPC_DATAMGR_WHATS_NEW: {
			PackedData packet ( msg->data(), msg->size() );
			int servID = packet.getLong();

			WorldObject *obj = roomMgr->findObject ( servID );

			// character is toast, exit...
			if ( !obj )
				return;

			// get the player
			RMPlayer *player = obj->player;

			if ( !player )
				return;

			PackedMsg response;

			int nCount = packet.getWord();

			for (int i = 0;i < nCount;i++) {
				response.putByte( 1 );

				char* ptr = packet.getString();
				response.putString( ptr );
				free( ptr );
			}

			response.putByte( 0 );

			roomMgr->sendTo ( _IPC_WHATS_NEW, response.data(), response.size(), player );
		}
		
		break;

		// handle completing a character creation
		case _IPC_DATAMGR_NEW_CHAR: {
			PackedData packet ( msg->data(), msg->size() );
			int servID = packet.getLong();
			int resultCode = packet.getLong();
			int sqlID = packet.getLong();
			int characterID = packet.getLong();

			WorldObject *obj = roomMgr->findObject ( servID );

			// character is toast, exit...
			if ( !obj )
				return;

			// get the player
			RMPlayer *player = obj->player;

			if ( !player )
				return;

			if ( resultCode != -1 ) {
				// remove the character from the list of characters on the player
				BPlayer *bplayer = (BPlayer *)player->player->getBase ( _BPLAYER );
				bplayer->delCharacter ( obj );

				// destroy the character that was being created
				delete obj;

				roomMgr->sendNAK ( _IPC_PLAYER_CREATE_CHARACTER, resultCode, player );
			} else {
				// creation was handled properly...
				if ( player->checkAccess ( _ACCESS_MODERATOR ) ) {
					roomMgr->sendSystemMsg ( "Special Access", player, "Your character has been granted special access to moderator level commands.  Please use this character responsibly." );
				}

				// create the mail file
				char *name = obj->getName();

				// log the creation
				BCharacter *bchar = (BCharacter *)obj->getBase ( _BCHARACTER );
				gDataMgr->logPermanent ( player->getLogin(), name, _DMGR_PLOG_CREATECHAR, "%s,%s", name, bchar->title ); 

				// create the house
				BPlayer *bplayer = (BPlayer *)player->player->getBase ( _BPLAYER );
				int firstHouse = 0;

				if ( bplayer->charNames.size() == 1 ) 
					firstHouse = 1;

				Building *house = makeTemplateHouse ( name, player->getPassword(), firstHouse );

				if ( house ) {
					house->accountID = player->accountID;
					house->writeHouseData();
					delete house;
				}

				// send the response message
				PackedMsg ack;
				ack.putACKInfo ( _IPC_PLAYER_CREATE_CHARACTER );

				// put the object info
				obj->buildPacket ( &ack, 1 );
				obj->sqlID = sqlID;
				obj->characterID = characterID;

				roomMgr->sendTo ( _IPC_PLAYER_ACK, &ack, player );
			}
		}

		break;

		// this is the response to a load house message
		case _IPC_DATAMGR_GET_HOUSE: {
			PackedData packet ( msg->data(), msg->size() );
			int context = packet.getLong();
			hlcallback_t callback = (hlcallback_t)packet.getLong();
			int result = packet.getByte();

			if ( result == _DATAMGR_OKAY ) {
				int houseID = packet.getLong();
				int accountID = packet.getLong();
				char *name = packet.getString();

				int size = packet.getLong();
				char *data = (char *)malloc ( size );
				packet.getArray ( data, size );

				sprintf( sizeof( gCrashMessage ), gCrashMessage, "Loading house %s, id #%d, for accountID %d", name, houseID, accountID ); 

				callback ( result, context, houseID, accountID, name, size, data );

				gCrashMessage[0] = 0;

				free ( data );
				free ( name );
			} else {
				callback ( result, context, -1, -1, NULL, 0, NULL );
			}
		}

		break;

		// this is the response to a load house message
		case _IPC_DATAMGR_GET_ALL_HOUSES: {
			PackedData packet ( msg->data(), msg->size() );
			int result = packet.getByte();

			if ( result == _DATAMGR_OKAY ) {
				int houseID = packet.getLong();
				int accountID = packet.getLong();
				char *name = packet.getString();

				logDisplay( "Got load house for %s.", name );
				int size = packet.getLong();
				char *data = (char *)malloc ( size );
				packet.getArray ( data, size );

				cdLoadHouseTest ( result, houseID, accountID, name, size, data );

				free ( data );
				free ( name );
			} else {
				logDisplay( "Loaded All houses - failed." );
			}

			PackedData packet2;
			send ( _IPC_DATAMGR_GET_NEXT_HOUSE, &packet2 );
		}

		break;

		case _IPC_DATAMGR_MAIL: {
			PackedData packet ( msg->data(), msg->size() );
			int result = packet.getByte();
			char* pTemp;

			switch ( result ) {
				case _DATAMGR_MAIL_GET_LIST: {
					int servID = packet.getLong();

					WorldObject* obj = roomMgr->findObject ( servID );

					// character is toast, exit...
					if ( !obj )
						return;

					// get the player
					RMPlayer* player = obj->player;

					if ( !player )
						return;

					int nSize = packet.getWord();

					if ( nSize == 0 ) {
						roomMgr->sendNAK ( _IPC_MAIL_LIST_GET, _ERR_BAD_SERVID, player );
					} else {
						for (int nLoop = 0;nLoop < nSize;nLoop += 200) {
							int nCount = std::min(( nSize - nLoop ), 200);
							//( nSize - nLoop ) <? 200;

							PackedMsg response;
							response.putACKInfo( _IPC_MAIL_LIST_GET );

							response.putWord ( nLoop );
							response.putWord ( nCount );
							response.putByte( ( ( nLoop + nCount ) == nSize ) );

							for (int nMail = 0;nMail < nCount;nMail++) {
								int nMailID = packet.getLong();
								response.putLong( nMailID );		//	Unique Message ID
								response.putByte( packet.getByte() );		//	View Only
								response.putLong( packet.getLong() );		//	Time

								pTemp = packet.getString();

								if ( !pTemp )
									pTemp = strdup( "" );

								response.putString( pTemp );	//	Subject

								if ( pTemp )
									free( pTemp );

								pTemp = packet.getString();

								response.putString( pTemp );	//	From
								
								if ( pTemp )
									free( pTemp );
							}

							roomMgr->sendTo ( _IPC_PLAYER_ACK, &response, player );
						}
					}
				}

				break;

				case _DATAMGR_MAIL_GET: {
					int servID = packet.getLong();

					WorldObject* obj = roomMgr->findObject ( servID );

					// character is toast, exit...
					if ( !obj )
						return;

					// get the player
					RMPlayer* player = obj->player;

					if ( !player )
						return;

					PackedMsg response;
					response.putACKInfo( _IPC_MAIL_MESSAGE_GET );

					char* pTemp = packet.getString();

					if ( !pTemp )
						pTemp = strdup( "" );

					response.putString( pTemp );	//	Body

					if ( pTemp )
						free( pTemp );

					response.putByte( packet.getByte() );

					int nSize = packet.getByte();
					response.putByte( nSize );

					for (int nMail = 0;nMail < nSize;nMail++) {
						response.putString( pTemp = packet.getString() );	//	To

						if ( pTemp )
							free( pTemp );
					}

					roomMgr->sendTo ( _IPC_PLAYER_ACK, &response, player );
				}

				break;
				
				case _DATAMGR_MAIL_DELETE: {
					int servID = packet.getLong();

					WorldObject* obj = roomMgr->findObject ( servID );

					// character is toast, exit...
					if ( !obj )
						return;

					// get the player
					RMPlayer* player = obj->player;

					if ( !player )
						return;

					PackedMsg response;
					response.putACKInfo( _IPC_MAIL_MESSAGE_DELETE );
					response.putLong( packet.getLong() );

					roomMgr->sendTo ( _IPC_PLAYER_ACK, &response, player );
				}

				break;
				
				case _DATAMGR_MAIL_SEND: {
					int servID = packet.getLong();

					WorldObject* obj = roomMgr->findObject ( servID );

					// character is toast, exit...
					if ( !obj )
						return;

					// get the player
					RMPlayer* player = obj->player;

					if ( !player )
						return;

					int nResult = packet.getByte();

					if ( !nResult ) {
						char* pTemp;
						PackedMsg response;

						response.putNAKInfo( _IPC_MAIL_MESSAGE_SEND );
						response.putString( pTemp = packet.getString() );

						if ( pTemp )
							free( pTemp );

						roomMgr->sendTo ( _IPC_PLAYER_NAK, &response, player );
					} else {
						PackedMsg response;
						response.putACKInfo( _IPC_MAIL_MESSAGE_SEND );

						roomMgr->sendTo ( _IPC_PLAYER_ACK, &response, player );
					}

				}

				break;

				case _DATAMGR_MAIL_ARCHIVE: {
				}

				break;
				
				case _DATAMGR_MAIL_COMPLAIN: {
				}

				break;
			}
		}

		break;

		default: {
			IPCClient::handleMessage ( msg );
		}

		break;
	}

	if ( ( msg->type() > -1 ) && ( msg->type() < _IPC_MAX_MESSAGES ) ) {
		clock_t nEndTime = clock() - nStartTime;

		IPCStats::Msgs[ msg->type() ].addExecTime( nEndTime, msg );
	}
}

// this member is called to log something permanently
void DataMgrClient::logPermanent ( char *login, char *charName, int type, const char *format, ... )
{
	char output[1024];

	va_list args;
	va_start ( args, format );
	vsprintf ( sizeof ( output ), output, format, args );
	va_end ( args );

	PackedData packet;
	packet.putString ( login );
	packet.putString ( charName );
	packet.putByte ( type );
	packet.putString ( output );

	send ( _IPC_DATAMGR_LOG_PERMANENT, &packet );
}

// this member is called to log engraves
void DataMgrClient::logEngrave( const char* pLogin, const char* charName, const char* pName ) {
	PackedData packet;
	packet.putString ( pLogin );
	packet.putString( charName );
	packet.putString( pName );

	send ( _IPC_DATAMGR_LOG_ENGRAVE, &packet );
}

// this member is called to send the hello message to the server
void DataMgrClient::hello ( void )
{
	PackedData packet;
	packet.putWord ( gServerID );

	send ( _IPC_DATAMGR_HELLO, &packet );
}

// this member is called to log an account in
void DataMgrClient::login ( int id, char *login, char *password, int nOSVersion, int nCount, char *pID, int nIP )
{
	PackedData packet;
	packet.putLong ( id );
	packet.putString ( login );
	packet.putString ( password );
	packet.putByte ( gServerID );
	packet.putLong( nIP );
	packet.putLong( nOSVersion );
	packet.putByte( nCount );

	if ( nCount )
		packet.putString ( pID );

	send ( _IPC_DATAMGR_LOGIN, &packet );
}

// this member is called to get the config information for this character
void DataMgrClient::config( WorldObject *character ) {
        if ( !character )
                return;

        PackedData packet;

	packet.putByte( _DATAMGR_CONFIG_INFO_GET );
        packet.putLong( character->servID );
        packet.putLong( character->characterID );

        send( _IPC_DATAMGR_CONFIG_INFO, &packet );
}

// write out the quests data
void DataMgrClient::writeQuests( WorldObject* character, int nSize, int* pData ) {
	if ( !character )
		return;

	PackedData packet;

        packet.putByte( _DATAMGR_CONFIG_INFO_WRITE_QUESTS );
	packet.putLong( character->characterID );
	packet.putLong( nSize );
	packet.putArray( pData, nSize );

	send( _IPC_DATAMGR_CONFIG_INFO, &packet );
}

// write out the crimes data
void DataMgrClient::writeCrimes( WorldObject* character, int nSize, CrimeData* pData ) {
        if ( !character )
                return;

        PackedData packet;

        packet.putByte( _DATAMGR_CONFIG_INFO_WRITE_CRIMES );
        packet.putLong( character->characterID );
        packet.putLong( nSize );
        packet.putArray( pData, nSize );

        send( _IPC_DATAMGR_CONFIG_INFO, &packet );
}

// this member is called to log an account out of the system
void DataMgrClient::logout ( RMPlayer *target ) {
	PackedData packet;
	packet.putLong ( target->accountID ); 

	send ( _IPC_DATAMGR_LOGOUT, &packet );
}

// this member is called to log an account out of the system
void DataMgrClient::logout ( int id ) {
	PackedData packet;
	packet.putLong ( id ); 

	send ( _IPC_DATAMGR_LOGOUT, &packet );
}

// this member is called to revoke the gossip rights of an account
void DataMgrClient::revoke ( RMPlayer *target, int state )
{
	PackedData packet;
	packet.putLong ( target->accountID ); 
	packet.putByte ( state );

	send ( _IPC_DATAMGR_REVOKE, &packet );
}

// this member is called to gag an account
void DataMgrClient::gag ( RMPlayer *target, int state )
{
	PackedData packet;
	packet.putLong ( target->accountID ); 
	packet.putByte ( state );

	send ( _IPC_DATAMGR_GAG, &packet );
}

// this member is called to suspend an account
void DataMgrClient::suspend ( RMPlayer *target )
{
	PackedData packet;
	packet.putLong ( target->accountID ); 

	send ( _IPC_DATAMGR_SUSPEND, &packet );
}

// this member is called to disable an account
void DataMgrClient::disable ( RMPlayer *target )
{
	PackedData packet;
	packet.putLong ( target->accountID ); 

	send ( _IPC_DATAMGR_DISABLE, &packet );
}

// this member sets or clears the login message
void DataMgrClient::LoginMessage( char* pMessage )
{
	PackedData packet;
	packet.putString( pMessage );

	send ( _IPC_DATAMGR_LOGINMESSAGE, &packet );
}

// this member sets or clears the downtime message
void DataMgrClient::DowntimeMessage( char* pMessage )
{
	PackedData packet;
	packet.putString( pMessage );

	send ( _IPC_DATAMGR_DOWNTIMEMESSAGE, &packet );
}

// this member is called to set the password of an account
void DataMgrClient::setpass ( RMPlayer *target, char *password )
{
	PackedData packet;
	packet.putLong ( target->accountID ); 
	packet.putString ( password );

	send ( _IPC_DATAMGR_SETPASS, &packet );
}

// Give copper to this player
void DataMgrClient::copper ( RMPlayer* target, int nAmount ) {
	PackedData packet;
	packet.putLong ( target->accountID );
	packet.putLong ( nAmount );

	send ( _IPC_DATAMGR_COPPER, &packet );
}

// Give credit to this player
void DataMgrClient::credit ( RMPlayer* target, int nAmount ) {
	PackedData packet;

	packet.putLong ( target->accountID );
	packet.putLong ( nAmount );

	send ( _IPC_DATAMGR_CREDIT, &packet );
}

// this member is called to update an existing character in the database
void DataMgrClient::writeCharacter ( WorldObject *character, PackedData *data )
{
	if ( !character || !character->player )
		return;

	PackedData packet;
	packet.putLong ( character->sqlID );
	packet.putLong ( character->player->accountID );
	packet.putLong ( data->size() );
	packet.putArray ( data->data(), data->size() );

	send ( _IPC_DATAMGR_SAVE_CHAR, &packet );
}

// this member is called to create a new character in the database
void DataMgrClient::newCharacter ( WorldObject *character, PackedData *data )
{
	if ( !character || !character->player )
		return;

	PackedData packet;
	packet.putLong ( character->servID );
	packet.putString ( character->getName() );
	packet.putLong ( character->player->accountID );
	packet.putLong ( data->size() );
	packet.putArray ( data->data(), data->size() );

	send ( _IPC_DATAMGR_NEW_CHAR, &packet );
}

// this member is called to get the what's new information
void DataMgrClient::whatsNew ( WorldObject *character )
{
	if ( !character || !character->player )
		return;

	PackedData packet;
	packet.putLong ( character->servID );

	send ( _IPC_DATAMGR_WHATS_NEW, &packet );
}

// this member is called to delete an existing character
void DataMgrClient::delCharacter ( WorldObject *character )
{
	if ( !character || !character->player )
		return;

	PackedData packet;
	packet.putLong ( character->sqlID );
	packet.putLong ( character->player->accountID );

	send ( _IPC_DATAMGR_DEL_CHAR, &packet );
}

// this member is called to add a new house to the database
void DataMgrClient::newHouse ( Building *house, PackedData *data )
{
	PackedData packet;
	packet.putString ( house->_owner );
	packet.putLong ( house->accountID );
	packet.putLong ( data->size() );
	packet.putArray ( data->data(), data->size() );

	send ( _IPC_DATAMGR_NEW_HOUSE, &packet );
}

// this member is called to write a house to the database
void DataMgrClient::writeHouse ( Building *house, PackedData *data )
{
	PackedData packet;
	packet.putLong ( house->sqlID );
	packet.putLong ( house->accountID );
	packet.putLong ( data->size() );
	packet.putArray ( data->data(), data->size() );

	send ( _IPC_DATAMGR_WRITE_HOUSE, &packet );
}

// this member is called to read a house from the database
void DataMgrClient::loadHouse ( char *name, hlcallback_t callback, int context )
{
	PackedData packet;
	packet.putLong ( context );
	packet.putLong ( (int)callback );
	packet.putString ( name );
	
	send ( _IPC_DATAMGR_GET_HOUSE, &packet );
}

// this member is called to read a house from the database
void DataMgrClient::loadAllHouses ( )
{
	PackedData packet;

	send ( _IPC_DATAMGR_GET_ALL_HOUSES, &packet );
}

// this member is called to set the rights of an account
void DataMgrClient::rights ( RMPlayer *target )
{
	PackedData packet;
	packet.putLong ( target->accountID ); 

	send ( _IPC_DATAMGR_SET_RIGHTS, &packet );
}

// this member is called to get the list of mail messages
void DataMgrClient::getMailList( WorldObject* character ) {
	if ( !character || !character->player )
		return;

	PackedData packet;

	packet.putByte( _DATAMGR_MAIL_GET_LIST );
	packet.putLong( character->servID );
	packet.putLong( character->characterID );

	send ( _IPC_DATAMGR_MAIL, &packet );
}

// this member is called to get the list of mail messages
void DataMgrClient::getMailMessage( WorldObject* character, int nMsgID ) {
	if ( !character || !character->player )
		return;

	PackedData packet;

	packet.putByte( _DATAMGR_MAIL_GET );
	packet.putLong( character->servID );
	packet.putLong( character->characterID );
	packet.putLong( nMsgID );

	send ( _IPC_DATAMGR_MAIL, &packet );
}

// this member is called to get the list of mail messages
void DataMgrClient::deleteMail( WorldObject* character, int nMsgID ) {
	if ( !character || !character->player )
		return;

	PackedData packet;

	packet.putByte( _DATAMGR_MAIL_DELETE );
	packet.putLong( character->servID );
	packet.putLong( nMsgID );
	packet.putLong( character->characterID );

	send ( _IPC_DATAMGR_MAIL, &packet );
}

// this member is called to send a mail message
void DataMgrClient::sendMail ( WorldObject* character, char* pPtr, int nSize ) {
	if ( !character || !character->player )
		return;

	PackedData packet;

	packet.putByte( _DATAMGR_MAIL_SEND );
	packet.putLong( character->servID );
	packet.putLong( character->characterID );
	packet.putLong( character->player->accountID );

	packet.putWord( 1 );

	packet.putArray( pPtr, nSize );

	send ( _IPC_DATAMGR_MAIL, &packet );
}

//	this member is called to send a mail
void DataMgrClient::sendMail( WorldObject* character, char* pTo, char* pBody, int nCount ) {
	if ( !character || !character->player )
		return;

	PackedData packet;

	packet.putByte( _DATAMGR_MAIL_SEND );
	packet.putLong( character->servID );
	packet.putLong( character->characterID );
	packet.putLong( character->player->accountID );

	packet.putWord( nCount );

	packet.putByte( 0 );
	packet.putLong( 0 );
	packet.putString( "" );
	packet.putString( pBody );
	packet.putString( pTo );
	packet.putString( "" );

	send ( _IPC_DATAMGR_MAIL, &packet );
}

// handle adding a bounty on a player not online
void DataMgrClient::placeBounty( WorldObject* character, char* pName, long nBounty ) {
        if ( !character || !character->player )
                return;

        PackedData packet;

	packet.putLong( character->servID );
	packet.putString( pName );
	packet.putLong( nBounty );

	send( _IPC_DATAMGR_PLACE_BOUNTY, &packet );
}

// Handle adding a friend
void DataMgrClient::AddFriend( RMPlayer* player, char* pName ) {
        if ( !player || !player->character )
                return;

        PackedData packet;

	packet.putByte( _DATAMGR_CONFIG_INFO_ADD_FRIEND );
        packet.putLong( player->character->servID );
        packet.putLong( player->character->characterID );
        packet.putString( pName );

        send( _IPC_DATAMGR_CONFIG_INFO, &packet );
}

// Handle removing a friend
void DataMgrClient::DelFriend( RMPlayer* player, char* pName ) {
        if ( !player || !player->character )
                return;

        PackedData packet;

        packet.putByte( _DATAMGR_CONFIG_INFO_DEL_FRIEND );
        packet.putLong( player->character->characterID );
        packet.putString( pName );

        send( _IPC_DATAMGR_CONFIG_INFO, &packet );

	roomMgr->sendPlayerText ( player, "-fr%s", pName );
}

// Handle adding a foe
void DataMgrClient::AddFoe( RMPlayer* player, char* pName ) {
        if ( !player || !player->character )
                return;

        PackedData packet;

        packet.putByte( _DATAMGR_CONFIG_INFO_ADD_FOE );
        packet.putLong( player->character->servID );
        packet.putLong( player->character->characterID );
        packet.putString( pName );

        send( _IPC_DATAMGR_CONFIG_INFO, &packet );
}

// Handle removing a foe
void DataMgrClient::DelFoe( RMPlayer* player, char* pName ) {
        if ( !player || !player->character )
                return;

        PackedData packet;

        packet.putByte( _DATAMGR_CONFIG_INFO_DEL_FOE );
        packet.putLong( player->character->characterID );
        packet.putString( pName );

        send( _IPC_DATAMGR_CONFIG_INFO, &packet );

	roomMgr->sendPlayerText ( player, "-er%s", pName );
}

// handle checking for a valid player to ignore
void DataMgrClient::CheckFoe( RMPlayer* player, char* pName ) {
        if ( !player || !player->character )
                return;

        PackedData packet;

        packet.putByte( _DATAMGR_CONFIG_INFO_CHECK_FOE );
        packet.putLong( player->character->servID );
        packet.putLong( player->character->characterID );
        packet.putString( pName );

        send( _IPC_DATAMGR_CONFIG_INFO, &packet );
}

// handle writing their list of spells.
void DataMgrClient::WriteSpells( RMPlayer* player, IPCMessage* message ) {
	if ( !player || !player->character )
                return;

	PackedMsg spellPacket ( message->data(), message->size() );

	unsigned int nSize = spellPacket.getLong();
	if( nSize > 5000 ) {
		logHack( "%s:%d - DataMgrClient::WriteSpells() - requested array size is too large", __FILE__, __LINE__ );
		return;
	}

	char* pData = (char*) malloc( nSize );

	spellPacket.getArray( pData, nSize );

        PackedData packet;

        packet.putByte( _DATAMGR_CONFIG_INFO_WRITE_SPELLS );
        packet.putLong( player->character->characterID );
	packet.putLong( nSize );
        packet.putArray( pData, nSize );

        send( _IPC_DATAMGR_CONFIG_INFO, &packet );
}

// handle documenting the crasher
void DataMgrClient::crasher( int nID ) {
        PackedData packet;

		//don't disable people for crashing a test server
		if( IsThisATestServer() ) {
			packet.putLong( -1 );
		}
		else {
			packet.putLong( nID );
		}

        send( _IPC_DATAMGR_GAME_CRASHER, &packet );
}


int RMPlayer::process_IPC_VERB_GIVE ( IPCMessage *message ) {
	int retVal = 1;

	if ( !character || !character->player ) 
		return retVal;
		
	int questOwner = 0;

	PackedData movie;
	WorldObject *indirectObject, *directObject;

	PackedData packet ( message->data(), message->size() );
	preVerbMessage ( &packet, &movie, &directObject, &indirectObject );

	int result = _WO_ACTION_PROHIBITED;
	int tossIt = 0;

	if ( directObject && indirectObject && directObject->getBase ( _BCONTAIN ) && 
		indirectObject->getBase ( _BCARRY ) && indirectObject->isOwnedBy ( character ) && 
		directObject->getRoom() == character->room ) 
	{
		if ( indirectObject->getBase (_BHEAD ) ) {
//			disable ( "character %s attempted to give head.", getName() ); 
			goto endGiveVerb;
		}

		//if( indirectObject->hasAffect( _AFF_STAFF ) ) {
		//	roomMgr->sendSystemMsg ( "Sorry", this, "The '%s' is a staff-protected item. You may not give it to another character.", indirectObject->name );
		//	goto endGiveVerb;
		//}

		if ( directObject->getBase ( _BCHARACTER ) && directObject->player && !directObject->player->isNPC && !directObject->player->autoGive ) {
			roomMgr->sendPlayerText ( this, "|c43|Info> %s is not accepting items.", directObject->getName() );
			goto endGiveVerb;
		}

		BContainer *bcontain = (BContainer *) directObject->getBase ( _BCONTAIN );
		int nResult = 0;

		if ( (nResult = bcontain->accepts( indirectObject )) ) {
			result = _WO_ACTION_HANDLED;
			roomMgr->sendPlayerText( this, "|c43|Info> %s found %d item%s that %s took.", directObject->getName(), nResult, nResult > 1 ? "s" : "", directObject->getPronoun( _PRONOUN_HE ) );
		} else {
			indirectObject->makeVisible ( 1 );

			BContainer *bcontain = (BContainer *)indirectObject->getBase ( _BCONTAIN );

			if ( bcontain )
				bcontain->makeVisible ( 1 );

			BTalk *btalk = (BTalk *)directObject->getBase ( _BTALK );
			int needsItem = (directObject->player && directObject->player->isNPC)? 0 : 1;

			if ( quests ) {
				if ( btalk ) {
					LinkedElement *element = quests->head();

					while ( element ) {
						QuestInfo *info = (QuestInfo *)element->ptr();
						Quest *quest = info->quest;

						if ( quest && ( btalk->talkTreeID == quest->id ) ) {
							QuestInfo *myQuest = findQuest ( quest->number );

							if ( myQuest && !myQuest->endTime && myQuest->quest->item ) {
								if ( !strcmp ( myQuest->quest->item->classID, indirectObject->super ) || !strcmp ( myQuest->quest->item->classID, indirectObject->classID ) ) {
									movie.putByte ( _MOVIE_QUEST_COMPLETE );
									movie.putLong ( directObject->servID );
									movie.putString ( myQuest->quest->completed );
									movie.putLong ( character->servID );
									needsItem = 1;
									tossIt = 1;

									myQuest->endTime = getseconds();	
									writeQuestData();

									if ( quest->reward && quest->reward->classID ) {

										WorldObject* reward = character->addObject( quest->reward->classID );
										reward->makeVisible( 1 );

										//WorldObject* reward = new WorldObject;
										//reward->copy ( quest->reward );
										//reward->addToDatabase();
										//reward->forceIn ( character );
										//reward->makeVisible ( 1 );
									} else {
										roomMgr->sendSystemMsg ( "Quest Error", this, "You have encountered a quest with a missing reward object.  Please report this to the proper people." ); 
									}

									character->writeCharacterData();

									break;
								}
							}
						}

						element = element->next();
					}
				}
			}

			if ( needsItem ) {
				result = character->give ( indirectObject, directObject, &movie );
			} else {
				roomMgr->sendPlayerText ( this, "|c43|Info> %s has no use for that!", directObject->getName() );
			}
		}
	}

endGiveVerb:

	postVerbMessage ( result, message->type(), &movie, directObject, indirectObject );

	if ( tossIt )
		roomMgr->destroyObj ( indirectObject, 1, __FILE__, __LINE__ );

	return retVal;
}

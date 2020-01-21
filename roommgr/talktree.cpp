//
// talktree.cpp
//
// classes and code for talk trees
//
// Author: Stephen L. Nichols
//

#include "roommgr.hpp"

LinkedList gTalkTrees;

TalkTreeTopicOwner::TalkTreeTopicOwner() 
{
	type = _TREE_ROOT;
	banner = NULL;
	setBanner ( "Default banner" );
}

TalkTreeTopicOwner::~TalkTreeTopicOwner()
{
	setBanner ( NULL );
}

void TalkTreeTopicOwner::setBanner ( char *txt )
{
	if ( banner ) {
		free ( banner );
		banner = NULL;
	}

	if ( txt ) 
		banner = strdup ( txt );
}

TalkTreeTopic *TalkTreeTopicOwner::addTopic ( char *str )
{
	TalkTreeTopic *topic = new TalkTreeTopic ( str );
	topic->setBanner ( banner );
	topic->owner = this;

	topics.add ( topic );

	return topic;
}

void TalkTreeTopicOwner::buildPacket ( RMPlayer *player, PackedData *packet )
{
	packet->putString ( banner );
	packet->putByte ( topics.size() );

	LinkedElement *element = topics.head();

	while ( element ) {
		TalkTreeTopic *obj = (TalkTreeTopic *)element->ptr();

		if ( obj->filter ) {
			packet->putByte ( obj->filter->evaluate ( player ) );
		} else {
			packet->putByte ( 1 );
		}

		packet->putString ( obj->title );

		element = element->next();
	}
}

TalkTree::TalkTree ( char *str, long theID )
{
	type = _TREE_ROOT;
	id = theID;

	gTalkTrees.add ( this );
	setBanner ( str );
}

TalkTree::~TalkTree()
{
	gTalkTrees.del ( this );
}

TalkTreeTopic::TalkTreeTopic ( char *str )
{
	title = NULL;
	owner = NULL;
	filter = NULL;
	type = _TREE_NODE;

	setTitle ( str );
}

TalkTreeTopic::~TalkTreeTopic()
{
	setTitle ( NULL );
	owner = NULL;
}

void TalkTreeTopic::addText ( char *str )
{
	TalkTreeText *obj = new TalkTreeText ( str );
	text.add ( obj );
}

void TalkTreeTopic::addQuest ( Quest *quest )
{
	text.add ( new TalkTreeQuest ( quest ) );
}

void TalkTreeTopic::addFilter ( FilterObject *obj )
{
	if ( !filter )
		filter = new ConditionFilter;

	filter->add ( obj );
}

void TalkTreeTopic::setTitle ( char *txt )
{
	if ( title ) {
		free ( title );
		title = NULL;
	}

	if ( txt )
		title = strdup ( txt );
}

// 
// condition filters
//

ConditionFilter::ConditionFilter()
{
}

ConditionFilter::~ConditionFilter()
{
}

int ConditionFilter::evaluate ( RMPlayer *player )
{
	LinkedElement *element = head();

	while ( element ) {
		FilterObject *filter = (FilterObject *)element->ptr();

		int result = filter->evaluate ( player );

		if ( !result ) 
			return 0;

		element = element->next();
	}

	return 1;
}

int ConditionFilter::fromStr ( char *str )
{
	int retVal = 0;

	LinkedList *tokens = buildTokenList ( str, " \n\r," );
	LinkedElement *element = tokens->head();

	while ( element ) {
		StringObject *obj = (StringObject *)element->ptr();
		char *cmd = obj->data;

		switch ( cmd[0] ) {

			// completed filter 's<id>:<value>'
			case 'C': 
			case 'c': {
				char *ptr = strrchr ( cmd, ':' );
				int value = 0;

				if ( ptr ) {
					value = atoi ( ptr + 1 );
					*ptr = 0;
				}

				int skill = atoi ( cmd + 1 );

				FilterCompleted *filter = new FilterCompleted;
				filter->skill = skill;
				filter->value = value;

				add ( filter );

				retVal = 1;
			}

			break;

			// skill filter 's<id>:<value>'
			case 'S': 
			case 's': {
				char *ptr = strrchr ( cmd, ':' );
				int value = 0;

				if ( ptr ) {
					value = atoi ( ptr + 1 );
					*ptr = 0;
				}

				int skill = atoi ( cmd + 1 );

				FilterSkill *filter = new FilterSkill;
				filter->skill = skill;
				filter->value = value;

				add ( filter );

				retVal = 1;
			}

			break;

			// alignment filter 'a<coarse align>'
			case 'a': {
				int value = atoi ( obj->data + 1 );

				FilterAlignment *filter = new FilterAlignment;
				filter->alignment = value;

				add ( filter );

				retVal = 1;
			}

			break;

			// quest filter 'q<id>[:<completed>]'
			case 'q': {
				Quest *quest = NULL;
				int checkComplete = 0;

				// strip off completed flag
				char *ptr = strrchr ( cmd, ':' );

				if ( ptr ) {
					checkComplete = atoi ( ptr + 1 );
					*ptr = 0;
				}

				// skip header of command
				cmd++;

				char buf[1024], *bufPtr = buf;

				while ( *cmd && (*cmd != ':') ) {
					if ( *cmd != '-' )
						*bufPtr++ = *cmd; 
				
					cmd++;
				}

				*bufPtr = 0;

				// get quest number

//				quest = findQuest ( atoi ( buf ) );
				quest = findQuest ( atol ( buf ) );

				if ( quest ) {
					FilterQuest *filter = new FilterQuest;
					filter->quest = quest;
					filter->checkComplete = checkComplete;

					add ( filter );

					retVal = 1;
				} else {

					FilterQuest *filter = new FilterQuest;
					filter->checkComplete = checkComplete;

					add ( filter );

					retVal = 1;
				}
			}

			break;
		}

		element = element->next();
	}

	delete tokens;

	return retVal;
}

// 
// filter object
//

FilterObject::FilterObject()
{
}

FilterObject::~FilterObject()
{
}

int FilterObject::evaluate ( RMPlayer *player )
{
	return 1;
}

// 
// FilterQuest: is the player active on or finished with a particular quest?
//

FilterQuest::FilterQuest()
{
	checkComplete = 0;
	quest = NULL;
}

FilterQuest::~FilterQuest()
{
}

int FilterQuest::evaluate ( RMPlayer *player )
{
	if ( !quest ) {
		return 0;
	}

	QuestInfo *info = player->findQuest ( quest->number );

	if ( !info && (checkComplete == 2) ) 
		return 1;

	if ( !info && (checkComplete != 2) ) 
		return 0;

	if ( checkComplete && info->endTime )
		return 1;

	if ( info->endTime ) 
		return 0;

	return 1;
}

//
// FilterCompleted: has the player completed a certain area?
//

FilterCompleted::FilterCompleted()
{
	value = 0;
	skill = 0;
}

FilterCompleted::~FilterCompleted()
{
}

int FilterCompleted::evaluate ( RMPlayer *player )
{
	if ( player->character->getSkill ( skill ) < value )
		return 1;

	return 0;
}

//
// FilterSkill: is the player skilled in a certain area?
//

FilterSkill::FilterSkill()
{
	value = 0;
	skill = 0;
}

FilterSkill::~FilterSkill()
{
}

int FilterSkill::evaluate ( RMPlayer *player )
{
	if ( player->character->getSkill ( skill ) == value )
		return 1;

	return 0;
}

// 
// FilterAlignment: is the player a specific alignment?
//

FilterAlignment::FilterAlignment()
{
	alignment = 1;
}

FilterAlignment::~FilterAlignment()
{
}

int FilterAlignment::evaluate ( RMPlayer *player )
{
	if ( player->character->coarseAlignment() == alignment )
		return 1;

	return 0;
}


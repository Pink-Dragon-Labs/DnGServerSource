//
// quest.cpp
//
// Author: Stephen Nichols
//

// 
// You want comments?  Ha!
//

#include "quest.hpp"

LinkedList gQuestList;

Quest::Quest()
{
	type = _QUEST_FEDEX;
	name = proposal = accepted = declined = reminder = completed = NULL;
	item = reward = NULL;
// old	number = -1;
	id = -1;
	number = -1;

	setAccepted ( "Good luck!" );
	setDeclined ( "Okay." );
	setCompleted ( "Thank you." );
}

Quest::~Quest()
{
	setName ( NULL );
	setProposal ( NULL );
	setReminder ( NULL );
	setCompleted ( NULL );
	setAccepted ( NULL );
	setDeclined ( NULL );
	item = reward = NULL;
// old type = number = -1;
	id = -1;
	number = -1;
	type = -1;
}

char *Quest::setName ( char *str )
{
	if ( name ) {
		free ( name );
		name = NULL;
	}

	if ( str ) 
		name = strdup ( str );

	return name;
}

char *Quest::setProposal ( char *str )
{
	if ( proposal ) {
		free ( proposal );
		proposal = NULL;
	}

	if ( str )
		proposal = strdup ( str );

	return proposal;
}

char *Quest::setReminder ( char *str )
{
	if ( reminder ) {
		free ( reminder );
		reminder = NULL;
	}

	if ( str )
		reminder = strdup ( str );
	
	return reminder;
}

char *Quest::setCompleted ( char *str )
{
	if ( completed ) {
		free ( completed );
		completed = NULL;
	}

	if ( str )
		completed = strdup ( str );

	return completed;
}

char *Quest::setAccepted ( char *str )
{
	if ( accepted ) {
		free ( accepted );
		accepted = NULL;
	}

	if ( str )
		accepted = strdup ( str );

	return accepted;
}

char *Quest::setDeclined ( char *str )
{
	if ( declined ) {
		free ( declined );
		declined = NULL;
	}

	if ( str )
		declined = strdup ( str );

	return declined;
}

// Quest *findQuest ( int number )
Quest *findQuest ( long number )
{
	LinkedElement *element = gQuestList.head();

	while ( element ) {
		Quest *quest = (Quest *)element->ptr();

		if ( quest->number == number ) {
			return quest;
		}
		element = element->next();
	}

	return NULL;
}

QuestInfo::QuestInfo ( Quest *obj )
{
	quest = obj;
	startTime = endTime = 0;
}

QuestInfo::~QuestInfo()
{
	quest = NULL;
	startTime = endTime = 0;
}

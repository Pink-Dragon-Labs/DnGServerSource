//
// quest.hpp
//
// Author: Stephen Nichols
//

#ifndef _QUEST_HPP_
#define _QUEST_HPP_

#include "../global/system.hpp"

#define _QUEST_DELAY 604800
enum {
	_QUEST_FEDEX,
	_QUEST_MAX
};

class WorldObject;
class TalkTree;

class Quest : public ListObject
{
public:
	Quest();
	virtual ~Quest();

	char *setName ( char *str );
	char *setProposal ( char *str );
	char *setReminder ( char *str );
	char *setCompleted ( char *str );
	char *setAccepted ( char *str );
	char *setDeclined ( char *str );

	int type; 
//	int number; 
	long number;
	long id;
	char *name, *proposal, *accepted, *declined, *reminder, *completed;
	WorldObject *item, *reward;

};

class QuestInfo : public ListObject
{
public:
	QuestInfo ( Quest *obj );
	virtual ~QuestInfo();

	Quest *quest;
	int startTime, endTime;
};

extern LinkedList gQuestList;

// Quest *findQuest ( int number );

Quest *findQuest ( long number );

#endif

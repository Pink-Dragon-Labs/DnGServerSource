// 
// talktree.hpp
//
// classes and definitions for talk trees
//
// Author: Stephen L. Nichols
//

#ifndef _TALKTREE_HPP_
#define _TALKTREE_HPP_

#include "../global/system.hpp"

enum {
	_TREE_ROOT,
	_TREE_NODE,
	_TREE_QUEST_PROPOSAL,
	_TREE_QUEST_REMINDER,
};

class TalkTree;
class TalkTreeTopic;
class Quest;
class QuestInfo;

enum { 
	_TREE_TEXT,
	_TREE_QUEST,
};

class TalkTreeData : public ListObject
{
public:
	TalkTreeData() {};
	virtual ~TalkTreeData() {};

	int type;
};

class TalkTreeText : public TalkTreeData
{
public:
	TalkTreeText ( char *str ) {
		type = _TREE_TEXT;
		data = strdup ( str );
	};

	virtual ~TalkTreeText() {
		free ( data );
		data = NULL;
	};

	char *data;
};

class TalkTreeQuest : public TalkTreeData
{
public:
	TalkTreeQuest ( Quest *ptr ) {
		type = _TREE_QUEST;
		quest = ptr;
	};

	virtual ~TalkTreeQuest() {
		quest = NULL;
	}

	Quest *quest;
};

class TalkTreeTopicOwner : public ListObject
{
public:
	TalkTreeTopicOwner();
	virtual ~TalkTreeTopicOwner();

	// add a topic to this 
	virtual TalkTreeTopic *addTopic ( char *str );

	// set the banner for this topic
	virtual void setBanner ( char *txt );

	void buildPacket ( RMPlayer *player, PackedData *packet );

	LinkedList topics;
	char *banner;
	int type;
};

// 
// talk tree filters
//

#undef new 

class ConditionFilter : public LinkedList
{
public:
	ConditionFilter();
	virtual ~ConditionFilter();

	int evaluate ( RMPlayer *player );
	int fromStr ( char *str );

        void *operator new ( size_t size, char* file, int nLine ) { return db_malloc ( size, file, nLine ); }

        void operator delete ( void *ptr ) { free ( ptr ); }
};

#define new new( __FILE__, __LINE__ )

class FilterObject : public ListObject
{
public:
	FilterObject();
	virtual ~FilterObject();

	// does the player match this filter condition?
	virtual int evaluate ( RMPlayer *player );
};

class FilterQuest : public FilterObject
{
public:
	FilterQuest();
	virtual ~FilterQuest();

	virtual int evaluate ( RMPlayer *player );

	Quest *quest;
	int checkComplete;
};

class FilterCompleted : public FilterObject
{
public:
	FilterCompleted();
	virtual ~FilterCompleted();

	virtual int evaluate ( RMPlayer *player );

	int skill, value;
};

class FilterSkill : public FilterObject
{
public:
	FilterSkill();
	virtual ~FilterSkill();

	virtual int evaluate ( RMPlayer *player );

	int skill, value;
};

class FilterAlignment : public FilterObject
{
public:
	FilterAlignment();
	virtual ~FilterAlignment();

	virtual int evaluate ( RMPlayer *player );

	int alignment;
};

class TalkTreeTopic : public TalkTreeTopicOwner 
{
public:
	TalkTreeTopic ( char *str );
	virtual ~TalkTreeTopic();

	// add new text to this topic
	void addText ( char *str );

	// add a new quest to this topic
	void addQuest ( Quest *quest );

	// set the title for this topic
	void setTitle ( char *txt );

	// add a new filter to this topic
	void addFilter ( FilterObject *obj );

	LinkedList text;
	char *title;
	TalkTreeTopicOwner *owner;

	ConditionFilter *filter;
};

class TalkTree : public TalkTreeTopicOwner
{
public:
	TalkTree ( char *str, long theID );
	virtual ~TalkTree();

	long id;
};

extern LinkedList gTalkTrees;

#endif

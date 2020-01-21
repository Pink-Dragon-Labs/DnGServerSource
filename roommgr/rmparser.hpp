#ifndef _RMPARSER_HPP_ 
#define _RMPARSER_HPP_

#include "../global/list.hpp"

class RMPlayer;

int parseCommand ( char *str, RMPlayer *player = NULL );

#define _PLAYER_CMD			1
#define _CONSOLE_CMD			2
#define _GOD_CMD				4
#define _IMPLEMENTOR_CMD	8
#define _GUIDE_CMD			16

#define _GOSSIP_WRITE_CMD				3
#define _GOSSIP_READ_CMD				2
#define _SENSITIVE_CMD			1
#define _INSENSITIVE_CMD		0

typedef struct {
	char *name;
	void (*routine) ( LinkedList *list, char *str, RMPlayer *player );
	int type;
	int sensitive;
} ParseCommand;

extern void *shutdownThread ( void *arg );

#endif

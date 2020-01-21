#include "roommgr.hpp"
#include "actions.hpp"

// 
// utility functions
//

void putMovieText ( WorldObject *object, PackedData *movie, const char *format, ... ) 
{
	char output[20000];

	va_list args;
	va_start ( args, format );
	vsprintf ( sizeof ( output ), output, format, args );

	if ( output[0] ) {
		movie->putByte ( _MOVIE_TEXT );
		movie->putLong ( object? object->servID : -1 );
		movie->putString ( output );
	}
}

int savedFromSpell ( WorldObject *object, int modifier )
{
	save_t save = gSpellSaveTable[object->profession()];
	int saveNumber = (int)((double)save.basePercent + ((double)object->level * save.increment)) + modifier;

//	logDisplay ( "%d chance of saving from spell", saveNumber );

	int roll = random ( 0, 99 );
	return roll < saveNumber;	
}

int savedFromAttack ( WorldObject *object, int modifier )
{
	save_t save = gAttackSaveTable[object->profession()];
	int saveNumber = (int)((double)save.basePercent + ((double)object->level * save.increment)) + modifier;

//	logDisplay ( "%d chance of saving from attack", saveNumber );

	int roll = random ( 0, 99 );
	return roll < saveNumber;	
}

int savedFromParalysis ( WorldObject *object, int modifier )
{
	save_t save = gParalysisSaveTable[object->profession()];
	int saveNumber = (int)((double)save.basePercent + ((double)object->level * save.increment)) + modifier;

//	logDisplay ( "%d chance of saving from paralysis", saveNumber );

	int roll = random ( 0, 99 );
	return roll < saveNumber;	
}

int savedFromPoison ( WorldObject *object, int modifier )
{
	save_t save = gPoisonSaveTable[object->profession()];
	int saveNumber = (int)((double)save.basePercent + ((double)object->level * save.increment)) + modifier;

//	logDisplay ( "%d chance of saving from poison", saveNumber );

	int roll = random ( 0, 99 );
	return roll < saveNumber;	
}

int rand100 ( void ) 
{
	static char table[100000];
	static int index = 0;
	static int initted = 0;

	if ( !initted ) {
		for ( int i=0; i<100000; i++ )
			table[i] = (char)random ( 0, 100 );

		initted = 1;
	}

	int val = table[index];
	index++;

	if ( index == 100000 )
		index = 0;

	return val;
}

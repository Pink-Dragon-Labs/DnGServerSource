/*
	DiceRoll class
	author: Stephen Nichols
*/

#include "dice.hpp"
#include "../global/system.hpp"

DiceRoll::DiceRoll()
{
	diceCount = 1;
	diceSize = 6;
	rollModifier = 0;
}

DiceRoll::DiceRoll ( char *str )
{
	fromString ( str );
}

DiceRoll::DiceRoll ( int theCount, int theSize, int theModifier )
{
	diceCount = theCount;
	diceSize = theSize;
	rollModifier = theModifier;
}

int DiceRoll::roll ( void )
{
	return random ( diceCount, diceCount * diceSize ) + rollModifier;
}

void DiceRoll::fromString ( char *str )
{
	char *dup = strdup ( str ), *ptr = dup, *token = dup;

	diceCount = 1;
	diceSize = 6;
	rollModifier = 0;

	// look for 'd'
	while ( *ptr ) {
		if ( *ptr == 'd' ) {
			*ptr++ = 0;
			break;
		}

		ptr++;
	}

	if ( *ptr ) {
		diceCount = atoi ( token );
		token = ptr;

		// look for '+'
		while ( *ptr ) {
			if ( *ptr == '+' ) {
				*ptr++ = 0;
				break;
			}

			ptr++;
		}

		if ( *token ) {
			diceSize = atoi ( token );
			token = ptr;
		}

		if ( *token )
			rollModifier = atoi ( token );
	}

	free ( dup );
}

int rollDice ( const char *format, ... )
{
	char str[1024];
	va_list args;

	va_start ( args, format );
	vsprintf ( sizeof ( str ), str, format, args );

	DiceRoll dice;
	dice.fromString ( str );

	return dice.roll();
}

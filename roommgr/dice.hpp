/*
	DiceRoll class
	author: Stephen Nichols
*/

#ifndef _DICE_HPP_
#define _DICE_HPP_

class DiceRollData
{
public:
	int diceCount, diceSize, rollModifier;
};

// This class represents a potential roll of the dice.  When rolled, the
// class will return a number between rollModifier and
// rollModifier + (diceCount * diceSize)
//
class DiceRoll : public DiceRollData
{
public:
	DiceRoll();
	DiceRoll ( char *str );
	DiceRoll ( int theCount, int theSize, int theModifier );

	// handle comparing two DiceRoll classes
	int operator == ( DiceRoll &dice ) {
		return diceCount == dice.diceCount && diceSize == dice.diceSize && rollModifier == dice.rollModifier;
	}

	// handle adding two DiceRoll classes
	DiceRoll operator + ( DiceRoll &dice ) {
		return DiceRoll ( diceCount + dice.diceCount, diceSize + dice.diceSize, rollModifier + dice.rollModifier );
	}

	// handle subtracting two DiceRoll classes
	DiceRoll operator - ( DiceRoll &dice ) {
		return DiceRoll ( diceCount - dice.diceCount, diceSize - dice.diceSize, rollModifier - dice.rollModifier );
	}

	// divide a DiceRoll by an integer
	DiceRoll operator / ( int &val ) {
		return DiceRoll ( diceCount / val, diceSize / val, rollModifier / val );
	}

	// set this dice roll based on a DiceRollData
	DiceRoll &operator = ( DiceRollData &data ) {
		diceCount = data.diceCount;
		diceSize = data.diceSize;
		rollModifier = data.rollModifier;

		return *this;
	}

	// roll the dice and return the result
	int roll ( void );

	// set the dice rolling parameters via a string like "10d6+13" 
	void fromString ( char *string );
};

int rollDice ( const char *format, ... );

#endif

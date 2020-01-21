#ifndef _VALIDATE_HPP_
#define _VALIDATE_HPP_

#include "ctype.h"
#include "string.h"

#include "malloc.hpp"

// this verifies the given string's contents based on the provided parameters
#define _STRVAL_SPACE 			(1 << 0)
#define _STRVAL_TAB				(1 << 1)
#define _STRVAL_ALPHA			(1 << 2)
#define _STRVAL_PUNCT			(1 << 3)
#define _STRVAL_NUMERIC			(1 << 4)
#define _STRVAL_BLANK			(1 << 5)

int validateString ( char *string, int mask );

// this validates the given e-mail address to make sure it is a valid format
int validateEmailAddress ( char *string );

// this validates the given state code
int validateState ( char *string );

// this validates the given zip code to be 5 digits or 5-4 digits.
int validateZip ( char *string );

// this validates the given CVV number must be 3 or 4 digit number
int validateCVV ( char *string );

// this validates the given address to start with numbers
int validateAddress ( char *string );

// this function validates the given credit card number and returns if it is valid
int validateCreditCard ( char *number );

#endif

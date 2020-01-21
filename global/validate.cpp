#include "validate.hpp"

// this validates the given string based on the provided mask
int validateString ( char *string, int mask ) {
	char *ptr = string;

	// don't allow balnk strings that are not specified
	if ( !*ptr && !(mask & _STRVAL_BLANK) )
		return 0;

	while ( *ptr ) {
		char ch = *ptr;

		// check for single space first
		if ( ch == ' ' && !(mask & _STRVAL_SPACE) )
			return 0;

		else if ( ch == '\t' && !(mask & _STRVAL_TAB) )
			return 0;

		else if ( isalpha ( ch ) && !(mask & _STRVAL_ALPHA) )
			return 0;

		else if ( isdigit ( ch ) && !(mask & _STRVAL_NUMERIC) )
			return 0;

		else if ( ispunct ( ch ) && !(mask & _STRVAL_PUNCT) )
			return 0;

		ptr++;
	}

	return 1;
}

// this validates the given email address
int validateEmailAddress ( char *string ) {
	char *theString = strdup ( string );
	int retVal = 1;

	// this is the pointer we use to traverse the string
	char *ptr = NULL;

	// look for the presence of an @ symbol
	int atCount = 0;
	char *atPtr = NULL;
	ptr = theString;

	while ( *ptr ) {
		if ( *ptr == '@' ) {
			atPtr = ptr;
			atCount++;
		}

		ptr++;
	}

	// there must be exactly one @ symbol
	if ( atCount == 1 ) {
		// separate the address into its components
		char *user = theString;
		char *host = (char *)(atPtr+1);
		*atPtr = 0;

		// check the length of the user name
		if ( strlen ( user ) ) {
			// check the host component
			if ( strlen ( host ) > 2 ) {
				char *dotPtr = strchr ( host, '.' );

				if ( dotPtr && (strlen ( dotPtr ) > 1) ) {
					if ( (int)(dotPtr - host) < 1 )
						retVal = 0;	
				} else {
					retVal = 0;
				}
			} else {
				retVal = 0;
			}
		} else {
			retVal = 0;
		}
	} else {
		retVal = 0;
	}

	free ( theString );
	return retVal;
}

// this validates the given state code
int validateState ( char *string ) {
	if ( strlen( string ) != 2 )
		return 0;

	string[0] = toupper( string[0] );
	string[1] = toupper( string[1] );

	switch ( string[0] ) {
		case 'A':
			switch ( string[1] ) {
				case 'A':	//	Armed Forces Americas
				case 'E':	//	Armed Forces (Africa, Canada, Europe, Middle East)
				case 'L':	//	Alabama
				case 'K':	//	Alaska
				case 'P':	//	Armed Forces Pacific
				case 'R':	//	Arkansas
				case 'S':	//	American Samo
				case 'Z':	//	Arizona
					return 1;
				default:
					return 0;
			}

			break;
		case 'C':
			switch ( string[1] ) {
				case 'A':	//	California
				case 'O':	//	Colorado
				case 'T':	//	Connecticut
					return 1;
				default:
					return 0;
			}

			break;
		case 'D':
			switch ( string[1] ) {
				case 'C':	//	District of Columbia
				case 'E':	//	Delaware
					return 1;
				default:
					return 0;
			}

			break;
		case 'F':
			switch ( string[1] ) {
				case 'L':	//	Florida
				case 'M':	//	Federated States of Micronesia
					return 1;
				default:
					return 0;
			}

			break;
		case 'G':
			switch ( string[1] ) {
				case 'A':	//	Georgia
				case 'U':	//	Guma
					return 1;
				default:
					return 0;
			}

			break;
		case 'H':
			switch ( string[1] ) {
				case 'I':	//	Hawaii
					return 1;
				default:
					return 0;
			}

			break;
		case 'I':
			switch ( string[1] ) {
				case 'A':	//	Iowa
				case 'D':	//	Idaho
				case 'L':	//	Illinois
				case 'N':	//	Indiana
					return 1;
				default:
					return 0;
			}

			break;
		case 'K':
			switch ( string[1] ) {
				case 'S':	//	Kansas
				case 'Y':	//	Kentucky
					return 1;
				default:
					return 0;
			}

			break;
		case 'L':
			switch ( string[1] ) {
				case 'A':	//	Louisianna
					return 1;
				default:
					return 0;
			}

			break;
		case 'M':
			switch ( string[1] ) {
				case 'A':	//	Massachusetts
				case 'D':	//	Maryland
				case 'E':	//	Maine
				case 'H':	//	Marshall Islands
				case 'I':	//	Michigan
				case 'N':	//	Minnesota
				case 'O':	//	Missouri
				case 'P':	//	Northern Mariana Islands
				case 'S':	//	Mississippi
				case 'T':	//	Montana
					return 1;
				default:
					return 0;
			}

			break;
		case 'N':
			switch ( string[1] ) {
				case 'C':	//	North Carolina
				case 'D':	//	North Dakota
				case 'E':	//	Nebraska
				case 'H':	//	New Hampshire
				case 'J':	//	New Jersey
				case 'M':	//	New Mexico
				case 'V':	//	Nevada
				case 'Y':	//	New York
					return 1;
				default:
					return 0;
			}

			break;
		case 'O':
			switch ( string[1] ) {
				case 'H':	//	Ohio
				case 'K':	//	Oklahoma
				case 'R':	//	Oregon
					return 1;
				default:
					return 0;
			}

			break;
		case 'P':
			switch ( string[1] ) {
				case 'A':	//	Pennsylvania
				case 'R':	//	Puerto Rico
				case 'W':	//	Palau
					return 1;
				default:
					return 0;
			}

			break;
		case 'R':
			switch ( string[1] ) {
				case 'I':	//	Rhode Island
					return 1;
				default:
					return 0;
			}

			break;
		case 'S':
			switch ( string[1] ) {
				case 'C':	//	South Carolina
				case 'D':	//	South Dakota
					return 1;
				default:
					return 0;
			}

			break;
		case 'T':
			switch ( string[1] ) {
				case 'N':	//	Tennessee
				case 'X':	//	Texas
					return 1;
				default:
					return 0;
			}

			break;
		case 'U':
			switch ( string[1] ) {
				case 'T':	//	Utah
					return 1;
				default:
					return 0;
			}

			break;
		case 'V':
			switch ( string[1] ) {
				case 'A':	//	Virginia
				case 'I':	//	Virgin Islands
				case 'T':	//	Vermont
					return 1;
				default:
					return 0;
			}

			break;
		case 'W':
			switch ( string[1] ) {
				case 'A':	//	Washington
				case 'I':	//	Wisconsin
				case 'V':	//	West Virginia
				case 'Y':	//	Wyoming
					return 1;
				default:
					return 0;
			}

			break;
		default:
			return 0;
	}

	return 1;
}

// this validates the given zip code to be 5 digits or 5-4 digits.
int validateZip ( char *string ) {
	int nSize = strlen( string );

	if ( nSize == 5 ) {
		if ( isdigit( string[0] ) && isdigit( string[1] ) && isdigit( string[2] ) && isdigit( string[3] ) && isdigit( string[4] ) )
			return 1;
	}

	if ( nSize == 10 ) {
		if ( isdigit( string[0] ) && isdigit( string[1] ) && isdigit( string[2] ) && isdigit( string[3] ) && isdigit( string[4] ) && string[5] == '-' && isdigit( string[6] ) && isdigit( string[7] ) && isdigit( string[8] ) && isdigit( string[9] ) )
			return 1;
	}

	return 0;
}

// this validates the given CVV number must be 3 or 4 digit number
int validateCVV ( char *string ) {
	int nSize = strlen( string );

	if ( nSize == 3 ) {
		if ( isdigit( string[0] ) && isdigit( string[1] ) && isdigit( string[2] ) )
			return 1;
	}

	if ( nSize == 4 ) {
		if ( isdigit( string[0] ) && isdigit( string[1] ) && isdigit( string[2] ) && isdigit( string[3] ) )
			return 1;
	}

	return 0;
}

// this validates the given address to start with numbers
int validateAddress ( char *string ) {
	if ( !isdigit( string[0] ) )
		return 0;

	return 1;
}

// this function validates the given credit card number
int validateCreditCard ( char *number ) {
	static char *prefixes[] = { "4", "51", "52", "53", "54", "55", "34", "37", NULL };
	static int sums[] = { 0, 2, 4, 6, 8, 1, 3, 5, 7, 9 };

	// check the length of the number
	int length = strlen ( number );

	// all known credit card numbers must be between 13 and 16 digits
	if ( length < 13 || length > 16 )
		return 0;

	// check to make sure the number is all digits
	char *ptr = number;

	while ( *ptr ) {
		if ( !isdigit ( *ptr ) )
			return 0;

		ptr++;
	}

	// validate the number prefix 
	int prefix = 0;

	while ( prefixes[prefix] ) {
		// compare the prefix...
		if ( !strncmp ( number, prefixes[prefix], strlen ( prefixes[prefix] ) ) )
			break;

		prefix++;
	}

	// no known prefix value...
	if ( !prefixes[prefix] )
		return 0;

	// perform the MOD10 check on the digits
	int sum = 0, index = length-1;
	ptr = &number[length];

	// check every other digit and create the sum
	while ( index >= 0 ) {
		// skip every other digit
		index--;

		if ( index < 0 )
			break; 

		int digit = number[index] - '0';
		sum += sums[digit]; 

		index--;
	}

	// now, step through the even digits and add them too
	index = length-1;

	while ( index >= 0 ) {
		int digit = number[index] - '0';
		sum += digit;
		index-=2;
	}

	return !(sum%10);
}

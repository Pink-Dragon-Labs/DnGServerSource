#ifndef _GENERATOR_HPP_
#define _GENERATOR_HPP_

#include "sql.hpp"
#include "tools.hpp"
#include <string.h>
#include <stdio.h>

// this function generates a random string
char *genRandomStr ( void );

// this function generates a login name and stores it in the database
int genLoginName ( SQLDatabase *sql, char **namePtr );

// this function generates a 10 digit order number and stores it in the database
char *genOrderNumber ( SQLDatabase *sql );

// this class handles referral id from players or sites
class referralID {
protected:
	int			m_nValue;
	char		m_sValue[8];

	static char	m_sKey[51];
public:	
	referralID( int nValue );
	referralID( char* pValue );

	int getnID();
	char* getpID();

	bool setID( int nValue );
	bool setID( char* pValue );
};

#endif

// This file contains all of the global string generation routines.

#include "generator.hpp"

// this function generates a random string
char *genRandomStr ( void )
{
	static char charTbl[] = { '2','3','4','5','6','7','8','9','a','b','c','d','e','?','g','h','k','j','m','n','p','q','r','s','t','u','v','w','x','y','z','-' };

	char string[7];
	string[6] = 0;

	for ( int i=0; i<6; i++ )
		string[i] = charTbl[random ( 0, (sizeof ( charTbl ) / sizeof ( char )) - 1 )];

	return strdup ( string );
}

// this function generates a login name and stores it in the database
int genLoginName ( SQLDatabase *sql, char **namePtr )
{
	for ( ;; ) {
		char *name = genRandomStr();

		// try inserting the login name
		SQLResponse *response = sql->query ( "insert into loginNames set id=0, name='%s'", name );

		if ( response )
			delete response;

		if ( !sql->error ) {
			*namePtr = name;
			return sql->lastInsertID();
		} else {
			free ( name );
		}
	}

	return 0;
}

// this function generates a 10 digit order number and stores it in the database
char *genOrderNumber( SQLDatabase *sql ) {
	static char tbl[] = "346789ABCDEFGHJKLMNPQRSTUWXYZ";

	char number[11];
	number[10] = 0;

	for ( ;; ) {
		for ( int i=0; i<10; i++ )
			number[i] = tbl[random ( 0, 28 )];

		// see if the number already exists in the database
		SQLResponse *response = sql->query ( "insert into productOrders set id=0, number='%s', accountOwnerID=-1, accountID=-1, date=now()", number );

		if ( response )
			delete response;

		if ( !sql->error )
			break;
	}

	return strdup ( number );
}

// --------------------------------------------------------------------------------------
char referralID::m_sKey[51] = "QWERTYUOPASDFGHJKLZXCVBNMQWERTYUOPASDFGHJKLZXCVBNM";

referralID::referralID( int nValue ) {
	setID( nValue );
}

referralID::referralID( char* pValue ) {
	if ( strlen( pValue ) != 7 ) {
		setID( 0 );
	} else {
		setID( pValue );
	}
}

int referralID::getnID() {
	return m_nValue;
}

char* referralID::getpID() {
	return m_sValue;
}

bool referralID::setID( int nValue ) {
	int nDivisor;
	int nRemainder = 0;

	m_nValue = nValue;

	m_sValue[7] = 0;

	for (int i = 6;i > -1;i--) {
		nDivisor = nValue / 25;
		m_sValue[i] = m_sKey[ ( nRemainder = ( nValue - ( nDivisor * 25 ) ) ) + i ];
		nValue = nDivisor;
	}

	return true;
}

bool referralID::setID( char* pValue ) {
	char nValue[7];
	char* pMatch;

	if ( strlen( pValue ) != 7 )
		return false;

	m_sValue[7] = 0;

	for (int i = 6;i > -1;i--) {
		m_sValue[i] = pValue[i];

		if ( pMatch = strchr( &m_sKey[i], pValue[i] ) ) {
			nValue[i] = int ( pMatch ) - int ( &m_sKey[i] );
		} else {
			setID( 0 );
			return false;
		}
	}

	m_nValue = 0;

	for (int j = 0;j < 7;j++) {
		m_nValue = m_nValue * 25 + nValue[j];
	}

	return true;
}


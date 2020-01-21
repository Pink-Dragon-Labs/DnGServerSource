#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <crypt.h>
#include <rpc/des_crypt.h>

#include "cencryptionstring.hpp"

char CEncryptionString::m_sKey[9] = "snRasaJ";

//      Copy Encrypted String

#define CopyEncryptedString \
ecb_crypt( m_sKey, (char*) m_sText, 104, DES_ENCRYPT ); \
for (int i = 0;i < 104;i++) {\
	sprintf( &m_sEncryptedString[ ( i * 2 ) ], "%02X", m_sText[ i ] );\
} \
ecb_crypt( m_sKey, (char*) m_sText, 104, DES_DECRYPT ); 

CEncryptionString::CEncryptionString() {
	for (int i = 0;i < 26;i++)
		m_nRandom[i] = random();

	m_data.nAccountID = 0;
	m_data.nLoginNameID = 0;
	m_data.nAccountOwnerID = 0;
	m_data.nLoginNameLen = 0;
	m_data.nPasswordLen = 0;
	m_data.nSecretLen = 0;
	m_data.sLoginName[0] = 0;
	m_data.sPassword[0] = 0;
	m_data.sSecret[0] = 0;
	m_data.nTest = 0;

	CopyEncryptedString
}

CEncryptionString::CEncryptionString( SQLDatabase* sql, int nAccountID ) {
	for (int i = 0;i < 26;i++)
		m_nRandom[i] = random();

	m_data.nAccountID = nAccountID;

	if ( sql ) {
		SQLResponse* qAccount = sql->query( "select loginNames.id, loginNames.name, password, accountOwners.maidenName, accountOwners.id from accounts, loginNames, accountOwners where accounts.id = %d and accountOwners.accountID = accounts.id and accounts.loginNameID = loginNames.id order by accountOwners.date DESC limit 1", m_data.nAccountID );

		if ( qAccount ) {
			m_data.nLoginNameID = atoi( qAccount->table( 0, 0 ) );

			m_data.nLoginNameLen = strlen( qAccount->table( 0, 1 ) );
			strcpy( m_data.sLoginName, qAccount->table( 0, 1 ) );

			m_data.nPasswordLen = strlen( qAccount->table( 0, 2 ) );
			strcpy( m_data.sPassword, qAccount->table( 0, 2 ) );

			m_data.nSecretLen = strlen( qAccount->table( 0, 3 ) );
			strcpy( m_data.sSecret, qAccount->table( 0, 3 ) );

			m_data.nAccountOwnerID = atoi( qAccount->table( 0, 4 ) );

			delete qAccount;
		} else {
			qAccount = sql->query( "select loginNames.id, loginNames.name, password from accounts, loginNames where accounts.id = %d and accounts.loginNameID = loginNames.id", m_data.nAccountID );

			if ( qAccount ) {
	                        m_data.nLoginNameID = atoi( qAccount->table( 0, 0 ) );

                        	m_data.nLoginNameLen = strlen( qAccount->table( 0, 1 ) );
                	        strcpy( m_data.sLoginName, qAccount->table( 0, 1 ) );

        	                m_data.nPasswordLen = strlen( qAccount->table( 0, 2 ) );
	                        strcpy( m_data.sPassword, qAccount->table( 0, 2 ) );

				m_data.nSecretLen = 0;
				m_data.sSecret[0] = 0;

	                        delete qAccount;
			} else {
				m_data.nLoginNameID = 0;
				m_data.nAccountOwnerID = 0;
				m_data.nLoginNameLen = 0;
				m_data.nPasswordLen = 0;
				m_data.nSecretLen = 0;
				m_data.sLoginName[0] = 0;
				m_data.sPassword[0] = 0;
				m_data.sSecret[0] = 0;
				m_data.nTest = 0;
			}
		}
	} else {
		m_data.nLoginNameID = 0;
		m_data.nAccountOwnerID = 0;
		m_data.nLoginNameLen = 0;
		m_data.nPasswordLen = 0;
		m_data.nSecretLen = 0;
		m_data.sLoginName[0] = 0;
		m_data.sPassword[0] = 0;
		m_data.sSecret[0] = 0;
		m_data.nTest = 0;
	}

	CopyEncryptedString
}

CEncryptionString::CEncryptionString( char* pString) {
	char* pDigits = "0123456789ABCDEF";

	strncpy( m_sEncryptedString, pString, 208 );

	for (int i = 0;i < 104;i++) {
		m_sText[ i ] = ( strchr( pDigits, pString[ ( i * 2 ) ] ) - pDigits ) * 16 + ( strchr( pDigits, pString[ ( i * 2 ) + 1 ] ) - pDigits );
	}

	ecb_crypt( m_sKey, (char*) m_sText, 104, DES_DECRYPT );

	if ( ( m_data.nLoginNameLen > 32 ) || ( m_data.nLoginNameLen < 1 ) ||
		( m_data.nPasswordLen > 16 ) || ( m_data.nPasswordLen < 1 ) ||
		( m_data.nLoginNameLen != strlen( m_data.sLoginName ) ) ||
		( m_data.nPasswordLen != strlen( m_data.sPassword ) ) ) {

		memset( m_sText, 0, sizeof( m_data ) );

		CopyEncryptedString
	}
}

CEncryptionString::CEncryptionString( int nAccountID, char* pPassword, int nLoginNameID, char* pLoginName, int nAccountOwnerID, char* pSecret ) {
	if ( ( strlen( pPassword ) > 16 ) ||
		( strlen( pLoginName ) > 16 ) ||
		( strlen( pSecret ) > 32 ) ) {

		memset( m_sText, 0, sizeof( m_data ) );

		CopyEncryptedString
	} else {
		for (int i = 0;i < 26;i++)
			m_nRandom[i] = random();

		m_data.nAccountID = nAccountID;
		m_data.nLoginNameID = nLoginNameID;
		m_data.nAccountOwnerID = nAccountOwnerID;
		m_data.nLoginNameLen = strlen( pLoginName );
		m_data.nPasswordLen = strlen( pPassword );
		m_data.nSecretLen = strlen( pSecret );
		strcpy( m_data.sLoginName, pLoginName );
		strcpy( m_data.sPassword, pPassword );
		strcpy( m_data.sSecret, pSecret );

		CopyEncryptedString
	}
}

void CEncryptionString::setPassword( char* pPassword ) {
	if ( strlen( pPassword ) > 16 ) {
                memset( m_sText, 0, sizeof( m_data ) );

                CopyEncryptedString
	
		return;
	}

	m_data.nPasswordLen = strlen( pPassword );
	strcpy( m_data.sPassword, pPassword );

	CopyEncryptedString
}

void CEncryptionString::setLoginName( char* pLoginName ) {
	if ( strlen( pLoginName ) > 32 ) {

                memset( m_sText, 0, sizeof( m_data ) );

                CopyEncryptedString

		return;
	}

	m_data.nLoginNameLen = strlen( pLoginName );
	strcpy( m_data.sLoginName, pLoginName );

	CopyEncryptedString
}

void CEncryptionString::setSecret( char* pSecret ) {
        if ( strlen( pSecret ) > 32 ) {

                memset( m_sText, 0, sizeof( m_data ) );

                CopyEncryptedString

                return;
        }

        m_data.nSecretLen = strlen( pSecret );
        strcpy( m_data.sSecret, pSecret );

        CopyEncryptedString
}

void CEncryptionString::setAccountID( int nID, SQLDatabase* sql ) {
	m_data.nAccountID = nID;

	if ( NULL != sql ) {
		SQLResponse* qAccount = sql->query( "select loginNames.id, loginNames.name, password, accountOwners.maidenName, accountOwners.id from accounts, loginNames, accountOwners where accounts.id = %d and accountOwners.accountID = accounts.id and accounts.loginNameID = loginNames.id order by accountOwners.date DESC limit 1", m_data.nAccountID );

		if ( qAccount ) {
			m_data.nLoginNameID = atoi( qAccount->table( 0, 0 ) );

			m_data.nLoginNameLen = strlen( qAccount->table( 0, 1 ) );
			strcpy( m_data.sLoginName, qAccount->table( 0, 1 ) );

			m_data.nPasswordLen = strlen( qAccount->table( 0, 2 ) );
			strcpy( m_data.sPassword, qAccount->table( 0, 2 ) );

	        m_data.nSecretLen = strlen( qAccount->table( 0, 3 ) );
		    strcpy( m_data.sSecret, qAccount->table( 0, 3 ) );

			m_data.nAccountOwnerID = atoi( qAccount->table( 0, 4 ) );

			delete qAccount;
		}
	}

	CopyEncryptedString
}

void CEncryptionString::setLoginNameID( int nID ) {
	m_data.nLoginNameID = nID;

	CopyEncryptedString
}

void CEncryptionString::setAccountOwnerID( int nID ) {
        m_data.nAccountOwnerID = nID;

        CopyEncryptedString
}

void CEncryptionString::setTest( int nValue ) {
        m_data.nTest = nValue;

        CopyEncryptedString
}

bool CEncryptionString::validate( SQLDatabase* sql ) {
	SQLResponse* qAccount = sql->query( "select accounts.id from accounts, loginNames where accounts.id = %d and accounts.loginNameID = %d and accounts.password = '%s' and accounts.loginNameID = loginNames.id and loginNames.name = '%s'", m_data.nAccountID, m_data.nLoginNameID, m_data.sPassword, m_data.sLoginName );

	if ( qAccount ) {
		delete qAccount;

		qAccount = sql->query( "select id, maidenName from accountOwners where accountID = %d order by date DESC limit 1", m_data.nAccountID );

		if ( qAccount ) {
			if ( atoi( qAccount->table( 0, 0 ) ) == m_data.nAccountOwnerID ) {
				if ( !strcmp( qAccount->table( 0, 1 ), m_data.sSecret ) ) {
					delete qAccount;

					return true;
				}
			}

			delete qAccount;
		} else {
			return true;
		}
	}

	// Clear everything as BAD!!
	m_data.nAccountID = 0;
	m_data.nLoginNameID = 0;
	m_data.nAccountOwnerID = 0;
	m_data.nLoginNameLen = 0;
	m_data.nPasswordLen = 0;
	m_data.nSecretLen = 0;
	m_data.sLoginName[0] = 0;
	m_data.sPassword[0] = 0;
	m_data.sSecret[0] = 0;
	m_data.nTest = 0;

	CopyEncryptedString

	return false;
}


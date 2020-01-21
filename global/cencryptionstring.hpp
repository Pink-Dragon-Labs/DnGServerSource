#ifndef CENCRYPTIONSTRING_HPP
#define CENCRYPTIONSTRING_HPP

#include "sql.hpp"

class CEncryptionString {
protected:
	struct LoginPassword {
	        int				nAccountID;
        	int				nLoginNameID;
			int				nAccountOwnerID;
	        char			sPassword[17];
	        unsigned char	nPasswordLen;
        	char			sLoginName[33];
        	unsigned char	nLoginNameLen;
			char			sSecret[33];
			unsigned char	nSecretLen;
			int				nTest;
	};

	union {
			LoginPassword   m_data;
	        unsigned char   m_sText[104];
        	int             m_nRandom[26];
	};

	char	m_sEncryptedString[256];

	static char m_sKey[9];

public:
	CEncryptionString();
	CEncryptionString( SQLDatabase* sql, int nID );
	CEncryptionString( char* pString );
	CEncryptionString( int nAccountID, char* pPassword, int nLoginNameID, char* pLoginName, int nAccountOwnerID, char* pSecret );

	char* getEncryptedString() { return m_sEncryptedString; }

	char* getPassword() { return m_data.sPassword; }
	void setPassword( char* pPassword );

	char* getLoginName() { return m_data.sLoginName; }
	void setLoginName( char* pLoginName );

	char* getSecret() { return m_data.sSecret; }
	void setSecret( char* pSecret );

	int getAccountID() { return m_data.nAccountID; }
	void setAccountID( int nID, SQLDatabase* sql = NULL );

	int getLoginNameID() { return m_data.nLoginNameID; }
	void setLoginNameID( int nID );

    int getAccountOwnerID() { return m_data.nAccountOwnerID; }
    void setAccountOwnerID( int nID );

    int getTest() { return m_data.nTest; }
    void setTest( int nValue );

	bool validate( SQLDatabase* sql );
};

#endif

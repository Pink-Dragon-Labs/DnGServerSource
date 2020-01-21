//
// SQL.HPP
//
// Mini-SQL interface class library.
//
// Author: Stephen Nichols
//

#ifndef _SQL_HPP_
#define _SQL_HPP_

#include <mysql.h>

// 
// SQLResponse: This class represents and provides interface to responses from
// the SQL server.
//

class SQLResponse
{
	char ***rowData;
	int **lengths;

public:
	SQLResponse ( MYSQL *connection );
	virtual ~SQLResponse();

	inline char *table ( int r, int c ) { return rowData[r][c]; };
	inline int length ( int r, int c ) { return lengths[r][c]; };

	// actual response information
	int rows, cols, size;
	char *error;
};

//
// SQLDatabase: This class provides the interface to the Mini-SQL API.
//

class SQLDatabase
{
	MYSQL *connection;

public:
	SQLDatabase ( char *host, char *user, char *password, char *db );
	virtual ~SQLDatabase();

	// post an SQL query
	SQLResponse *query ( const char *format, ... );

	// get the last insert id
	int lastInsertID ( void );

	// the mysql error string
	char *error;
};

#endif

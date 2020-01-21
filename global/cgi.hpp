#ifndef _CGI_HPP_
#define _CGI_HPP_

#define _MAX_CGI_PARAMS 512

//
// This class reads all CGI paramaters from stdin and categorizes them.
//
class CGIParams 
{
	char *_names[_MAX_CGI_PARAMS];
	char *_values[_MAX_CGI_PARAMS];
public:	

	CGIParams();
	CGIParams( char *queryString, char terminator = '&' );
	virtual ~CGIParams();

	void parseQueryString ( char *queryString, char terminator = '&' );

	// get the value of a parameter
	char *get ( char *name );

	void dump();

	// set a value
	int set ( char* name, int nValue );
	int set ( char* name, char* pValue );
};

#endif

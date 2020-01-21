//
// configmgr
//
// This file contains the ConfigMgr class and supporting structures.
//
// author: Stephen Nichols
//

#ifndef _CONFIGMGR_HPP_
#define _CONFIGMGR_HPP_

// define the maximum number of variables in a config file
#define _CONFIGMGR_MAX_ENTRIES 512

//
// ConfigMgr: This class manages generic configuration files.  The parser
// that this is based on is very sensitive.  The format of a config file
// is "var value".  The parser expects exactly one space between the variable
// and value.  If more than one space is between them, then the space will
// be considered part of the value.  Comments are denoted by '#' and may be
// embedded anywhere on the line.  Leading and trailing whitespace is trimmed.
//

class ConfigMgr
{
public:
	ConfigMgr();
	virtual ~ConfigMgr();

	// this member is called to load a configuration file
	int load ( const char *name, ... );

	// this member is called to unload a configuration file
	void unload ( void );

	// this member is called to get a value from the config file
	char *get ( char *varName, int doFatal = 1 );

	// this is the number of loaded variables
	int varCount;

	// this is the table of variable names 
	char *variables[_CONFIGMGR_MAX_ENTRIES];

	// this is the table of variable values
	char *values[_CONFIGMGR_MAX_ENTRIES];
};

#endif

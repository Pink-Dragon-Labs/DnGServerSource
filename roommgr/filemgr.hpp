//
// filemgr
//
// This file contains the FileMgr and FMRequest classes.
//
// author: Stephen Nichols
//

#ifndef _FILEMGR_HPP_
#define _FILEMGR_HPP_

#include "../global/list.hpp"
#include "../global/filemgrdefs.hpp"
#include "../global/ipcclient.hpp"

enum cbDefines {
	cbNone = 0,
	cbReadWhatsNew,
	cbReadMailFile,
};

//
// FileMgr
//
// This class handles interfacing between a remote FILEMGR process and
// this server.
//

class FileMgr : public IPCClient
{
public:
	FileMgr();
	virtual ~FileMgr();

	// this member is called to write a file
	void writeFile ( char *name, void *data, int size, cbDefines callback, int context = 0 );

	// this member is called to write a file via PackedData
	void writeFile ( char *name, PackedData *data, cbDefines callback, int context = 0 );

	// this member is called to exclusive create a file
	void exclusiveCreate ( char *name, cbDefines callback, int context = 0 );

	// this member is called to read a file
	void readFile ( char *name, cbDefines callback, int context = 0 );

	// this member is called to erase a file
	void eraseFile ( char *name, cbDefines callback, int context = 0 );

	// this member is called to append a string to a file (usually for logs)
	void appendToFile ( char *name, char *str, cbDefines callback, int context = 0 );

	// this member is called to append a packed data to a file 
	void appendToFile ( char *name, PackedData *data, cbDefines callback, int context = 0 );

	// this member is called to verify a file exists
	void exists ( char *name, cbDefines callback, int context = 0 );

	// this member is called to handle incoming messages
	void handleMessage ( IPCMessage *msg );
};

// this function is called to erase a file without a callback
void eraseFile ( const char *format, ... );

void ReadWhatsNew ( PackedData* packet );
void ReadMailFile ( PackedData* packet );

#endif

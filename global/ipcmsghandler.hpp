//
// ipcmsghandler
//
// This module contains the IPCMsgHandler class.
//
// author: Stephen Nichols
//

#ifndef _IPCMSGHANDLER_HPP_
#define _IPCMSGHANDLER_HPP_

#include "list.hpp"

class IPCMessage;

class IPCMsgHandler : public ListObject
{
public:
	virtual void handleMessage ( IPCMessage *msg ) {};
};

#endif

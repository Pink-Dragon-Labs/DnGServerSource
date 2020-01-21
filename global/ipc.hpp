/*
	interprocess communication (ipc) class library
	author: Stephen Nichols
*/

#ifndef _IPC_HPP_
#define _IPC_HPP_

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <syslog.h>
#include <stropts.h>
#include <poll.h>

#include "tools.hpp"
#include "window.hpp"
#include "list.hpp"
#include "msgs.hpp"
#include "counter.hpp"

#include "new.hpp"
#include "malloc.hpp"
#include "memorypool.hpp"
#include "ipcmsg.hpp"
#include "ipcmsghandler.hpp"
#include "ipcclient.hpp"
#include "ipcencryption.hpp"
#include "ipcserver.hpp"
#include "ipcpollmgr.hpp"

extern IPCMessage *gCurMsg;
extern int gMaxConnectCount;

#endif

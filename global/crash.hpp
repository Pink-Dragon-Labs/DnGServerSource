//
// crash
//
// This file contains crash maintenance routines.
//
// author: Stephen Nichols
//

#ifndef _CRASH_HPP_
#define _CRASH_HPP_

// declare the app crash handler
extern void (*appCrashHandler) ( int );

// call this function to init the signal handler
void initSignalHandlers ( void );

#endif

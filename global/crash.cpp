//
// crash
//
// This file contains crash maintenance routines.
//
// author: Stephen Nichols
//

#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

#include "crash.hpp"

// declare the app crash handler
void (*appCrashHandler) ( int ) = NULL;

// this is called anytime there is a signal caught
void sigcrash ( int signum )
{
	static int isCrashing = 0;
	printf ( "sigcrash ( %d ) - %d\n", signum, isCrashing );

	// can't crash twice
	if ( isCrashing )
		abort();

	isCrashing = 1;

	printf ( "appCrashHandler = 0x%p\n", appCrashHandler );

	if ( appCrashHandler )
		appCrashHandler ( signum );

	// uninstall the signal handler
	signal ( signum, NULL );
}

// call this function to init the signal handler
void initSignalHandlers ( void )
{
	sigset_t signalSet = { 0 };
	struct sigaction action = { 0 };

	// fill up the signal set
	sigfillset ( &signalSet );

	// mark each signal that we want to handle
	sigdelset ( &signalSet, SIGINT );
	sigdelset ( &signalSet, SIGSEGV );
	sigdelset ( &signalSet, SIGALRM );
	sigdelset ( &signalSet, SIGFPE );
	sigdelset ( &signalSet, SIGBUS );
	sigdelset ( &signalSet, SIGILL );
	sigdelset ( &signalSet, SIGSYS );
	sigdelset ( &signalSet, SIGQUIT );
//	sigdelset ( &signalSet, SIGHUP );

	// update the signal mask
	sigprocmask ( SIG_SETMASK, &signalSet, NULL );

	// prepare the action mask
	sigfillset ( &action.sa_mask );

	action.sa_handler = sigcrash;
	sigaction ( SIGINT, &action, NULL );
	sigaction ( SIGSEGV, &action, NULL );
	sigaction ( SIGALRM, &action, NULL );
	sigaction ( SIGFPE, &action, NULL );
	sigaction ( SIGBUS, &action, NULL );
	sigaction ( SIGILL, &action, NULL );
	sigaction ( SIGSYS, &action, NULL );
	sigaction ( SIGQUIT, &action, NULL );
//	sigaction ( SIGHUP, &action, NULL );
}

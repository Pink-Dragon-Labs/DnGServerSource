//
// TIMESYS.CPP
// Module for management of time based events.
//
// Author: Stephen Nichols
//

#include "roommgr.hpp"

// this is the current number of active timers
int gActiveTimerCount = 0;

// this is the array of active timers 
#define _MAX_TIMERS_ALLOWED	512
TimedEvent gTimerArray[_MAX_TIMERS_ALLOWED];

//
// This function is called to install a new time-release message.
//
void installTimer ( int interval, int message )
{
	TimedEvent *timer = &gTimerArray[gActiveTimerCount];
	timer->interval = interval;
	timer->message = message;
	timer->elapsed = 0;

	gActiveTimerCount++;
}

//
// This function is called to process the timers.
//
int processTimers ( void )
{
	static int lastCallTime = 0;

	if ( !lastCallTime ) {
		lastCallTime = getsecondsfast();
		return 0;
	}

	// get the elapsed time
	int elapsedTime = getsecondsfast() - lastCallTime;

	if ( !elapsedTime )
		return 0;

	// update the last call time
	lastCallTime += elapsedTime;

	// process the timers
	TimedEvent *timer = &gTimerArray[0];

	for ( int i=0; i<gActiveTimerCount; i++ ) {
		// adjust the elapsed time and see if the timer should be triggered
		timer->elapsed += elapsedTime;

		// should the timer be triggered?
		if ( timer->elapsed >= timer->interval ) {
			timer->elapsed = 0;
			
			// put a message in the roomMgr queue
			IPCMessage *msg = new IPCMessage;
			msg->type() = timer->message;

			roomMgr->addMessage ( msg );
		}

		// advance to the next timer
		timer++;
	}

	return elapsedTime;
}

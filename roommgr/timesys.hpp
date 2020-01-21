//
// TIMESYS.HPP
// Module for management of time based events.
//
// Author: Stephen Nichols
//

#ifndef _TIMESYS_HPP_
#define _TIMESYS_HPP_

#define _TS_SECONDS_PER_TICK			15

#define _TS_TICKS_PER_REAL_MINUTE	(60 / _TS_SECONDS_PER_TICK)
#define _TS_TICKS_PER_REAL_HOUR		(60 * _TS_TICKS_PER_MINUTE)
#define _TS_TICKS_PER_REAL_DAY		(24 * _TS_TICKS_PER_HOUR)

#define _TS_TICKS_PER_GAME_MINUTE	(_TS_TICKS_PER_REAL_MINUTE / 4)
#define _TS_TICKS_PER_GAME_HOUR		(60 * _TS_TICKS_PER_GAME_MINUTE)
#define _TS_TICKS_PER_GAME_DAY		(24 * _TS_TICKS_PER_GAME_HOUR)
#define _TS_TICKS_PER_GAME_MONTH		(30 * _TS_TICKS_PER_GAME_DAY)
#define _TS_TICKS_PER_GAME_YEAR		(12 * _TS_TICKS_PER_GAME_MONTH)

#include "../global/system.hpp"

// define the timer structure
typedef struct {
	int interval;
	int message;
	int elapsed;
} TimedEvent;

//
// This function is called to install a new time-release message.
//
void installTimer ( int interval, int message );

//
// This function is called to process all installed timers.
//
int processTimers ( void );

#endif

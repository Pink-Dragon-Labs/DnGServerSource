/*
	room manager statistical window display
	author: Stephen Nichols
*/

#ifndef _STATSWINDOW_HPP_
#define _STATSWINDOW_HPP_

#include "system.hpp"

extern LinkedList gRequestQueue;
extern LinkedList gLoginQueue;

class StatsWindow : public Window
{
public:
	StatsWindow ( int x, int y, int width, int height ) : Window ( x, y, width, height ) {
		iPacketCount = iPacketBytes = 0;
		oPacketCount = oPacketBytes = 0;
		connections = 0;

		lastCallTime = time ( NULL );
		secondsElapsed = series = packetsLastSeries = packetsThisSeries = 0;
	};

	int lastCallTime, secondsElapsed, series, packetsLastSeries, packetsThisSeries;
	int iPacketCount, iPacketBytes;
	int oPacketCount, oPacketBytes;
	int connections;

	double income;

	void calcStats ( void ) {
		/* calculate the number of seconds elapsed */
		secondsElapsed += time ( NULL ) - lastCallTime;
		lastCallTime = time ( NULL );

		packetsThisSeries = (iPacketCount + oPacketCount) - packetsLastSeries;
		packetsLastSeries = (iPacketCount + oPacketCount);

//		income += (0.000125 * connections) * 2;

		series++;
	};

	virtual void draw ( void );
};

#endif

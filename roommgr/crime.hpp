// 
// crime.hpp
//
// crime file management
//
// author: Stephen Nichols
// Updated to database: Bryan Waters
//

#ifndef _CRIME_HPP_
#define _CRIME_HPP_

struct CrimeData {
	int criminal, loadTime;
	unsigned murders;
	unsigned pickedPockets;
	unsigned tourneyKills, tourneyDeaths;
	unsigned arenaKills, arenaDeaths;
	unsigned criminalsKilled;
	unsigned bountyCollected;
	unsigned bountyOnHead;
	int timeLeft;
};

#endif

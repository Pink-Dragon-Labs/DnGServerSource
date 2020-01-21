//
// globals
//
// This file contains all of the global variable definitions for the roommgr.
//
// author: Stephen Nichols
//

#ifndef _GLOBALS_HPP_
#define _GLOBALS_HPP_

#include "../global/list.hpp"
#include "zone.hpp"
#include "../global/stringcache.hpp"

// define the global zone list
extern LinkedList gZones; 

// define the crowded rooms list
extern LinkedList gCrowdedRooms;

// define the affected objects list
extern LinkedList gAffectedObjects;

// define the dead combat list
extern LinkedList gDeadCombats;

// define the NPC list
extern LinkedList gNpcList;

// define the pending combat list
extern LinkedList gPendingCombats;

// this is the global shutdown timer
extern int gShutdownTimer;
extern void*	gShutdownPlayer;

// this is the list of loading houses
extern LinkedList gLoadingHouses;

// this is the global server ID
extern int gServerID;

//this memory pool is reserved on application startup... it is freed back to the OS during
//a crash handler, so that we have guaranteed some nice memory to work within.
extern void* gPanicMemoryPool;

// this is the distance required to touch someone
extern int gTouchDistance;

// global temporary extra SDM...
extern int gTempSDM;

// This is the active server name
extern char* gServerName;

// Global Magi Mail Flag
extern int gMagicMailOff;

// Are we on the test server
extern bool gTestServer;

extern StringCache::Manager gStringCache;

#endif

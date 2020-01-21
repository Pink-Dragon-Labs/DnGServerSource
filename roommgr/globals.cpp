//
// globals
//
// This file contains all of the global variable definitions for the roommgr.
//
// author: Stephen Nichols
//

#include "globals.hpp"

// define the global zone list
LinkedList gZones; 

// define the crowded rooms list
LinkedList gCrowdedRooms;

// define the affected objects list
LinkedList gAffectedObjects;

// define the dead combat list
LinkedList gDeadCombats;

// define the NPC list
LinkedList gNpcList;

// define the pending combat list
LinkedList gPendingCombats;

// this is the global shutdown timer
int gShutdownTimer = -1;
void*	gShutdownPlayer = NULL;

// this is the global server ID
int gServerID = -1;

// this is the distance required to touch someone
int gTouchDistance = 4;

// global temporary SDM...
int gTempSDM = 0;

// This is the active server name
char* gServerName = NULL;

void* gPanicMemoryPool = 0;

// global mail flag
int gMagicMailOff = 0;

// Global Test Server???
bool gTestServer = 0;

StringCache::Manager gStringCache;

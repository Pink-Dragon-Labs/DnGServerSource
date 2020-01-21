/*
	MISC.HPP
	Miscellaneous utility functions.

	Author: Stephen Nichols
*/

#ifndef _MISC_HPP_
#define _MISC_HPP_

class WorldObject;

#include "actions.hpp"

int textToVerb ( char *text );
int textToWearMask ( char *text);
WorldObject* textToObject ( const char* text );
int exitToDir ( int exit );
int textToSkill ( char *text );
int textToSkillLevel ( char *text );
int reverseLoop ( int loop );
int projectXViaLoop ( int loop, int len );
int projectYViaLoop ( int loop, int len );
int directionToLoop ( int direction );
int calcChance(int baseValue);
int isValidForwardProcID(int id); 
int isValidReverseProcID(int id);
int getRandomForwardProc();
int calcCombatMod ( WorldObject *character );

action_t getActionFunction ( char *text );

#endif

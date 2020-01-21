/*
	CHARACTERS.HPP
	Template Character Creation Functions
*/

#ifndef _CHARACTERS_HPP_
#define _CHARACTERS_HPP_

#include "wobject.hpp"
#include "rmroom.hpp"

WorldObject *makeTemplateCharacter ( int profession, int race, int sex, char *name, char *password = "password", int isDemo = 0, WorldObject *source = NULL );
Building *makeTemplateHouse ( char *name, char *password, int firstCreation = 0 );

#endif

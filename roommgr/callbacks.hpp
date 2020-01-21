//
// callbacks
//
// This file contains lots of callback functions.
//
// author: Stephen Nichols
//

#ifndef _CALLBACKS_HPP_
#define _CALLBACKS_HPP_

//
// TELEPORT FUNCTIONS
//

// this function is called to teleport a WorldObject to a named house
void teleportHouse ( WorldObject *obj, char *name );

// this callback completes the teleport house call
void cbTeleportHouse ( int result, int context, int houseID, int accountID, char *name, int size, char *data );

#endif

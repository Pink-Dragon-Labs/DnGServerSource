#ifndef _RMTOOLS_HPP_
#define _RMTOOLS_HPP_

class WorldObject;
class PackedData;

int savedFromSpell ( WorldObject *obj, int mod );
int savedFromParalysis ( WorldObject *obj, int mod );
int savedFromPoison ( WorldObject *obj, int mod );
int savedFromAttack ( WorldObject *obj, int mod );
void putMovieText ( WorldObject *obj, PackedData *movie, const char *format, ... );

int rand100 ( void );

#endif

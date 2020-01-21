//
// crime.cpp
//
// crime file management
//
// author: Stephen Nichols
//

#include "roommgr.hpp"

CrimeData gDefaultCrime;

CrimeData::CrimeData()
{
	criminal = 0;
	murders = 0;
	pickedPockets = 0;
	tourneyKills = 0;
	tourneyDeaths = 0;
	arenaKills = 0;
	arenaDeaths = 0;
	criminalsKilled = 0;
	bountyCollected = 0;
	bountyOnHead = 0;
	loadTime = 0;
	timeLeft = 10;
	name = NULL;
}

CrimeData::~CrimeData()
{
	if ( name ) {
		free ( name );
		name = NULL;
	}
}

void CrimeData::readFromFile ( void )
{
	char filename[1024];
	sprintf ( sizeof ( filename ), filename, "../data/crimes/%s", name );

	if ( exists ( filename ) ) {
		File *file = new File ( filename );

		if ( file->isOpen() && file->size() ) {
			// don't load large files
			if ( file->size() < 1 || file->size() > 100 ) {
				logInfo ( _LOG_ALWAYS, "loading crime: '%s' with size of %d", filename, file->size() );

				delete file;
				return;
			}

			int bufferSize = file->size();
			char str[1024];

			char *buffer = (char *)malloc ( bufferSize + 1 );
			buffer[bufferSize] = 0;
			file->read ( buffer, bufferSize );

			char *ptr = buffer;

			criminal = bufgetint ( str, &ptr, &bufferSize );
			murders = bufgetint ( str, &ptr, &bufferSize );
			pickedPockets = bufgetint ( str, &ptr, &bufferSize );
			tourneyKills = bufgetint ( str, &ptr, &bufferSize );
			tourneyDeaths = bufgetint ( str, &ptr, &bufferSize );
			arenaKills = bufgetint ( str, &ptr, &bufferSize );
			arenaDeaths = bufgetint ( str, &ptr, &bufferSize );
			criminalsKilled = bufgetint ( str, &ptr, &bufferSize );
			bountyCollected = bufgetint ( str, &ptr, &bufferSize );
			bountyOnHead = bufgetint ( str, &ptr, &bufferSize );

			free ( buffer );
		}

		delete file;
	}
}

void CrimeData::writeToFile ( void )
{
	char filename[1024];
	sprintf ( sizeof ( filename ), filename, "../data/crimes/%s", name );

	PackedData *data = new PackedData;

	data->printf ( "%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n", criminal, murders, pickedPockets, tourneyKills, tourneyDeaths, arenaKills, arenaDeaths, criminalsKilled, bountyCollected, bountyOnHead );

	File *file = new File ( filename );

	if ( file->isOpen() ) {
		file->truncate();
		file->write ( data->data(), data->size() );
	}

	delete data;
	delete file;
}

CrimeData *getCrimeObj ( WorldObject *obj )
{
	if ( !obj->playerControlled )
		return new CrimeData;

	return getCrimeObj ( obj->getName() );
}

CrimeData *getCrimeObj ( char *name )
{
	char str[1024];
	strcpy ( str, name );
	strlower ( str );

	CrimeData *data;

	data = new CrimeData;
	data->name = strdup ( str );
	data->readFromFile();
	
	return data;
}

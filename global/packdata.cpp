#include "packdata.hpp"
#include "logmgr.hpp"

PackedData::PackedData()
{
	_getIndex = _putIndex = 0;
}

PackedData::PackedData ( char *ptr, int size ) : Array ( ptr, size )
{
	_freeBuffer = 0;
	_data = (unsigned char *)ptr; 

	_actualSize = size;
	_size = size;
	_getIndex = 0;
	_putIndex = size;
}

PackedData::~PackedData()
{
}

void PackedData::init ( char *ptr, int size )
{
	if ( _data ) {
		free ( _data );
		_data = NULL;
	}

	_data = (unsigned char *)malloc ( size );
	memcpy ( _data, ptr, size );

	_actualSize = size;
	_getIndex = 0;
	_putIndex = size;
}

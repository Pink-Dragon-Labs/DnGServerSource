#include "new.hpp"
#include "malloc.hpp"

#undef new

void* operator new ( size_t size, const char* file, int nLine ) {
	return db_malloc ( size, file, nLine );
}

void* operator new[] ( size_t size, const char* file, const int nLine ) {
	return db_malloc ( size, file, nLine );
}

void operator delete ( void* ptr ) {
	db_free ( ptr, __FILE__, __LINE__ );
}

void operator delete[] ( void* ptr ) {
	db_free ( ptr, __FILE__, __LINE__ );
}

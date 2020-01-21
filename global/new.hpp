#ifndef _NEW_HPP_
#define _NEW_HPP_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "malloc.hpp"

#undef new

extern void* operator new ( size_t size, const char* file = __FILE__, const int nLine = __LINE__ );
extern void* operator new[] ( size_t size, const char* file = __FILE__, const int nLine = __LINE__ );
extern void operator delete ( void *ptr );
extern void operator delete[] ( void *ptr );

#define new new( __FILE__, __LINE__ )

#endif

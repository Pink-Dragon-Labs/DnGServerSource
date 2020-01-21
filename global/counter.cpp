//
// counter.cpp
// semaphore based counting class
//
// author: Stephen Nichols

#include "counter.hpp"
#include "system.hpp"

Counter::Counter()
{
	_val = 0;
}

Counter::~Counter()
{
}

// increment this counter
void Counter::increment ( int val )
{
	_val += val;

	if ( _val < 0 )
		fatal ( "Counter now negative! DOH! %d", _val );
}

// decrement this counter
void Counter::decrement ( int val )
{
	increment ( -val );
}

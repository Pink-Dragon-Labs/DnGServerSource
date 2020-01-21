//
// counter.hpp
// MT-Safe counting class
//
// author: Stephen Nichols

#ifndef _COUNTER_HPP_
#define _COUNTER_HPP_

#include "new.hpp"
#include "malloc.hpp"

class Counter 
{
	int _val;

public:
	Counter();
	virtual ~Counter();

	inline int val ( void ) { return _val; };

	// increment this counter
	void increment ( int val = 1 );

	// decrement this counter
	void decrement ( int val = 1 );
};

#endif

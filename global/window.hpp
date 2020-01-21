/*
	window management class library 
	author: Stephen Nichols
*/

#ifndef _WINDOW_HPP_
#define _WINDOW_HPP_

//#include <curses.h>
#include <stdarg.h>

#include "new.hpp"

//#define _WINDOWS

class WindowManager 
{
	int _initted;

public:
	WindowManager();
	virtual ~WindowManager();
};

extern WindowManager *windowMgr;

class Window 
{
	Window *_window;

public:
	Window ( int x, int y, int width, int height );
	virtual ~Window();

	virtual void draw ( void );
	void printf ( const char *fmt, ... );
	void gets ( char *str );

	inline void setxy ( int x, int y ) { 
#ifdef _WINDOWS
		if ( !windowMgr )
			return;

		_window->_curx = x; 	
		_window->_cury = y; 
#endif
	};

	inline void setx ( int x ) { 
#ifdef _WINDOWS
		if ( !windowMgr )
			return;

		_window->_curx = x;
#endif
	};

	inline void sety ( int y ) { 
#ifdef _WINDOWS
		if ( !windowMgr )
			return;

		_window->_cury = y;
#endif
	};

	inline void cls ( void ) { 
#ifdef _WINDOWS
		if ( !windowMgr )
			return;

		werase ( _window ); 
#endif
	};
};


#endif

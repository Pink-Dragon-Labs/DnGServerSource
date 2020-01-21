/*
	window management class library 
	author: Stephen Nichols
*/


#include "window.hpp"

// #define _WINDOWS

WindowManager *windowMgr = NULL;

WindowManager::WindowManager()
{
#ifdef _WINDOWS
	initscr();
	cbreak();
	nonl();
	intrflush ( stdscr, FALSE );
	keypad ( stdscr, TRUE );
#endif
}

WindowManager::~WindowManager()
{
}

Window::Window ( int x, int y, int width, int height )
{
#ifdef _WINDOWS
	if ( windowMgr ) {
		_window = newwin ( height - 2, width - 2, y + 1, x + 1 );
		scrollok ( _window, TRUE );
		idlok ( _window, TRUE );

		// draw a border around the window
		WINDOW *borderWindow = newwin ( height, width, y, x );
		box ( borderWindow, ACS_VLINE, ACS_HLINE );
		wrefresh ( borderWindow );
		delwin ( borderWindow );
	}
#endif
}

Window::~Window()
{
#ifdef _WINDOWS
	if ( windowMgr ) {
		windowMgr->takeMTControl();
		delwin ( _window );
		windowMgr->giveMTControl();
	}
#endif
}

void Window::draw ( void )
{
}

void Window::printf ( const char *format, ... )
{
#ifdef _WINDOWS
	if ( windowMgr ) {
		windowMgr->takeMTControl();
		va_list args;

		va_start ( args, format );
		vwprintw ( _window, format, args );
		va_end ( args );

		wrefresh ( _window );
		windowMgr->giveMTControl();
	}
#else
	va_list args;

	va_start ( args, format );
	vprintf ( format, args );
	va_end ( args );
#endif
}

void Window::gets ( char *str ) 
{
#ifdef _WINDOWS
	if ( windowMgr ) {
		takeMTControl();

		wgetstr ( _window, str );

		giveMTControl();
	}
#else
	::gets ( str );
#endif
}

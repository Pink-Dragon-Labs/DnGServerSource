//
// html
//
// This file contains various useful HTML generation functions.
//
// author: Stephen Nichols
//

#ifndef _HTML_HPP_
#define _HTML_HPP_

#include <stdarg.h>

// this is called to start a html document
void htmlBegin ( char *title, ... );

// this is called to start a html document minimally 
void htmlBeginRaw ( void );

// this is called to end a html document
void htmlEnd ( void );

// this is called to start a new body section
void htmlNewBody ( char *bgcolor = NULL, char *textColor = NULL, char *linkColor = NULL, char *vlinkcolor = NULL, char *format = NULL, ... );

// this is called to end a body section
void htmlEndBody ( void );

// this is called to start a new table
void htmlNewTable ( int border = 0, char *width = NULL, char *bgcolor = NULL, char *textColor = NULL, char *linkColor = NULL, char *cellSpacing = NULL, char *cellPadding = NULL, char *align = NULL, char *sClass = NULL, char *sBorder = NULL );

// this is called to end a table section
void htmlEndTable ( void );

// this is called to start a new table row
void htmlNewTableRow ( char *theClass = NULL );

// this is called to end a table row
void htmlEndTableRow ( void );

// this is called to start a new table heading
void htmlNewTableHeading ( char *format = NULL, ... );
void htmlNewTableHeading ( int nBG, char *format = NULL, ... );
void htmlNewTableHeadingStyle ( char *theClass );

// this is called to end a table heading
void htmlEndTableHeading ( void );

// this is called to start a new table heading that spans more than 1 col
void htmlNewTableHeadingSpanCol ( int spanCount, char *format = NULL, ... );
void htmlNewTableHeadingSpanCol ( int nBG, int spanCount, char *format = NULL, ... );
 
// this is called to start a new table heading that is left aligned
void htmlNewTableHeadingAlign ( char* sAlign, char *format = NULL, ... );
void htmlNewTableHeadingLeft ( char *format = NULL, ... );

// this is called to start a new table data
void htmlNewTableData ( char *format = NULL, ... );
void htmlNewTableData ( int nBG, char *format = NULL, ... );
void htmlNewTableDataRight ( char *format, ... );

// this is called to start a new table cell with more features
void htmlNewTableDataMax ( char *align = NULL, char *colspan = NULL, char* valign = NULL, char* width = NULL );

// this is called to end table data
void htmlEndTableData ( void );

// this is called to start a form
void htmlNewForm ( char *action, char *name = "theForm", char *server = NULL, char *method = "POST" );

// this is called to end a form
void htmlEndForm ( void );

// this is called to start a paragraph
void htmlNewParagraph ( char *format=NULL, ... );

// this is called to end a paragraph
void htmlEndParagraph ( void );

// this is called to start a new line
void htmlNewLine ( int count=1 );

// this is called to draw a horizontal line
void htmlLineBreak ( char* pColor );

// this is called to indent a line
void htmlIndentLine ( int level, int spacesPerIndent = 2 );

// this is called to add an edit field
void htmlEditField ( char *name, int size, int maxLen, char *value = NULL, int tabIndex = 1 );

// this is called to add a password field
void htmlPasswordField ( char *name, int size, int maxLen, int tabIndex = 1 );

// this is called to add an image field
void htmlImageField ( char* src, char *name, char* alt, char* style );

// this is called to add a textarea. any text printed before htmlEndTextarea will
// be appended
void htmlTextArea ( char *name = NULL, int rows = 20, int cols = 80 );

// this is called to end a textarea
void htmlEndTextArea ( void );

// this is called to add a hidden item
//void htmlHiddenItem ( char *name, char *value );
void htmlHiddenItem ( char *name, char *value, ... );

// this is called to create a form submit button
void htmlSubmit ( char *format, ... );

// this is called to create a form submit button with a name
void htmlSubmitName ( int tabIndex, char *name, char *format, ... );

// this is called to print something
void htmlSimplePrint ( char *format );
void htmlPrint ( char *format, ... );
void htmlRawPrint ( char *format, ... );

// this is called to print something
void htmlPrintTranslate ( char *format, ... );

// this is called to print something in bold
void htmlPrintBold ( char *format, ... );

// this is called to print something in italic
void htmlPrintItalic ( char *format, ... );

// this starts a new center block
void htmlNewCenter ( void );

// this ends a center block
void htmlEndCenter ( void );

// this displays a hyperlink
void htmlLink ( char *text, char *link, ... );
void htmlLinkStart ( char *link, ... );
void htmlLinkEnd ();

// this displays a hyperlink without a <br> after it
void htmlLinkNoBreak ( char *text, char *link, ... );

// this displays a hyperlink and sends the opened page to a separate window
void htmlLinkNewWindow ( char *text, char *link, ... );

// this adds a checkbox
void htmlCheckbox ( char *name, int enabled, int isChecked, char *format, ... );
void htmlCheckbox ( char *name, int enabled, int isChecked, int tabIndex, char *format, ... );

// this adds a radio button
void htmlRadioButton ( char *name, char *value, int enabled, int state, char *format, ... );
void htmlRadioButton ( char *name, char *value, int enabled, int state, int tabIndex, char *format, ...);

// this adds a radio button that excepts javascript
void htmlRadioButtonJS ( char *name, char *value, int enabled, int state, char *script, char *format, ...);

// this add a new button
void htmlButton ( char *name, char *javaLink, char *format, ... );

// this loads some java script from a file
void htmlNewJavaScript ( char* format, ... );

// this adds a new selection list
void htmlNewSelectionList ( char *name, int tabIndex=1 );

// this ends a selection list
void htmlEndSelectionList ( void );

// this adds a new selection list option
void htmlNewOption ( char *value, int selected, char *format, ... );

// this sets the title of this document
void htmlNewTitle ( char *format, ... );

// this ends the title section
void htmlEndTitle ( void );

// this sets the head of this document
void htmlNewHead ( char *format, ... );

// this ends the head section
void htmlEndHead ( void );

// this creates a new style section
void htmlNewStyle ( char *type );

// this ends the style section
void htmlEndStyle ( void );

// this adds a new image to the document
void htmlNewImage ( char *src, char *alt, int width = -1, int height = -1, int border = -1, char *align = NULL, char* name = NULL, char* hSpace = NULL, char* vSpace = NULL ); 

// this handles writing a cookie
void htmlWriteCookie( char* name, char* value, int nExpires=0, char* path=NULL, char* domain=NULL, char* secure=NULL );

// this starts a new heading
void htmlNewHeading( int nLevel, char* pAlign = NULL );

// this adds a new heading
void htmlHeading( int nLevel, char* pAlign, char* pFormat, ... );

// this closes heading
void htmlEndHeading( int nLevel );

// this function encodes the passed string for use in a query string
char *cgiEncode ( char *str );

#endif


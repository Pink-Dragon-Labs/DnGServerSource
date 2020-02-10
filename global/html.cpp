//
// html
//
// This file contains various useful HTML generation functions.
//
// author: Stephen Nichols
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "html.hpp"

// this is called to start a html document
void htmlBegin ( char *title, ... ) {
	char output[1024];
	va_list args;

	va_start ( args, title );
	vsprintf ( output, title, args );
	va_end ( args );

	printf ( "Content-type: text/html\n\n<html>\n<head>\n<title>\n" );
	htmlSimplePrint( output );
	printf ( "</title>\n</head>\n" );
}

// this is called to start a html document without a head or title
void htmlBeginRaw ( void ) {
	printf ( "Content-type: text/html\n\n<html>\n" );
}

// this is called to end a html document
void htmlEnd ( void ) {
	printf ( "</html>\n" );
}

// this is called to start a new body section
void htmlNewBody ( char *bgcolor, char *textcolor, char *linkcolor, char *vlinkcolor, char *format, ... ) {
	printf ( "<body" );

	if ( bgcolor )
		printf ( " bgcolor=\"#%s\"", bgcolor );

	if ( textcolor )
		printf ( " text=\"#%s\"", textcolor );

	if ( linkcolor )
		printf ( " link=\"#%s\"", linkcolor );

	if ( vlinkcolor )
		printf ( " vlink=\"#%s\"", vlinkcolor );

	if ( format ) {
		char output[1024];
		va_list args;

		va_start ( args, format );
		vsprintf ( output, format, args );
		va_end ( args );

		printf ( " %s>\n", output );
	} else {
		printf ( ">\n" );
	}
}

// this is called to end a body section
void htmlEndBody ( void ) {
	printf ( "</body>\n" );
}

// this is called to start a new table section
void htmlNewTable ( int border, char *width, char *bgcolor, char *textcolor, char *linkcolor, char *cellSpacing, char *cellPadding, char *align, char* sClass, char* sBorder ) {
	printf ( "<table" );
	
	if ( sClass )
		printf ( " class=\"%s\"", sClass );

	if ( border != -1 )
		printf( " border=\"%d\"", border );

	if ( width )
		printf ( " width=\"%s\"", width );

	if ( bgcolor )
		printf ( " bgcolor=\"#%s\"", bgcolor );

	if ( textcolor )
		printf ( " text=\"#%s\"", textcolor );

	if ( linkcolor )
		printf ( " link=\"#%s\"", linkcolor );

	if ( cellSpacing )
		printf ( " cellspacing=\"%s\"", cellSpacing );

	if ( cellPadding )
		printf ( " cellpadding=\"%s\"", cellPadding );

	if ( align )
		printf ( " align=\"%s\"", align );

	if ( sBorder )
		printf ( " borderColor=\"%s\"", sBorder );

	printf ( ">\n" );
}

// this is called to end a table
void htmlEndTable ( void ) {
	printf ( "</table>\n" );
}

// this is called to start a new table row
void htmlNewTableRow ( char *theClass ) {
	printf ( "<tr" );

	if ( theClass )
		printf ( " class=\"%s\"", theClass );

	printf ( ">\n" );
}

// this is called to end a table row
void htmlEndTableRow ( void ) {
	printf ( "</tr>\n" );
}

// this is called to start a new table heading
void htmlNewTableHeading ( char *format, ... ) {
	if ( format ) {
		char output[1024];
		va_list args;

		va_start ( args, format );
		vsprintf ( output, format, args );
		va_end ( args );

		printf ( "<th>\n" );
		htmlSimplePrint( output );
		printf ( "\n</th>\n" );
	} else {
		printf ( "<th>\n" );
	}
}

// this is called to start a new table heading with a bg color
void htmlNewTableHeading ( int nBG, char *format, ... ) {
	if ( format ) {
		char output[1024];
		va_list args;

		va_start ( args, format );
		vsprintf ( output, format, args );
		va_end ( args );

		printf ( "<th bgcolor=\"#%X\">\n", nBG );
		htmlSimplePrint( output );
		printf ( "\n</th>\n" );
	} else {
		printf ( "<th bgcolor=\"#%X\">\n", nBG );
	}
}

// this is called to start a new table heading with a style class
void htmlNewTableHeadingStyle ( char *theClass ) {
	printf ( "<th" );

	if ( theClass )
		printf ( " class=\"%s\"", theClass );

	printf ( ">\n" );
}

// this is called to start a new table heading that is left aligned
void htmlNewTableHeadingLeft ( char *format, ... ) {
	if ( format ) {
		char output[1024];
		va_list args;

		va_start ( args, format );
		vsprintf ( output, format, args );
		va_end ( args );

		printf ( "<th align=\"left\">\n" );
		htmlSimplePrint( output );
		printf ( "\n</th>\n" );
	} else {
		printf ( "<th align=\"left\">\n" );
	}
}

// this is called to start a new table heading that is aligned
void htmlNewTableHeadingAlign ( char* sAlign, char *format, ... ) {
	if ( format ) {
		char output[1024];
		va_list args;

		va_start ( args, format );
		vsprintf ( output, format, args );
		va_end ( args );

		printf ( "<th align=\"%s\">\n", sAlign );
		htmlSimplePrint( output );
		printf ( "\n</th>\n" );
	} else {
		printf ( "<th align=\"%s\">\n", sAlign );
	}
}

// this is called to end a table heading
void htmlEndTableHeading ( void ) {
	printf ( "</th>\n" );
}

// this is called to start a new table heading that spans more than 1 column
void htmlNewTableHeadingSpanCol ( int spanCount, char *format, ... ) {
	if ( format ) {
		char output[1024];
		va_list args;

		va_start ( args, format );
		vsprintf ( output, format, args );
		va_end ( args );

		printf ( "<th colspan=%d>\n", spanCount );
		htmlSimplePrint ( output );
		printf ( "\n</th>\n" );
	} else {
		printf ( "<th colspan=%d>\n", spanCount );
	}
}

// this is called to start a new table heading that spans more than 1 column with bg color
void htmlNewTableHeadingSpanCol ( int nBG, int spanCount, char *format, ... ) {
	if ( format ) {
		char output[1024];
		va_list args;

		va_start ( args, format );
		vsprintf ( output, format, args );
		va_end ( args );

		printf ( "<th colspan=%d bgcolor=\"#%X\">\n", spanCount, nBG );
		htmlSimplePrint ( output );
		printf ( "\n</th>\n" );
	} else {
		printf ( "<th colspan=%d bgcolor=\"#%X\">\n", spanCount, nBG );
	}
}

// this is called to start new table data
void htmlNewTableData ( char *format, ... ) {
	if ( format ) {
		char output[1024];
		va_list args;

		va_start ( args, format );
		vsprintf ( output, format, args );
		va_end ( args );

		printf ( "<td>\n" );
		htmlSimplePrint( output );
		printf ( "\n</td>\n" );
	} else {
		printf ( "<td>\n" );
	}
}

// this is called to start new table data
void htmlNewTableDataRight ( char *format, ... ) {
	if ( format ) {
		char output[1024];
		va_list args;

		va_start ( args, format );
		vsprintf ( output, format, args );
		va_end ( args );

		printf ( "<td align=\"right\">\n" );
		htmlSimplePrint( output );
		printf ( "\n</td>\n" );
	} else {
		printf ( "<td align=\"right\">\n" );
	}
}

// this is called to start new table data
void htmlNewTableData ( int nBG, char *format, ... ) {
	if ( format ) {
		char output[1024];
		va_list args;

		va_start ( args, format );
		vsprintf ( output, format, args );
		va_end ( args );

		printf ( "<td bgcolor=\"#%X\">\n", nBG );
		htmlSimplePrint( output );
		printf ( "\n</td>\n" );
	} else {
		printf ( "<td bgcolor=\"#%X\">\n", nBG );
	}
}

// this is a fully-functional table-data tag processor
void htmlNewTableDataMax ( char *align, char *colspan, char* valign, char* width ) {
	printf ( "<td" );

	if ( align )
		printf ( " align=\"%s\"", align );

	if ( valign )
		printf ( " valign=\"%s\"", valign );

	if ( colspan )
		printf ( " colspan=\"%s\"", colspan );

	if ( width )
		printf ( " width=\"%s\"", width );

	printf ( ">\n" );
}

// this is called to end a table data
void htmlEndTableData ( void ) {
	printf ( "</td>\n" );
}

// this is called to start a form
void htmlNewForm ( char *action, char *name, char *server, char *method ) {
	printf ( "<form name=\"%s\" method=\"%s\" action=\"%s\">\n", name, method, action );
}

// this is called to end a form
void htmlEndForm ( void ) {
	printf ( "</form>\n" );
}

// this is called to start a paragraph
void htmlNewParagraph ( char *format, ... ) {
	printf ( "<p>\n" );

	if ( format ) {
		char output[1024];
		va_list args;

		va_start ( args, format );
		vsprintf ( output, format, args );
		va_end ( args );

		htmlSimplePrint( output );
	}
}

// this is called to end a paragraph
void htmlEndParagraph ( void ) {
	printf ( "</p>\n" );
}

// this is called to start a new line
void htmlNewLine ( int count ) {
	while ( count-- )
		printf ( "<br>\n" );
}

// this is called to draw a horizontal line
void htmlLineBreak ( char* pColor ) {
	if ( pColor ) 
		printf ( "<hr color=\"%s\">\n", pColor );
	else
		printf ( "<hr>\n" );
}

// this is called to indent a line
void htmlIndentLine ( int level, int spacesPerIndent ) {
	for ( int i = level * spacesPerIndent; i; --i )
		printf ( "&nbsp; " );
}

// this is called to add an edit field
void htmlEditField ( char *name, int size, int maxLen, char *value, int tabIndex ) {
	printf ( "<input type=\"text\" name=\"%s\" size=\"%d\" maxlength=\"%d\" value=\"%s\" tabIndex=%d>\n", name, size, maxLen, value? value : "", tabIndex );
}

// this is called to add a password field
void htmlPasswordField ( char *name, int size, int maxLen, int tabIndex ) {
	printf ( "<input type=\"password\" name=\"%s\" size=\"%d\" maxlength=\"%d\" tabIndex=%d>\n", name, size, maxLen, tabIndex );
}

// this is called to add an image field
void htmlImageField ( char* src, char *name, char* alt, char* style ) {
	printf ( "<input type=\"image\" src=\"%s\"", src );
	
	if ( name )
		printf( " name=\"%s\"", name );
	
	if ( alt )
		printf( " title=\"%s\" alt=\"%s\"", alt, alt );
	
	if ( style )
		printf( " style=\"%s\"", style );
	
	printf( ">\n" );
}

// this is called to add a text area
void htmlTextArea ( char *name, int rows, int cols ) {
	printf ( "<textarea name=\"%s\" rows=%d cols=%d>\n", name, rows, cols );
}

// this is called to end a text area
void htmlEndTextArea ( void ) {
	printf ( "</textarea>\n" );
}

// this is called to add a hidden item
void htmlHiddenItem ( char *name, char *value, ... ) {
	char output[1024];
	va_list args;

	va_start ( args, value );
	vsprintf ( output, value, args );
	va_end ( args );

	printf ( "<input type=\"hidden\" name=\"%s\" value=\"%s\">\n", name, output );
}

// this is called to add a submit button
void htmlSubmit ( char *name, ... ) {
	char output[1024];
	va_list args;

	va_start ( args, name );
	vsprintf ( output, name, args );
	va_end ( args );

	printf ( "<input type=\"submit\" class=\"submitBtn\" value=\"%s\">\n", output );
}

// this is called to add a submit button with a name
void htmlSubmitName ( int tabIndex, char *name, char *format, ... ) {
	char output[1024];
	va_list args;

	va_start ( args, format );
	vsprintf ( output, format, args );
	va_end ( args );

	printf ( "<input type=\"submit\" class=\"submitBtn\" name=\"%s\" value=\"%s\" tabIndex=%d>\n", name, output, tabIndex );
}

// this is called to print something
void htmlPrint ( char *format, ... ) {
	char output[40960];
	va_list args;

	va_start ( args, format );
	vsprintf ( output, format, args );
	va_end ( args );

	char* pOut = output;
	int nCount = 0;

	while ( *pOut ) {
		if ( *pOut == '<' ) {
			printf( "&lt" );
		} else if ( *pOut == '>' ) {
			printf( "&gt" );
		} else {
			putchar( *pOut );
		}

		pOut++;
		nCount++;

		if ( nCount == 512 )
			putchar( '\n' );
	}

	putchar( '\n' );
}

// this is called to print something
void htmlRawPrint ( char *format, ... ) {
	char output[40960];
	va_list args;

	va_start ( args, format );
	vsprintf ( output, format, args );
	va_end ( args );

	printf( "%s", output );
}

// this is called to print something
void htmlSimplePrint ( char *format ) {
	char* pOut = format;
	int nCount = 0;

	while ( *pOut ) {
		if ( *pOut == '<' ) {
			printf( "&lt" );
		} else if ( *pOut == '>' ) {
			printf( "&gt" );
		} else {
			putchar( *pOut );
		}

		pOut++;
		nCount++;

		if ( nCount == 512 )
			putchar( '\n' );
	}

	putchar( '\n' );
}

// this is called to print something
void htmlPrintTranslate ( char *format, ... ) {
	char output[40960];
	va_list args;

	va_start ( args, format );
	vsprintf ( output, format, args );
	va_end ( args );

	char* pOut = output;
	int nCount = 0;

	while ( *pOut ) {
		if ( *pOut == '<' ) {
			printf( "&lt" );
		} else if ( *pOut == '>' ) {
			printf( "&gt" );
		} else if ( *pOut == '\\' ) {
			pOut++;

			if ( *pOut == 'n' )
				printf( "<br>\n" );
		} else {
			putchar( *pOut );
		}

		pOut++;
		nCount++;

		if ( nCount == 512 )
			putchar( '\n' );
	}

	putchar( '\n' );
}

// this is called to print something in bold
void htmlPrintBold ( char *format, ... ) {
	char output[1024];
	va_list args;

	va_start ( args, format );
	vsprintf ( output, format, args );
	va_end ( args );

	printf ( "<strong>\n" );
	htmlSimplePrint ( output );
	printf ( "\n</strong>\n");
}

// this is called to print something in italic
void htmlPrintItalic ( char *format, ... ) {
	char output[1024];
	va_list args;

	va_start ( args, format );
	vsprintf ( output, format, args );
	va_end ( args );

	printf ( "<i>\n" );
	htmlSimplePrint ( output );
	printf ( "\n</i>\n" );
}

// this is called to add a new hyperlink
void htmlLink ( char *name, char *link, ... ) {
	char output[1024];
	va_list args;

	va_start ( args, link );
	vsprintf ( output, link, args );
	va_end ( args );

	printf ( "<a href=\"%s\">\n%s\n</a>\n<br>\n", output, name );
}

// this is called to add a new hyperlink
void htmlLinkStart ( char *link, ... ) {
	char output[1024];
	va_list args;

	va_start ( args, link );
	vsprintf ( output, link, args );
	va_end ( args );

	printf ( "<a href=\"%s\">\n", output );
}

// this is called to add a new hyperlink
void htmlLinkEnd () {
	printf ( "</a>\n" );
}

// this is called to add a new hyperlink without a <br> after it
void htmlLinkNoBreak ( char *name, char *link, ... ) {
	char output[1024];
	va_list args;

	va_start ( args, link );
	vsprintf ( output, link, args );
	va_end ( args );

	printf ( "<a href=\"%s\">\n%s\n</a>\n", output, name );
}

// this is called to add a new hyperlink that opens it's target in a new window
void htmlLinkNewWindow ( char *name, char *link, ... ) {
	char output[1024];
	va_list args;

	va_start ( args, link );
	vsprintf ( output, link, args );
	va_end ( args );

	printf ( "<a href=\"%s\" target=_blank>\n%s\n</a>\n<br>\n", output, name );
}

// this is called to start a new center block
void htmlNewCenter ( void ) {
	printf ( "<center>\n" );
}

// this is called to end a center block
void htmlEndCenter ( void ) {
	printf ( "</center>\n" );
}

// this adds a checkbox
void htmlCheckbox ( char *name, int enabled, int isChecked, char *format, ... ) {
	printf ( "<input type=\"checkbox\" name=\"%s\"", name );

	if ( isChecked )
		printf ( " checked" );

	if ( !enabled )
		printf ( " disabled" );

	printf ( ">\n " );

	if ( format ) {
		va_list args;
		va_start ( args, format );
		vprintf ( format, args );
		va_end ( args );
	}
}

// this adds a checkbox
void htmlCheckbox ( char *name, int enabled, int isChecked, int tabIndex, char *format, ... ) {
	printf ( "<input type=\"checkbox\" name=\"%s\"", name );

	if ( isChecked )
		printf ( " checked" );

	if ( !enabled )
		printf ( " disabled" );

	if ( tabIndex )
		printf ( " tabIndex=%d", tabIndex );

	printf ( ">\n " );

	if ( format ) {
		va_list args;
		va_start ( args, format );
		vprintf ( format, args );
		va_end ( args );
	}
}

// this adds a radio button
void htmlRadioButton ( char *name, char *value, int enabled, int state, int tabIndex, char *format, ...) {
	printf ( "<input type=\"radio\" name=\"%s\" value=\"%s\"", name, value );

	if ( state )
		printf ( " checked" );

	if ( !enabled )
		printf ( " disabled" );

	if ( tabIndex )
		printf ( " tabIndex=%d", tabIndex );

	printf ( ">\n " );

	if ( format )
	{
		va_list args;
		va_start ( args, format );
		vprintf ( format, args );
		va_end ( args );
	}
}

// this adds a radio button
void htmlRadioButton ( char *name, char *value, int enabled, int state, char *format, ...) {
	printf ( "<input type=\"radio\" name=\"%s\" value=\"%s\"", name, value );

	if ( state )
		printf ( " checked" );

	if ( !enabled )
		printf ( " disabled" );

	printf ( ">\n " );

	if ( format )
	{
		va_list args;
		va_start ( args, format );
		vprintf ( format, args );
		va_end ( args );
	}
}

// this adds a radio button that excepts javascript
void htmlRadioButtonJS ( char *name, char *value, int enabled, int state, char *script, char *format, ...) {
	printf ( "<input type=\"radio\" name=\"%s\" value=\"%s\"", name, value );

	if ( state )
		printf ( " checked" );

	if ( !enabled )
		printf ( " disabled" );
	
	if ( script )
		printf ( " %s", script );

	printf ( ">\n " );

	if ( format )
	{
		va_list args;
		va_start ( args, format );
		vprintf ( format, args );
		va_end ( args );
	}
}

// this function loads a java script
void htmlNewJavaScript ( char* format, ... ) {
	char output[10240];
	va_list args;

	va_start ( args, format );
	vsprintf ( output, format, args );
	va_end ( args );

	printf ( "<script language=\"javascript\" src=\"%s\">\ndocument.write ( 'JavaScript %s could not be loaded!' );\n</script>\n", output, output );
}

// this makes a new button
void htmlButton ( char *name, char *javaLink, char *format, ... ) {
	printf ( "<input type=\"button\" name=\"%s\"", name );

	if ( javaLink )
		printf ( " %s", javaLink );

	if ( format ) {
		printf ( " value=\"" );
		va_list args;
		va_start ( args, format );
		vprintf ( format, args );
		va_end ( args );
		printf ( "\"" );
	}

	printf ( ">\n " );
}

// this makes a new selection list
void htmlNewSelectionList ( char *name, int tabIndex ) {
	printf ( "<select name='%s' tabindex=%d>\n", name, tabIndex );
}

// this ends a selection list
void htmlEndSelectionList ( void ) {
	printf ( "</select>\n" );
}

// this adds a new selection option
void htmlNewOption ( char *value, int selected, char *format, ... ) {
	if ( format ) {
		printf ( "<option value=\"%s\"%s>", value, selected? " selected" : "" );
		va_list args;
		va_start ( args, format );
		vprintf ( format, args );
		va_end ( args );
		printf ( "</option>\n" );
	}
}

// this sets the title of the document
void htmlNewTitle ( char *format, ... ) {
	printf ( "<title>\n" );

	if ( format ) {
		char output[1024];
		va_list args;
		va_start ( args, format );
		vsprintf ( output, format, args );
		va_end ( args );
		
		htmlSimplePrint ( output );
		printf ( "</title>\n" );
	}
}

// this ends the title section
void htmlEndTitle ( void ) {
	printf ( "</title>\n" );
}

// this sets the head of the document
void htmlNewHead ( char *format, ... ) {
	printf ( "<head>\n" );

	if ( format ) {
		char output[1024];
		va_list args;
		va_start ( args, format );
		vsprintf ( output, format, args );
		va_end ( args );
		
		htmlSimplePrint ( output );
		printf ( "</head>\n" );
	}
}

// this ends the head section
void htmlEndHead ( void ) {
	printf ( "</head>\n" );
}

// this starts a new style section
void htmlNewStyle ( char *type ) {
	printf ( "<style type=\"%s\">\n<!--", type );
}

// this ends a style section
void htmlEndStyle ( void ) {
	printf ( "-->\n</style>\n" );
}

// this adds a new image to the document
void htmlNewImage ( char *src, char *alt, int width, int height, int border, char *align, char* name, char* hSpace, char* vSpace ) {
	printf ( "<img src=\"%s\" alt=\"%s\" title=\"%s\"", src, alt, alt );

	if ( name )
		printf( " name=\"%s\"", name );

	if ( width != -1 )
		printf ( " width=%d", width );

	if ( height != -1 )
		printf ( " height=%d", height );

	if ( border != -1 )
		printf ( " border=%d", border );

	if ( align )
		printf ( " align=\"%s\"", align );

	if ( hSpace )
		printf ( " hspace=\"%s\"", hSpace );

	if ( vSpace )
		printf ( " vspace=\"%s\"", vSpace );

	printf ( ">\n" );
}

char days[7][4] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
char months[12][4] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

// this handles writing a cookie (the last values are in name value pairs!)
void htmlWriteCookie( char* name, char* value, int nExpires, char* path, char* domain, char* secure ) {
	struct tm* GMT;
	char GMTime[1024];

	printf( "<script language=\"javascript\">\n" );

	time_t nowTime = time( NULL );
	nowTime += nExpires;

	/* Obtain coordinated universal time: */
	GMT = gmtime( &nowTime );

	sprintf( GMTime, "%s, %02d-%s-%02d %02d:%02d:%02d GMT", 
		days[ GMT->tm_wday ],
		GMT->tm_mday,
		months[ GMT->tm_mon ],
		( GMT->tm_year % 100 ),
		GMT->tm_hour,
		GMT->tm_min,
		GMT->tm_sec
		);

	printf( "  document.cookie = \"%s=%s;expires=%s%s%s%s%s%s\"\n", 
		name, 
		cgiEncode( value ), 
		GMTime,
		path ? ";path=" : "",
		path ? path : "",
		domain ? ";domain=" : "",
		domain ? domain : "",
		secure ? ";secure" : ""
	);

	printf( "</script>\n" );
}

// this starts a new heading
void htmlNewHeading( int nLevel, char* pAlign ) {
	printf( "<h%d", nLevel );

	if ( pAlign )
		printf( " align=\'%s\"", pAlign );

	printf( ">" );
}

// this adds a new heading
void htmlHeading( int nLevel, char* pAlign, char* pFormat, ... ) {
	char output[1024];

	printf( "<h%d", nLevel );

	if ( pAlign )
		printf( " align=\"%s\"", pAlign );

	printf( ">" );

	va_list args;
	va_start ( args, pFormat );
	vsprintf ( output, pFormat, args );
	va_end ( args );
	
	printf( "%s", output );

	printf( "</h%d>", nLevel );
}

// this closes heading
void htmlEndHeading( int nLevel ) {
	printf( "</h%d>", nLevel );
}

// this function encodes the passed string for use in a query string
char *cgiEncode ( char *str ) {
	char scratch[10240];

	char *in = str;
	char *out = scratch;

	while ( *in ) {
		if ( isalnum ( *in ) ) {
			*out++ = *in;
		} else if ( *in == ' ' ) {
			*out++ = '+';
		} else {
			char hex[3];
			sprintf ( hex, "%02x", (unsigned char)*in );
			*out++ = '%';
			*out++ = hex[0];
			*out++ = hex[1];
		}

		in++;
	}

	*out = 0;

	return strdup ( scratch );
}


#include "../global/system.hpp"

// this function is called to convert a text file to a binary token file
void tokenizeFile ( char *name )
{
	static char data[1000000];

	if ( !exists ( name ) )
		return;

	memset ( data, 0, sizeof ( data ) );

	logDisplay ( "tokenizing '%s'...", name );

	File file ( name );
	int bufferSize = file.size();
	char *buffer = (char *)malloc ( bufferSize ), *ptr = buffer;
	file.read ( buffer, bufferSize );

	// make the packed data to hold the tokenized data
	int index = 0;
	char str[2048];

	while ( bufferSize ) {
		bufgets ( str, &ptr, &bufferSize );

		// strip the comments
		char *comment = strchr ( str, '#' );

		if ( comment )
			*comment = 0;

		// make a token list
		LinkedList *tokens = buildTokenList ( str );

		// put the size of the token list
		*((unsigned short *)&data[index]) = tokens->size();
		index += 2;

		// step through and put each token
		LinkedElement *element = tokens->head();

		while ( element ) {
			StringObject *string = (StringObject *)element->ptr();
			element = element->next();

			int len = strlen ( string->data ) + 1;
			len += len % 2;

			*((unsigned short *)&data[index]) = len;
			index += 2;

			// strip the windows-specific apostrophe...
			char *pPtr = NULL;

			while ( pPtr = strchr ( string->data, 146 ) )
				*pPtr = 39;

			strcpy ( &data[index], string->data );
			index += len;
		}

		delete tokens;
	}

	free ( buffer );

	// dump the output data
	char filename[1024];
	sprintf ( sizeof ( filename ), filename, "%s.tok", name );

	File outFile ( filename );
	outFile.truncate();
	outFile.write ( data, index );
}

int main ( int argc, char **argv )
{
	if ( argc == 1 ) {
		printf ( "no files specified!\n" );
		return -1;
	}

	for ( int i=1; i<argc; i++ )
		tokenizeFile ( argv[i] );
}


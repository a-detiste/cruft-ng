#include <stdio.h>

#define FALSE false
#define TRUE  true

/* 0 on no match, non-zero on match */
int shellexp( char* string, char* pattern ) {
    /*  printf( "...matching( \"%s\", \"%s\" )\n", string, pattern ); */

    switch( pattern[0] ) {
    case '\0':
	return string[0] == '\0';
    case '?':
	switch( string[0] ) {
	case '\0':
	case '/':
	    return FALSE;
	default:
	    return shellexp( string+1, pattern+1 );
	}
    case '/':
	if ( pattern[1] == '*' && pattern[2] == '*' ) {
	    char* pch = string;
	    if ( pattern[3] != '/' && pattern[3] != '\0' ) {
		fprintf( stderr, "Bad expression.\n" );
		return -1;
	    }
	    if ( *pch != '/' ) return FALSE;
	    if ( pattern[3] == '\0' ) return TRUE;
	    while ( *pch != '\0' ) {
		if ( *pch == '/' ) {
		    int ret = shellexp( pch, pattern + 3 );
		    if ( ret == TRUE || ret == -1 )
		    	return ret;
		}
		pch++;
	    }
	    return FALSE;
	} else if ( string[0] == '/' ) {
		return shellexp( string+1, pattern+1 );
	} else
		return FALSE;
    case '*':
	if ( string[0] == '/' ) return shellexp( string, pattern+1 );
	{
		int ret = shellexp( string, pattern+1 );
		if (ret == FALSE)
			return string[0] != '\0' ? shellexp( string + 1, pattern ) : FALSE;
		else
			return ret;
	}
    case '[': 
	if ( string[0] == '\0' ) return FALSE;
	{
	    int nott = FALSE;
	    int okay = FALSE;
	    pattern++;
	    if ( pattern[0] == '!' || pattern[0] == '^' ) {
		nott = TRUE;
		pattern++;
	    }

	    if ( pattern[0] == ']' ) {
		if ( string[0] == ']' ) { okay = TRUE; }
		pattern++;
	    }

	    while( pattern[0] != ']' && pattern[0] != '\0' ) {
		if ( pattern[0] == string[0] ) {
		    okay = TRUE;
		} else if ( pattern[1] == '-' && pattern[2] != ']' ) {
		    if ( pattern[0] <= string[0] && string[0] <= pattern[2] ) {
			okay = TRUE;
		    }
		    pattern += 2;
		}
		pattern++;
	    }

	    if ( pattern[0] == '\0' ) {
		fprintf( stderr, "Bad shell expression\n" );
		return -1;
	    }

	    if (! (nott ? !okay : okay))
	    	return FALSE;
	    else
	    	return shellexp( string + 1, pattern + 1 );
	}
    default:
    	if (pattern[0] != string[0])
		return FALSE;
	else
		return shellexp( string + 1, pattern + 1 );
    }
}


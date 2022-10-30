#include <stdio.h>
#include <stdbool.h>

/* 0 on no match, non-zero on match */
int shellexp(const char* string, const char* pattern ) {
    /*  printf( "...matching( \"%s\", \"%s\" )\n", string, pattern ); */

    switch( pattern[0] ) {
    case '\0':
	return string[0] == '\0';
    case '?':
	switch( string[0] ) {
	case '\0':
	case '/':
	    return false;
	default:
	    return shellexp( string+1, pattern+1 );
	}
    case '/':
	if ( pattern[1] == '*' && pattern[2] == '*' ) {
	    const char* pch = string;
	    if ( pattern[3] != '/' && pattern[3] != '\0' ) {
		fprintf( stderr, "Bad expression.\n" );
		return -1;
	    }
	    if ( *pch != '/' ) return false;
	    if ( pattern[3] == '\0' ) return true;
	    while ( *pch != '\0' ) {
		if ( *pch == '/' ) {
		    int ret = shellexp( pch, pattern + 3 );
		    if ( ret == true || ret == -1 )
		    	return ret;
		}
		pch++;
	    }
	    return false;
	} else if ( string[0] == '/' ) {
		return shellexp( string+1, pattern+1 );
	} else
		return false;
    case '*':
	if ( string[0] == '/' ) return shellexp( string, pattern+1 );
	{
		int ret = shellexp( string, pattern+1 );
		if (ret == false)
			return string[0] != '\0' ? shellexp( string + 1, pattern ) : false;
		else
			return ret;
	}
    case '[': 
	if ( string[0] == '\0' ) return false;
	{
	    int nott = false;
	    int okay = false;
	    pattern++;
	    if ( pattern[0] == '!' || pattern[0] == '^' ) {
		nott = true;
		pattern++;
	    }

	    if ( pattern[0] == ']' ) {
		if ( string[0] == ']' ) { okay = true; }
		pattern++;
	    }

	    while( pattern[0] != ']' && pattern[0] != '\0' ) {
		if ( pattern[0] == string[0] ) {
		    okay = true;
		} else if ( pattern[1] == '-' && pattern[2] != ']' ) {
		    if ( pattern[0] <= string[0] && string[0] <= pattern[2] ) {
			okay = true;
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
		return false;
	    else
	    	return shellexp( string + 1, pattern + 1 );
	}
    default:
    	if (pattern[0] != string[0])
		return false;
	else
		return shellexp( string + 1, pattern + 1 );
    }
}


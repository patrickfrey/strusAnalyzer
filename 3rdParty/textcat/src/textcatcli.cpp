#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <cstring>

extern "C" {
#include "textcat.h"
}

using namespace std;

int main( int argc, char *argv[] )
{
	char *profileConfig = argv[1];
	char *filename = argv[2];
	ifstream f( filename, ios::binary | ios::in | ios::ate );

	if( !f.good( ) ) {
		cerr << "ERROR: unable to open file '" << filename << "'!" << endl;
		return 1;
	}

	streamsize size = f.tellg( );
	f.seekg( 0, ios::beg );

	vector<char> buf( size );
	if( !f.read( buf.data( ), size ) ) {
	cerr << "ERROR: unable to read file into memory" << endl;
		return 1;
	}

	void *h = textcat_Init( profileConfig );

	char *languages;
	char *memblock = reinterpret_cast<char *>( &buf[0] );
	languages = textcat_Classify( h, memblock, size );
	if( strcmp( languages, _TEXTCAT_RESULT_UNKOWN ) == 0 ) {
		cout << "UNKNOWN\t" << filename << endl;
	} else if( strcmp( languages, _TEXTCAT_RESULT_SHORT ) == 0 ) {
		cout << "TOO SHORT\t" << filename << endl;
	} else {
		cout << languages << "\t" << filename << endl;
	}

	return 0;
}

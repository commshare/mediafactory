// FFTest.cpp : Defines the entry point for the application.
//

#include <string>
#include <queue>

#include "player.h"

int main(int argc, char* argv[])
{
	if( argc < 2 )
		return -1;
	
	return ShowSDL(argv[1], 0);
}
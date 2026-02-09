#include <stdio.h>

#include "game.h"
#include "constants.h"
#include "time.h"

// Main Entry Point
int main(void)
{
	srand(time(nullptr));
	Game game;
	game.run();

	return 1; //success
}
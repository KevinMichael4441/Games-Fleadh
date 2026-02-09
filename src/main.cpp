#include <stdio.h>

#include "game.hpp"
#include "constants.h"
#include "time.h"

// Main Entry Point
int main(void)
{
	srand(time(nullptr));
	Game game;
	game.Run();

	return 1; //success
}
#ifndef GAME_H
#define GAME_H

#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>
#include "constants.h"
#include "math.h"
#include "rlgl.h"

#include "ooze.h"

class Game
{
public:
	Game();
	void run();

private:

	void initGame();
	void update(float t_dt);
	void draw();


	Ooze ooze;
};

#endif // GAME_H
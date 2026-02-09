#include "game.h"

Game::Game()
{
}

void Game::run()
{
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Slilab");
	SetTargetFPS(TARGET_FPS);
	initGame();

	// Update Loop
	while (!WindowShouldClose())
	{
		float deltaTime = GetFrameTime(); // Get delta time frame time expressed in seconds
		update(deltaTime);

		BeginDrawing();
 		// Clear the Frame
 		ClearBackground(RAYWHITE);
 		// Draw the Game Objects
 		draw();
 		// Raylib End drawing to Frame Buffer
 		EndDrawing();	
	}
}

void Game::initGame()
{
	// OOZEY WHIZY
	float springConstant = 0.01;
	float damp = 0.95;
	Vector2 centrePoint = {0,0};
	float speed = 0.5;
	float jumpAmount = 0.8;
	ooze.initialize(springConstant, damp, centrePoint, speed, jumpAmount);


}

void Game::update(float t_dt)
{
	ooze.update(t_dt);
}

void Game::draw()
{
	ooze.draw();
}
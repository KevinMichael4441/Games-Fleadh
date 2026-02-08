#include <stdio.h>

#include "game.h"
#include "constants.h"
#include "time.h"

// Function Prototypes
// Game Loop Function
void GameLoop(GameData *data);

// Main Entry Point
int main(void)
{
	srand(time(NULL));

	// Init raylib window
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Raylib Template StarterKit GPPI");

	// Create GameData Pointer
	GameData *data = (GameData *)(malloc(sizeof(GameData)));

	// Initialise Game
	InitGame(data);

	// Set Target FPS
	SetTargetFPS(TARGET_FPS);

	// Raylib while loop
	while (!WindowShouldClose()) // Detect window close button or ESC key
	{
		// Call GameLoop
		GameLoop(data);
	}

	// Free resources
	CloseGame(data);

	// Close Raylib Window
	CloseWindow();

	return 0;
}

// HUD Messages
int counter = 0;
char message[11];

void GameLoop(GameData *data)
{
	// Update Game Data
	// Should be outside BeginDrawing(); and EndDrawing();
	float deltaTime = GetFrameTime();
	
	UpdateGame(data, deltaTime);

	// Raylib function Draw
	BeginDrawing();

	// Clear the Frame
	ClearBackground(RAYWHITE);


	// Draw the Game Objects
	DrawGame(data);

	// Raylib End drawing to Frame Buffer
	EndDrawing();
}

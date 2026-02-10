#include "game.hpp"

Game::Game()
{
}

void Game::Run()
{
	// R36S window size is 640x480
#if defined(GAME_NAME) && defined(GAME_DESCRIPTION)
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, GAME_NAME " - " GAME_DESCRIPTION);
#else
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "StarterKit R36S | Linux | Windows | Web");
#endif

	SetTargetFPS(TARGET_FPS);
	InitGame();

	// Update Loop
	while (!WindowShouldClose())
	{
		float deltaTime = GetFrameTime(); // Get delta time frame time expressed in seconds
		Update(deltaTime);

		BeginDrawing();
 		// Clear the Frame
 		ClearBackground(RAYWHITE);
 		// Draw the Game Objects
 		Draw();
 		// Raylib End drawing to Frame Buffer
 		EndDrawing();	
	}
}

void Game::InitGame()
{
	// OOZEY WHIZY
	float springConstant = 0.01;
	float damp = 0.95;
	Vector2 centrePoint = {0,0};
	float speed = 0.5;
	float jumpAmount = 0.8;
	ooze.Initialize(springConstant, damp, centrePoint, speed, jumpAmount);

	//--------Mech--------------
	SuperMech_Init(&mech, {0,0});

	#if defined(PLATFORM_R36S) || defined(PLATFORM_LINUX)
		// Telemetry Init
		glRendererStr = (const char *)glGetString(GL_RENDERER);
		glVersionStr = (const char *)glGetString(GL_VERSION);
		glslVersionStr = (const char *)glGetString(GL_SHADING_LANGUAGE_VERSION);
		InitTelemetry(&r36s_telemetry);
	#endif // Init Telemetry R36S and Linux only
}

void Game::Update(float t_dt)
{
	ooze.Update(t_dt);



	SuperMech_Update(&mech, {0,0}, true, t_dt);

#if defined(PLATFORM_R36S) || defined(PLATFORM_LINUX)
	// Telemetry Update
	UpdateTelemetryFrame(&r36s_telemetry, GetFrameTime(), GetTime(), GetFPS());
	UpdateTelemetry(&r36s_telemetry, GetTime());
#endif // TELEMETRY Update R36S and Linux only

}

void Game::Draw()
{
	ooze.Draw();

	SuperMech_Draw(&mech);

	#if defined(PLATFORM_R36S) || defined(PLATFORM_LINUX)
	// Draw Telemetry
	if (show_telemetry)
	{
		DrawTelemetry(&r36s_telemetry, 8, 8, glRendererStr, glVersionStr, glslVersionStr);
	}
	#endif // Draw Telemetry R36S and Linux only
}
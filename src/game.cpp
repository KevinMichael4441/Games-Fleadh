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

	// Initial GameState
	gamestate = GAME_PLAY;

	//------------- OOZEY WHIZY------------------//
	float springConstant = 0.01;
	float damp = 0.95;
	Vector2 centrePoint = {0,0};
	float speed = 5;
	float jumpAmount = 5;	// 0.8
	ooze.Initialize(springConstant, damp, centrePoint, speed, jumpAmount);

	//--------Input Manager---------------------//
	InitInputManager();

	//----------------Telemetry------------------//
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
	m_activeCommand = PollInput();
	NonGameInputs();

	if(gamestate == GAME_PLAY)
	{
		ooze.Update(t_dt, m_activeCommand);
	}

// -----------------TELEMETRY UPDATES----------------------------------------//

	#if defined(PLATFORM_R36S) || defined(PLATFORM_LINUX)
	// Telemetry Update
	UpdateTelemetryFrame(&r36s_telemetry, GetFrameTime(), GetTime(), GetFPS());
	UpdateTelemetry(&r36s_telemetry, GetTime());
	#endif // TELEMETRY Update R36S and Linux only

//-------------------TELEMETRY UPDATES------------------------------------------//

}

void Game::NonGameInputs()
{
	if(IsCommandActive(VOLUME_UP, m_activeCommand))
	{
		// Increase Volume
	}

	if (IsCommandActive(VOLUME_DOWN, m_activeCommand))
	{
		// Decrease Volume
	}

	if (IsCommandActive(MENU_TOGGLE, m_activeCommand))
	{
		#if defined(PLATFORM_R36S) || defined(PLATFORM_LINUX)
		// Toggle Telemetry (for now)
		show_telemetry = !show_telemetry;
		#endif
	}

	if (IsCommandActive(START_GAME, m_activeCommand))
	{
		// (?)
	}

	if (IsCommandActive(EXIT_COMMAND, m_activeCommand))
	{
		// (?)
	}

	if (IsCommandActive(POWER_COMMAND, m_activeCommand))
	{
		// (?)
	}
}

void Game::Draw()
{
	if(gamestate == GAME_PLAY)
	{
		ooze.Draw();
	}

	#if defined(PLATFORM_R36S) || defined(PLATFORM_LINUX)
	// Draw Telemetry
	if (show_telemetry)
	{
		DrawTelemetry(&r36s_telemetry, 8, 8, glRendererStr, glVersionStr, glslVersionStr);
	}
	#endif // Draw Telemetry R36S and Linux only
}
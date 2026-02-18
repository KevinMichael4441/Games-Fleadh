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
		camera.begin();

		DrawTexture(temp_background, 0, 0, WHITE);

 		// Draw the Game Objects
 		Draw();
 		// Raylib End drawing to Frame Buffer
		camera.end();
 		EndDrawing();	
	}
	UnloadTexture(temp_background);
}

void Game::InitGame()
{

	// Initial GameState
	gamestate = GAME_PLAY;



	//-------------Level Loading-----------------//

	if (!Level_Load(&m_level, "./assets/maps/MyFirstMap.json", "./assets/maps/", "./assets/images/LabTilesTest.png"))
    {
        TraceLog(LOG_ERROR, "Failed to load level");
    }

	// Temporary ----------------------------------------------------
	temp_background = LoadTexture("./assets/1280x960_temp.jpg");
	// --------------------------------------------------------------

	//------------- OOZEY WHIZY------------------//
	float springConstant = 0.01;
	float damp = 0.9;
	Vector2 centrePoint = {0,0};
	float speed = 4.0f;		// 1.2
	float jumpAmount = 5.0f;	// 0.8
	ooze.Initialize(springConstant, damp, centrePoint, speed, jumpAmount);

	//--------Mech--------------//
	SuperMech_Init(&mech, {100,380});

	isDeathActive = false;
    deathTimer = 0.0f;
    deathTimerDuration = 2.0f;

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
		if (isDeathActive)
    	{
        	deathTimer += t_dt;

        	if (deathTimer >= deathTimerDuration)
        	{
            	Respawn();
        	}

        	return;
    	}

		ooze.Update(t_dt, m_activeCommand);

		camera.update(ooze.CalculateCenter());

		SuperMech_Update(&mech, ooze.getPosition(), true, t_dt);

		const Point* points = ooze.GetPoints();
    	int count = ooze.GetPointCount();

    	for (int i = 0; i < count; i++)
    	{
        	if (SuperMech_CheckCollision_Player( &mech, points[i].m_position, points[i].m_radiusX))
    	    {
				isDeathActive = true;
				deathTimer = 0.0f;
            	break;
        	}
    	}
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

void Game::Respawn()
{
    ooze.Reset({SCREEN_WIDTH/2, SCREEN_HEIGHT/2});
    SuperMech_Reset(&mech, {100,380});

    isDeathActive = false;
}

void Game::Draw()
{
	if(gamestate == GAME_PLAY)
	{
		if (isDeathActive)
    	{
    	    float alpha = (sinf(deathTimer * 10.0f) * 0.5f + 0.5f) * 0.6f;

        	DrawRectangle(
            	0, 0,
            	GetScreenWidth(),
            	GetScreenHeight(),
            	Fade(RED, alpha)
        	);

        	DrawText(
            	"DEATH BY SUPERMECH",
            	GetScreenWidth()/2 - 180,
            	GetScreenHeight()/2,
            	40,
            	WHITE
        	);

			return;
    	}

		if (m_level.levelLayer)
		{
			DrawTileLayer(&m_level, m_level.levelLayer);
		}

		ooze.Draw();
		SuperMech_Draw(&mech);

		if (m_level, m_level.foregroundLayer)
		{
			DrawTileLayer(&m_level, m_level.foregroundLayer);
		}
	}

	#if defined(PLATFORM_R36S) || defined(PLATFORM_LINUX)
	// Draw Telemetry
	if (show_telemetry)
	{
		DrawTelemetry(&r36s_telemetry, 8, 8, glRendererStr, glVersionStr, glslVersionStr);
	}
	#endif // Draw Telemetry R36S and Linux only
}
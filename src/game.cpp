#include "game.hpp"
#include "cJSON.h"

static void DebugCountBoundaryTiles(const LevelData* level);

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

 		// Draw the Game Objects
 		Draw();
 		// Raylib End drawing to Frame Buffer
		camera.end();
 		EndDrawing();	
	}
	UnloadTexture(temp_background);
	chunkCacheUnload(&m_level);
	Level_Unload(&m_level);
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
	DebugCountBoundaryTiles(&m_level);

	if (!chunkCacheInit(&m_level, SCREEN_WIDTH, SCREEN_HEIGHT))
	{
    	TraceLog(LOG_ERROR, "chunkCacheInit failed");
	}

	// Temporary ----------------------------------------------------
	temp_background = LoadTexture("./assets/1280x960_temp.jpg");
	// --------------------------------------------------------------

	//------------- OOZEY WHIZY------------------//
	Vector2 centrePoint = {0,0};
	ooze.Initialize(SPRING_CONSTANT, DAMP, centrePoint, OOZE_SPEED, JUMP_AMOUNT);
	ooze.SetLevel(&m_level);

	//--------Mech--------------//
	SuperMech_Init(&mech, {100,200}, &m_level);

	//---------------Security System------------//
	m_securitySystem.initialize(&m_level);
	m_laseDoor_manager.Initialize({576, 300}, {520, 350}, 8);

	
	//-------Collectibles-------//
	score = 0;
	collectibles.AddCollectible({400, 200}, 8);
	collectibles.AddCollectible({300, 220}, 8);
	collectibles.AddCollectible({400, 100}, 8);
	collectibles.AddCollectible({260, 260}, 8);
	collectibles.AddCollectible({100, 200}, 8);

	//-----------Teleporter------------------------//
	m_teleporter_manager.Initialize({960, 384}, {1120, 384});

	//-----------JumpPad------------------------//
	for (int index = 0; index < MAX_JUMPPADS; index++)
	{
		m_jumpPadds[index].initialize({672,384});
	}

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
	if (t_dt > 0.04f) return;

	m_activeCommand = PollInput();
	NonGameInputs();
	
	ui_manager.changeUI(gamestate, camera.screen.target);
	ui_manager.updateUI(t_dt);

	switch (gamestate)
	{
		case GAME_START:
		break;
		case GAME_PLAY:
		{
			Vector2 center = ooze.CalculateCenter();
			camera.update(center);
			m_laseDoor_manager.Update(ooze, t_dt);
			m_teleporter_manager.Update(ooze, t_dt);
			chunkCacheUpdate(&m_level, center);
			SuperMech_Uppdate(&mech, ooze.getPosition(), (m_securitySystem.update(t_dt, ooze)), t_dt);
			checkMechOozeCollision();
			collectibles.Update(ooze, score);

			//-----------JumpPad------------------------//
			for (int index = 0; index < MAX_JUMPPADS; index++)
			{
				m_jumpPadds[index].update(ooze);
			}

			ooze.Update(t_dt, m_activeCommand);
		}
		break;
		case GAME_PAUSE:
		break;
		case GAME_END:
			if(ui_manager.stingAnim.timeToSpawn())
			{
				Respawn();
				camera.update(ooze.CalculateCenter());
				ui_manager.stingAnim.setStingPos(camera.screen.target);
			}
			if(!ui_manager.stingAnim.playingAnim())
			{
				gamestate = GAME_PLAY;
			}
		break;
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
	// Temporary GameState switching --
	if (IsKeyPressed(KEY_ONE))
	if(gamestate != GAME_START){
		gamestate = GAME_START;
	}
	if (IsKeyPressed(KEY_TWO))
		if(gamestate != GAME_PLAY){
		gamestate = GAME_PLAY;
	}
	if (IsKeyPressed(KEY_THREE))
		if(gamestate != GAME_PAUSE){
		gamestate = GAME_PAUSE;
	}
	if (IsKeyPressed(KEY_FOUR))
		if(gamestate != GAME_END){
		gamestate = GAME_END;
	}
	// --------------------------------
}

void Game::Draw()
{
	switch (gamestate){
		case GAME_START:
			DrawTexture(temp_background, 0, 0, WHITE);
		break;
		case GAME_PLAY:
			DrawTexture(temp_background, 0, 0, WHITE);
			
			chunkCacheDraw(&m_level);
			SuperMech_Draw(&mech);
			ooze.Draw();
			m_securitySystem.draw();
			m_laseDoor_manager.Draw();
			m_teleporter_manager.Draw();
			collectibles.Draw();
			//-----------JumpPad------------------------//
			for (int index = 0; index < MAX_JUMPPADS; index++)
			{
				m_jumpPadds[index].draw();
			}
			DrawText(TextFormat("Score: %d", score), camera.screen.target.x - (SCREEN_WIDTH/2), camera.screen.target.y - (SCREEN_HEIGHT/2), 30, WHITE);
		break;
		case GAME_PAUSE:
			DrawTexture(temp_background, 0, 0, WHITE);
			chunkCacheDraw(&m_level);
			ooze.Draw();
			SuperMech_Draw(&mech);
			m_securitySystem.draw();
			collectibles.Draw();

			//-----------JumpPad------------------------//
			for (int index = 0; index < MAX_JUMPPADS; index++)
			{
				m_jumpPadds[index].draw();
			}

		break;
		case GAME_END:
			DrawTexture(temp_background, 0, 0, WHITE);
			chunkCacheDraw(&m_level);
		break;
	}

	ui_manager.drawUI();

	#if defined(PLATFORM_R36S) || defined(PLATFORM_LINUX)
	// Draw Telemetry
	if (show_telemetry)
	{
		DrawTelemetry(&r36s_telemetry, camera.screen.target.x - (SCREEN_WIDTH/2), camera.screen.target.y - (SCREEN_HEIGHT/2), glRendererStr, glVersionStr, glslVersionStr);
	}
	#endif // Draw Telemetry R36S and Linux only
}

void Game::Respawn()
{
    ooze.Reset({SCREEN_WIDTH/2, SCREEN_HEIGHT/2});
    SuperMech_Reset(&mech, {100,200});
}

void Game::checkMechOozeCollision()
{
	const Point* points = ooze.GetPoints();
    int count = ooze.GetPointCount();

    for (int i = 0; i < count; i++){
		if (SuperMech_CheckCollision_Player( &mech, points[i].m_position, points[i].m_radiusX))
		{
			gamestate = GAME_END;
			break;
		}
	}
}

static void DebugCountBoundaryTiles(const LevelData* level)
{
    if (!level || !level->boundaryLayer)
    {
        TraceLog(LOG_INFO, "Boundary layer missing");
        return;
    }

    int nonZero = 0;
    int total = level->levelWidth * level->levelHeight;

    for (int i = 0; i < total; i++)
    {
        cJSON* tileItem = cJSON_GetArrayItem(level->boundaryLayer, i);
        if (tileItem && cJSON_IsNumber(tileItem) && tileItem->valueint != 0)
            nonZero++;
    }

    TraceLog(LOG_INFO, "Boundary tiles: %d / %d non-zero", nonZero, total);
}
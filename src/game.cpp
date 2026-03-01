#include "game.hpp"
#include "cJSON.h"

Rectangle AABBToRectangle(const c2AABB& box)
{
    return Rectangle{ box.min.x, box.min.y, box.max.x - box.min.x, box.max.y - box.min.y };
}

static bool IsMechOffScreen(const SuperMech& mech, const CameraManager& camera)
{
    Rectangle camRect = camera.getScreenRect();
    Rectangle mechRect = AABBToRectangle(SuperMech_GetBoundingBox(&mech));

    return !CheckCollisionRecs(camRect, mechRect);
}

static bool Level_IsSolidTile(const LevelData* level, Vector2 pos)
{
    int tx = (int)(pos.x / level->tileWidth);
    int ty = (int)(pos.y / level->tileHeight);

    if (tx < 0 || ty < 0 || tx >= level->levelWidth || ty >= level->levelHeight) return true;

    cJSON* tileItem = cJSON_GetArrayItem(level->boundaryLayer, ty * level->levelWidth + tx);
    if (tileItem && cJSON_IsNumber(tileItem) && tileItem->valueint != 0) return true;

    return false;
}

static bool FindGroundBelow(const LevelData* level, Vector2 startPos, Vector2& outPos, int maxFall = 512)
{
	float mechHeight = 98.0f;
	int step = level->tileHeight;
    Vector2 pos = startPos;

    for (int i = 0; i < maxFall; i += step)
    {
        Vector2 below = { pos.x, pos.y + step };

        if (Level_IsSolidTile(level, below))
        {
            // Move spawn UP by mech height so feet land on ground
            outPos = { pos.x, below.y - mechHeight };
            return true;
        }

        pos.y += step;
    }

    return false;
}

static Vector2 FindValidRespawnPosition(const LevelData* level, const CameraManager& camera, Vector2 playerPos)
{
    Rectangle camRect = camera.getScreenRect();
    int step = level->tileWidth;

    std::vector<Vector2> priorityCandidates;
    std::vector<Vector2> fallbackCandidates;

    //PRIORITY: same Y level as player (left/right of camera)
    for (int xOffset = step; xOffset <= camRect.width; xOffset += step)
    {
        priorityCandidates.push_back({ camRect.x - xOffset, playerPos.y });
        priorityCandidates.push_back({ camRect.x + camRect.width + xOffset, playerPos.y });
    }

    //FALLBACK: camera edges
    for (int y = (int)camRect.y; y < camRect.y + camRect.height; y += step)
    {
        fallbackCandidates.push_back({ camRect.x - step, (float)y });
        fallbackCandidates.push_back({ camRect.x + camRect.width + step, (float)y });
    }

    for (int x = (int)camRect.x; x < camRect.x + camRect.width; x += step)
    {
        fallbackCandidates.push_back({ (float)x, camRect.y - step });
        fallbackCandidates.push_back({ (float)x, camRect.y + camRect.height + step });
    }

    Vector2 groundPos;

    //try priority candidates first
    for (auto& pos : priorityCandidates)
    {
        if (!Level_IsSolidTile(level, pos) &&
            FindGroundBelow(level, pos, groundPos))
        {
            return groundPos;
        }
    }

    for (auto& pos : fallbackCandidates)
    {
        if (!Level_IsSolidTile(level, pos) &&
            FindGroundBelow(level, pos, groundPos))
        {
            return groundPos;
        }
    }

    //last resort
    return { camRect.x - step, playerPos.y };
}

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
	gamestate = GAME_START;
	//-------------Level Loading-----------------//

	if (!Level_Load(&m_level, "./assets/maps/MyFirstMap.json", "./assets/maps/", "./assets/images/LabTilesTest.png"))
    {
        TraceLog(LOG_ERROR, "Failed to load level");
    }
	DebugCountBoundaryTiles(&m_level);
	if (!LevelLoadObjects(&m_level, "Objects"))
	{
		TraceLog(LOG_ERROR, "Failed to load objects");
	}

	if (!chunkCacheInit(&m_level, SCREEN_WIDTH, SCREEN_HEIGHT))
	{
    	TraceLog(LOG_ERROR, "chunkCacheInit failed");
	}
	//------------- OOZEY WHIZY------------------//
	Vector2 centrePoint = {SCREEN_WIDTH/2,SCREEN_HEIGHT/2};
	ooze.Initialize(SPRING_CONSTANT, DAMP, centrePoint, OOZE_SPEED, JUMP_AMOUNT);
	ooze.SetLevel(&m_level);

	//--------Mech--------------//
	SuperMech_Init(&mech, {100,200}, &m_level);

	//---------------Security System------------//
	m_securitySystem.initialize(&m_level);
	m_laseDoor_manager.Initialize({576, 300}, {520, 350}, 8);
	m_securitySystem.m_lasers[0].initialize(800.0f, 200.0f);
    m_securitySystem.m_laserCount = 1;
	
	//-------Collectibles-------//
	score = 0;
	m_collectibles_manager.Initialize({640, 350}, 8);

	//-----------Teleporter------------------------//
	m_teleporter_manager.Initialize({960, 384}, {1120, 384});

	//-----------JumpPad------------------------//
	m_jumpPadd_manager.Initialize({672, 384});

	//--------Input Manager---------------------//
	InitInputManager();

	//------------UI Manager-------------------//
	ui_manager.initialize();

	//----------------Telemetry------------------//
	#if defined(PLATFORM_R36S) || defined(PLATFORM_LINUX)
		// Telemetry Init
		glRendererStr = (const char *)glGetString(GL_RENDERER);
		glVersionStr = (const char *)glGetString(GL_VERSION);
		glslVersionStr = (const char *)glGetString(GL_SHADING_LANGUAGE_VERSION);
		InitTelemetry(&r36s_telemetry);
	#endif // Init Telemetry R36S and Linux only

	gamestate = GAME_MENU;
}

void Game::Update(float t_dt)
{
	if (t_dt > 0.04f) return;

	m_activeCommand = PollInput();
	NonGameInputs();
	
	ui_manager.changeUI(gamestate, camera.screen.target);
	gamestate = ui_manager.updateUI(t_dt, camera.screen.target, m_activeCommand);

	switch (gamestate)
	{
		case GAME_START:
		break;
		case GAME_MENU:
		break;
		case GAME_PLAY:
		{
			if(!ui_manager.stingAnim.playingAnim())
			{
				Vector2 center = ooze.CalculateCenter();
				camera.update(center);
				m_laseDoor_manager.Update(ooze, t_dt);
				m_teleporter_manager.Update(ooze, t_dt);
				chunkCacheUpdate(&m_level, center);
				SuperMech_Uppdate(&mech, ooze.getPosition(), (m_securitySystem.update(t_dt, ooze)), t_dt);
				checkMechOozeCollision();
				m_collectibles_manager.Update(ooze, score, t_dt);
				m_jumpPadd_manager.Update(ooze);
				ooze.Update(t_dt, m_activeCommand);
			}
			else
			{
				if(ui_manager.stingAnim.timeToSpawn()){
				Respawn();
				camera.update(ooze.CalculateCenter());
				ui_manager.stingAnim.setStingPos(camera.screen.target);}
			}

			// ----------- NEW: Respawn mech if off-screen ------------
    		mechRespawnCooldown += t_dt;

			if (IsMechOffScreen(mech, camera) && mechRespawnCooldown >= 5.0f)
			{
    			Vector2 spawnPos = FindValidRespawnPosition(&m_level, camera, ooze.getPosition());
			    SuperMech_Reset(&mech, ooze.getPosition(), spawnPos);
			    mechRespawnCooldown = 0.0f;
			    TraceLog(LOG_INFO, "Mech respawned at (%.1f, %.1f)", spawnPos.x, spawnPos.y);
			}
		}
		break;
		case GAME_PAUSE:
		break;
		case GAME_INSTRUCTION:
		break;
		case GAME_END:
			if(ui_manager.stingAnim.timeToSpawn()){
				Respawn();
				camera.update(ooze.CalculateCenter());
				ui_manager.stingAnim.setStingPos(camera.screen.target);}
			if(!ui_manager.stingAnim.playingAnim())
			{
				gamestate = GAME_PLAY;
			}
		break;
		case GAME_EXIT:
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
	// if(IsCommandActive(VOLUME_UP, m_activeCommand))
	// {
	// 	// Increase Volume
	// }

	// if (IsCommandActive(VOLUME_DOWN, m_activeCommand))
	// {
	// 	// Decrease Volume
	// }

	if (IsCommandActive(MENU_TOGGLE, m_activeCommand))
	{
		#if defined(PLATFORM_R36S) || defined(PLATFORM_LINUX)
		// Toggle Telemetry (for now)
		show_telemetry = !show_telemetry;
		#endif
	}

	// if (IsCommandActive(START_GAME, m_activeCommand))
	// {
	// 	// (?)
	// }

	// if (IsCommandActive(EXIT_COMMAND, m_activeCommand))
	// {
	// 	// (?)
	// }

	// if (IsCommandActive(POWER_COMMAND, m_activeCommand))
	// {
	// 	// (?)
	// }
	// Temporary GameState switching --
	if (IsKeyPressed(KEY_ONE))
	if(gamestate != GAME_START){
		gamestate = GAME_START;
	}
	if (IsKeyPressed(KEY_TWO))
		if(gamestate != GAME_MENU){
		gamestate = GAME_MENU;
	}
	if (IsKeyPressed(KEY_THREE))
		if(gamestate != GAME_PLAY){
		gamestate = GAME_PLAY;
	}
	if (IsKeyPressed(KEY_FOUR))
		if(gamestate != GAME_PAUSE){
		gamestate = GAME_PAUSE;
	}
	if (IsKeyPressed(KEY_FIVE))
		if(gamestate != GAME_INSTRUCTION){
		gamestate = GAME_INSTRUCTION;
	}
	if (IsKeyPressed(KEY_SIX))
		if(gamestate != GAME_END){
		gamestate = GAME_END;
	}
	// --------------------------------
}

void Game::Draw()
{
	switch (gamestate){
		case GAME_START:
		break;
		case GAME_MENU:
			chunkCacheDraw(&m_level);
		break;
		case GAME_PLAY:
			DrawTexture(temp_background, 0, 0, WHITE);
			chunkCacheDrawBackground(&m_level);

			m_securitySystem.draw();
			m_laseDoor_manager.Draw();
			m_jumpPadd_manager.Draw();
			m_teleporter_manager.Draw();
			m_collectibles_manager.Draw();

			SuperMech_Draw(&mech);
			ooze.Draw();

			chunkCacheDraw(&m_level);

			DrawText(TextFormat("Score: %d", score), camera.screen.target.x - (SCREEN_WIDTH/2), camera.screen.target.y - (SCREEN_HEIGHT/2), 30, WHITE);
			if(ui_manager.stingAnim.playingAnim()){ui_manager.stingAnim.draw();}
		break;
		case GAME_PAUSE:
			DrawTexture(temp_background, 0, 0, WHITE);
			chunkCacheDrawBackground(&m_level);

			m_securitySystem.draw();
			m_laseDoor_manager.Draw();
			m_jumpPadd_manager.Draw();
			m_teleporter_manager.Draw();
			m_collectibles_manager.Draw();

			SuperMech_Draw(&mech);
			ooze.Draw();

			chunkCacheDraw(&m_level);

			DrawText(TextFormat("Score: %d", score), camera.screen.target.x - (SCREEN_WIDTH/2), camera.screen.target.y - (SCREEN_HEIGHT/2), 30, WHITE);
		case GAME_INSTRUCTION:
		break;
		case GAME_END:
			DrawTexture(temp_background, 0, 0, WHITE);
			chunkCacheDrawBackground(&m_level);

			m_securitySystem.draw();
			m_laseDoor_manager.Draw();
			m_jumpPadd_manager.Draw();
			m_teleporter_manager.Draw();
			m_collectibles_manager.Draw();

			SuperMech_Draw(&mech);
			ooze.Draw();

			chunkCacheDraw(&m_level);
			DrawText(TextFormat("Score: %d", score), camera.screen.target.x - (SCREEN_WIDTH/2), camera.screen.target.y - (SCREEN_HEIGHT/2), 30, WHITE);
		break;
		case GAME_EXIT:
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
	Vector2 oozeSpawn = {SCREEN_WIDTH/2, SCREEN_HEIGHT/2};
	chunkCacheUpdate(&m_level, oozeSpawn);
    ooze.Reset(oozeSpawn);
    SuperMech_Reset(&mech, ooze.getPosition(), {100, 200});
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
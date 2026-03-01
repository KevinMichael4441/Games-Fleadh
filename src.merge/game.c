//=================================================================
// Game
//
// Owns the game lifecycle and splash loading steps.
//
// Init   : Window setup, scene registration, splash sequence
// Input      : Per-frame input polling
// Update     : System manager updates + scene dispatch
// Draw       : Scene draw dispatch
// Exit       : Ordered exit of all game_* managers
// GameLoop   : Single entry point called every frame
//
// Splash loading step order matters, see comments in
// loading_steps[] below.  To add a new system: write a Loading*
// function and add an entry to the table.
//=================================================================
#include "game.h"

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

//=================================================================
// Game Data
//=================================================================
static GameData game_data = {0};

//=================================================================
// Splash loading steps
// Each step advances the progress bar by one unit.
// Runs synchronously on native (one step per frame draw).
// Runs one-per-frame on web via SplashUpdate.
//
// game_data is file-scope so all step functions can reach it.
//
// Order matters:
//	FastMath		: Loading Optimised Math Library
//	Telemetry		: Must be after InitWindow (needs GL context)
//	Input			: After Telemetry
//	Sound			: Before Achivements
//	Spine			: Before Player (CreateSpineEntity needs manager)
//	Shaders			: Before Gameplay enter (shader handles needed)
//	Achievements	: Before Player (manager needed for notify calls)
//	Player			: After Spine + Achievements
//	NPCs			: After Spine + Achievements
//=================================================================
static void LoadingNumberCruncher(void)
{
	FastMathInit();
}

static void LoadingTelemetry(void)
{
	GameTelemetryInit();
}

static void LoadingBackground(void)
{
	game_data.background = LoadTexture("assets/R36SControls480.png");
}

static void LoadingInput(void)
{
	GameInputInit();
}

static void LoadingSound(void)
{
	GameSoundInit();
}

static void LoadingSpine(void)
{
	GameSpineInit();
}

static void LoadingShaders(void)
{
	GameShaderInit(&game_data);
	game_data.damage_flash = 0.0f;
}

static void LoadingAchievements(void)
{
	GameAchievementsInit(&game_data);
	ModalInit(&game_data.level_modal);
}

static void LoadingPlayer(void)
{
	game_data.player_position = (Vector2){SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f};
	game_data.player_speed = 5.0f;
	game_data.player_radius = 20.0f;
	game_data.player_points = 0;

	game_data.player = CreateSpineEntity(
		"Supermech_Spine.atlas",
		"Supermech_Spine.json",
		game_data.player_position.x,
		game_data.player_position.y,
		1.0f);

	if (!game_data.player)
	{
		TraceLog(LOG_ERROR, "GAME: Failed to create player Spine entity");
		game_data.exit_game = true;
		game_data.exit_game_time = GetTime();
		return;
	}

	PlaySpineAnimation(game_data.player, 1, "idle_alert", SPINE_ANIM_LOOP);
	SetSpineFlip(game_data.player, true, false);
	SetSpineAnimationSpeed(game_data.player, 0.5f);
}

static void LoadingReticle(void)
{
	game_data.reticle_position = game_data.player_position;
	game_data.reticle_position.x += game_data.player_radius + 40.0f;
	game_data.reticle_speed = 5.0f;
	game_data.reticle_radius = 20.0f;

	game_data.min_X = CONTROLLER_OVERLAY_OFFSET_X + 45.0f + game_data.player_radius;
	game_data.max_X = SCREEN_WIDTH - (CONTROLLER_OVERLAY_OFFSET_X + 50.0f + game_data.player_radius);
	game_data.min_Y = 35.0f + game_data.player_radius;
	game_data.max_Y = SCREEN_HEIGHT - (game_data.player_radius + 255.0f);

	if (game_data.max_X < game_data.min_X)
		game_data.max_X = game_data.min_X;
	if (game_data.max_Y < game_data.min_Y)
		game_data.max_Y = game_data.min_Y;
}

static void LoadingOrganisms(void)
{
	// Init Organism
	OrganismInit(&game_data.organism);

	// Initialise membrane at first organism's position
	Vector2 center = game_data.organism.position[0];
	OrganismMembraneInit(&game_data.membrane, center);
}

static void LoadingSceneBackgrounds(void)
{
	// Load scene-specific backgrounds
	game_data.background_gameplay = LoadTexture(SCENE_GAMEPLAY_BACKGROUND_FILE);
	game_data.background_level_complete = LoadTexture(SCENE_LEVEL_COMPLETE_BACKGROUND_FILE);
	game_data.background_game_over = LoadTexture(SCENE_GAME_OVER_BACKGROUND_FILE);
	game_data.background_game_complete = LoadTexture(SCENE_GAME_COMPLETE_BACKGROUND_FILE);
}

static const SplashLoadingStep loading_steps[] = {
	{"Waking up the tiny math leprechauns...", LoadingNumberCruncher},
	{"Eavesdropping on alien transmissions...", LoadingTelemetry},
	{"Painting the stage (badly, then fixn it)...", LoadingBackground},
	{"Teaching them speakers how to shout...", LoadingInput},
	{"Giving the speakers a swift kick up the .... ...", LoadingSound},
	{"Convincing animations to stop acting the maggot...", LoadingSpine},
	{"Summoning ancient glow magic from newgrange...", LoadingShaders},
	{"Polishing achievements like they are for the Mammy...", LoadingAchievements},
	{"Stuffing courage into the players pockets...", LoadingPlayer},
	{"Drawing a circle and calling it deadly...", LoadingReticle},
	{"Releasing the creatures, sure it will all be grand...", LoadingOrganisms},
	{"Hanging up a few pictures of sheep and cows...", LoadingSceneBackgrounds},
};

//=================================================================
// GameLoop
// Single entry point called every frame on all platforms.
// On web: called by Emscripten scheduler.
// On native: called by the while loop in main().
//=================================================================
void GameLoop(void)
{
	Input();
	Update();
	Draw();

	if (game_data.exit_game && (game_data.now - game_data.exit_game_time) > 0.50f)
	{
#if defined(PLATFORM_WEB)
		emscripten_cancel_main_loop();
		Exit();
#else
		CloseWindow(); // Breaks WindowShouldClose() on next iteration
#endif
	}
}

//=================================================================
// Init Game
//=================================================================
void Init(void)
{
	game_data.exit_game = false;
	game_data.exit_game_time = 0.0f;

	// Levels are 1-based - zero-init would set this to 0 causing
	// the kill threshold (current_level * 30) to fire immediately
	game_data.current_level = 1;
	game_data.kills_this_level = 0;

#if defined(DEBUG)
	SetTraceLogLevel(LOG_DEBUG);
	SetSpineDebugMode(true);
#else
	SetTraceLogLevel(LOG_NONE);
#endif

	// QEMU Sanity Test - see qemu.mk in rules/qemu.mk
	const char *qemu_headless = getenv("QEMU_HEADLESS");
	if (qemu_headless && strcmp(qemu_headless, "1") == 0)
	{
		SetTraceLogLevel(LOG_INFO);
		TraceLog(LOG_INFO, "QEMU: Headless");
		return;
	}

#if defined(GAME_NAME) && defined(GAME_DESCRIPTION)
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, GAME_NAME " - " GAME_DESCRIPTION);
#else
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "StarterKit R36S | Linux | Windows | Web");
#endif

	SetTargetFPS(TARGET_FRAME_RATE);

	//-------------------------------------------------------------
	// Register scenes then launch splash
	// All heavy system init runs inside the loading steps so the
	// progress bar is visible during loading.
	//-------------------------------------------------------------
	GameSplashRegister(loading_steps, (int)(sizeof(loading_steps) / sizeof(loading_steps[0])));
	GameGameplayRegister();
	// TODO: register Game Menu.... scenes here

	GameSceneSet(SPLASH_SCENE, &game_data);

#ifndef PLATFORM_WEB
	// Native: SplashEnter blocked until all steps done + hold elapsed
	GameSceneSet(GAMEPLAY_SCENE, &game_data);
#endif
}

//=================================================================
// Input
//=================================================================
void Input(void)
{
	GameInputPoll(&game_data);
}

//=================================================================
// Update
//=================================================================
void Update(void)
{
	game_data.now = GetTime();
	game_data.frame_time = GetFrameTime();

	// Updates
	OrganismUpdate(&game_data.organism, game_data.now);

	// Rebuild membrane from ALL organisms (creates one membrane around all 7)
	OrganismMembraneRebuildFromOrganism(&game_data.membrane, &game_data.organism);

	// Update membrane physics
	OrganismMembraneUpdate(&game_data.membrane, game_data.frame_time, game_data.now);

	// Compute the outline (this is the expensive operation)
	PerformanceSample outline_performance = PerformanceBegin("OrganismMembraneComputeOutline", 8.0f);
	OrganismMembraneComputeOutline(&game_data.membrane, game_data.now);
	PerformanceEndAndLog(&outline_performance);

	GameTelemetryUpdate(game_data.now, game_data.frame_time, GetFPS());
	GameSoundUpdate();
	GameAchievementsUpdate(&game_data);
	GameSpineUpdate(game_data.frame_time);
	GameShaderUpdate();

	// Commands active in all scenes
	if (IsCommandActive(game_data.command, EXIT_COMMAND) && !game_data.exit_game)
	{
		game_data.exit_game = true;
		game_data.exit_game_time = game_data.now;
		PlaySFX(SFX_UI_SELECT);
	}
	if (IsCommandActive(game_data.command, POWER_COMMAND))
		PlaySFX(SFX_UI_SELECT);
	if (IsCommandActive(game_data.command, VOLUME_UP))
		PlaySFX(SFX_UI_SELECT);
	if (IsCommandActive(game_data.command, VOLUME_DOWN))
		PlaySFX(SFX_UI_SELECT);
	if (IsCommandActive(game_data.command, ACTION_JUMP))
		PlaySFX(SFX_JUMPING);
	if (IsCommandActive(game_data.command, ACTION_CROUCH))
		PlaySFX(SFX_CROUCH);
	if (IsCommandActive(game_data.command, ACTION_PICKUP))
		PlaySFX(SFX_PICKUP);
	if (IsCommandActive(game_data.command, ACTION_RUN))
		PlaySFX(SFX_PICKUP);

	// Dispatch to active scene
	GameSceneUpdate(&game_data);
}

//=================================================================
// Draw
//=================================================================
void Draw(void)
{
	GameSceneDraw(&game_data);
}

//=================================================================
// Exit
//=================================================================
void Exit(void)
{
	// Scene exit handler fires automatically on last GameSceneSet,
	// but call explicitly here in case break out of loop early
	OrganismExit(&game_data.organism);
	OrganismMembraneExit(&game_data.membrane);
	GameSpineExit();
	GameAchievementsExit(&game_data);
	GameSoundExit();
	GameInputExit();
	GameTelemetryExit();
	GameShaderExit(&game_data);
	ModalCleanup(&game_data.level_modal);
	UnloadTexture(game_data.background);
	UnloadTexture(game_data.background_gameplay);
	UnloadTexture(game_data.background_level_complete);
	UnloadTexture(game_data.background_game_over);
	UnloadTexture(game_data.background_game_complete);
	CloseWindow();
}
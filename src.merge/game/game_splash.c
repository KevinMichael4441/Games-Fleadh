//=================================================================
// Game Splash
// Splash screen with step-based loading progress bar.
//
// Note:	Read game_splash.h to understand what this module does.
//			Roam from here when you want to understand game_splash.
//
// Native:	SplashEnter() blocks, running all steps and drawing
//			each frame. Returns when complete - caller transitions
//			to the next scene (no nested GameSceneSet).
//
// Web:		Emscripten drives the render loop - blocking is not
//			allowed. SplashUpdate() advances one step per frame
//			and transitions to GAMEPLAY_SCENE when complete.
//=================================================================

#include <math.h>
#include <raylib.h>
#include <stddef.h>
#include <string.h>

#include "game/game_splash.h"
#include "game/game_scene.h"
#include "constants.h"

//=================================================================
// Splash Screen Layout
//=================================================================
#define SPLASH_HOLD_SECONDS 2.0
#define SPLASH_BAR_W 400
#define SPLASH_BAR_H 24
#define SPLASH_BAR_RADIUS 4.0f
#define SPLASH_BAR_COLOR ((Color){60, 180, 100, 255})
#define SPLASH_BG_COLOR ((Color){40, 40, 40, 255})
#define SPLASH_SHIMMER_COLOR ((Color){180, 255, 200, 120})
#define SPLASH_TEXT_COLOR WHITE
#define SPLASH_LABEL_COLOR ((Color){180, 180, 180, 255})

//=================================================================
// Splash Screen State
//=================================================================
static const SplashLoadingStep *splash_loading_step = NULL;
static int step_count = 0;
static int step_current = 0;
static double step_start = -1.0;
static const char *step_last_label = "Starting...";

static Texture2D splash_background = {0};

//=================================================================
// Draw Progress Bar
//=================================================================
static void DrawProgressBar(int step, int total)
{
	const float progress_bar_x = (SCREEN_WIDTH - SPLASH_BAR_W) / 2.0f;
	const float progress_bar_y = SCREEN_HEIGHT * 0.70f;

	DrawRectangleRounded(
		(Rectangle){progress_bar_x, progress_bar_y, SPLASH_BAR_W, SPLASH_BAR_H},
		SPLASH_BAR_RADIUS / SPLASH_BAR_H, 8, SPLASH_BG_COLOR);

	float fill = (total > 0) ? (float)step / (float)total : 0.0f;
	if (fill > 1.0f)
		fill = 1.0f;
	const float filled_w = SPLASH_BAR_W * fill;

	if (filled_w > 0.0f)
	{
		DrawRectangleRounded(
			(Rectangle){progress_bar_x, progress_bar_y, filled_w, SPLASH_BAR_H},
			SPLASH_BAR_RADIUS / SPLASH_BAR_H, 8, SPLASH_BAR_COLOR);

		float t = (float)GetTime();
		float pulse = (sinf(t * 4.0f) + 1.0f) * 0.5f;
		float shimmer_w = 30.0f * pulse;
		float shimmer_x = progress_bar_x + filled_w - shimmer_w * 0.5f;

		DrawRectangleRounded(
			(Rectangle){shimmer_x, progress_bar_y, shimmer_w, SPLASH_BAR_H},
			SPLASH_BAR_RADIUS / SPLASH_BAR_H, 8,
			(Color){
				SPLASH_SHIMMER_COLOR.r,
				SPLASH_SHIMMER_COLOR.g,
				SPLASH_SHIMMER_COLOR.b,
				(unsigned char)(SPLASH_SHIMMER_COLOR.a * pulse)});
	}

	const char *pct = TextFormat("%d%%", (int)(fill * 100.0f));
	int pct_w = MeasureText(pct, SMALL_FONT_SIZE);
	DrawText(pct,
			 (int)(progress_bar_x + (SPLASH_BAR_W - pct_w) / 2.0f),
			 (int)(progress_bar_y + SPLASH_BAR_H + 10),
			 SMALL_FONT_SIZE, SPLASH_LABEL_COLOR);
}

//=================================================================
// Advance Splash Screen Step
//=================================================================
static void SplashStep(void)
{
	if (step_current >= step_count)
		return;

	splash_loading_step[step_current].function();
	step_last_label = splash_loading_step[step_current].label;
	step_current++;
}

//=================================================================
// Register Splash Screen Callbacks
//=================================================================
void GameSplashRegister(const SplashLoadingStep *steps, int count)
{
	splash_loading_step = steps;
	step_count = count;

	GameSceneRegister(SPLASH_SCENE,
					  SplashEnter,
					  SplashUpdate,
					  SplashDraw,
					  SplashExit);
}

//=================================================================
// Draw Splash Screen
//=================================================================
void GameSplashDraw(void)
{
	const char *label = (step_current >= step_count) ? "Ready" : step_last_label;

	BeginDrawing();

	if (splash_background.id != 0)
	{
		DrawTexturePro(
			splash_background,
			(Rectangle){0, 0, (float)splash_background.width, (float)splash_background.height},
			(Rectangle){0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()},
			(Vector2){0, 0}, 0.0f, WHITE);
	}
	else
	{
		ClearBackground((Color){20, 20, 20, 255});
	}

	const float progress_width = (float)GetScreenWidth();
	const float sh = (float)GetScreenHeight();

	const char *title = "StarterKit";
	int title_w = MeasureText(title, LARGE_FONT_SIZE);
	DrawText(title,
			 (int)((progress_width - title_w) / 2.0f),
			 (int)(sh * 0.35f),
			 LARGE_FONT_SIZE, SPLASH_TEXT_COLOR);

	DrawProgressBar(step_current, step_count);

	if (label && label[0] != '\0')
	{
		int lw = MeasureText(label, SMALL_FONT_SIZE);
		DrawText(label,
				 (int)((progress_width - lw) / 2.0f),
				 (int)(sh * 0.70f) + SPLASH_BAR_H + 34,
				 SMALL_FONT_SIZE, SPLASH_LABEL_COLOR);
	}

	EndDrawing();
}

//=================================================================
// GameSplashIsComplete: all initialisation steps complete
//=================================================================
bool GameSplashIsComplete(void)
{
	if (step_current < step_count)
		return false;
	return (GetTime() - step_start) >= SPLASH_HOLD_SECONDS;
}

//=================================================================
// Splash Screen Callbacks
//=================================================================

//=================================================================
// Splash Enter
//=================================================================
// GameSceneRegister(
//					SPLASH_SCENE,
//					SplashEnter, // **
//					SplashUpdate,
//					SplashDraw,
//					SplashExit
//					);
//=================================================================
static void SplashEnter(GameData *game_data)
{
	(void)game_data;

	step_current = 0;
	step_start = GetTime();
	step_last_label = "Starting...";
	splash_background = LoadTexture(SCENE_SPLASH_BACKGROUND_FILE);

	TraceLog(LOG_INFO, "GAME_SPLASH: Entered (%d steps)", step_count);

#ifndef PLATFORM_WEB
	// Native: block here - draw initial frame, run all steps, hold
	// Caller (main.c Init) transitions to GAMEPLAY after return
	GameSplashDraw(); // Show 0% before first step

	for (int i = 0; i < step_count; i++)
	{
		SplashStep();
		GameSplashDraw();
	}

	while (!GameSplashIsComplete())
		GameSplashDraw();
#endif
}

//=================================================================
// Splash Update
//=================================================================
// GameSceneRegister(
//					SPLASH_SCENE,
//					SplashEnter, 
//					SplashUpdate, // **
//					SplashDraw,
//					SplashExit
//					);
//=================================================================
static void SplashUpdate(GameData *game_data)
{
#ifdef PLATFORM_WEB
	// Web: one step per frame, then transition when complete
	SplashStep();

	if (GameSplashIsComplete())
		GameSceneSet(GAMEPLAY_SCENE, game_data);
#else
	(void)game_data;
#endif
}

//=================================================================
// Splash Draw
//=================================================================
// GameSceneRegister(
//					SPLASH_SCENE,
//					SplashEnter, 
//					SplashUpdate,
//					SplashDraw,	// **
//					SplashExit
//					);
//=================================================================
static void SplashDraw(GameData *game_data)
{
	(void)game_data;
	GameSplashDraw();
}

//=================================================================
// Splash Exit
//=================================================================
// GameSceneRegister(
//					SPLASH_SCENE,
//					SplashEnter, 
//					SplashUpdate, 
//					SplashDraw,
//					SplashExit // **
//					);
//=================================================================
static void SplashExit(GameData *game_data)
{
	(void)game_data;

	if (splash_background.id != 0)
	{
		UnloadTexture(splash_background);
		splash_background.id = 0;
	}

	TraceLog(LOG_INFO, "GAME_SPLASH: Exited");
}
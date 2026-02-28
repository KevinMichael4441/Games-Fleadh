//=================================================================
// Game Splash
// Splash screen with step-based loading progress bar.
//
// On native (Linux/Windows/R36S) init runs as a blocking loop -
// each step completes then one frame is drawn.
//
// On Web (Emscripten) the browser controls the render loop so
// blocking is not possible. Instead GameSplashLoadingStep() is called
// once per frame from Update(), advancing one init step at a time.
// The main loop keeps running normally - the splash is just a scene.
//
// Usage:
//	GameSplashRegister(steps, count)	// call once in Init()
//										// registers enter/update/draw/exit
//										// handlers with the scene manager
//
// Enter:	loads splash texture, runs all steps (native blocks,
//			web returns immediately)
// Update:	web only - advances one step per frame, transitions to
//			GAMEPLAY_SCENE when complete
// Draw:	renders progress bar and background
// Exit:	unloads splash texture
//=================================================================

#ifndef GAME_SPLASH_H
#define GAME_SPLASH_H

#include <stdbool.h>

#include "game.h"

//=================================================================
// C linkage (C++ compiler flag)
//=================================================================
#ifdef __cplusplus
extern "C"
{
#endif

	// Splash step function
	typedef void (*SplashLoadingStepFunction)(void);

	typedef struct
	{
		const char *label;
		SplashLoadingStepFunction function;
	} SplashLoadingStep;

	//=================================================================
	// GameSplashRegister
	// Binds splash enter / update / draw / exit handlers into the
	// scene manager and stores the loading step table.
	// Call once in Init() before GameSceneSet(SPLASH_SCENE, ...).
	//=================================================================
	void GameSplashRegister(const SplashLoadingStep *steps, int count);

	//=================================================================
	// GameSplashIsComplete
	// Returns true once all steps are done AND the minimum hold time
	// has elapsed. Useful for external polling if needed.
	//=================================================================
	bool GameSplashIsComplete(void);

	//=================================================================
	// GameSplashDraw
	// Renders one splash frame. Used internally and available
	// externally if needed.
	//=================================================================
	void GameSplashDraw(void);

	//=================================================================
	// Callback Function declarations
	//=================================================================
	static void SplashEnter(GameData *game_data);
	static void SplashUpdate(GameData *game_data);
	static void SplashDraw(GameData *game_data);
	static void SplashExit(GameData *game_data);

#ifdef __cplusplus
}
#endif

#endif // GAME_SPLASH_H
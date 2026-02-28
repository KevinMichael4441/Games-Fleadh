//=================================================================
// Game Gameplay Scene
// Owns the gameplay scene lifecycle: game objects setup,
// per-frame update logic, draw, and cleanup.
//
// Does not manage the scene transition itself - call
// GameSceneRegister() then GameSceneSet(GAMEPLAY_SCENE, ...).
//
// Usage:
//	GameGameplayRegister()	call once in Init() to bind handlers
//							into the scene manager
//=================================================================

#ifndef GAME_GAMEPLAY_H
#define GAME_GAMEPLAY_H

#include <math.h>
#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <string.h>

#include "game.h"
#include "game/game_scene.h"
#include "game/game_modal.h"
#include "game/game_telemetry.h"
#include "game/game_input.h"
#include "game/game_sound.h"
#include "game/game_spine.h"
#include "game/game_shader.h"
#include "game/game_achievements.h"
#include "game/game_modal.h"

#include "core/collider.h"

#include "simulation/organism.h"

#include "constants.h"

// Forward declaration: avoids circular include with game.h
typedef struct GameData GameData;

#ifdef __cplusplus
extern "C"
{
#endif

	//=================================================================
	// GameGameplayRegister
	// Registers gameplay enter / update / draw / exit handlers with
	// the scene manager.
	// Call once in Init() before GameSceneSet().
	//=================================================================
	void GameGameplayRegister(void);

	//=================================================================
	// Gameplay Function Callbacks
	//=================================================================
	static void GameplayEnter(GameData *game_data);
	static void GameplayUpdate(GameData *game_data);
	static void GameplayDraw(GameData *game_data);
	static void GameplayExit(GameData *game_data);

	static void LevelCompleteEnter(GameData *game_data);
	static void LevelCompleteUpdate(GameData *game_data);
	static void LevelCompleteDraw(GameData *game_data);
	static void LevelCompleteExit(GameData *game_data);

	static void GameOverEnter(GameData *game_data);
	static void GameOverUpdate(GameData *game_data);
	static void GameOverDraw(GameData *game_data);
	static void GameOverExit(GameData *game_data);

	static void GameCompleteEnter(GameData *game_data);
	static void GameCompleteUpdate(GameData *game_data);
	static void GameCompleteDraw(GameData *game_data);
	static void GameCompleteExit(GameData *game_data);

	static void LevelComplete(GameData *game_data);
	static void GameOver(GameData *game_data);
	static void GameComplete(GameData *game_data);
	static void ReplayLevel(GameData *game_data);
	static void ResetGame(GameData *game_data);

#ifdef __cplusplus
}
#endif

#endif // GAME_GAMEPLAY_H
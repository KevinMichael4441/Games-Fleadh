//=================================================================
// Game Scene Manager (GSM) (Note: not a fully fledged FSM)
// Tracks the active game scene and drives per-scene
// enter / update / draw / exit dispatch.
//
//	Scenes:
//		SPLASH_SCENE			loading screen on startup
//		GAMEPLAY_SCENE			normal play loop
//		LEVEL_COMPLETE_SCENE	modal overlay between levels
//		GAME_OVER_SCENE			player failed, modal + wait for reset input
//		GAME_COMPLETE_SCENE		all levels done, congratulations modal
//
// Reset is not a scene.
// Reset is an action triggered from GAME_OVER_SCENE or GAME_COMPLETE_SCENE
// when the modal expires or the player presses the reset button.
// ResetGame() runs then transitions back to GAMEPLAY_SCENE.
//
// SPLASH_SCENE
//     |
//     | (init complete)
//     v
// GAMEPLAY_SCENE
//     | \
//     |  \ (player dies)
//     |   v
//     |  GAME_OVER_SCENE
//     |       |
//     |       | (modal expires)
//     |       v
//     |   ResetGame()
//     |       |
//     |       v
//     |   GAMEPLAY_SCENE
//     |
//     | (level cleared)
//     v
// LEVEL_COMPLETE_SCENE
//     |
//     | (modal expires)
//     v
// GAMEPLAY_SCENE
//     |
//     | (last level cleared)
//     v
// GAME_COMPLETE_SCENE
//     |
//     | (modal expires)
//     v
// ResetGame()
//     |
//     v
// GAMEPLAY_SCENE
//
// Usage:
//	GameSceneRegister()		: Bind enter/update/draw/exit handlers for a scene
//	GameSceneSet()			: Transition to a new scene (calls exit then enter)
//	GameSceneGet()			: Read current scene (for conditional logic)
//	GameSceneUpdate()		: Dispatch update to the active scene
//	GameSceneDraw()			: Dispatch draw to the active scene
//
// The active scene is also stored in game_data.scene so
// draw code can branch without calling GameSceneGet().
//=================================================================

#ifndef GAME_SCENE_H
#define GAME_SCENE_H

//=================================================================
// C linkage (C++ compiler flag)
//=================================================================
#ifdef __cplusplus
extern "C"
{
#endif

	// Forward declaration of GameData (defined in game.h)
	// Avoids circular include with game.h
	// (game.h includes game_scene.h for the GameScene scene field)
	typedef struct GameData GameData;

	//=================================================================
	// Game Scenes
	//=================================================================
	typedef enum GameScene
	{
		SPLASH_SCENE,		  // Splash / loading screen on startup
		GAMEPLAY_SCENE,		  // Normal play
		LEVEL_COMPLETE_SCENE, // Modal overlay between levels
		GAME_OVER_SCENE,	  // Player failed: modal, then ResetGame()
		GAME_COMPLETE_SCENE,  // All levels done: modal, then ResetGame()
		GAME_SCENE_COUNT	  // Must be last
	} GameScene;

	//=================================================================
	// Game Scene handler function pointers
	//=================================================================
	typedef void (*GameSceneEnterFunction)(GameData *game_data);
	typedef void (*GameSceneUpdateFunction)(GameData *game_data);
	typedef void (*GameSceneDrawFunction)(GameData *game_data);
	typedef void (*GameSceneExitFunction)(GameData *game_data);

	//=================================================================
	// GameSceneRegister
	// Bind enter / update / draw / exit handlers for a scene.
	// Any handler may be NULL (no-op for that phase).
	// Call once per scene during Init(), before GameSceneSet().
	//=================================================================
	void GameSceneRegister(GameScene scene,
						   GameSceneEnterFunction on_enter,
						   GameSceneUpdateFunction on_update,
						   GameSceneDrawFunction on_draw,
						   GameSceneExitFunction on_exit);

	//=================================================================
	// GameSceneSet
	// Transition to a new scene.
	// Calls on_exit for the outgoing scene then on_enter for the
	// incoming scene. No-op if already in that scene.
	//=================================================================
	void GameSceneSet(GameScene scene, GameData *game_data);

	//=================================================================
	// GameSceneGet
	// Returns the currently active scene.
	//=================================================================
	GameScene GameSceneGet(void);

	//=================================================================
	// GameSceneUpdate
	// Dispatches Update to the active scene's on_update handler.
	// Call once per frame from main Update().
	//=================================================================
	void GameSceneUpdate(GameData *game_data);

	//=================================================================
	// GameSceneDraw
	// Dispatches Draw to the active scene's on_draw handler.
	// Call once per frame from main Draw().
	//=================================================================
	void GameSceneDraw(GameData *game_data);

#ifdef __cplusplus
}
#endif

#endif // GAME_SCENE_H
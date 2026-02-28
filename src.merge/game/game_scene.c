//=================================================================
// Game Scene Manager (GSM)
//
// Note:	Read game_scene.h to understand GSM.
//=================================================================

#include <raylib.h>
#include "game.h"
#include "game/game_scene.h"

//=================================================================
// Scene Handlers Callbacks
//=================================================================
typedef struct
{
	GameSceneEnterFunction on_enter;
	GameSceneUpdateFunction on_update;
	GameSceneDrawFunction on_draw;
	GameSceneExitFunction on_exit;
} SceneHandlers;

static SceneHandlers scene_handlers[GAME_SCENE_COUNT] = {0};
static GameScene scene_current = GAME_SCENE_COUNT; // sentinel: no scene active yet

//=================================================================
// Scene GSM returns the name of the current scene
//=================================================================
static const char *SceneName(GameScene scene)
{
	switch (scene)
	{
	case SPLASH_SCENE:
		return "SPLASH";
	case GAMEPLAY_SCENE:
		return "GAMEPLAY";
	case LEVEL_COMPLETE_SCENE:
		return "LEVEL_COMPLETE";
	case GAME_OVER_SCENE:
		return "GAME_OVER";
	case GAME_COMPLETE_SCENE:
		return "GAME_COMPLETE";
	default:
		return "UNKNOWN";
	}
}

//=================================================================
// Register Game Scenes
//=================================================================
void GameSceneRegister(GameScene scene,
					   GameSceneEnterFunction on_enter,
					   GameSceneUpdateFunction on_update,
					   GameSceneDrawFunction on_draw,
					   GameSceneExitFunction on_exit)
{
	if (scene < 0 || scene >= GAME_SCENE_COUNT)
		return;

	scene_handlers[scene].on_enter = on_enter;
	scene_handlers[scene].on_update = on_update;
	scene_handlers[scene].on_draw = on_draw;
	scene_handlers[scene].on_exit = on_exit;
}

//=================================================================
// Game Scene Entry Exit Protocol
//=================================================================
void GameSceneSet(GameScene scene, GameData *game_data)
{
	if (scene == scene_current)
		return;

	TraceLog(LOG_INFO, "GAME_SCENE: %s -> %s", SceneName(scene_current), SceneName(scene));

	// Exit Protocol
	// Exit outgoing scene (guard against GAME_SCENE_COUNT value)
	if (scene_current < GAME_SCENE_COUNT && scene_handlers[scene_current].on_exit)
		scene_handlers[scene_current].on_exit(game_data);

	scene_current = scene;

	// Entry Protocol
	// Enter incoming scene
	if (scene_handlers[scene_current].on_enter)
		scene_handlers[scene_current].on_enter(game_data);

	game_data->scene = scene;
}

//=================================================================
// Get Game Scene
//=================================================================
GameScene GameSceneGet(void)
{
	return scene_current;
}

//=================================================================
// Game Scene Update
//=================================================================
void GameSceneUpdate(GameData *game_data)
{
	if (scene_handlers[scene_current].on_update)
		scene_handlers[scene_current].on_update(game_data);
}

//=================================================================
// Game Scene Draw
//=================================================================
void GameSceneDraw(GameData *game_data)
{
	if (scene_handlers[scene_current].on_draw)
		scene_handlers[scene_current].on_draw(game_data);
}
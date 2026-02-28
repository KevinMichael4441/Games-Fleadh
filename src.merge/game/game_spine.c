//=================================================================
// Game Spine
// Owns Spine manager lifecycle and asset path configuration.
//
// Note: Read game_spine.h to understand what this module does.
//       Roam from here when you want to understand game_spine.
//=================================================================

#include <raylib.h>

#include "game/game_spine.h"
#include "spine/spine_manager.h"

//=================================================================
// Spine Asset path (eventially loaded with  Spine Library Hooks
// void *_spUtil_readFile(const char *path, int *length)
//=================================================================
#define SPINE_ASSETS_PATH "assets/animation/spine/"

//=================================================================
// Public API
//=================================================================
void GameSpineInit(void)
{
	InitSpineManager();
	SetSpineAssetsPath(SPINE_ASSETS_PATH);

	TraceLog(LOG_INFO, "GAME_SPINE: Initialised");
}

void GameSpineUpdate(float dt)
{
	UpdateAllSpineEntities(dt);
}

void GameSpineDraw(void)
{
	DrawAllSpineEntities();
}

void GameSpineExit(void)
{
	ExitSpineManager();

	TraceLog(LOG_INFO, "GAME_SPINE: Exited");
}
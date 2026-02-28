//=================================================================
// Game Spine
// Owns Spine manager lifecycle and asset path configuration.
//
// game_spine hides the boilerplate of initialising, updating,
// and shutting down the Spine animation system. The asset path
// is configured here. Game code just calls
// GameSpineInit / GameSpineUpdate / GameSpineDraw / GameSpineExit.
//=================================================================

#ifndef GAME_SPINE_H
#define GAME_SPINE_H

//=================================================================
// C linkage (C++ compiler flag)
//=================================================================
#ifdef __cplusplus
extern "C"
{
#endif

	//=================================================================
	// GameSpine
	//=================================================================

	//=================================================================
	// GameSpineInit
	// Call in Init() after InitWindow().
	// Starts the Spine Manager and sets the asset search path.
	//=================================================================
	void GameSpineInit(void);

	//=================================================================
	// GameSpineUpdate
	// Call every frame in Update() to advance all Spine animations.
	//=================================================================
	void GameSpineUpdate(float dt);

	//=================================================================
	// GameSpineDraw
	// Call in Draw() inside a BeginTextureMode() / EndTextureMode()
	// block. The caller owns the render texture framing so it can
	// apply shaders (e.g. outline) around this draw pass.
	//=================================================================
	void GameSpineDraw(void);

	//=================================================================
	// GameSpineExit
	// Call in Exit() before CloseWindow().
	// Shuts down the Spine manager and frees all loaded entities.
	//=================================================================
	void GameSpineExit(void);

#ifdef __cplusplus
}
#endif

#endif // GAME_SPINE_H
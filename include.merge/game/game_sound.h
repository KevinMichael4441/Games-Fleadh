//=================================================================
// Game Sound
// Owns sound manager lifecycle, config loading, and music startup.
//
// game_sound hides the boilerplate of initialising,
// updating, and shutting down the sound system. All asset paths
// and startup music are configured here. Game code just
// calls GameSoundInit / GameSoundUpdate / GameSoundExit.
//=================================================================

#ifndef GAME_SOUND_H
#define GAME_SOUND_H

#include "constants.h"

//=================================================================
// C linkage (C++ compiler flag)
//=================================================================
#ifdef __cplusplus
extern "C"
{
#endif

	//=================================================================
	// GameSound
	//=================================================================

	//=================================================================
	// GameSoundInit
	// Call in Init() after InitWindow().
	// Loads settings, SFX config, music config, and starts gameplay music.
	// Returns false if a fatal asset is missing (e.g. audio_sfx.ini).
	// On false, set exit_game = true in your Init().
	//=================================================================
	int GameSoundInit(void);

	//=================================================================
	// GameSoundUpdate
	// Call every frame in Update() to stream music.
	//=================================================================
	void GameSoundUpdate(void);

	//=================================================================
	// GameSoundExit
	// Call in Exit() before ExitInputManager() / CloseWindow().
	// Saves volume settings and shuts down the sound manager.
	//=================================================================
	void GameSoundExit(void);

#ifdef __cplusplus
}
#endif

#endif // GAME_SOUND_H
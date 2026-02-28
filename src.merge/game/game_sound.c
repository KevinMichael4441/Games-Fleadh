//=================================================================
// Game Sound
// Owns sound manager lifecycle, config loading, and music startup.
//
// Note: Read game_sound.h to understand what this module does.
//       Rome from here when you want to understand game_sound.
//=================================================================

#ifdef __cplusplus
#include <cstdio>
#else
#include <stdio.h>
#endif

#include <raylib.h>

#include "game/game_sound.h"
#include "core/sound_manager.h"

//=================================================================
// Public API
//=================================================================
int GameSoundInit(void)
{
	// Initialise sound manager
	InitSoundManager();

	// Load player volume preferences (created/updated when player saves)
	LoadSoundSettings(SOUND_SETTINGS_PATH);

	// Load SFX config
	// preload=false: paths stored, sounds lazy-loaded on first play
	if (!LoadSFXConfig(SFX_CONFIG_PATH, false))
	{
		TraceLog(LOG_ERROR, "GAME_SOUND: Exit due to invalid or missing %s", SFX_CONFIG_PATH);
		return 0; // Fatal - caller should set exit_game = true
	}

	// Load music config (false = lazy load, tracks are larger files)
	LoadMusicConfig(MUSIC_CONFIG_PATH, false);

	// Start gameplay music
	PlayMusicTrack(MUSIC_GAMEPLAY);

	TraceLog(LOG_INFO, "GAME_SOUND: Initialised - Master=%.2f  SFX=%.2f  Music=%.2f",
			 GetMasterSoundVolume(), GetSFXVolume(), GetMusicTrackVolume());

	return 1;
}

void GameSoundUpdate(void)
{
	UpdateSoundManager();
}

void GameSoundExit(void)
{
	// Save volume preferences before shutdown
	SaveSoundSettings(SOUND_SETTINGS_PATH);
	ExitSoundManager();
}
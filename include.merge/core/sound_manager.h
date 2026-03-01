#ifndef SOUND_MANAGER_H
#define SOUND_MANAGER_H

#include <raylib.h>
#include <stdbool.h>

#include "constants.h"

//=================================================================
// C linkage (C++ compiler flag)
//=================================================================
#ifdef __cplusplus
extern "C"
{
#endif

	//=================================================================
	// SFX Types (Mapped to Commands)
	//=================================================================
	typedef enum
	{
		// Movement & Locomotion
		SFX_WALKING, // ACTION_RUN / MOVE_* (footsteps)
		SFX_RUNNING, // ACTION_RUN (faster footsteps)
		SFX_JUMPING, // ACTION_JUMP
		SFX_LANDING, // ACTION_JUMP (when land)
		SFX_CROUCH,	 // ACTION_CROUCH

		// Combat
		SFX_ATTACK_PRIMARY,	  // ATTACK_PRIMARY (melee/primary weapon)
		SFX_ATTACK_SECONDARY, // ATTACK_SECONDARY (special attack)
		SFX_WEAPON_FIRE,	  // ATTACK_PRIMARY (ranged weapon)
		SFX_WEAPON_RELOAD,	  // Reload action

		// Special Abilities
		SFX_SPECIAL_1, // ACTION_SPECIAL_1 (sweep kick)
		SFX_SPECIAL_2, // ACTION_SPECIAL_2 (defend)
		SFX_SPECIAL_3, // ACTION_SPECIAL_3 (dash)
		SFX_SPECIAL_4, // ACTION_SPECIAL_4 (defend block)

		// Interactions
		SFX_PICKUP, // ACTION_PICKUP
		SFX_USE,	// ACTION_PICKUP

		// Player Feedback
		SFX_DAMAGE,	 // Taking damage
		SFX_DEATH,	 // Player death
		SFX_HEAL,	 // Healing/recovery
		SFX_POWERUP, // Pickup powerup

		// Aiming
		SFX_AIM_START, // Start aiming (AIM_*)
		SFX_AIM_STOP,  // Stop aiming
		SFX_AIM_LOCK,  // Lock onto target

		// Environmental
		SFX_EXPLOSION,	  // Explosion nearby
		SFX_IMPACT_HEAVY, // Heavy impact
		SFX_IMPACT_LIGHT, // Light impact

		// UI and System
		SFX_UI_SELECT,	// MENU_TOGGLE
		SFX_UI_CONFIRM, // START_GAME / Confirm
		SFX_UI_CANCEL,	// EXIT_COMMAND / Back
		SFX_UI_ERROR,	// Invalid action
		SFX_PAUSE,		// POWER_COMMAND (Pause)
		SFX_UNPAUSE,	// POWER_COMMAND (Unpause)

		// Achievements and Milestones (optional)
		SFX_ACHIEVEMENT_LEVEL,		// Level up or milestone
		SFX_ACHIEVEMENT_COMBAT,		// Combat achievement
		SFX_ACHIEVEMENT_COLLECTION, // Collectible found

		SFX_TYPE_COUNT // Count of SFX
	} SFXType;

	//=================================================================
	// Music Types
	//=================================================================
	typedef enum
	{
		MUSIC_MENU,		 // Main menu backing track
		MUSIC_GAMEPLAY,	 // Gameplay (switch on tempo mood)
		MUSIC_VICTORY,	 // For stings ?
		MUSIC_GAMEOVER,	 // For stings ?
		MUSIC_TYPE_COUNT // Track count
	} MusicType;

	//=================================================================
	// SFX Playing with priority
	//=================================================================
	typedef struct
	{
		SFXType type; // SFX_TYPE_####
		int priority; // see audio_sfx.ini
		bool active;  // SFX on list
	} SFXPlaying;

	//=================================================================
	// Sound Manager Stats (debugging/monitoring)
	//=================================================================
	typedef struct
	{
		int sfx_loaded;			// Count SFX loaded (lazy loading)
		int music_loaded;		// Music tracks loaded (carefull memory realestate)
		int sfx_playing;		// Count SFX playing
		int total_sfx_types;	// Count of SFX types
		int total_music_types;	// Count of Track types
		int max_concurrent_sfx; // Max sounds loaded and playing conncurrent
	} SoundManagerStats;

	//=================================================================
	// Init Sound Manager
	//=================================================================
	void InitSoundManager();

	//=================================================================
	// Load SFX Config (key=value)
	//=================================================================
	bool LoadSFXConfig(const char *filepath, bool preload);

	//=================================================================
	// Load Music Config (key=value)
	//=================================================================
	bool LoadMusicConfig(const char *filepath, bool preload);

	//=================================================================
	// Load SFX (with lazy loading support)
	//=================================================================
	bool LoadSFXAsset(SFXType type, const char *filepath);

	//=================================================================
	// Load Music Track
	//=================================================================
	bool LoadMusicAsset(MusicType type, const char *filepath);

	//=================================================================
	// Unload SFX (free memory for unused sounds)
	//=================================================================
	void UnloadSFXAsset(SFXType type);

	//=================================================================
	// Unload Music (free memory for unused music)
	//=================================================================
	void UnloadMusicAsset(MusicType type);

	//=================================================================
	// Play SFX (with concurrent limit)
	//=================================================================
	void PlaySFX(SFXType type);

	//=================================================================
	// Play Music
	//=================================================================
	void PlayMusicTrack(MusicType type);

	//=================================================================
	// Stop Music
	//=================================================================
	void StopMusicTrack();

	//=================================================================
	// Update Music Stream (call every frame)
	//=================================================================
	void UpdateSoundManager();

	//=================================================================
	// Set Master Volume (0.0f to 1.0f)
	// On R36S Vol+/-
	// Menu Settings
	//=================================================================
	void SetMasterSoundVolume(float volume);

	//=================================================================
	// Set SFX Volume (0.0f to 1.0f)
	// On R36S Vol+/-
	//=================================================================
	void SetSFXVolume(float volume);

	//=================================================================
	// Set Music Volume (0.0f to 1.0f)
	// On R36S Vol+/-
	//=================================================================
	void SetMusicTrackVolume(float volume);

	//=================================================================
	// Get Master Volume
	//=================================================================
	float GetMasterSoundVolume();

	//=================================================================
	// Get SFX Volume
	//=================================================================
	float GetSFXVolume();

	//=================================================================
	// Get Music Volume
	//=================================================================
	float GetMusicTrackVolume();

	//=================================================================
	// Pause/Resume Music
	//=================================================================
	void PauseMusicTrack();
	void ResumeMusicTrack();

	//=================================================================
	// Check if music is playing
	//=================================================================
	bool IsMusicTrackPlaying();

	//=================================================================
	// Save User Sound Settings
	//=================================================================
	bool SaveSoundSettings(const char *filepath);

	//=================================================================
	// Load User Sound Settings
	//=================================================================
	bool LoadSoundSettings(const char *filepath);

	//=================================================================
	// Get Sound Manager Stats (for monitoring)
	//=================================================================
	SoundManagerStats GetSoundManagerStats();

	//=================================================================
	// Cleanup Sound Manager
	//=================================================================
	void ExitSoundManager();

#ifdef __cplusplus
}
#endif

#endif // SOUND_MANAGER_H
#include <raylib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "core/sound_manager.h"

//=================================================================
// Sound Manager Internal State
//=================================================================
static Sound sfxs[SFX_TYPE_COUNT];
static Music music_tracks[MUSIC_TYPE_COUNT];
static bool sfx_loaded[SFX_TYPE_COUNT];
static bool music_loaded[MUSIC_TYPE_COUNT];

// Thes are read from sound_settings.ini
static float master_volume = 1.0f;
static float sfx_volume = 1.0f;
static float music_volume = 0.7f;

// Backing track
static int current_music = -1;
static bool is_music_playing = false;

// Lazy loading paths
static char sfx_paths[SFX_TYPE_COUNT][256];
static char music_paths[MUSIC_TYPE_COUNT][256];

// Resource tracking
static int sfx_currently_loaded = 0;
static int music_currently_loaded = 0;
static int sfx_currently_playing = 0;

// Track last time SFX was played
static double sfx_last_played_time[SFX_TYPE_COUNT] = {0};

// Runtime arrays (data from audio_sfx.ini)
static float sfx_cooldowns[SFX_TYPE_COUNT];
static int sfx_priorities[SFX_TYPE_COUNT];

//=================================================================
// Compile time safety checks
//=================================================================
#if SFX_TYPE_COUNT > MAX_SFX
#error "SOUND_MANAGER: SFX_TYPE_COUNT exceeds MAX_SFX limit.\n\tIncrease MAX_SFX or reduce SFX_TYPE_#####"
#endif

#if MUSIC_TYPE_COUNT > MAX_MUSIC_TRACKS
#error "SOUND_MANAGER: MUSIC_TYPE_COUNT exceeds MAX_MUSIC_TRACKS limit.\n\tIncrease MAX_MUSIC_TRACKS or reduce MUSIC_TYPE_#####"
#endif

// Mark as currently not playing
static SFXPlaying sfxs_currently_playing[MAX_CONCURRENT_SFX] = {0};

// Check SFX cooldown
static bool IsSFXOnCooldown(SFXType type)
{
	double current_time = GetTime();
	double time_since_last = current_time - sfx_last_played_time[type];
	return time_since_last < sfx_cooldowns[type];
}

// Find free SFX slot based on priority
static int FindSFXSlot(SFXType type, int priority)
{
	// Is SFX Playing
	for (int i = 0; i < MAX_CONCURRENT_SFX; i++)
	{
		if (sfxs_currently_playing[i].active)
		{
			// Check if this SFX is still actually playing
			if (!sfx_loaded[sfxs_currently_playing[i].type] ||
				!IsSoundPlaying(sfxs[sfxs_currently_playing[i].type]))
			{
				sfxs_currently_playing[i].active = false;
			}
		}
	}

	// Empty SFX Slot
	for (int i = 0; i < MAX_CONCURRENT_SFX; i++)
	{
		if (!sfxs_currently_playing[i].active)
		{
			return i;
		}
	}

	// Replace lower priorty SFX if no SFX slot
	int lowest_priority_idx = -1;
	int lowest_priority = priority;

	for (int i = 0; i < MAX_CONCURRENT_SFX; i++)
	{
		if (sfxs_currently_playing[i].priority < lowest_priority)
		{
			lowest_priority = sfxs_currently_playing[i].priority;
			lowest_priority_idx = i;
		}
	}

	// Stop lower priority SFX
	if (lowest_priority_idx != -1)
	{
		SFXType old_type = sfxs_currently_playing[lowest_priority_idx].type;
		if (sfx_loaded[old_type])
		{
			StopSound(sfxs[old_type]);
		}
		return lowest_priority_idx;
	}

	// No SFX Slot and priority of this SFX too Low
	return -1;
}

//=================================================================
// Init Sound Manager
//=================================================================
void InitSoundManager()
{
	TraceLog(LOG_INFO, "SOUND_MANAGER: Initialising");

	// Initialise audio device (Raylib)
	InitAudioDevice();

	if (!IsAudioDeviceReady())
	{
		TraceLog(LOG_ERROR, "SOUND_MANAGER: Failed to initialise audio device");
		return;
	}

	TraceLog(LOG_INFO, "SOUND_MANAGER: Audio device initialised successfully");

	// Initialise arrays
	for (int i = 0; i < SFX_TYPE_COUNT; i++)
	{
		sfx_loaded[i] = false;
		sfx_paths[i][0] = '\0';
		sfx_last_played_time[i] = 0.0;
	}

	for (int i = 0; i < MUSIC_TYPE_COUNT; i++)
	{
		music_loaded[i] = false;
		music_paths[i][0] = '\0';
	}

	// Mark as invalid until valid paths, cooldowns and priorities are loaded
	for (int i = 0; i < SFX_TYPE_COUNT; i++)
	{
		sfx_cooldowns[i] = -1.0f; // invalid
		sfx_priorities[i] = -1;	  // invalid
		sfx_paths[i][0] = '\0';	  // missing path
	}

	// Initialise playing SFX slots
	for (int i = 0; i < MAX_CONCURRENT_SFX; i++)
	{
		sfxs_currently_playing[i].active = false;
		sfxs_currently_playing[i].type = 0;
		sfxs_currently_playing[i].priority = 0;
	}

	// Reset counters
	sfx_currently_loaded = 0;
	music_currently_loaded = 0;
	sfx_currently_playing = 0;

	TraceLog(LOG_INFO, "SOUND_MANAGER: Sound properties initialised with default values");

	// Set initial volume
	SetMasterSoundVolume(master_volume);

#ifdef PLATFORM_R36S
	TraceLog(LOG_INFO, "SOUND_MANAGER: Sound Manager configured for R36S platform");
	TraceLog(LOG_INFO, "\tMax concurrent sounds: %d", MAX_CONCURRENT_SFX);
	TraceLog(LOG_INFO, "\tDefault preload: sounds=%s, music=%s",
			 DEFAULT_PRELOAD_SFX ? "true" : "false",
			 DEFAULT_PRELOAD_MUSIC ? "true" : "false");
	// Platform-specific audio settings for R36S
	// R36S tends to pick up a lot of static noise from poor PCB grounding
	// It might be better to have a switch here to disable sound completely
	// during development of possible reverse this
	//
	// Initialise audio device (Raylib)
	// InitAudioDevice();
	// Lower default volume for handheld
	SetMasterSoundVolume(0.8f);
#else
	TraceLog(LOG_INFO, "SOUND_MANAGER: Configured for standard platform");
	TraceLog(LOG_INFO, "\tMax concurrent sounds: %d", MAX_CONCURRENT_SFX);
#endif

	TraceLog(LOG_INFO, "SOUND_MANAGER: Capacity: %d sounds, %d music tracks",
			 MAX_SFX, MAX_MUSIC_TRACKS);
	TraceLog(LOG_INFO, "\tSound types defined: %d, Music types defined: %d",
			 SFX_TYPE_COUNT, MUSIC_TYPE_COUNT);
}

//=================================================================
// Load SFX Asset
//=================================================================
bool LoadSFXAsset(SFXType type, const char *filepath)
{
	if (type < 0 || type >= SFX_TYPE_COUNT)
	{
		TraceLog(LOG_WARNING, "SOUND_MANAGER: Invalid SFX type: %d", type);
		return false;
	}

	// Check if at capacity
	if (!sfx_loaded[type] && sfx_currently_loaded >= MAX_SFX)
	{
		TraceLog(LOG_ERROR, "SOUND_MANAGER: Cannot load SFX [%d]: MAX_SFX (%d) limit reached",
				 type, MAX_SFX);
		return false;
	}

	// Unload existing SFX if already loaded
	if (sfx_loaded[type])
	{
		UnloadSound(sfxs[type]);
		sfx_loaded[type] = false;
		sfx_currently_loaded--;
	}

	// Load the SFX
	sfxs[type] = LoadSound(filepath);

	if (sfxs[type].frameCount == 0)
	{
		TraceLog(LOG_ERROR, "SOUND_MANAGER: Failed to load SFX: %s", filepath);
		return false;
	}

	sfx_loaded[type] = true;
	sfx_currently_loaded++;

	TraceLog(LOG_INFO, "SOUND_MANAGER: Loaded SFX [%d]: %s (%d/%d loaded)",
			 type, filepath, sfx_currently_loaded, MAX_SFX);

	// Warning when limit approaching
	if (sfx_currently_loaded >= MAX_SFX * 0.8f)
	{
		TraceLog(LOG_WARNING, "SOUND_MANAGER: SFX memory usage high: %d/%d SFXs loaded (%.0f%%)",
				 sfx_currently_loaded, MAX_SFX,
				 (sfx_currently_loaded * 100.0f) / MAX_SFX);
	}

	return true;
}

//=================================================================
// Load Music Asset
//=================================================================
bool LoadMusicAsset(MusicType type, const char *filepath)
{
	if (type < 0 || type >= MUSIC_TYPE_COUNT)
	{
		TraceLog(LOG_WARNING, "SOUND_MANAGER: Invalid music type: %d", type);
		return false;
	}

	// Check if at capacity
	if (!music_loaded[type] && music_currently_loaded >= MAX_MUSIC_TRACKS)
	{
		TraceLog(LOG_ERROR, "SOUND_MANAGER: Cannot load music [%d]: MAX_MUSIC_TRACKS (%d) limit reached",
				 type, MAX_MUSIC_TRACKS);
		return false;
	}

	// Unload existing music if already loaded
	if (music_loaded[type])
	{
		UnloadMusicStream(music_tracks[type]);
		music_loaded[type] = false;
		music_currently_loaded--;
	}

	// Load the music
	music_tracks[type] = LoadMusicStream(filepath);

	if (music_tracks[type].frameCount == 0)
	{
		TraceLog(LOG_ERROR, "SOUND_MANAGER: Failed to load music: %s", filepath);
		return false;
	}

	music_loaded[type] = true;
	music_currently_loaded++;

	TraceLog(LOG_INFO, "SOUND_MANAGER: Loaded music [%d]: %s (%d/%d loaded)",
			 type, filepath, music_currently_loaded, MAX_MUSIC_TRACKS);

	return true;
}

//=================================================================
// Unload SFX Asset
//=================================================================
void UnloadSFXAsset(SFXType type)
{
	if (type < 0 || type >= SFX_TYPE_COUNT)
	{
		TraceLog(LOG_WARNING, "SOUND_MANAGER: Invalid SFX type: %d", type);
		return;
	}

	if (sfx_loaded[type])
	{
		// Stop if playing
		if (IsSoundPlaying(sfxs[type]))
		{
			StopSound(sfxs[type]);
		}

		// Remove from tracking
		for (int i = 0; i < MAX_CONCURRENT_SFX; i++)
		{
			if (sfxs_currently_playing[i].active && sfxs_currently_playing[i].type == type)
			{
				sfxs_currently_playing[i].active = false;
			}
		}

		UnloadSound(sfxs[type]);
		sfx_loaded[type] = false;
		sfx_currently_loaded--;

		TraceLog(LOG_INFO, "SOUND_MANAGER: Unloaded SFX [%d] (%d/%d loaded)", type, sfx_currently_loaded, MAX_SFX);
	}
}

//=================================================================
// Unload Music Asset
//=================================================================
void UnloadMusicAsset(MusicType type)
{
	if (type < 0 || type >= MUSIC_TYPE_COUNT)
	{
		TraceLog(LOG_WARNING, "SOUND_MANAGER: Invalid music type: %d", type);
		return;
	}

	if (music_loaded[type])
	{
		// Stop if currently playing
		if (current_music == (int)type)
		{
			StopMusicStream(music_tracks[type]);
			current_music = -1;
			is_music_playing = false;
		}

		UnloadMusicStream(music_tracks[type]);
		music_loaded[type] = false;
		music_currently_loaded--;
		TraceLog(LOG_INFO, "SOUND_MANAGER: Unloaded music [%d] (%d/%d loaded)",
				 type, music_currently_loaded, MAX_MUSIC_TRACKS);
	}
}

//=================================================================
// Play SFX
//=================================================================
void PlaySFX(SFXType type)
{
	if (type < 0 || type >= SFX_TYPE_COUNT)
	{
		TraceLog(LOG_WARNING, "SOUND_MANAGER: Invalid SFX type: %d", type);
		return;
	}

	if (IsSFXOnCooldown(type))
	{
		TraceLog(LOG_DEBUG, "SOUND_MANAGER: SFX [%d] is on cooldown (%.2fs remaining)",
				 type, sfx_cooldowns[type] - (GetTime() - sfx_last_played_time[type]));
		return;
	}

	// Lazy load SFX if not loaded but path exists
	if (!sfx_loaded[type] && sfx_paths[type][0] != '\0')
	{
		TraceLog(LOG_INFO, "SOUND_MANAGER: Lazy loading SFX [%d]: %s", type, sfx_paths[type]);
		if (!LoadSFXAsset(type, sfx_paths[type]))
		{
			TraceLog(LOG_WARNING, "SOUND_MANAGER: Lazy loading SFX [%d]: %s failed", type, sfx_paths[type]);
			return; // Failed lazy load
		}
	}

	// Check if SFX is loaded
	if (!sfx_loaded[type])
	{
		TraceLog(LOG_WARNING, "SOUND_MANAGER: SFX [%d] not loaded and no path available", type);
		return;
	}

	// Check if SFX already playing
	if (IsSoundPlaying(sfxs[type]))
	{
		TraceLog(LOG_DEBUG, "SOUND_MANAGER: SFX [%d] is already playing, skipping", type);
		return;
	}

	int priority = sfx_priorities[type];
	int slot = FindSFXSlot(type, priority);

	if (slot == -1)
	{
		TraceLog(LOG_DEBUG, "SOUND_MANAGER: No available slot for SFX [%d] (priority %d)",
				 type, priority);
		return;
	}

	// Check concurrent playback limit
	// Note: Raylib doesn't provide an easy way to count playing sounds,
	// so this is a soft limit. Track approximately.
	if (sfx_currently_playing >= MAX_CONCURRENT_SFX)
	{
		TraceLog(LOG_DEBUG, "SOUND_MANAGER: Max concurrent SFXs (%d) reached, SFXs may be clipped",
				 MAX_CONCURRENT_SFX);
		// Still play the SFX: hopefully raylib OR OpenAL will handle it
		// but we log the warning for debugging
	}

	// Set SFX volume based on master and SFX volume
	SetSoundVolume(sfxs[type], master_volume * sfx_volume);

	// Play SFX
	PlaySound(sfxs[type]);

	// Update tracking
	sfxs_currently_playing[slot].active = true;
	sfxs_currently_playing[slot].type = type;
	sfxs_currently_playing[slot].priority = priority;
	sfx_last_played_time[type] = GetTime();

	// Approximate tracking (dont know when SFX finish)
	// Count actual playing SFXs
	sfx_currently_playing = 0;
	for (int i = 0; i < MAX_CONCURRENT_SFX; i++)
	{
		if (sfxs_currently_playing[i].active)
		{
			sfx_currently_playing++;
		}
	}

	TraceLog(LOG_DEBUG, "SOUND_MANAGER: Playing SFX [%d] in slot %d (priority %d, %d/%d slots used)",
			 type, slot, priority, sfx_currently_playing, MAX_CONCURRENT_SFX);
}

//=================================================================
// Play Music Track
//=================================================================
void PlayMusicTrack(MusicType type)
{
	if (type < 0 || type >= MUSIC_TYPE_COUNT)
	{
		TraceLog(LOG_WARNING, "SOUND_MANAGER: Invalid music type: %d", type);
		return;
	}

	// Lazy load if not loaded but path exists
	if (!music_loaded[type] && music_paths[type][0] != '\0')
	{
		TraceLog(LOG_INFO, "SOUND_MANAGER: Lazy loading music [%d]: %s", type, music_paths[type]);
		if (!LoadMusicAsset(type, music_paths[type]))
		{
			return; // Failed to lazy load
		}
	}

	if (!music_loaded[type])
	{
		TraceLog(LOG_WARNING, "SOUND_MANAGER: Music [%d] not loaded and no path available", type);
		return;
	}

	// Stop current music if playing
	if (is_music_playing && current_music >= 0 && music_loaded[current_music])
	{
		StopMusicStream(music_tracks[current_music]);
	}

	// Set volume and play new music
	SetMusicVolume(music_tracks[type], master_volume * music_volume);
	PlayMusicStream(music_tracks[type]);

	current_music = (int)type;
	is_music_playing = true;

	TraceLog(LOG_INFO, "SOUND_MANAGER: Playing music track [%d]", type);
}

//=================================================================
// Stop Music Track
//=================================================================
void StopMusicTrack()
{
	if (is_music_playing && current_music >= 0 && music_loaded[current_music])
	{
		StopMusicStream(music_tracks[current_music]);
		is_music_playing = false;
		TraceLog(LOG_INFO, "SOUND_MANAGER: Stopped music track [%d]", current_music);
	}
}

//=================================================================
// Update Sound Manager (call every frame)
//=================================================================
void UpdateSoundManager()
{
	// Update currently playing music stream
	if (current_music >= 0 && is_music_playing && music_loaded[current_music])
	{
		UpdateMusicStream(music_tracks[current_music]);

		if (!IsMusicStreamPlaying(music_tracks[current_music]))
		{
			is_music_playing = false;
		}
	}

	// Depending on Raylib IsSoundPlaying()
	int active_count = 0;
	for (int i = 0; i < MAX_CONCURRENT_SFX; i++)
	{
		if (sfxs_currently_playing[i].active)
		{
			SFXType type = sfxs_currently_playing[i].type;
			if (!sfx_loaded[type] || !IsSoundPlaying(sfxs[type]))
			{
				sfxs_currently_playing[i].active = false;
			}
			else
			{
				active_count++;
			}
		}
	}
	sfx_currently_playing = active_count;
}

//=================================================================
// Set Master Volume
//=================================================================
void SetMasterSoundVolume(float volume)
{
	master_volume = (volume < 0.0f) ? 0.0f : (volume > 1.0f) ? 1.0f
															 : volume;

	// Use raylib master volume
	SetMasterVolume(master_volume);

	TraceLog(LOG_INFO, "SOUND_MANAGER: Master volume set to: %.2f", master_volume);

	// Update currently playing music
	if (is_music_playing && current_music >= 0 && music_loaded[current_music])
	{
		SetMusicVolume(music_tracks[current_music], master_volume * music_volume);
	}
}

//=================================================================
// Set SFX Volume
//=================================================================
void SetSFXVolume(float volume)
{
	sfx_volume = (volume < 0.0f) ? 0.0f : (volume > 1.0f) ? 1.0f
														  : volume;
	TraceLog(LOG_INFO, "SOUND_MANAGER: SFX volume set to: %.2f", sfx_volume);
}

//=================================================================
// Set Music Volume
//=================================================================
void SetMusicTrackVolume(float volume)
{
	music_volume = (volume < 0.0f) ? 0.0f : (volume > 1.0f) ? 1.0f
															: volume;
	TraceLog(LOG_INFO, "SOUND_MANAGER: Music volume set to: %.2f", music_volume);

	// Update currently playing music
	if (is_music_playing && current_music >= 0 && music_loaded[current_music])
	{
		SetMusicVolume(music_tracks[current_music], master_volume * music_volume);
	}
}

//=================================================================
// Get Master Volume
//=================================================================
float GetMasterSoundVolume()
{
	return master_volume;
}

//=================================================================
// Get SFX Volume
//=================================================================
float GetSFXVolume()
{
	return sfx_volume;
}

//=================================================================
// Get Music Volume
//=================================================================
float GetMusicTrackVolume()
{
	return music_volume;
}

//=================================================================
// Pause Music Track
//=================================================================
void PauseMusicTrack()
{
	if (is_music_playing && current_music >= 0 && music_loaded[current_music])
	{
		PauseMusicStream(music_tracks[current_music]);
		TraceLog(LOG_INFO, "SOUND_MANAGER: Paused music track [%d]", current_music);
	}
}

//=================================================================
// Resume Music Track
//=================================================================
void ResumeMusicTrack()
{
	if (current_music >= 0 && music_loaded[current_music])
	{
		ResumeMusicStream(music_tracks[current_music]);
		is_music_playing = true;
		TraceLog(LOG_INFO, "SOUND_MANAGER: Resumed music track [%d]", current_music);
	}
}

//=================================================================
// Check if music is playing
//=================================================================
bool IsMusicTrackPlaying()
{
	if (current_music >= 0 && music_loaded[current_music])
	{
		return IsMusicStreamPlaying(music_tracks[current_music]);
	}
	return false;
}

//=================================================================
// Get Sound Manager Stats
//=================================================================
SoundManagerStats GetSoundManagerStats()
{
	// TODO : Some stats on telemetry
	SoundManagerStats stats;
	stats.sfx_loaded = sfx_currently_loaded;
	stats.music_loaded = music_currently_loaded;
	stats.sfx_playing = sfx_currently_playing;
	stats.total_sfx_types = SFX_TYPE_COUNT;
	stats.total_music_types = MUSIC_TYPE_COUNT;
	stats.max_concurrent_sfx = MAX_CONCURRENT_SFX;
	return stats;
}

//=================================================================
// Cleanup Sound Manager
//=================================================================
void ExitSoundManager()
{
	TraceLog(LOG_INFO, "SOUND_MANAGER: Cleaning up Sound Manager");

	// Stop and unload all music
	for (int i = 0; i < MUSIC_TYPE_COUNT; i++)
	{
		if (music_loaded[i])
		{
			StopMusicStream(music_tracks[i]);
			UnloadMusicStream(music_tracks[i]);
			music_loaded[i] = false;
		}
	}

	// Unload all SFXs
	for (int i = 0; i < SFX_TYPE_COUNT; i++)
	{
		if (sfx_loaded[i])
		{
			UnloadSound(sfxs[i]);
			sfx_loaded[i] = false;
		}
	}

	// Reset counters
	sfx_currently_loaded = 0;
	music_currently_loaded = 0;
	sfx_currently_playing = 0;

	// Close audio device
	CloseAudioDevice();

	TraceLog(LOG_INFO, "SOUND_MANAGER: Sound Manager cleanup complete");
}

//=================================================================
// Trim whitespace in-place
//=================================================================
static char *Trim(char *s)
{
	while (*s && isspace((unsigned char)*s))
		s++;
	if (*s == 0)
		return s;

	char *end = s + strlen(s) - 1;
	while (end > s && isspace((unsigned char)*end))
		end--;
	end[1] = '\0';
	return s;
}

//=================================================================
// Retrieve SFXType from string key
//=================================================================
static bool SFXTypeFromString(const char *key, SFXType *out)
{
	//-------------------------------------------------------------
	// Movement and Locomotion
	//-------------------------------------------------------------
	if (strcmp(key, "SFX_WALKING") == 0)
	{
		*out = SFX_WALKING;
		return true;
	}
	if (strcmp(key, "SFX_RUNNING") == 0)
	{
		*out = SFX_RUNNING;
		return true;
	}
	if (strcmp(key, "SFX_JUMPING") == 0)
	{
		*out = SFX_JUMPING;
		return true;
	}
	if (strcmp(key, "SFX_LANDING") == 0)
	{
		*out = SFX_LANDING;
		return true;
	}
	if (strcmp(key, "SFX_CROUCH") == 0)
	{
		*out = SFX_CROUCH;
		return true;
	}

	//-------------------------------------------------------------
	// Combat
	//-------------------------------------------------------------
	if (strcmp(key, "SFX_ATTACK_PRIMARY") == 0)
	{
		*out = SFX_ATTACK_PRIMARY;
		return true;
	}
	if (strcmp(key, "SFX_ATTACK_SECONDARY") == 0)
	{
		*out = SFX_ATTACK_SECONDARY;
		return true;
	}
	if (strcmp(key, "SFX_WEAPON_FIRE") == 0)
	{
		*out = SFX_WEAPON_FIRE;
		return true;
	}
	if (strcmp(key, "SFX_WEAPON_RELOAD") == 0)
	{
		*out = SFX_WEAPON_RELOAD;
		return true;
	}

	//-------------------------------------------------------------
	// Special Abilities
	//-------------------------------------------------------------
	if (strcmp(key, "SFX_SPECIAL_1") == 0)
	{
		*out = SFX_SPECIAL_1;
		return true;
	}
	if (strcmp(key, "SFX_SPECIAL_2") == 0)
	{
		*out = SFX_SPECIAL_2;
		return true;
	}
	if (strcmp(key, "SFX_SPECIAL_3") == 0)
	{
		*out = SFX_SPECIAL_3;
		return true;
	}
	if (strcmp(key, "SFX_SPECIAL_4") == 0)
	{
		*out = SFX_SPECIAL_4;
		return true;
	}

	//-------------------------------------------------------------
	// Interactions
	//-------------------------------------------------------------
	if (strcmp(key, "SFX_PICKUP") == 0)
	{
		*out = SFX_PICKUP;
		return true;
	}
	if (strcmp(key, "SFX_USE") == 0)
	{
		*out = SFX_USE;
		return true;
	}

	//-------------------------------------------------------------
	// Player Feedback
	//-------------------------------------------------------------
	if (strcmp(key, "SFX_DAMAGE") == 0)
	{
		*out = SFX_DAMAGE;
		return true;
	}
	if (strcmp(key, "SFX_DEATH") == 0)
	{
		*out = SFX_DEATH;
		return true;
	}
	if (strcmp(key, "SFX_HEAL") == 0)
	{
		*out = SFX_HEAL;
		return true;
	}
	if (strcmp(key, "SFX_POWERUP") == 0)
	{
		*out = SFX_POWERUP;
		return true;
	}

	//-------------------------------------------------------------
	// Aiming
	//-------------------------------------------------------------
	if (strcmp(key, "SFX_AIM_START") == 0)
	{
		*out = SFX_AIM_START;
		return true;
	}
	if (strcmp(key, "SFX_AIM_STOP") == 0)
	{
		*out = SFX_AIM_STOP;
		return true;
	}
	if (strcmp(key, "SFX_AIM_LOCK") == 0)
	{
		*out = SFX_AIM_LOCK;
		return true;
	}

	//-------------------------------------------------------------
	// Environmental
	//-------------------------------------------------------------
	if (strcmp(key, "SFX_EXPLOSION") == 0)
	{
		*out = SFX_EXPLOSION;
		return true;
	}
	if (strcmp(key, "SFX_IMPACT_HEAVY") == 0)
	{
		*out = SFX_IMPACT_HEAVY;
		return true;
	}
	if (strcmp(key, "SFX_IMPACT_LIGHT") == 0)
	{
		*out = SFX_IMPACT_LIGHT;
		return true;
	}

	//-------------------------------------------------------------
	// UI and System
	//-------------------------------------------------------------
	if (strcmp(key, "SFX_UI_SELECT") == 0)
	{
		*out = SFX_UI_SELECT;
		return true;
	}
	if (strcmp(key, "SFX_UI_CONFIRM") == 0)
	{
		*out = SFX_UI_CONFIRM;
		return true;
	}
	if (strcmp(key, "SFX_UI_CANCEL") == 0)
	{
		*out = SFX_UI_CANCEL;
		return true;
	}
	if (strcmp(key, "SFX_UI_ERROR") == 0)
	{
		*out = SFX_UI_ERROR;
		return true;
	}
	if (strcmp(key, "SFX_PAUSE") == 0)
	{
		*out = SFX_PAUSE;
		return true;
	}
	if (strcmp(key, "SFX_UNPAUSE") == 0)
	{
		*out = SFX_UNPAUSE;
		return true;
	}

	//-------------------------------------------------------------
	// Achievements and Progression
	//-------------------------------------------------------------
	if (strcmp(key, "SFX_ACHIEVEMENT_LEVEL") == 0)
	{
		*out = SFX_ACHIEVEMENT_LEVEL;
		return true;
	}
	if (strcmp(key, "SFX_ACHIEVEMENT_COMBAT") == 0)
	{
		*out = SFX_ACHIEVEMENT_COMBAT;
		return true;
	}
	if (strcmp(key, "SFX_ACHIEVEMENT_COLLECTION") == 0)
	{
		*out = SFX_ACHIEVEMENT_COLLECTION;
		return true;
	}

	return false;
}

//=================================================================
// Retrieve MusicType from string key
//=================================================================
static bool MusicTypeFromString(const char *key, MusicType *out)
{
	if (strcmp(key, "MUSIC_MENU") == 0)
	{
		*out = MUSIC_MENU;
		return true;
	}
	if (strcmp(key, "MUSIC_GAMEPLAY") == 0)
	{
		*out = MUSIC_GAMEPLAY;
		return true;
	}
	if (strcmp(key, "MUSIC_VICTORY") == 0)
	{
		*out = MUSIC_VICTORY;
		return true;
	}
	if (strcmp(key, "MUSIC_GAMEOVER") == 0)
	{
		*out = MUSIC_GAMEOVER;
		return true;
	}
	return false;
}

//=================================================================
// Load SFX Config (key=value)
//=================================================================
bool LoadSFXConfig(const char *filepath, bool preload)
{
    FILE *f = fopen(filepath, "r");
    if (!f)
    {
        TraceLog(LOG_ERROR, "SOUND_MANAGER: SFX config file missing: %s", filepath);
        return false;
    }

    bool configured[SFX_TYPE_COUNT] = {0};
    char line[512];
    int line_no = 0;

    while (fgets(line, (int)sizeof(line), f))
    {
        line_no++;

        char *s = Trim(line);

        // Skip empty/comment lines
        if (*s == '\0' || *s == '#' || *s == ';')
            continue;

        // Split key=value
        char *equals = strchr(s, '=');
        if (!equals)
        {
            TraceLog(LOG_ERROR, "SOUND_MANAGER: %s:%d invalid line (missing '='): %s", filepath, line_no, s);
            fclose(f);
            return false;
        }

        *equals = '\0';
        char *key = Trim(s);
        char *val = Trim(equals + 1);

        // Parse SFXType
        SFXType type;
        if (!SFXTypeFromString(key, &type))
        {
            TraceLog(LOG_ERROR, "SOUND_MANAGER: %s:%d unknown SFX key: %s", filepath, line_no, key);
            fclose(f);
            return false;
        }

        // Reject duplicates
        if (configured[type])
        {
            TraceLog(LOG_ERROR, "SOUND_MANAGER: %s:%d duplicate SFX key: %s", filepath, line_no, key);
            fclose(f);
            return false;
        }

        // Split value by semicolons: path;cooldown;priority
        char *path = strtok(val, ";");
        char *cooldown_str = strtok(NULL, ";");
        char *priority_str = strtok(NULL, ";");
        char *extra = strtok(NULL, ";");

        if (!path || !cooldown_str || !priority_str || extra)
        {
            TraceLog(LOG_ERROR,
                     "SOUND_MANAGER: %s:%d invalid format for %s (expected path;cooldown;priority)",
                     filepath, line_no, key);
            fclose(f);
            return false;
        }

        path = Trim(path);
        cooldown_str = Trim(cooldown_str);
        priority_str = Trim(priority_str);

        // Validate path non-empty
        if (path[0] == '\0')
        {
            TraceLog(LOG_ERROR, "SOUND_MANAGER: %s:%d empty path for %s", filepath, line_no, key);
            fclose(f);
            return false;
        }

        float cooldown = 0.0f;
        int priority = 0;

        // Parse cooldown and priority
        if (sscanf(cooldown_str, "%f", &cooldown) != 1)
        {
            TraceLog(LOG_ERROR, "SOUND_MANAGER: %s:%d invalid cooldown for %s: %s",
                     filepath, line_no, key, cooldown_str);
            fclose(f);
            return false;
        }
        if (sscanf(priority_str, "%d", &priority) != 1)
        {
            TraceLog(LOG_ERROR, "SOUND_MANAGER: %s:%d invalid priority for %s: %s",
                     filepath, line_no, key, priority_str);
            fclose(f);
            return false;
        }

        // Validate ranges (strict: reject, not clamp)
        if (cooldown < 0.0f || cooldown > 10.0f)
        {
            TraceLog(LOG_ERROR, "SOUND_MANAGER: %s:%d cooldown out of range for %s: %.2f",
                     filepath, line_no, key, cooldown);
            fclose(f);
            return false;
        }
        if (priority < 0 || priority > 10)
        {
            TraceLog(LOG_ERROR, "SOUND_MANAGER: %s:%d priority out of range for %s: %d",
                     filepath, line_no, key, priority);
            fclose(f);
            return false;
        }

        // Ensure file exists now (recommended)
        FILE *test = fopen(path, "rb");
        if (!test)
        {
            TraceLog(LOG_ERROR, "SOUND_MANAGER: %s:%d referenced SFX file not found: %s (key=%s)",
                     filepath, line_no, path, key);
            fclose(f);
            return false;
        }
        fclose(test);

        // Store config
        snprintf(sfx_paths[type], sizeof(sfx_paths[type]), "%s", path);
        sfx_cooldowns[type] = cooldown;
        sfx_priorities[type] = priority;
        configured[type] = true;

        // Preload if preload=true
        if (preload)
        {
            if (!LoadSFXAsset(type, path))
            {
                TraceLog(LOG_ERROR, "SOUND_MANAGER: %s:%d failed to preload SFX %s (%s)",
                         filepath, line_no, key, path);
                fclose(f);
                return false;
            }
        }
    }

    fclose(f);

    // Ensure all SFX types are defined in config
    for (int i = 0; i < SFX_TYPE_COUNT; i++)
    {
        if (!configured[i])
        {
            TraceLog(LOG_ERROR, "SOUND_MANAGER: Missing SFX entry in %s for type index %d", filepath, i);
            return false;
        }
        if (sfx_paths[i][0] == '\0' || sfx_cooldowns[i] < 0.0f || sfx_priorities[i] < 0)
        {
            TraceLog(LOG_ERROR, "SOUND_MANAGER: Incomplete SFX config for type index %d", i);
            return false;
        }
    }

    TraceLog(LOG_INFO, "SOUND_MANAGER: SFX config loaded OK: %s (preload=%s)", filepath, preload ? "true" : "false");
    return true;
}

//=================================================================
// Load Music Config (key=value)
//=================================================================
bool LoadMusicConfig(const char *filepath, bool preload)
{
	FILE *f = fopen(filepath, "r");
	if (!f)
	{
		TraceLog(LOG_ERROR, "SOUND_MANAGER: Failed to open music config: %s", filepath);
		return false;
	}

	char line[512];
	int loaded_count = 0;
	int path_count = 0;

	while (fgets(line, (int)sizeof(line), f))
	{
		char *s = Trim(line);

		// Skip empty / comment lines
		if (*s == '\0' || *s == '#' || *s == ';')
			continue;

		// Split key=value
		char *equals = strchr(s, '=');
		if (!equals)
		{
			TraceLog(LOG_WARNING, "SOUND_MANAGER: Invalid line (missing '='): %s", s);
			continue;
		}

		*equals = '\0';
		char *key = Trim(s);
		char *val = Trim(equals + 1);

		MusicType type;
		if (!MusicTypeFromString(key, &type))
		{
			TraceLog(LOG_WARNING, "SOUND_MANAGER: Unknown music key: %s", key);
			continue;
		}

		// Store path for lazy load
		snprintf(music_paths[type], sizeof(music_paths[type]), "%s", val);
		path_count++;

		if (preload)
		{
			if (LoadMusicAsset(type, val))
				loaded_count++;
		}
	}

	fclose(f);

	TraceLog(LOG_INFO, "SOUND_MANAGER: Music config loaded: %s (preload=%d, paths=%d, loaded=%d)",
			 filepath, preload ? 1 : 0, path_count, loaded_count);
	return (loaded_count > 0) || !preload;
}

//=================================================================
// Save User Sound Settings
//=================================================================
bool SaveSoundSettings(const char *filepath)
{
	FILE *f = fopen(filepath, "w");
	if (!f)
	{
		TraceLog(LOG_ERROR, "SOUND_MANAGER: Failed to open file for writing: %s", filepath);
		return false;
	}

	fprintf(f, "# Game Sound Manager: Player Settings\n");
	fprintf(f, "# Auto-generated: DO NOT EDIT\n\n");
	fprintf(f, "MASTER_VOLUME=%.3f\n", master_volume);
	fprintf(f, "SFX_VOLUME=%.3f\n", sfx_volume);
	fprintf(f, "MUSIC_VOLUME=%.3f\n", music_volume);

	fclose(f);

	TraceLog(LOG_INFO, "SOUND_MANAGER: Sound settings saved: %s", filepath);
	return true;
}

//=================================================================
// Load User Sound Settings
//=================================================================
bool LoadSoundSettings(const char *filepath)
{
	FILE *f = fopen(filepath, "r");
	if (!f)
	{
		TraceLog(LOG_WARNING, "SOUND_MANAGER: Sound settings file not found: %s (using defaults)", filepath);
		return false;
	}

	char line[256];
	int loaded_count = 0;

	while (fgets(line, (int)sizeof(line), f))
	{
		char *s = Trim(line);

		// Skip empty/comment lines
		if (*s == '\0' || *s == '#' || *s == ';')
			continue;

		// Split key=value
		char *equals = strchr(s, '=');
		if (!equals)
			continue;

		*equals = '\0';
		char *key = Trim(s);
		char *val = Trim(equals + 1);

		float value = 0.0f;
		if (sscanf(val, "%f", &value) != 1)
		{
			TraceLog(LOG_WARNING, "SOUND_MANAGER: Invalid value for %s: %s", key, val);
			continue;
		}

		// Clamp to valid range
		value = (value < 0.0f) ? 0.0f : (value > 1.0f) ? 1.0f
													   : value;

		if (strcmp(key, "MASTER_VOLUME") == 0)
		{
			SetMasterSoundVolume(value);
			loaded_count++;
		}
		else if (strcmp(key, "SFX_VOLUME") == 0)
		{
			SetSFXVolume(value);
			loaded_count++;
		}
		else if (strcmp(key, "MUSIC_VOLUME") == 0)
		{
			SetMusicTrackVolume(value);
			loaded_count++;
		}
		else
		{
			TraceLog(LOG_WARNING, "SOUND_MANAGER: Unknown settings key: %s", key);
		}
	}

	fclose(f);

	TraceLog(LOG_INFO, "SOUND_MANAGER: Sound settings loaded: %s (loaded %d settings)", filepath, loaded_count);
	return loaded_count > 0;
}
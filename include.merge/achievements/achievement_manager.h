#ifndef ACHIEVEMENT_MANAGER_H
#define ACHIEVEMENT_MANAGER_H

#include <raylib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <xxhash.h>

#include "constants.h"
#include "core/sound_manager.h"

//=================================================================
// C linkage (C++ compiler flag)
//=================================================================
#ifdef __cplusplus
extern "C"
{
#endif

//=================================================================
// Compile-time Definition checks
//=================================================================
#ifndef MAX_ACHIEVEMENTS
#error "ACHIEVEMENT_MANAGER: MAX_ACHIEVEMENTS not defined in constants.h"
#endif
#ifndef MAX_CATEGORIES
#error "ACHIEVEMENT_MANAGER: MAX_CATEGORIES not defined in constants.h"
#endif
#ifndef MAX_OBSERVERS
#error "ACHIEVEMENT_MANAGER: MAX_OBSERVERS not defined in constants.h"
#endif
#ifndef MAX_LEVELS
#error "ACHIEVEMENT_MANAGER: MAX_LEVELS not defined in constants.h"
#endif
#ifndef MAX_ACHIEVEMENT_NAME
#error "ACHIEVEMENT_MANAGER: MAX_ACHIEVEMENT_NAME not defined in constants.h"
#endif
#ifndef MAX_ACHIEVEMENT_DESC
#error "ACHIEVEMENT_MANAGER: MAX_ACHIEVEMENT_DESC not defined in constants.h"
#endif
#ifndef MAX_CATEGORY_NAME
#error "ACHIEVEMENT_MANAGER: MAX_CATEGORY_NAME not defined in constants.h"
#endif
#ifndef ACHIEVEMENT_DISPLAY_DURATION
#error "ACHIEVEMENT_MANAGER: ACHIEVEMENT_DISPLAY_DURATION not defined in constants.h"
#endif
#ifndef ACHIEVEMENT_POPUP_WIDTH
#error "ACHIEVEMENT_MANAGER: ACHIEVEMENT_POPUP_WIDTH not defined in constants.h"
#endif
#ifndef ACHIEVEMENT_POPUP_HEIGHT
#error "ACHIEVEMENT_MANAGER: ACHIEVEMENT_POPUP_HEIGHT not defined in constants.h"
#endif
#ifndef ACHIEVEMENT_POPUP_X_OFFSET
#error "ACHIEVEMENT_MANAGER: ACHIEVEMENT_POPUP_X_OFFSET not defined in constants.h"
#endif
#ifndef ACHIEVEMENT_POPUP_Y_START
#error "ACHIEVEMENT_MANAGER: ACHIEVEMENT_POPUP_Y_START not defined in constants.h"
#endif
#ifndef ACHIEVEMENT_POPUP_Y_SPACING
#error "ACHIEVEMENT_MANAGER: ACHIEVEMENT_POPUP_Y_SPACING not defined in constants.h"
#endif
#ifndef ALL_LEVELS_ACHIEVEMENT
#error "ACHIEVEMENT_MANAGER: ALL_LEVELS_ACHIEVEMENT not defined in constants.h"
#endif
#ifndef ACHIEVEMENT_PROGRESS_FILE
#error "ACHIEVEMENT_MANAGER: ACHIEVEMENT_PROGRESS_FILE not defined in constants.h"
#endif
#ifndef LEVEL_PROGRESS_FILE
#error "ACHIEVEMENT_MANAGER: LEVEL_PROGRESS_FILE not defined in constants.h"
#endif
#ifndef SCENE_SPLASH_BACKGROUND_FILE
#error "ACHIEVEMENT_MANAGER: SCENE_SPLASH_BACKGROUND_FILE not defined in constants.h"
#endif
#ifndef SCENE_GAMEPLAY_BACKGROUND_FILE
#error "ACHIEVEMENT_MANAGER: SCENE_GAMEPLAY_BACKGROUND_FILE not defined in constants.h"
#endif
#ifndef SCENE_LEVEL_COMPLETE_BACKGROUND_FILE
#error "ACHIEVEMENT_MANAGER: SCENE_LEVEL_COMPLETE_BACKGROUND_FILE not defined in constants.h"
#endif
#ifndef SCENE_GAME_OVER_BACKGROUND_FILE
#error "ACHIEVEMENT_MANAGER: SCENE_GAME_OVER_BACKGROUND_FILE not defined in constants.h"
#endif
#ifndef SCENE_GAME_COMPLETE_BACKGROUND_FILE
#error "ACHIEVEMENT_MANAGER: SCENE_GAME_COMPLETE_BACKGROUND_FILE not defined in constants.h"
#endif

	//=================================================================
	// Achievement type
	//=================================================================
	typedef enum
	{
		ACHIEVEMENT_INSTANT, // Unlocked in one call
		ACHIEVEMENT_PROGRESS // Unlocked when current_progress >= target_progress
	} AchievementType;

	//=================================================================
	// Category sound type
	//=================================================================
	typedef enum
	{
		ACHIEVEMENT_LEVEL_SOUND,	 // e.g. complete a level
		ACHIEVEMENT_COMBAT_SOUND,	 // e.g. kills / combos
		ACHIEVEMENT_COLLECTION_SOUND // e.g. collectibles
	} AchievementSoundType;

	//=================================================================
	// Category
	//=================================================================
	typedef struct
	{
		char name[MAX_CATEGORY_NAME];
		Color color;
		AchievementSoundType sound_type;
	} Category;

	//=================================================================
	// Achievement
	//=================================================================
	typedef struct
	{
		uint64_t unique_ID;
		char name[MAX_ACHIEVEMENT_NAME];
		char description[MAX_ACHIEVEMENT_DESC];
		bool unlocked;
		float display_timer; // Countdown while popup is visible
		bool is_achievement_displaying;
		AchievementType type;
		int current_progress;
		int target_progress;
		int category_index;
		int level; // 0 = all levels, 1..N = specific level
	} Achievement;

	//=================================================================
	// Level progress
	//=================================================================
	typedef struct
	{
		int current_level;
		bool level_unlocked[MAX_LEVELS];
	} LevelProgress;

	//=================================================================
	// Observer (caller allocates, manager holds a pointer)
	//=================================================================
	typedef struct
	{
		void *context;								 // Caller-owned context (manager never frees this)
		void (*callback)(void *context, void *data); // Called on achievement unlock
		uint64_t unique_ID;							 // Assigned by AddObserver()
	} AchievementObserver;

	//=================================================================
	// Achievement Manager
	//=================================================================
	typedef struct
	{
		Achievement achievements[MAX_ACHIEVEMENTS];
		int achievement_count;

		AchievementObserver *observers[MAX_OBSERVERS];
		int observer_count;

		Category categories[MAX_CATEGORIES];
		int category_count;

		LevelProgress level_progress;
	} AchievementManager;

	//----------------------------------------------------------------
	// Lifecycle
	//----------------------------------------------------------------
	AchievementManager *InitAchievementManager(void);
	void ExitAchievementManager(AchievementManager *manager);

	//----------------------------------------------------------------
	// Categories
	//----------------------------------------------------------------
	int AddCategory(AchievementManager *manager, const char *name, Color color, AchievementSoundType sound_type);
	Category *GetCategory(AchievementManager *manager, int index);

	//----------------------------------------------------------------
	// Adding achievements
	//----------------------------------------------------------------
	uint64_t AddInstantAchievement(AchievementManager *manager, const char *name, const char *description,
								   int category_index, int level);

	uint64_t AddProgressAchievement(AchievementManager *manager, const char *name, const char *description,
									int target_progress, int category_index, int level);

	// Convenience: register one achievement per level (1..MAX_LEVELS)
	void AddInstantAchievementAllLevels(AchievementManager *manager, const char *name, const char *description,
										int category_index, uint64_t *unique_ID_out, int *count_out);

	void AddProgressAchievementAllLevels(AchievementManager *manager, const char *name, const char *description,
										 int target_progress, int category_index,
										 uint64_t *unique_ID_out, int *count_out);

	//----------------------------------------------------------------
	// Lookup
	//----------------------------------------------------------------
	Achievement *FindAchievementByID(AchievementManager *manager, uint64_t unique_ID);
	Achievement *FindAchievementByName(AchievementManager *manager, const char *name);
	Achievement *FindAchievementByNameAndLevel(AchievementManager *manager, const char *name, int level);

	//----------------------------------------------------------------
	// Observers (caller owns AchievementObserver memory)
	//----------------------------------------------------------------
	uint64_t GenerateObserverUniqueID(void *context, void (*callback)(void *, void *));
	uint64_t AddObserver(AchievementManager *manager, AchievementObserver *observer);
	bool RemoveObserver(AchievementManager *manager, uint64_t observer_ID);
	void NotifyObservers(AchievementManager *manager, void *data);

	//----------------------------------------------------------------
	// Unlocking
	//----------------------------------------------------------------
	void UnlockAchievementByName(AchievementManager *manager, const char *name, int level);
	void UnlockAchievementByID(AchievementManager *manager, uint64_t unique_ID, int level);
	void UnlockInstantAchievementAtThreshold(AchievementManager *manager, const char *name, int level,
											 int threshold, int current_value);

	//----------------------------------------------------------------
	// Progress tracking
	//----------------------------------------------------------------
	void UpdateAchievementProgress(AchievementManager *manager, const char *name, int progress, int level);

	//----------------------------------------------------------------
	// Per-frame update & draw
	//----------------------------------------------------------------
	void UpdateAchievements(AchievementManager *manager);
	void DrawAchievements(AchievementManager *manager);

	//----------------------------------------------------------------
	// Persistence
	//----------------------------------------------------------------
	void LoadAchievements(AchievementManager *manager);
	void SaveAchievements(AchievementManager *manager);

	//----------------------------------------------------------------
	// Level progress helpers
	//----------------------------------------------------------------
	void InitLevelProgress(LevelProgress *progress);
	void LoadLevelProgress(LevelProgress *progress);
	void SaveLevelProgress(const LevelProgress *progress);
	bool IsLevelUnlocked(AchievementManager *manager, int level);
	int GetCurrentLevel(AchievementManager *manager);
	void SetCurrentLevel(AchievementManager *manager, int level);
	void UnlockNextLevel(AchievementManager *manager);

	//----------------------------------------------------------------
	// Category sounds
	// Sounds are played via PlaySFX() in sound_manager - no separate
	// load/unload needed here.  Configure paths in audio_sfx.ini:
	//   SFX_ACHIEVEMENT_LEVEL / SFX_ACHIEVEMENT_COMBAT / SFX_ACHIEVEMENT_COLLECTION
	//----------------------------------------------------------------
	void PlayAchievementSound(AchievementManager *manager, Achievement *achievement);

	//----------------------------------------------------------------
	// ID generation (exposed for testing; prefer using Add* functions)
	//----------------------------------------------------------------
	uint64_t GenerateAchievementUniqueID(const char *name, const char *description, int category_index);

#ifdef __cplusplus
}
#endif

#endif // ACHIEVEMENT_MANAGER_H
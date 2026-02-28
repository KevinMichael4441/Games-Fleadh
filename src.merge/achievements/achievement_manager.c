#include <raylib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <xxhash.h>

#include "achievements/achievement_manager.h"
#include "core/sound_manager.h"
#include "memory/memory_manager.h"

//=================================================================
// Compile-time capacity checks
//=================================================================
#if MAX_ACHIEVEMENTS > 512
#error "ACHIEVEMENT_MANAGER: MAX_ACHIEVEMENTS seems unreasonably large for R36S (>512)"
#endif
#if MAX_OBSERVERS > 64
#error "ACHIEVEMENTS: MAX_OBSERVERS seems unreasonably large for R36S (>64)"
#endif

//=================================================================
// Helpers
//=================================================================

static uint64_t HashString(const char *s, uint64_t seed)
{
	return XXH64(s, strlen(s), seed);
}

static void InitAchievementDefaults(Achievement *achievement, const char *name, const char *description,
									AchievementType type, int target_progress,
									int category_index, int level)
{
	strncpy(achievement->name, name, MAX_ACHIEVEMENT_NAME - 1);
	achievement->name[MAX_ACHIEVEMENT_NAME - 1] = '\0';

	strncpy(achievement->description, description, MAX_ACHIEVEMENT_DESC - 1);
	achievement->description[MAX_ACHIEVEMENT_DESC - 1] = '\0';

	achievement->unlocked = false;
	achievement->display_timer = 0.0f;
	achievement->is_achievement_displaying = false;
	achievement->type = type;
	achievement->current_progress = 0;
	achievement->target_progress = target_progress;
	achievement->category_index = category_index;
	achievement->level = level;
	achievement->unique_ID = 0;
}

// Internal unlock: shared by name/ID paths and UpdateAchievementProgress
static void UnlockAchievement(AchievementManager *manager, Achievement *achievement)
{
	achievement->unlocked = true;
	achievement->is_achievement_displaying = true;
	achievement->display_timer = ACHIEVEMENT_DISPLAY_DURATION;

	PlayAchievementSound(manager, achievement);
	NotifyObservers(manager, achievement);
	SaveAchievements(manager);

	TraceLog(LOG_INFO, "ACHIEVEMENT_MANAGER: Unlocked '%s' (level %d)", achievement->name, achievement->level);
}

//=================================================================
// Lifecycle
//=================================================================

AchievementManager *InitAchievementManager(void)
{
	TraceLog(LOG_INFO, "ACHIEVEMENT_MANAGER: Initialising");

	AchievementManager *manager = (AchievementManager *)MemoryAlloc(sizeof(AchievementManager),
																	"AchievementManager");
	if (!manager)
	{
		TraceLog(LOG_ERROR, "ACHIEVEMENT_MANAGER: Failed to allocate AchievementManager");
		return NULL;
	}

	manager->achievement_count = 0;
	manager->observer_count = 0;
	manager->category_count = 0;

	InitLevelProgress(&manager->level_progress);
	LoadLevelProgress(&manager->level_progress);

	// Sounds are owned by the sound manager: no loading here.
	// Ensure InitSoundManager() and LoadSFXConfig() are called before InitAchievementManager().
	TraceLog(LOG_INFO, "ACHIEVEMENT_MANAGER: Ready  (achievements: 0/%d  categories: 0/%d  observers: 0/%d)",
			 MAX_ACHIEVEMENTS, MAX_CATEGORIES, MAX_OBSERVERS);

	return manager;
}

//=================================================================
//
// Ownership rules:
//
//   AchievementManager*          Allocated by InitAchievementManager via MemoryAlloc.
//                                Freed here.
//
//   AchievementObserver*         CALLER must allocate via MemoryAlloc before calling
//                                AddObserver(). The manager takes ownership on
//                                AddObserver() and frees it here.
//
//   AchievementObserver.context  CALLER must allocate via MemoryAlloc.
//                                The manager frees it here alongside the observer.
//
// Notes:
//   Once an observer is registered the manager is the only entity holding its
//   pointer. Freeing it anywhere else is a use-after-free. Using MemoryAlloc
//   for all heap objects means MemoryCleanup() catches any slip-through at exit.
//=================================================================
void ExitAchievementManager(AchievementManager *manager)
{
	if (!manager)
		return;

	TraceLog(LOG_INFO, "ACHIEVEMENT_MANAGER: Exiting (%d achievements, %d observers)",
			 manager->achievement_count, manager->observer_count);

	// Sounds are owned by the Sound Manager
	// ExitSoundManager() unloads them.

	for (int i = 0; i < manager->observer_count; i++)
	{
		AchievementObserver *observer = manager->observers[i];
		if (!observer)
			continue;

		if (observer->context)
		{
			MemoryFree(observer->context);
			observer->context = NULL;
		}

		MemoryFree(observer);
		manager->observers[i] = NULL;
	}

	manager->observer_count = 0;

	MemoryFree(manager);
}

//=================================================================
// ID generation
//=================================================================

uint64_t GenerateAchievementUniqueID(const char *name, const char *description, int category_index)
{
	uint64_t hash = 0;
	hash = HashString(name, hash);
	hash = HashString(description, hash);
	hash = XXH64(&category_index, sizeof(int), hash);
	return hash;
}

uint64_t GenerateObserverUniqueID(void *context, void (*callback)(void *, void *))
{
	// Combine pointer values + timestamp so re-registration still gets a fresh ID
	uint64_t hash = 0;
	hash = XXH64(&context, sizeof(void *), hash);
	hash = XXH64(&callback, sizeof(void *), hash);
	uint64_t ts = (uint64_t)time(NULL);
	hash = XXH64(&ts, sizeof(uint64_t), hash);
	return hash;
}

//=================================================================
// Categories
//=================================================================

int AddCategory(AchievementManager *manager, const char *name, Color color, AchievementSoundType sound_type)
{
	if (!manager || !name)
		return -1;

	if (manager->category_count >= MAX_CATEGORIES)
	{
		TraceLog(LOG_WARNING, "ACHIEVEMENT_MANAGER: Cannot add category '%s' => MAX_CATEGORIES (%d) reached",
				 name, MAX_CATEGORIES);
		return -1;
	}

	Category *category = &manager->categories[manager->category_count];
	strncpy(category->name, name, MAX_CATEGORY_NAME - 1);
	category->name[MAX_CATEGORY_NAME - 1] = '\0';
	category->color = color;
	category->sound_type = sound_type;

	int idx = manager->category_count++;
	TraceLog(LOG_INFO, "ACHIEVEMENT_MANAGER: Category [%d] '%s' added", idx, name);
	return idx;
}

Category *GetCategory(AchievementManager *manager, int index)
{
	if (!manager || index < 0 || index >= manager->category_count)
		return NULL;
	return &manager->categories[index];
}

//=================================================================
// Adding achievements
// Achievements live in Achievement Manager inline array
//=================================================================

uint64_t AddInstantAchievement(AchievementManager *manager, const char *name, const char *description,
							   int category_index, int level)
{
	if (!manager || !name || !description)
		return 0;

	// NOTE: ALL_LEVELS_ACHIEVEMENT (0) is valid here for a one-shot global instant
	// achievement that can fire on any level (e.g. "Coin Collector" first coin ever).
	// AddInstantAchievementAllLevels() creates one separate achievement PER level,
	// which is a different pattern entirely.

	if (manager->achievement_count >= MAX_ACHIEVEMENTS)
	{
		TraceLog(LOG_ERROR, "ACHIEVEMENT_MANAGER: Cannot add '%s' => MAX_ACHIEVEMENTS (%d) reached",
				 name, MAX_ACHIEVEMENTS);
		return 0;
	}

	Achievement *achievement = &manager->achievements[manager->achievement_count++];
	InitAchievementDefaults(achievement, name, description, ACHIEVEMENT_INSTANT, 1, category_index, level);
	achievement->unique_ID = GenerateAchievementUniqueID(achievement->name, achievement->description, category_index);

	TraceLog(LOG_INFO, "ACHIEVEMENT_MANAGER: Instant  [%016llx] '%s' level=%d  (%d/%d)",
			 achievement->unique_ID, name, level, manager->achievement_count, MAX_ACHIEVEMENTS);
	return achievement->unique_ID;
}

uint64_t AddProgressAchievement(AchievementManager *manager, const char *name, const char *description,
								int target_progress, int category_index, int level)
{
	if (!manager || !name || !description)
		return 0;

	// NOTE: ALL_LEVELS_ACHIEVEMENT (0) is valid here for global progress counters
	// (e.g. total kills / coins across the whole game). The AllLevels variant
	// creates one progress achievement PER level, which is a different pattern.

	if (manager->achievement_count >= MAX_ACHIEVEMENTS)
	{
		TraceLog(LOG_ERROR, "ACHIEVEMENT_MANAGER: Cannot add '%s' => MAX_ACHIEVEMENTS (%d) reached",
				 name, MAX_ACHIEVEMENTS);
		return 0;
	}

	if (target_progress <= 0)
	{
		TraceLog(LOG_WARNING, "ACHIEVEMENT_MANAGER: target_progress for '%s' <= 0, clamping to 1", name);
		target_progress = 1;
	}

	Achievement *achievement = &manager->achievements[manager->achievement_count++];
	InitAchievementDefaults(achievement, name, description, ACHIEVEMENT_PROGRESS, target_progress, category_index, level);
	achievement->unique_ID = GenerateAchievementUniqueID(achievement->name, achievement->description, category_index);

	TraceLog(LOG_INFO, "ACHIEVEMENT_MANAGER: Progress [%016llx] '%s' target=%d level=%d  (%d/%d)",
			 achievement->unique_ID, name, target_progress, level, manager->achievement_count, MAX_ACHIEVEMENTS);
	return achievement->unique_ID;
}

void AddInstantAchievementAllLevels(AchievementManager *manager, const char *name, const char *description,
									int category_index, uint64_t *unique_ID_out, int *count_out)
{
	if (!manager || !name || !description || !unique_ID_out || !count_out)
		return;

	for (int i = 1; i <= MAX_LEVELS && manager->achievement_count < MAX_ACHIEVEMENTS; i++)
	{
		uint64_t id = AddInstantAchievement(manager, name, description, category_index, i);
		if (id != 0)
			unique_ID_out[(*count_out)++] = id;
	}
}

void AddProgressAchievementAllLevels(AchievementManager *manager, const char *name, const char *description,
									 int target_progress, int category_index,
									 uint64_t *unique_ID_out, int *count_out)
{
	if (!manager || !name || !description || !unique_ID_out || !count_out)
		return;

	for (int i = 1; i <= MAX_LEVELS && manager->achievement_count < MAX_ACHIEVEMENTS; i++)
	{
		uint64_t id = AddProgressAchievement(manager, name, description, target_progress, category_index, i);
		if (id != 0)
			unique_ID_out[(*count_out)++] = id;
	}
}

//=================================================================
// Lookup
//=================================================================

Achievement *FindAchievementByID(AchievementManager *manager, uint64_t unique_ID)
{
	if (!manager)
		return NULL;

	for (int i = 0; i < manager->achievement_count; i++)
	{
		if (manager->achievements[i].unique_ID == unique_ID)
			return &manager->achievements[i];
	}
	return NULL;
}

// Returns first name match. Ambiguous when same name spans levels -
// prefer FindAchievementByNameAndLevel.
Achievement *FindAchievementByName(AchievementManager *manager, const char *name)
{
	if (!manager || !name)
		return NULL;

	for (int i = 0; i < manager->achievement_count; i++)
	{
		if (strcmp(manager->achievements[i].name, name) == 0)
			return &manager->achievements[i];
	}
	return NULL;
}

Achievement *FindAchievementByNameAndLevel(AchievementManager *manager, const char *name, int level)
{
	if (!manager || !name)
		return NULL;

	for (int i = 0; i < manager->achievement_count; i++)
	{
		Achievement *achievement = &manager->achievements[i];
		bool name_match = (strcmp(achievement->name, name) == 0);
		bool level_match = (achievement->level == ALL_LEVELS_ACHIEVEMENT) || (achievement->level == level);
		if (name_match && level_match)
			return achievement;
	}
	return NULL;
}

//=================================================================
// Observers
//
// CALLER CONTRACT: observers MUST be heap-allocated via MemoryAlloc:
//
//   // Build context (e.g. from observers.h Observer struct)
//   Observer *context = (Observer *)MemoryAlloc(sizeof(Observer), "MyObserverContext");
//   context->params   = (AchievementFilterParameters *)MemoryAlloc(sizeof(*context->params), "MyParams");
//   InitAchievementFilterParams(context->params);
//   context->achievementManager = manager;
//   context->gameData           = gameData;
//
//   // Build and register the observer
//   AchievementObserver *observer = (AchievementObserver *)MemoryAlloc(sizeof(*observer), "MyObserver");
//   observer->context  = context;
//   observer->callback = MyCallbackFn;
//   AddObserver(manager, observer);
//
//   // Do NOT free observer or context manually - RemoveObserver / ExitAchievementManager does it.
//=================================================================

uint64_t AddObserver(AchievementManager *manager, AchievementObserver *observer)
{
	if (!manager || !observer)
	{
		TraceLog(LOG_WARNING, "ACHIEVEMENT_MANAGER: AddObserver => null argument");
		return 0;
	}

	if (manager->observer_count >= MAX_OBSERVERS)
	{
		TraceLog(LOG_WARNING, "ACHIEVEMENT_MANAGER: AddObserver => MAX_OBSERVERS (%d) reached", MAX_OBSERVERS);
		return 0;
	}

	uint64_t id = GenerateObserverUniqueID(observer->context, observer->callback);

	for (int i = 0; i < manager->observer_count; i++)
	{
		if (manager->observers[i]->unique_ID == id)
		{
			TraceLog(LOG_WARNING, "ACHIEVEMENT_MANAGER: Observer [%016llx] already registered", id);
			return 0;
		}
	}

	observer->unique_ID = id;
	manager->observers[manager->observer_count++] = observer;

	TraceLog(LOG_INFO, "ACHIEVEMENT_MANAGER: Observer [%016llx] registered  (%d/%d)",
			 id, manager->observer_count, MAX_OBSERVERS);
	return id;
}

// Removes the slot, then frees the observer AND its context.
// The caller must NOT use the observer pointer after this call.
bool RemoveObserver(AchievementManager *manager, uint64_t observer_ID)
{
	if (!manager)
		return false;

	for (int i = 0; i < manager->observer_count; i++)
	{
		if (!manager->observers[i] || manager->observers[i]->unique_ID != observer_ID)
			continue;

		AchievementObserver *observer = manager->observers[i];

		// Compact slot array
		int last = --manager->observer_count;
		if (i != last)
			manager->observers[i] = manager->observers[last];
		manager->observers[last] = NULL;

		TraceLog(LOG_INFO, "ACHIEVEMENT_MANAGER: Observer [%016llx] removed  (%d remaining)",
				 observer_ID, manager->observer_count);

		// Free context, then the observer struct
		if (observer->context)
		{
			MemoryFree(observer->context);
			observer->context = NULL;
		}
		MemoryFree(observer);

		return true;
	}

	TraceLog(LOG_WARNING, "ACHIEVEMENT_MANAGER: RemoveObserver => [%016llx] not found", observer_ID);
	return false;
}

void NotifyObservers(AchievementManager *manager, void *data)
{
	if (!manager)
		return;

	for (int i = 0; i < manager->observer_count; i++)
	{
		AchievementObserver *observer = manager->observers[i];
		if (observer && observer->callback)
			observer->callback(observer->context, data);
	}
}

//=================================================================
// Unlocking
//=================================================================

void UnlockAchievementByName(AchievementManager *manager, const char *name, int level)
{
	if (!manager || !name)
		return;

	Achievement *achievement = FindAchievementByNameAndLevel(manager, name, level);

	if (!achievement)
	{
		TraceLog(LOG_WARNING, "ACHIEVEMENT_MANAGER: UnlockByName => '%s' level=%d not found", name, level);
		return;
	}

	if (achievement->unlocked)
		return;

	UnlockAchievement(manager, achievement);
}

void UnlockAchievementByID(AchievementManager *manager, uint64_t unique_ID, int level)
{
	if (!manager)
		return;

	Achievement *achievement = FindAchievementByID(manager, unique_ID);

	if (!achievement)
	{
		TraceLog(LOG_WARNING, "ACHIEVEMENT_MANAGER: UnlockByID => [%016llx] not found", unique_ID);
		return;
	}

	if (achievement->unlocked)
		return;

	if (achievement->level != ALL_LEVELS_ACHIEVEMENT && achievement->level != level)
	{
		TraceLog(LOG_WARNING, "ACHIEVEMENT_MANAGER: UnlockByID => [%016llx] level mismatch (stored %d vs %d)",
				 unique_ID, achievement->level, level);
		return;
	}

	UnlockAchievement(manager, achievement);
}

void UnlockInstantAchievementAtThreshold(AchievementManager *manager, const char *name, int level,
										 int threshold, int current_value)
{
	if (current_value == threshold)
		UnlockAchievementByName(manager, name, level);
}

//=================================================================
// Progress tracking
//=================================================================

void UpdateAchievementProgress(AchievementManager *manager, const char *name, int progress, int level)
{
	if (!manager || !name)
		return;

	for (int i = 0; i < manager->achievement_count; i++)
	{
		Achievement *achievement = &manager->achievements[i];

		if (strcmp(achievement->name, name) != 0)
			continue;
		if (achievement->type != ACHIEVEMENT_PROGRESS)
			continue;
		if (achievement->level != ALL_LEVELS_ACHIEVEMENT && achievement->level != level)
			continue;
		if (achievement->unlocked)
			continue;

		achievement->current_progress = progress;

		if (achievement->current_progress >= achievement->target_progress)
			UnlockAchievement(manager, achievement);

		break; // One update per call
	}
}

//=================================================================
// Achievement Update & Draw
//=================================================================
void UpdateAchievements(AchievementManager *manager)
{
	if (!manager)
		return;

	float dt = GetFrameTime();

	for (int i = 0; i < manager->achievement_count; i++)
	{
		Achievement *achievement = &manager->achievements[i];
		if (!achievement->is_achievement_displaying)
			continue;

		achievement->display_timer -= dt;
		if (achievement->display_timer <= 0.0f)
		{
			achievement->display_timer = 0.0f;
			achievement->is_achievement_displaying = false;
		}
	}
}

void DrawAchievements(AchievementManager *manager)
{
	if (!manager)
		return;

	int display_count = 0;

	for (int i = 0; i < manager->achievement_count; i++)
	{
		Achievement *achievement = &manager->achievements[i];
		if (!achievement->is_achievement_displaying)
			continue;

		const int x = ACHIEVEMENT_POPUP_X_OFFSET;
		const int y = ACHIEVEMENT_POPUP_Y_START + (display_count * ACHIEVEMENT_POPUP_Y_SPACING);
		const int w = ACHIEVEMENT_POPUP_WIDTH;
		const int h = ACHIEVEMENT_POPUP_HEIGHT;

		Color category_color = (achievement->category_index >= 0 && achievement->category_index < manager->category_count)
								   ? manager->categories[achievement->category_index].color
								   : GRAY;

		DrawRectangle(x, y, w, h, ColorAlpha(BLACK, 0.8f));
		DrawRectangle(x, y, 5, h, category_color);
		DrawText("Achievement Unlocked!", x + 12, y + 8, 20, GOLD);
		DrawText(achievement->name, x + 12, y + 32, 15, WHITE);

		if (achievement->type == ACHIEVEMENT_PROGRESS && achievement->target_progress > 0)
		{
			float pct = (float)achievement->current_progress / (float)achievement->target_progress;
			if (pct > 1.0f)
				pct = 1.0f;

			const int barX = x + 12;
			const int barY = y + h - 8;
			const int barW = w - 24;

			DrawRectangle(barX, barY, barW, 5, DARKGRAY);
			DrawRectangle(barX, barY, (int)(barW * pct), 5, category_color);
		}

		display_count++;
	}
}

//=================================================================
// Persistence
//=================================================================

void LoadAchievements(AchievementManager *manager)
{
	if (!manager)
		return;

	FILE *f = fopen(ACHIEVEMENT_PROGRESS_FILE, "rb");
	if (!f)
	{
		TraceLog(LOG_INFO, "ACHIEVEMENT_MANAGER: '%s' not found => initialising all as locked",
				 ACHIEVEMENT_PROGRESS_FILE);

		for (int i = 0; i < manager->achievement_count; i++)
		{
			manager->achievements[i].unlocked = false;
			manager->achievements[i].current_progress = 0;
			manager->achievements[i].is_achievement_displaying = false;
			manager->achievements[i].display_timer = 0.0f;
		}

		SaveAchievements(manager);
		return;
	}

	int loaded = 0, failed = 0;

	for (int i = 0; i < manager->achievement_count; i++)
	{
		Achievement *achievement = &manager->achievements[i];
		bool ok = (fread(&achievement->unlocked, sizeof(bool), 1, f) == 1) &&
				  (fread(&achievement->current_progress, sizeof(int), 1, f) == 1);

		achievement->is_achievement_displaying = false;
		achievement->display_timer = 0.0f;

		if (ok)
			loaded++;
		else
		{
			achievement->unlocked = false;
			achievement->current_progress = 0;
			failed++;
		}
	}

	fclose(f);
	TraceLog(LOG_INFO, "ACHIEVEMENT_MANAGER: Loaded %d/%d records (%d failed) from '%s'",
			 loaded, manager->achievement_count, failed, ACHIEVEMENT_PROGRESS_FILE);
}

void SaveAchievements(AchievementManager *manager)
{
	if (!manager)
		return;

	FILE *f = fopen(ACHIEVEMENT_PROGRESS_FILE, "wb");
	if (!f)
	{
		TraceLog(LOG_ERROR, "ACHIEVEMENT_MANAGER: Failed to open '%s' for writing",
				 ACHIEVEMENT_PROGRESS_FILE);
		return;
	}

	bool ok = true;
	for (int i = 0; i < manager->achievement_count && ok; i++)
	{
		Achievement *achievement = &manager->achievements[i];
		ok = (fwrite(&achievement->unlocked, sizeof(bool), 1, f) == 1) &&
			 (fwrite(&achievement->current_progress, sizeof(int), 1, f) == 1);
	}

	fclose(f);

	if (!ok)
	{
		TraceLog(LOG_ERROR, "ACHIEVEMENT_MANAGER: Write error => removing corrupt '%s'",
				 ACHIEVEMENT_PROGRESS_FILE);
		remove(ACHIEVEMENT_PROGRESS_FILE);
	}
}

//=================================================================
// Level progress
//=================================================================

void InitLevelProgress(LevelProgress *progress)
{
	if (!progress)
		return;

	progress->current_level = 1;
	progress->level_unlocked[0] = true;

	for (int i = 1; i < MAX_LEVELS; i++)
		progress->level_unlocked[i] = false;
}

void LoadLevelProgress(LevelProgress *progress)
{
	if (!progress)
		return;

	FILE *f = fopen(LEVEL_PROGRESS_FILE, "rb");
	if (!f)
	{
		TraceLog(LOG_INFO, "ACHIEVEMENT_MANAGER: '%s' not found => initialising level progress",
				 LEVEL_PROGRESS_FILE);
		InitLevelProgress(progress);
		SaveLevelProgress(progress);
		return;
	}

	size_t r = fread(&progress->current_level, sizeof(int), 1, f) + fread(progress->level_unlocked, sizeof(bool), MAX_LEVELS, f);

	fclose(f);

	if ((int)r != MAX_LEVELS + 1)
	{
		TraceLog(LOG_WARNING, "ACHIEVEMENT_MANAGER: Level progress file corrupt => resetting");
		InitLevelProgress(progress);
		SaveLevelProgress(progress);
	}
	else
	{
		TraceLog(LOG_INFO, "ACHIEVEMENT_MANAGER: Level progress loaded (current: %d)",
				 progress->current_level);
	}
}

void SaveLevelProgress(const LevelProgress *progress)
{
	if (!progress)
		return;

	FILE *f = fopen(LEVEL_PROGRESS_FILE, "wb");
	if (!f)
	{
		TraceLog(LOG_ERROR, "ACHIEVEMENT_MANAGER: Failed to write '%s'", LEVEL_PROGRESS_FILE);
		return;
	}

	fwrite(&progress->current_level, sizeof(int), 1, f);
	fwrite(progress->level_unlocked, sizeof(bool), MAX_LEVELS, f);
	fclose(f);
}

bool IsLevelUnlocked(AchievementManager *manager, int level)
{
	if (!manager || level < 1 || level > MAX_LEVELS)
		return false;
	return manager->level_progress.level_unlocked[level - 1];
}

int GetCurrentLevel(AchievementManager *manager)
{
	return manager ? manager->level_progress.current_level : 1;
}

void SetCurrentLevel(AchievementManager *manager, int level)
{
	if (!manager || level < 1 || level > MAX_LEVELS)
		return;

	if (!manager->level_progress.level_unlocked[level - 1])
	{
		TraceLog(LOG_WARNING, "ACHIEVEMENT_MANAGER: SetCurrentLevel(%d) => not yet unlocked", level);
		return;
	}

	manager->level_progress.current_level = level;
	SaveLevelProgress(&manager->level_progress);
}

void UnlockNextLevel(AchievementManager *manager)
{
	if (!manager)
		return;

	int next = manager->level_progress.current_level + 1;
	if (next > MAX_LEVELS)
	{
		TraceLog(LOG_INFO, "ACHIEVEMENT_MANAGER: All %d levels already unlocked", MAX_LEVELS);
		return;
	}

	manager->level_progress.level_unlocked[next - 1] = true;
	SaveLevelProgress(&manager->level_progress);

	TraceLog(LOG_INFO, "ACHIEVEMENT_MANAGER: Level %d unlocked", next);
}

//=================================================================
// Category sounds
//
// Routed through the sound manager so that achievement pops respect
// master/sfx volume, cooldowns, and the concurrent priority queue -
// exactly like every other sound in the game.
//
// Mapping:  AchievementSoundType  ->  SFXType
//   ACHIEVEMENT_LEVEL_SOUND       ->  SFX_ACHIEVEMENT_LEVEL
//   ACHIEVEMENT_COMBAT_SOUND      ->  SFX_ACHIEVEMENT_COMBAT
//   ACHIEVEMENT_COLLECTION_SOUND  ->  SFX_ACHIEVEMENT_COLLECTION
//
// Configure file paths and properties in audio_sfx.ini.
//=================================================================

void PlayAchievementSound(AchievementManager *manager, Achievement *achievement)
{
	if (!manager || !achievement)
		return;

	Category *category = GetCategory(manager, achievement->category_index);
	if (!category)
		return;

	switch (category->sound_type)
	{
	case ACHIEVEMENT_LEVEL_SOUND:
		PlaySFX(SFX_ACHIEVEMENT_LEVEL);
		break;
	case ACHIEVEMENT_COMBAT_SOUND:
		PlaySFX(SFX_ACHIEVEMENT_COMBAT);
		break;
	case ACHIEVEMENT_COLLECTION_SOUND:
		PlaySFX(SFX_ACHIEVEMENT_COLLECTION);
		break;
	default:
		TraceLog(LOG_WARNING, "ACHIEVEMENT_MANAGER: Unknown sound type %d for category '%s'",
				 category->sound_type, category->name);
		break;
	}
}
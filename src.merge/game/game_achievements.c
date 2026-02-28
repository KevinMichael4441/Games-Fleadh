#include "game.h"
#include "game/game_achievements.h"

//=================================================================
// QUICK REFERENCE
//
//	SECTION A (RegisterAchievements)
//
//	GameAddAchievementCategory (manager, "Name", COLOR, SOUND_TYPE)
// 		SOUND_TYPE:
//       ACHIEVEMENT_LEVEL_SOUND       level completion sounds
//       ACHIEVEMENT_COMBAT_SOUND      combat/kill sounds
//       ACHIEVEMENT_COLLECTION_SOUND  item/collectible sounds
//
//   GameAddInstantAchievement  (manager, "Name", "Desc", category, level)
//     level = 1, 2, 3 ...          specific level
//     level = ALL_LEVELS_ACHIEVEMENT any level
//
//   GameAddProgressAchievement (manager, "Name", "Desc", target, category, level)
//     Unlocks when Notify total reaches target.
//     level = ALL_LEVELS_ACHIEVEMENT for a single global counter.
//
//   GameAddPerLevelAchievement (manager, "Name", "Desc", category)
//     Creates one instant achievement per level automatically.
//
//	SECTION B (observer callbacks)
//
//   GameObserverContext *context = (GameObserverContext *)context;
//   Achievement         *a   = (Achievement *)data;
//   context->game_data    read / write GameData
//   context->manager      call UnlockNextLevel() etc.
//   a->name           e.g. "Warrior"
//   a->level          level it was on (0 = ALL_LEVELS_ACHIEVEMENT)
//
// 	Notify functions (call from game logic)
//
//   NotifyLevelComplete    (manager, level)
//   NotifyFlawlessLevel    (manager, level)
//   NotifyEnemyKilled      (manager, total_kills, level)
//   NotifyCoinCollected    (manager, total_coins, level)
//   NotifyAllSecretsFound  (manager, level)
//
//=================================================================
//
// Only edit SECTION A and B
//
//	SECTION A (RegisterAchievements)
//		Add categories and achievements here using helpers:
//			GameAddAchievementCategory
//			GameAddInstantAchievement
//			GameAddProgressAchievement
//			GameAddPerLevelAchievement
//	Note: One line per achievement, IDs are handled automatically.
//
//	SECTION B (observer callbacks)
//		Each callback fires when an achievement in its category
//		unlocks. Add your game-state reactions here.
//
// Everything else is internal plumbing.
//=================================================================

//=================================================================
// INTERNAL: per-observer context
//
// Carries the pointers each callback needs. Defined here so
// game_achievements.c is fully self-contained.
//=================================================================
typedef struct
{
	AchievementManager *manager; // For UnlockNextLevel etc.
	GameData *game_data;		 // For reading / writing game state
} GameObserverContext;

//=================================================================
// INTERNAL: registration helpers
//
// Thin wrappers that hide the ID outputs, call these
// inside RegisterAchievements() and never touch raw IDs.
//=================================================================

static int GameAddAchievementCategory(AchievementManager *manager,
									  const char *name, Color color,
									  AchievementSoundType sound_type)
{
	return AddCategory(manager, name, color, sound_type);
}

// One-shot: unlocked by a single Notify call
static void GameAddInstantAchievement(AchievementManager *manager,
									  const char *name, const char *description,
									  int category, int level)
{
	AddInstantAchievement(manager, name, description, category, level);
}

// Counter: unlocked when the running total passed to Notify reaches target
static void GameAddProgressAchievement(AchievementManager *manager,
									   const char *name, const char *description,
									   int target, int category, int level)
{
	AddProgressAchievement(manager, name, description, target, category, level);
}

// Per-level: creates one in>stant achievement for every level automatically
static void GameAddPerLevelAchievement(AchievementManager *manager,
									   const char *name, const char *description,
									   int category)
{
	int count = 0;
	uint64_t ids[MAX_LEVELS]; // Discarded: lookup is by name at runtime
	AddInstantAchievementAllLevels(manager, name, description, category, ids, &count);
}

//=================================================================
// INTERNAL: observer allocation helper
//
// Handles all MemoryAlloc bookkeeping so the callbacks below stay
// free of memory management boilerplate.
//=================================================================
// Forward declarations for callbacks defined in Section B
static void GameOnCombatAchievementUnlocked(void *context, void *data);
static void GameOnCollectionAchievementUnlocked(void *context, void *data);
static void GameOnLevelAchievementUnlocked(void *context, void *data);

static uint64_t RegisterObserver(AchievementManager *manager,
								 GameData *game_data,
								 void (*callback)(void *, void *),
								 const char *name)
{
	GameObserverContext *game_observer_context = (GameObserverContext *)MemoryAlloc(
		sizeof(GameObserverContext), name);
	if (!game_observer_context)
	{
		TraceLog(LOG_ERROR, "GAME_ACHIEVEMENTS: Failed to allocate context for '%s'", name);
		return 0;
	}

	game_observer_context->manager = manager;
	game_observer_context->game_data = game_data;

	AchievementObserver *observer = (AchievementObserver *)MemoryAlloc(
		sizeof(AchievementObserver), name);
	if (!observer)
	{
		TraceLog(LOG_ERROR, "GAME_ACHIEVEMENTS: Failed to allocate observer '%s'", name);
		MemoryFree(game_observer_context);
		return 0;
	}

	observer->context = game_observer_context;
	observer->callback = callback;
	observer->unique_ID = 0; // Assigned by AddObserver()

	return AddObserver(manager, observer);
	// observer and game_observer_context are now owned by the manager - do not free them here
}

//=================================================================
// SECTION A: Register categories and achievements
//=================================================================
//
//	GameAddAchievementCategory(manager, "Name", COLOR, SOUND_TYPE)
//	SOUND_TYPE is one of:
//		ACHIEVEMENT_LEVEL_SOUND			for level completion sounds
//		ACHIEVEMENT_COMBAT_SOUND		for combat/kill sounds
//		ACHIEVEMENT_COLLECTION_SOUND	for item/collectible sounds
//
//	GameAddInstantAchievement(manager, "Name", "Description", category, level)
//		level = 1, 2, 3 ...   for a specific level
//		level = ALL_LEVELS_ACHIEVEMENT  for any level
//
//	GameAddProgressAchievement(manager, "Name", "Description", target, category, level)
//		Unlocks automatically when the Notify total reaches target
//
//	GameAddPerLevelAchievement (manager, "Name", "Description", category)
//		Creates one achievement per level automatically
//
// Then add a matching Notify*() call in your game logic (Step 4).
//=================================================================
static void RegisterAchievements(AchievementManager *manager, GameAchievements *game_achievements)
{
	//--------------------------------------------------------------
	game_achievements->category_level = GameAddAchievementCategory(manager, "Level", GREEN, ACHIEVEMENT_LEVEL_SOUND);
	game_achievements->category_combat = GameAddAchievementCategory(manager, "Combat", RED, ACHIEVEMENT_COMBAT_SOUND);
	game_achievements->category_collection = GameAddAchievementCategory(manager, "Collection", GOLD, ACHIEVEMENT_COLLECTION_SOUND);

	// Instant achievements
	//--------------------------------------------------------------
	GameAddInstantAchievement(manager, "First Blood", "Defeated your first enemy", game_achievements->category_combat, 1);
	GameAddInstantAchievement(manager, "Coin Collector", "Collected your first coin", game_achievements->category_collection, ALL_LEVELS_ACHIEVEMENT);
	GameAddInstantAchievement(manager, "Numismatist", "Noo-miz-ma-tist", game_achievements->category_collection, ALL_LEVELS_ACHIEVEMENT);

	// Progress achievements
	//--------------------------------------------------------------
	GameAddProgressAchievement(manager, "Warrior", "Defeated 10 enemies", 10, game_achievements->category_combat, ALL_LEVELS_ACHIEVEMENT);
	GameAddProgressAchievement(manager, "Veteran", "Defeated 20 enemies", 20, game_achievements->category_combat, ALL_LEVELS_ACHIEVEMENT);
	GameAddProgressAchievement(manager, "Legend", "Defeated 30 enemies", 30, game_achievements->category_combat, ALL_LEVELS_ACHIEVEMENT);
	GameAddProgressAchievement(manager, "Hoarder", "Collected 10 coins", 10, game_achievements->category_collection, ALL_LEVELS_ACHIEVEMENT);
	GameAddProgressAchievement(manager, "Numismatist", "Collected 50 coins", 50, game_achievements->category_collection, ALL_LEVELS_ACHIEVEMENT);

	// Per-level achievements
	//--------------------------------------------------------------
	GameAddPerLevelAchievement(manager, "Level Complete", "Completed a level", game_achievements->category_level);
	GameAddPerLevelAchievement(manager, "Flawless", "Completed a level without taking damage", game_achievements->category_level);
	GameAddPerLevelAchievement(manager, "Secret Hunter", "Found all secrets in a level", game_achievements->category_collection);
}
//=================================================================

//=================================================================
//	Lifecycle:
//		GameAchievementsInit()
//		GameAchievementsUpdate()
//		GameAchievementsExit()
//=================================================================
void GameAchievementsInit(GameData *game_data)
{
	if (!game_data)
		return;

	game_data->manager = InitAchievementManager();
	if (!game_data->manager)
	{
		TraceLog(LOG_ERROR, "EXAMPLE: InitAchievementManager failed : aborting");
		game_data->exit_game = true;
		return;
	}

	game_data->game_achievements = (GameAchievements){0};

	TraceLog(LOG_INFO, "GAME_ACHIEVEMENTS: Initialising");

	RegisterAchievements(game_data->manager, &game_data->game_achievements);
	LoadAchievements(game_data->manager);

	game_data->game_achievements.kill_observer_ID =
		RegisterObserver(game_data->manager, game_data,
						 GameOnCombatAchievementUnlocked,
						 "CombatObserver");

	game_data->game_achievements.coin_observer_ID =
		RegisterObserver(game_data->manager, game_data,
						 GameOnCollectionAchievementUnlocked,
						 "CollectionObserver");

	game_data->game_achievements.level_observer_ID =
		RegisterObserver(game_data->manager, game_data,
						 GameOnLevelAchievementUnlocked,
						 "LevelObserver");

	MemoryPrintAllocations();

	TraceLog(LOG_INFO, "GAME_ACHIEVEMENTS: Initialised");
}

void GameAchievementsUpdate(GameData *game_data)
{
	UpdateAchievements(game_data->manager);
}

void GameAchievementsExit(GameData *game_data)
{
	if (!game_data)
		return;
	if (!game_data->manager)
		return;

	// Remove observers (struct stored by value, so use '.')
	RemoveObserver(game_data->manager, game_data->game_achievements.kill_observer_ID);
	RemoveObserver(game_data->manager, game_data->game_achievements.coin_observer_ID);
	RemoveObserver(game_data->manager, game_data->game_achievements.level_observer_ID);

	game_data->game_achievements.kill_observer_ID = 0;
	game_data->game_achievements.coin_observer_ID = 0;
	game_data->game_achievements.level_observer_ID = 0;

	// Exit the Achievements Manager
	ExitAchievementManager(game_data->manager);
	game_data->manager = NULL;

	MemoryPrintAllocations();

	TraceLog(LOG_INFO, "GAME_ACHIEVEMENTS: Exited");
}

//=================================================================
//	SECTION B: Observer callbacks
//
//	Each callback fires when ANY achievement in its category unlocks.
//
//	What you receive:
//		achievement->name	e.g. "Warrior"
//		achievement->level	which level it was on (0 = all levels)
//
//	What you can do:
//	GameObserverContext *game_observer_context = (GameObserverContext *)context;
//	game_observer_context->game_data	read or write GameData
//	game_observer_context->manager		call UnlockNextLevel() etc.
//
//	Examples:
//		game_observer_context->game_data->coins += 50;			// bonus coins
//		game_observer_context->game_data->bonus_stage = true;	// unlock something
//		UnlockNextLevel(game_observer_context->manager);		// advance the game
//=================================================================
static void GameOnCombatAchievementUnlocked(void *context, void *data)
{
	GameObserverContext *game_observer_context = (GameObserverContext *)context;
	Achievement *achievement = (Achievement *)data;

	if (!game_observer_context || !achievement)
		return;

	// Only handle Combat category
	Category *category = GetCategory(game_observer_context->manager, achievement->category_index);
	if (!category || strcmp(category->name, "Combat") != 0)
		return;

	TraceLog(LOG_INFO, "GAME_ACHIEVEMENTS: Combat unlocked: '%s' (level %d)",
			 achievement->name, achievement->level);

	// Example: give the player a bonus reward on any combat achievement
	// game_observer_context->game_data->coins += 25;
}

static void GameOnCollectionAchievementUnlocked(void *context, void *data)
{
	GameObserverContext *game_observer_context = (GameObserverContext *)context;
	Achievement *achievement = (Achievement *)data;

	if (!game_observer_context || !achievement)
		return;

	// Only handle Collection category
	Category *category = GetCategory(game_observer_context->manager, achievement->category_index);
	if (!category || strcmp(category->name, "Collection") != 0)
		return;

	TraceLog(LOG_INFO, "GAME_ACHIEVEMENTS: Collection unlocked: '%s'",
			 achievement->name);

	// Example: open a bonus stage when the player becomes a Hoarder
	// if (strcmp(achievement->name, "Hoarder") == 0)
	//     game_observer_context->game_data->bonus_stage_open = true;
}

static void GameOnLevelAchievementUnlocked(void *context, void *data)
{
	GameObserverContext *game_observer_context = (GameObserverContext *)context;
	Achievement *achievement = (Achievement *)data;

	if (!game_observer_context || !achievement)
		return;

	// Only handle Level category
	Category *category = GetCategory(game_observer_context->manager, achievement->category_index);
	if (!category || strcmp(category->name, "Level") != 0)
		return;

	TraceLog(LOG_INFO, "GAME_ACHIEVEMENTS: Level unlocked: '%s' (level %d)",
			 achievement->name, achievement->level);

	// Unlock the next level automatically when a level is completed
	if (strcmp(achievement->name, "Level Complete") == 0)
		UnlockNextLevel(game_observer_context->manager);
}

//=================================================================

//=================================================================
// Notify functions
//
// Called from game logic. Each one updates the relevant achievements.
// Pass RUNNING TOTALS, not +1 increments.
//=================================================================

void NotifyLevelComplete(AchievementManager *manager, int level)
{
	UnlockAchievementByName(manager, "Level Complete", level);
}

void NotifyFlawlessLevel(AchievementManager *manager, int level)
{
	UnlockAchievementByName(manager, "Flawless", level);
}

void NotifyEnemyKilled(AchievementManager *manager, int total_kills, int level)
{
	// Instant: fires exactly when total reaches 1 on level 1
	UnlockInstantAchievementAtThreshold(manager, "First Blood", 1, 1, total_kills);

	// Progress: feed the running total into every kill milestone
	UpdateAchievementProgress(manager, "Warrior", total_kills, ALL_LEVELS_ACHIEVEMENT);
	UpdateAchievementProgress(manager, "Veteran", total_kills, ALL_LEVELS_ACHIEVEMENT);
	UpdateAchievementProgress(manager, "Legend", total_kills, ALL_LEVELS_ACHIEVEMENT);
}

void NotifyCoinCollected(AchievementManager *manager, int total_coins, int level)
{
	// Instant: fires exactly when the first coin is collected
	UnlockInstantAchievementAtThreshold(manager, "Coin Collector",
										ALL_LEVELS_ACHIEVEMENT, 1, total_coins);

	UnlockInstantAchievementAtThreshold(manager, "Coin Collector",
										ALL_LEVELS_ACHIEVEMENT, 50, total_coins);

	UnlockInstantAchievementAtThreshold(manager, "Coin Collector",
										ALL_LEVELS_ACHIEVEMENT, 100, total_coins);

	// Progress: feed the running total into the coin milestone
	UpdateAchievementProgress(manager, "Hoarder", total_coins, ALL_LEVELS_ACHIEVEMENT);
}

void NotifyAllSecretsFound(AchievementManager *manager, int level)
{
	UnlockAchievementByName(manager, "Secret Hunter", level);
}
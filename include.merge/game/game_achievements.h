#ifndef GAME_ACHIEVEMENTS_H
#define GAME_ACHIEVEMENTS_H

//=================================================================
// game_achievements.h
//
//
// To add achievements to game:
//
//	STEP 1: Add a category field to GameAchievements below
//			int category_exploration;
//
//	STEP 2: Open game_achievements.c -> SECTION A
//	Register your category and achievements using:
//			GameAddAchievementCategory	(manager, "Name", COLOR, SOUND_TYPE)
//			GameAddInstantAchievement	(manager, "Name", "Desc", category, level)
//			GameAddProgressAchievement	(manager, "Name", "Desc", target, category, level)
//			GameAddPerLevelAchievement	(manager, "Name", "Desc", category)
//
//	STEP 3: Open game_achievements.c -> SECTION B
//			Add code to the matching callback to respond when
//			an achievement in that category unlocks.
//
//	STEP 4: Call the matching Notify* from your game logic:
//			NotifyLevelComplete		(manager, level)
//			NotifyFlawlessLevel		(manager, level)
//			NotifyEnemyKilled		(manager, total_kills, level)
//			NotifyCoinCollected		(manager, total_coins, level)
//			NotifyAllSecretsFound	(manager, level)
//=================================================================
#include "memory/memory_manager.h"
#include "achievements/achievement_manager.h"

//=================================================================
// C linkage (C++ compiler flag)
//=================================================================
#ifdef __cplusplus
extern "C"
{
#endif

	//=================================================================
	// GameAchievements
	//
	// Holds one int per category (the index returned by AddCategory).
	// Add a new int field here for each category you create.
	//
	// Note: Do not edit them.
	// Observer ID fields at the bottom are internal
	//=================================================================
	typedef struct
	{
		// Add Category ints here
		int category_level;
		int category_combat;
		int category_collection;

		// Internal: observer_IDs used by GameAchievementsExit
		uint64_t kill_observer_ID;
		uint64_t coin_observer_ID;
		uint64_t level_observer_ID;

	} GameAchievements;

	// Forward declaration: GameData is defined in game.h
	typedef struct GameData GameData;

	//=================================================================
	// Lifecycle: InitAchievementManager / ExitAchievementManager pattern
	//=================================================================

	//=================================================================
	// Call once after InitAchievementManager().
	// Registers categories, achievements, observers, then loads save.
	//=================================================================
	void GameAchievementsInit(GameData *game_data);

	//=================================================================
	// GameAchievementsUpdate
	// Call every frame in Update() to update achievement.
	//=================================================================
	void GameAchievementsUpdate(GameData *game_data);

	//=================================================================
	// Call before ExitAchievementManager().
	// ExitAchievementManager handles cleanup.
	// Useful when returning to a main menu without fully quitting.
	//=================================================================
	void GameAchievementsExit(GameData *game_data);

	//=================================================================
	// Notify functions
	//
	// Call these from game logic when the event occurs.
	// Pass RUNNING TOTALS (the accumulated count), not +1 increments.
	//=================================================================
	void NotifyLevelComplete(AchievementManager *manager, int level);
	void NotifyFlawlessLevel(AchievementManager *manager, int level);
	void NotifyEnemyKilled(AchievementManager *manager, int total_kills, int level);
	void NotifyCoinCollected(AchievementManager *manager, int total_coins, int level);
	void NotifyAllSecretsFound(AchievementManager *manager, int level);

#ifdef __cplusplus
}
#endif

#endif // GAME_ACHIEVEMENTS_H
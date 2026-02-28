//=================================================================
// Game Gameplay Scene
// Owns the gameplay scene lifecycle and all gameplay-adjacent
// modal scenes (Level Complete, Game Over, Game Complete).
//
// Note: Read game_gameplay.h to understand what this module does.
//       Roam from here when you want to understand game_gameplay.
//
// Registered scenes:
//	GAMEPLAY_SCENE			: Gameplay loop
//	LEVEL_COMPLETE_SCENE	: modal overlay between levels
//	GAME_OVER_SCENE			: Player killed, replay or quit
//	GAME_COMPLETE_SCENE		: All levels complete, replay or quit
//
// Enter:	Validate player entity, reset animation/position.
//			Player is created once in LoadingPlayer (main.c splash step).
//
// Update:	Movement, collision with kill cooldown, level progression,
//			damage flash, shader params, attack/action commands.
//
// Draw:	Full scene render (shaders, organisms, HUD, Spine, modals)
//
// Exit:	No-op (Player entity persists, freed by GameSpineExit).
//=================================================================

#include <math.h>
#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <string.h>

#include "game.h"
#include "game/game_scene.h"
#include "game/game_gameplay.h"
#include "game/game_modal.h"

#include "game/game_telemetry.h"
#include "game/game_input.h"
#include "game/game_sound.h"
#include "game/game_spine.h"
#include "game/game_shader.h"
#include "game/game_achievements.h"
#include "game/game_modal.h"

#include "core/collider.h"
#include "core/fast_math.h"

#include "simulation/organism.h"

#include "constants.h"

//=================================================================
// Kill cooldown
// Prevents one overlap registering kills every frame at 60fps.
//=================================================================
#define KILL_COOLDOWN_SECONDS 0.5f
static double last_kill_time = -KILL_COOLDOWN_SECONDS; // Ready on first frame

//=================================================================
// Display message (throttled)
//=================================================================
static char last_message[BUFFER_SIZE] = "";
static double last_update = 0.0;

//=================================================================
// Points display (throttled)
//=================================================================
static char points_message[BUFFER_SIZE] = "";
static double points_update = 0.0;

//=================================================================
// Command bitmap
//=================================================================
static char command_bitmap[COMMAND_COUNT + 1];

//=================================================================
// Game Register Game Scenes
// Callback declarations
//=================================================================
static void GameplayEnter(GameData *game_data);
static void GameplayUpdate(GameData *game_data);
static void GameplayDraw(GameData *game_data);
static void GameplayExit(GameData *game_data);

static void LevelCompleteEnter(GameData *game_data);
static void LevelCompleteUpdate(GameData *game_data);
static void LevelCompleteDraw(GameData *game_data);
static void LevelCompleteExit(GameData *game_data);

static void GameOverEnter(GameData *game_data);
static void GameOverUpdate(GameData *game_data);
static void GameOverDraw(GameData *game_data);
static void GameOverExit(GameData *game_data);

static void GameCompleteEnter(GameData *game_data);
static void GameCompleteUpdate(GameData *game_data);
static void GameCompleteDraw(GameData *game_data);
static void GameCompleteExit(GameData *game_data);

static void LevelComplete(GameData *game_data);
static void GameOver(GameData *game_data);
static void GameComplete(GameData *game_data);
static void ReplayLevel(GameData *game_data);
static void ResetGame(GameData *game_data);

//=================================================================
// GameGameplayRegister
// Registers all gameplay-adjacent scenes with the scene manager.
//=================================================================
void GameGameplayRegister(void)
{
	GameSceneRegister(GAMEPLAY_SCENE,
					  GameplayEnter,
					  GameplayUpdate,
					  GameplayDraw,
					  GameplayExit);

	GameSceneRegister(LEVEL_COMPLETE_SCENE,
					  LevelCompleteEnter,
					  LevelCompleteUpdate,
					  LevelCompleteDraw,
					  LevelCompleteExit);

	GameSceneRegister(GAME_OVER_SCENE,
					  GameOverEnter,
					  GameOverUpdate,
					  GameOverDraw,
					  GameOverExit);

	GameSceneRegister(GAME_COMPLETE_SCENE,
					  GameCompleteEnter,
					  GameCompleteUpdate,
					  GameCompleteDraw,
					  GameCompleteExit);
}

//=================================================================
// GAMEPLAY_SCENE
//=================================================================
static void GameplayEnter(GameData *game_data)
{
	// Player is created once in StepPlayer during splash loading.
	// On re-entry (ReplayLevel / ResetGame) it already exists.
	if (!game_data->player)
	{
		TraceLog(LOG_ERROR, "GAME_GAMEPLAY: Player entity is NULL on enter");
		game_data->exit_game = true;
		game_data->exit_game_time = game_data->now;
		return;
	}

	// Reset animation and position
	PlaySpineAnimation(game_data->player, 1, "idle_alert", SPINE_ANIM_LOOP);
	SetSpineFlip(game_data->player, true, false);
	SetSpineAnimationSpeed(game_data->player, 0.5f);
	SetSpinePosition(game_data->player,
					 game_data->player_position.x,
					 game_data->player_position.y);

	TraceLog(LOG_INFO, "GAME_GAMEPLAY: Entered (level %d)", game_data->current_level);
}

static void GameplayUpdate(GameData *game_data)
{
	// Move Player
	if (IsCommandActive(game_data->command, MOVE_UP))
	{
		game_data->player_position.y -= game_data->player_speed;
		PlaySFX(SFX_WALKING);
	}
	if (IsCommandActive(game_data->command, MOVE_DOWN))
	{
		game_data->player_position.y += game_data->player_speed;
		PlaySFX(SFX_WALKING);
	}
	if (IsCommandActive(game_data->command, MOVE_LEFT))
	{
		game_data->player_position.x -= game_data->player_speed;
		PlaySFX(SFX_WALKING);
	}
	if (IsCommandActive(game_data->command, MOVE_RIGHT))
	{
		game_data->player_position.x += game_data->player_speed;
		PlaySFX(SFX_WALKING);
	}

	// Move Reticle
	if (IsCommandActive(game_data->command, AIM_UP))
	{
		game_data->reticle_position.y -= game_data->reticle_speed;
		PlaySFX(SFX_AIM_START);
	}
	if (IsCommandActive(game_data->command, AIM_DOWN))
	{
		game_data->reticle_position.y += game_data->reticle_speed;
		PlaySFX(SFX_AIM_START);
	}
	if (IsCommandActive(game_data->command, AIM_LEFT))
	{
		game_data->reticle_position.x -= game_data->reticle_speed;
		PlaySFX(SFX_AIM_START);
	}
	if (IsCommandActive(game_data->command, AIM_RIGHT))
	{
		game_data->reticle_position.x += game_data->reticle_speed;
		PlaySFX(SFX_AIM_START);
	}

	// Clamp Player within screen bounds
	game_data->player_position.x = Clamp(game_data->player_position.x, game_data->min_X, game_data->max_X);
	game_data->player_position.y = Clamp(game_data->player_position.y, game_data->min_Y, game_data->max_Y);
	SetSpinePosition(game_data->player, game_data->player_position.x, game_data->player_position.y);

	// Clamp Reticle within screen bounds
	game_data->reticle_position.x = Clamp(game_data->reticle_position.x, game_data->min_X, game_data->max_X);
	game_data->reticle_position.y = Clamp(game_data->reticle_position.y, game_data->min_Y, game_data->max_Y);

	// Default colors
	game_data->player_color = DARKGRAY;
	game_data->reticle_color = ORANGE;

	// Reticle collider
	game_data->reticle_collider.p.x = game_data->reticle_position.x;
	game_data->reticle_collider.p.y = game_data->reticle_position.y;
	game_data->reticle_collider.r = game_data->reticle_radius;

	// Collision: reticle vs player - cooldown prevents registering every frame
	if (CheckCircleSpineEntityCollision_c2(game_data->reticle_collider, game_data->player) &&
		(game_data->now - last_kill_time) >= KILL_COOLDOWN_SECONDS)
	{
		last_kill_time = game_data->now;

		TraceLog(LOG_WARNING, "GAME_GAMEPLAY: Collision detected");
		game_data->player_points += 10;
		game_data->damage_flash = 1.0f;
		game_data->took_damage_this_level = true;
		PlaySFX(SFX_PICKUP);

		// Kills
		game_data->total_kills++;
		game_data->kills_this_level++;
		NotifyEnemyKilled(game_data->manager, game_data->total_kills, game_data->current_level);

		// Coins
		game_data->total_coins++;
		NotifyCoinCollected(game_data->manager, game_data->total_coins, game_data->current_level);

		// Secrets
		game_data->secrets_found_this_level++;
		TraceLog(LOG_INFO, "GAME_GAMEPLAY: Secret found (%d/%d)",
				 game_data->secrets_found_this_level,
				 game_data->total_secrets_this_level);

		if (game_data->secrets_found_this_level >= game_data->total_secrets_this_level)
			NotifyAllSecretsFound(game_data->manager, game_data->current_level);
	}

	// Level complete: threshold = current_level * 3 (level 1=3, 2=6, 3=9...)
	// Guard: never fire on 0 kills (would trigger if current_level were ever 0)
	int kill_threshold = game_data->current_level * 3;
	if (game_data->kills_this_level > 0 && game_data->kills_this_level >= kill_threshold)
		LevelComplete(game_data);

	// Damage flash fade
	if (game_data->damage_flash > 0.0f)
	{
		game_data->damage_flash -= (float)game_data->frame_time * 2.0f;
		if (game_data->damage_flash < 0.0f)
			game_data->damage_flash = 0.0f;
	}

	// Per-frame shader params
	GameShaderSetParams(game_data->palette_tint, (GameShaderParams){
													 .tintColor = RED,
													 .intensity = game_data->damage_flash});

	// Spotlight animation using fast sine (low priority optimization - only one call per frame)
	float scan_y = 0.5f + FastSin((float)game_data->now * 0.4f) * 0.15f;
	GameShaderSetParams(game_data->spotlight, (GameShaderParams){
												  .resolution = (Vector2){SCREEN_WIDTH, SCREEN_HEIGHT},
												  .intensity = 1.0f,
												  .tintColor = (Color){220, 210, 255, 255},
												  .dissolveThreshold = 0.05f,
												  .center = (Vector2){0.92f, scan_y}});

	// Update organism positions
	OrganismUpdate(&game_data->organism, (float)game_data->now);

	// Attack / action commands
	if (IsCommandActive(game_data->command, ATTACK_PRIMARY))
	{
		game_data->player_color = RED;
		PlaySFX(SFX_ATTACK_PRIMARY);
	}
	if (IsCommandActive(game_data->command, ATTACK_SECONDARY))
	{
		game_data->player_color = DARKPURPLE;
		PlaySFX(SFX_ATTACK_SECONDARY);
	}
	if (IsCommandActive(game_data->command, ACTION_SPECIAL_1))
	{
		game_data->player_color = DARKBLUE;
		PlaySFX(SFX_SPECIAL_1);
	}
	if (IsCommandActive(game_data->command, ACTION_SPECIAL_2))
	{
		game_data->player_color = DARKBLUE;
		PlaySFX(SFX_SPECIAL_2);
	}
	if (IsCommandActive(game_data->command, ACTION_SPECIAL_3))
	{
		game_data->player_color = DARKBLUE;
		PlaySFX(SFX_SPECIAL_3);
	}
	if (IsCommandActive(game_data->command, ACTION_SPECIAL_4))
	{
		game_data->player_color = DARKBLUE;
		PlaySFX(SFX_SPECIAL_4);
	}

	// Throttle display message
	const char *input_msg = GameInputGetMessage();
	if (strcmp(input_msg, last_message) != 0 || game_data->now - last_update > 0.2)
	{
		strcpy(last_message, input_msg);
		last_update = game_data->now;
	}

	// Points message
	char points_buffer[BUFFER_SIZE];
	snprintf(points_buffer, sizeof(points_buffer), "%d", game_data->player_points);
	if (strcmp(points_buffer, points_message) != 0 || game_data->now - points_update > 0.2)
	{
		strcpy(points_message, points_buffer);
		points_update = game_data->now;
	}

	// Command bitmap
	GetCommandBits(game_data->command, command_bitmap);
}

static void GameplayDraw(GameData *game_data)
{
	BeginDrawing();
	ClearBackground(BLACK);

	// Scene background - drawn first, full-screen behind everything
	if (game_data->background_gameplay.id != 0)
	{
		DrawTexturePro(game_data->background_gameplay,
					   (Rectangle){0, 0, (float)game_data->background_gameplay.width, (float)game_data->background_gameplay.height},
					   (Rectangle){0, 0, SCREEN_WIDTH, SCREEN_HEIGHT},
					   (Vector2){0, 0}, 0.0f, WHITE);
	}

	// Slime shader as background
	GameShaderBegin(game_data->slime);
	OrganismDraw(&game_data->organism);
	OrganismMembraneDrawWithGlow(&game_data->membrane);
	GameShaderEnd();

	// Background picture with edge burn shader
	GameShaderBegin(game_data->edge_burn);
	if (game_data->background.id != 0)
	{
		const int x = (SCREEN_WIDTH - (int)game_data->background.width) / 2;
		const int y = (SCREEN_HEIGHT - (int)game_data->background.height) / 2;
		DrawTexture(game_data->background, x, y, WHITE);
	}
	GameShaderEnd();

	// HUD
	DrawText("R36S | Xbox Input", CONTROLLER_OVERLAY_OFFSET_X + 70, 50, SMALL_FONT_SIZE, DARKGRAY);

#if defined(PLATFORM_R36S)
	DrawText("Platform: R36S", CONTROLLER_OVERLAY_OFFSET_X + 70, 70, SMALL_FONT_SIZE, DARKGRAY);
	DrawText("Press FN to exit", CONTROLLER_OVERLAY_OFFSET_X + 70, 90, SMALL_FONT_SIZE, DARKGRAY);
#elif defined(PLATFORM_LINUX)
	DrawText("Platform: Linux", CONTROLLER_OVERLAY_OFFSET_X + 70, 70, SMALL_FONT_SIZE, DARKGRAY);
#elif defined(PLATFORM_WINDOWS)
	DrawText("Platform: Windows", CONTROLLER_OVERLAY_OFFSET_X + 70, 70, SMALL_FONT_SIZE, DARKGRAY);
#elif defined(PLATFORM_WEB)
	DrawText("Platform: Web", CONTROLLER_OVERLAY_OFFSET_X + 70, 70, SMALL_FONT_SIZE, DARKGRAY);
#endif

	DrawText(last_message, CONTROLLER_OVERLAY_OFFSET_X + 70, 110, SMALL_FONT_SIZE, WHITE);

	// Level and kill progress
	int kill_target = game_data->current_level * 3;
	DrawText(TextFormat("Level: %d / %d", game_data->current_level, MAX_LEVELS),
			 CONTROLLER_OVERLAY_OFFSET_X + 70, 130, SMALL_FONT_SIZE, YELLOW);
	DrawText(TextFormat("Kills: %d / %d", game_data->kills_this_level, kill_target),
			 CONTROLLER_OVERLAY_OFFSET_X + 70, 150, SMALL_FONT_SIZE, YELLOW);

	// Thumbstick values
	float stick_Left_X = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X);
	float stick_Left_Y = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y);
	float stick_Right_X = GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_X);
	float stick_Right_Y = GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_Y);
	DrawText(TextFormat("X:%.2f Y:%.2f", stick_Left_X, stick_Left_Y), CONTROLLER_OVERLAY_OFFSET_X + 40, 420, SMALL_FONT_SIZE, DARKGRAY);
	DrawText(TextFormat("X:%.2f Y:%.2f", stick_Right_X, stick_Right_Y), CONTROLLER_OVERLAY_OFFSET_X + 190, 420, SMALL_FONT_SIZE, DARKGRAY);
	DrawText(command_bitmap, CONTROLLER_OVERLAY_OFFSET_X + 60, 220 - 20, SMALL_FONT_SIZE, DARKGRAY);

	// Player circle (damage flash shader)
	if (game_data->damage_flash > 0.0f)
	{
		GameShaderBegin(game_data->palette_tint);
		DrawCircleV(game_data->player_position, game_data->player_radius, game_data->player_color);
		GameShaderEnd();
	}
	else
	{
		DrawCircleV(game_data->player_position, game_data->player_radius, game_data->player_color);
	}

	// Spine entities -> render texture -> outline shader
	BeginTextureMode(game_data->outline_render_texture);
	ClearBackground(BLANK);
	GameSpineDraw();
	EndTextureMode();

	GameShaderSetParams(game_data->outline, (GameShaderParams){
												.resolution = (Vector2){SCREEN_WIDTH, SCREEN_HEIGHT},
												.intensity = 1.0f,
												.outlineColor = RED});

	GameShaderBegin(game_data->outline);
	DrawTextureRec(game_data->outline_render_texture.texture,
				   (Rectangle){0, 0, SCREEN_WIDTH, -SCREEN_HEIGHT},
				   (Vector2){0, 0}, WHITE);
	GameShaderEnd();

	// Reticle
	GameShaderSetParams(game_data->sparks, (GameShaderParams){
											   .resolution = (Vector2){SCREEN_WIDTH, SCREEN_HEIGHT},
											   .intensity = 1.0f,
											   .tintColor = ORANGE});
	DrawCircleV(game_data->reticle_position, game_data->reticle_radius, game_data->reticle_color);

#if defined(DEBUG)
	DrawRing(game_data->reticle_position,
			 game_data->reticle_radius, game_data->reticle_radius + 2.0f,
			 0.0f, 360.0f, 36, RED);
	DrawCircleV(game_data->reticle_position, 2.0f, YELLOW);
#endif

	// Fireball shader (skip GPU pass during 'gone' phase)
	float fireball_t = fmodf(fmodf((float)game_data->now, 100.0f), 4.0f);
	if (fireball_t < 1.5f)
	{
		GameShaderSetParams(game_data->fireball, (GameShaderParams){
													 .resolution = (Vector2){SCREEN_WIDTH, SCREEN_HEIGHT},
													 .intensity = 1.0f,
													 .tintColor = ORANGE,
													 .center = (Vector2){
														 game_data->reticle_position.x / (float)SCREEN_WIDTH,
														 1.0f - game_data->reticle_position.y / (float)SCREEN_HEIGHT}});

		BeginTextureMode(game_data->fireball_render_texture);
		ClearBackground(BLANK);
		EndTextureMode();

		GameShaderBegin(game_data->fireball);
		DrawTextureRec(game_data->fireball_render_texture.texture,
					   (Rectangle){0, 0, SCREEN_WIDTH, -SCREEN_HEIGHT},
					   (Vector2){0, 0}, WHITE);
		GameShaderEnd();
	}

	// Controller overlay + points
	GameInputDrawControllerOverlay();
	DrawText(points_message, CONTROLLER_OVERLAY_OFFSET_X + 140, 380, SMALL_FONT_SIZE, WHITE);

	// Telemetry
	GameTelemetryDraw();

	// Achievements always last - popups sit on top
	DrawAchievements(game_data->manager);

	// Modal sits above everything (level complete / game over / game complete)
	ModalDraw(&game_data->level_modal);

	EndDrawing();
}

//=================================================================
// GameplayExit
// Called once when leaving GAMEPLAY_SCENE.
//=================================================================
static void GameplayExit(GameData *game_data)
{
	// Player entity is owned by the Spine manager
	// freed by GameSpineExit().
	// Clear the pointer so stale access is caught immediately.
	(void)game_data;
	TraceLog(LOG_INFO, "GAME_GAMEPLAY: Gameplay Exited");
}

//=================================================================
// LEVEL_COMPLETE_SCENE
// Modal overlay: gameplay frozen, shows level complete message.
//=================================================================
static void LevelCompleteEnter(GameData *game_data)
{
	// GameData Unused
	(void)game_data;
	TraceLog(LOG_INFO, "GAME_GAMEPLAY: Level Complete Scene Entered");
}

static void LevelCompleteUpdate(GameData *game_data)
{
	ModalUpdate(&game_data->level_modal, (float)game_data->frame_time);

	// START_GAME - skip to next level immediately
	if (IsCommandActive(game_data->command, START_GAME))
	{
		PlaySFX(SFX_UI_SELECT);
		ModalCleanup(&game_data->level_modal);
		GameSceneSet(GAMEPLAY_SCENE, game_data);
		return;
	}
	// EXIT_COMMAND - quit game
	if (IsCommandActive(game_data->command, EXIT_COMMAND))
	{
		game_data->exit_game = true;
		game_data->exit_game_time = game_data->now;
		PlaySFX(SFX_UI_SELECT);
		return;
	}
	// Timer expired - advance automatically
	if (!ModalIsActive(&game_data->level_modal))
		GameSceneSet(GAMEPLAY_SCENE, game_data);
}

static void LevelCompleteDraw(GameData *game_data)
{
	// Clear to black - ModalDraw draws the background texture inside the box
	BeginDrawing();
	ClearBackground(BLACK);
	ModalDraw(&game_data->level_modal);
	EndDrawing();
}

static void LevelCompleteExit(GameData *game_data)
{
	// GameData Unused
	(void)game_data;
	TraceLog(LOG_INFO, "GAME_GAMEPLAY: Level Complete Scene Exited");
}

//=================================================================
// GAME_OVER_SCENE
// Modal overlay
// Replay or quit.
//=================================================================
static void GameOverEnter(GameData *game_data)
{
	// GameData Unused
	(void)game_data;
	TraceLog(LOG_INFO, "GAME_GAMEPLAY: Game Over Scene Entered");
}

static void GameOverUpdate(GameData *game_data)
{
	ModalUpdate(&game_data->level_modal, (float)game_data->frame_time);

	// Debug: show which commands are active on R36S
	char bmp[COMMAND_COUNT + 1];
	GetCommandBits(game_data->command, bmp);
	if (bmp[0] != '\0' && strcmp(bmp, command_bitmap) != 0)
		TraceLog(LOG_INFO, "GAME_OVER: command=%s", bmp);

	// START_GAME - replay level immediately
	if (IsCommandActive(game_data->command, START_GAME))
	{
		PlaySFX(SFX_UI_SELECT);
		ReplayLevel(game_data);
		return;
	}
	// EXIT_COMMAND
	// Quit game
	if (IsCommandActive(game_data->command, EXIT_COMMAND))
	{
		game_data->exit_game = true;
		game_data->exit_game_time = game_data->now;
		PlaySFX(SFX_UI_SELECT);
		return;
	}
	// Timer expired
	// auto replay
	if (!ModalIsActive(&game_data->level_modal))
		ReplayLevel(game_data);
}

static void GameOverDraw(GameData *game_data)
{
	// Clear to black - ModalDraw draws the background texture inside the box
	BeginDrawing();
	ClearBackground(BLACK);
	ModalDraw(&game_data->level_modal);
	EndDrawing();
}

static void GameOverExit(GameData *game_data)
{
	// GameData Unused
	(void)game_data;
	TraceLog(LOG_INFO, "GAME_GAMEPLAY: Game Over Scene Exited");
}

//=================================================================
// GAME_COMPLETE_SCENE
// Modal overlay
// All levels done.
// Replay game or quit.
//=================================================================
static void GameCompleteEnter(GameData *game_data)
{
	// GameData Unused
	(void)game_data;
	TraceLog(LOG_INFO, "GAME_GAMEPLAY: Game Complete Scene Entered");
}

static void GameCompleteUpdate(GameData *game_data)
{
	// No ModalUpdate - no timer, waits for explicit input

	// Debug: show which commands are active on R36S
	char bmp[COMMAND_COUNT + 1];
	GetCommandBits(game_data->command, bmp);
	if (bmp[0] != '\0' && strcmp(bmp, command_bitmap) != 0)
		TraceLog(LOG_INFO, "GAME_COMPLETE: command=%s", bmp);

	// START_GAME
	// replay game from level 1
	if (IsCommandActive(game_data->command, START_GAME))
	{
		PlaySFX(SFX_UI_SELECT);
		ResetGame(game_data);
		return;
	}
	// EXIT_COMMAND
	// quit game
	if (IsCommandActive(game_data->command, EXIT_COMMAND))
	{
		game_data->exit_game = true;
		game_data->exit_game_time = game_data->now;
		PlaySFX(SFX_UI_SELECT);
	}
}

static void GameCompleteDraw(GameData *game_data)
{
	// Clear to black - ModalDraw draws the background texture inside the box
	BeginDrawing();
	ClearBackground(BLACK);
	ModalDraw(&game_data->level_modal);
	EndDrawing();
}

static void GameCompleteExit(GameData *game_data)
{
	// GameData Unused
	(void)game_data;
	TraceLog(LOG_INFO, "GAME_GAMEPLAY: Game Complete Exited");
}

//=================================================================
// LevelComplete
// Notify achievements, advance level, show modal.
//=================================================================
static void LevelComplete(GameData *game_data)
{
	int level = game_data->current_level;

	NotifyLevelComplete(game_data->manager, level);

	if (!game_data->took_damage_this_level)
		NotifyFlawlessLevel(game_data->manager, level);

	if (game_data->secrets_found_this_level >= game_data->total_secrets_this_level)
		NotifyAllSecretsFound(game_data->manager, level);

	// Reset per-level state
	game_data->kills_this_level = 0;
	game_data->took_damage_this_level = false;
	game_data->secrets_found_this_level = 0;

	TraceLog(LOG_INFO, "GAME_GAMEPLAY: Level %d complete", level);

	// Last level
	// Go to game complete
	if (level >= MAX_LEVELS)
	{
		GameComplete(game_data);
		return;
	}

	// Advance and show level complete modal
	game_data->current_level++;
	TraceLog(LOG_INFO, "GAME_GAMEPLAY: Advancing to level %d", game_data->current_level);

	char title_buf[MODAL_TITLE_MAX];
	char msg_buf[MODAL_MESSAGE_MAX];
	snprintf(title_buf, sizeof(title_buf), "Level %d Complete!", level);
	snprintf(msg_buf, sizeof(msg_buf), "Starting Level %d...", game_data->current_level);
	ModalCreate(&game_data->level_modal, title_buf, msg_buf, 3.0f, &game_data->background_level_complete);

	GameSceneSet(LEVEL_COMPLETE_SCENE, game_data);
}

//=================================================================
// GameOver
//=================================================================
static void GameOver(GameData *game_data)
{
	char title_buf[MODAL_TITLE_MAX];
	char msg_buf[MODAL_MESSAGE_MAX];
	snprintf(title_buf, sizeof(title_buf), "Game Over");
	snprintf(msg_buf, sizeof(msg_buf), "Start (Replay Game)\t\tFN (Quit)");
	ModalCreate(&game_data->level_modal, title_buf, msg_buf, 3.0f, &game_data->background_game_over);
	GameSceneSet(GAME_OVER_SCENE, game_data);
}

//=================================================================
// GameComplete
//=================================================================
static void GameComplete(GameData *game_data)
{
	char title_buf[MODAL_TITLE_MAX];
	char msg_buf[MODAL_MESSAGE_MAX];
	snprintf(title_buf, sizeof(title_buf), "All Levels Complete");
	snprintf(msg_buf, sizeof(msg_buf), "Start (Replay Game)\t\tFN (Quit)");
	ModalCreate(&game_data->level_modal, title_buf, msg_buf, 0.0f, &game_data->background_game_complete);
	GameSceneSet(GAME_COMPLETE_SCENE, game_data);
}

//=================================================================
// ReplayLevel
// Resets per-level state only
// Level number unchanged
//=================================================================
static void ReplayLevel(GameData *game_data)
{
	ModalCleanup(&game_data->level_modal);

	game_data->kills_this_level = 0;
	game_data->took_damage_this_level = false;
	game_data->secrets_found_this_level = 0;

	game_data->player_position = (Vector2){SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f};
	SetSpinePosition(game_data->player,
					 game_data->player_position.x,
					 game_data->player_position.y);

	TraceLog(LOG_INFO, "GAME_GAMEPLAY: Replaying level %d", game_data->current_level);
	GameSceneSet(GAMEPLAY_SCENE, game_data);
}

//=================================================================
// ResetGame
// Full reset (all levels, kills, coins)
// Returns to level 1.
//=================================================================
static void ResetGame(GameData *game_data)
{
	ModalCleanup(&game_data->level_modal);

	game_data->player_points = 0;
	game_data->current_level = 1;
	game_data->total_kills = 0;
	game_data->kills_this_level = 0;
	game_data->total_coins = 0;
	game_data->took_damage_this_level = false;
	game_data->secrets_found_this_level = 0;

	game_data->player_position = (Vector2){SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f};
	SetSpinePosition(game_data->player,
					 game_data->player_position.x,
					 game_data->player_position.y);

	// Reset achievements
	GameAchievementsExit(game_data);
	GameAchievementsInit(game_data);

	TraceLog(LOG_INFO, "GAME_GAMEPLAY: Game reset and returning to level 1");
	GameSceneSet(GAMEPLAY_SCENE, game_data);
}
//=================================================================
// Game Input
// Owns input polling, volume control, the per-frame
// human-readable message buffer, and the controller overlay.
//=================================================================

#ifdef __cplusplus
#include <cstring>
#include <cstdio>
#include <cstddef>
#else
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#endif

#include <raylib.h>
#include <math.h>

#include "game/game_input.h"
#include "game/game_telemetry.h"

#include "core/input_manager.h"
#include "core/sound_manager.h"

#include "constants.h"

//=================================================================
// Last polled command (used by DrawOverlay)
//=================================================================
static Command last_command = {0};

//=================================================================
// Message Buffer
//=================================================================
static char message[BUFFER_SIZE] = "";
static bool has_messages = false;

static void ResetMessage(void)
{
	message[0] = '\0';
	has_messages = false;
}

static void AppendMessage(const char *text)
{
	if (strstr(message, text) == NULL)
	{
		size_t len = strlen(message);
		snprintf(message + len, BUFFER_SIZE - len, "%s%s",
				 has_messages ? "\n" : "", text);
		has_messages = true;
	}
}

//=================================================================
// Volume tracking (R36S only)
//=================================================================
static bool volume_up_was_pressed = false;
static bool volume_down_was_pressed = false;

//=================================================================
// Telemetry toggle tracking (R36S / Linux only)
//=================================================================
#if defined(PLATFORM_R36S) || defined(PLATFORM_LINUX)
static bool action_special_2_was_pressed = false;
#endif

//=================================================================
// Handle volume buttons (R36S only)
//=================================================================
#if defined(PLATFORM_R36S)
static void PollVolume(Command command)
{
	bool volume_up_pressed = IsCommandActive(command, VOLUME_UP);
	bool volume_down_pressed = IsCommandActive(command, VOLUME_DOWN);

	if (volume_up_pressed && !volume_up_was_pressed)
	{
		float new_volume = GetMasterSoundVolume() + 0.1f;
		if (new_volume > 1.0f)
			new_volume = 1.0f;
		SetMasterSoundVolume(new_volume);
		SaveSoundSettings("assets/sound_settings.ini");
		PlaySFX(SFX_UI_SELECT);
		TraceLog(LOG_INFO, "GAME_INPUT: Volume Up -> %.2f", new_volume);
	}

	if (volume_down_pressed && !volume_down_was_pressed)
	{
		float new_volume = GetMasterSoundVolume() - 0.1f;
		if (new_volume < 0.0f)
			new_volume = 0.0f;
		SetMasterSoundVolume(new_volume);
		SaveSoundSettings("assets/sound_settings.ini");
		PlaySFX(SFX_UI_SELECT);
		TraceLog(LOG_INFO, "GAME_INPUT: Volume Down -> %.2f", new_volume);
	}

	volume_up_was_pressed = volume_up_pressed;
	volume_down_was_pressed = volume_down_pressed;
}
#endif

//=================================================================
// Handle telemetry toggle button (R36S / Linux only)
// Input owns the button mapping, telemetry owns the state
//=================================================================
#if defined(PLATFORM_R36S) || defined(PLATFORM_LINUX)
static void PollTelemetryToggle(Command command)
{
	bool pressed = IsCommandActive(command, ACTION_SPECIAL_2);

	if (pressed && !action_special_2_was_pressed)
		GameTelemetryToggle();

	action_special_2_was_pressed = pressed;
}
#endif

//=================================================================
// Build the human-readable message for this frame
//=================================================================
static void BuildMessage(Command command)
{
	// Movement
	if (IsCommandActive(command, MOVE_UP))
		AppendMessage("MOVE_UP");
	if (IsCommandActive(command, MOVE_DOWN))
		AppendMessage("MOVE_DOWN");
	if (IsCommandActive(command, MOVE_LEFT))
		AppendMessage("MOVE_LEFT");
	if (IsCommandActive(command, MOVE_RIGHT))
		AppendMessage("MOVE_RIGHT");

	// Aiming
	if (IsCommandActive(command, AIM_UP))
		AppendMessage("AIM_UP");
	if (IsCommandActive(command, AIM_DOWN))
		AppendMessage("AIM_DOWN");
	if (IsCommandActive(command, AIM_LEFT))
		AppendMessage("AIM_LEFT");
	if (IsCommandActive(command, AIM_RIGHT))
		AppendMessage("AIM_RIGHT");

	// Actions
	if (IsCommandActive(command, ACTION_JUMP))
		AppendMessage("ACTION_JUMP");
	if (IsCommandActive(command, ACTION_CROUCH))
		AppendMessage("ACTION_CROUCH");
	if (IsCommandActive(command, ACTION_PICKUP))
		AppendMessage("ACTION_PICKUP");
	if (IsCommandActive(command, ACTION_RUN))
		AppendMessage("ACTION_RUN");

	// Attacks
	if (IsCommandActive(command, ATTACK_PRIMARY))
		AppendMessage("ATTACK_PRIMARY");
	if (IsCommandActive(command, ATTACK_SECONDARY))
		AppendMessage("ATTACK_SECONDARY");

	// Specials
	if (IsCommandActive(command, ACTION_SPECIAL_1))
		AppendMessage("ACTION_SPECIAL_1");
	if (IsCommandActive(command, ACTION_SPECIAL_2))
		AppendMessage("ACTION_SPECIAL_2");
	if (IsCommandActive(command, ACTION_SPECIAL_3))
		AppendMessage("ACTION_SPECIAL_3");
	if (IsCommandActive(command, ACTION_SPECIAL_4))
		AppendMessage("ACTION_SPECIAL_4");

	// System
	if (IsCommandActive(command, EXIT_COMMAND))
		AppendMessage("EXIT");
	if (IsCommandActive(command, POWER_COMMAND))
		AppendMessage("POWER");
	if (IsCommandActive(command, START_GAME))
		AppendMessage("START");
	if (IsCommandActive(command, MENU_TOGGLE))
		AppendMessage("MENU");
	if (IsCommandActive(command, VOLUME_UP))
		AppendMessage("VOLUME++");
	if (IsCommandActive(command, VOLUME_DOWN))
		AppendMessage("VOLUME--");

	// Fallback
	if (!has_messages)
		AppendMessage("NONE");
}

//=================================================================
// Game Input uses Input Manager
// Abstract from hardware specifics
//=================================================================
void GameInputInit(void)
{
	// Initialise the platform hardware layer first.
	// On R36S this opens /dev/input/event0,2,3 for power, gamepad,
	// and volume buttons.  On other platforms it is a no-op log.
	// Must be called after InitWindow() (Raylib gamepad scan runs
	// during window creation).
	InitInputManager();
}

void GameInputPoll(GameData *game_data)
{
	ResetMessage();

	// Poll hardware -> write command into game_data and cache locally
	game_data->command = PollInput();
	last_command = game_data->command;

#if defined(PLATFORM_R36S) || defined(PLATFORM_LINUX)
	PollTelemetryToggle(game_data->command);
#endif

#if defined(PLATFORM_R36S)
	PollVolume(game_data->command);
#endif

	BuildMessage(game_data->command);
}

const char *GameInputGetMessage(void)
{
	return message;
}

//=================================================================
// Draw the R36S / Xbox controller overlay
// This is not required for Game, its just an input helper
// Uses the command(s) from most recent GameInputPoll call
//=================================================================
void GameInputDrawControllerOverlay(void)
{
	Command command = last_command;

	// Pulse animation
	float pulse = (sinf(GetTime() * 6.0f) + 1.0f) * 0.5f;

	// D-pad
	if (IsCommandActive(command, MOVE_UP))
	{
		// D-Pad Up
		Color pulseColor = Fade(DARKGRAY, 0.4f + 0.6f * pulse);
		DrawCircle(CONTROLLER_OVERLAY_OFFSET_X + 87, 260, 10, pulseColor);
	}
	if (IsCommandActive(command, MOVE_DOWN))
	{
		// D-Pad Down
		Color pulseColor = Fade(DARKGRAY, 0.4f + 0.6f * pulse);
		DrawCircle(CONTROLLER_OVERLAY_OFFSET_X + 87, 310, 10, pulseColor);
	}
	if (IsCommandActive(command, MOVE_LEFT))
	{
		// D-Pad Left
		Color pulseColor = Fade(DARKGRAY, 0.4f + 0.6f * pulse);
		DrawCircle(CONTROLLER_OVERLAY_OFFSET_X + 62, 285, 10, pulseColor);
	}
	if (IsCommandActive(command, MOVE_RIGHT))
	{
		// D-Pad Right
		Color pulseColor = Fade(DARKGRAY, 0.4f + 0.6f * pulse);
		DrawCircle(CONTROLLER_OVERLAY_OFFSET_X + 110, 285, 10, pulseColor);
	}

	// Thumbsticks
	if (IsCommandActive(command, AIM_UP) || IsCommandActive(command, AIM_DOWN) ||
		IsCommandActive(command, AIM_LEFT) || IsCommandActive(command, AIM_RIGHT) ||
		IsCommandActive(command, ACTION_SPECIAL_4))
	{
		// Right Thumbstick (R3)
		Color pulseColor = Fade(DARKGRAY, 0.4f + 0.6f * pulse);
		DrawCircle(CONTROLLER_OVERLAY_OFFSET_X + 245, 375, 20, pulseColor);
	}
	if (IsCommandActive(command, MOVE_UP) || IsCommandActive(command, MOVE_DOWN) ||
		IsCommandActive(command, MOVE_LEFT) || IsCommandActive(command, MOVE_RIGHT) ||
		IsCommandActive(command, ACTION_SPECIAL_3))
	{
		// Left Thumbstick (L3)
		Color pulseColor = Fade(DARKGRAY, 0.4f + 0.6f * pulse);
		DrawCircle(CONTROLLER_OVERLAY_OFFSET_X + 85, 375, 20, pulseColor);
	}

	// Trigger buttons (L1, L2, R1, R2)
	if (IsCommandActive(command, ACTION_SPECIAL_1))
	{
		// L1 (R36S) / LT (Xbox) - Left trigger
		Color pulseColor = Fade(DARKGRAY, 0.3f + 0.5f * pulse);
		DrawRectangle(CONTROLLER_OVERLAY_OFFSET_X + 50, 225, 50, 70, pulseColor);
	}
	if (IsCommandActive(command, ACTION_SPECIAL_2))
	{
		// L2 (R36S) / LB (Xbox) - Left bumper trigger
		Color pulseColor = Fade(DARKGRAY, 0.3f + 0.5f * pulse);
		DrawRectangle(CONTROLLER_OVERLAY_OFFSET_X + 110, 225, 50, 70, pulseColor);
	}
	if (IsCommandActive(command, ATTACK_PRIMARY))
	{
		// R1 (R36S) / RT (Xbox) - Right trigger
		Color pulseColor = Fade(DARKGRAY, 0.3f + 0.5f * pulse);
		DrawRectangle(CONTROLLER_OVERLAY_OFFSET_X + 240, 225, 50, 70, pulseColor);
	}
	if (IsCommandActive(command, ATTACK_SECONDARY))
	{
		// R2 (R36S) / RB (Xbox) - Right bumper trigger
		Color pulseColor = Fade(DARKGRAY, 0.3f + 0.5f * pulse);
		DrawRectangle(CONTROLLER_OVERLAY_OFFSET_X + 190, 225, 50, 70, pulseColor);
	}

	// Face buttons
#if defined(PLATFORM_R36S)
	// R36S face buttons: Top = X (Blue), Bottom = B (Yellow), Left = Y (Green), Right = A (Red)
	if (IsCommandActive(command, ACTION_PICKUP))
	{
		// X (Top - Blue)
		Color pulseColor = Fade(DARKBLUE, 0.4f + 0.6f * pulse);
		DrawCircle(CONTROLLER_OVERLAY_OFFSET_X + 250, 260, 12, pulseColor);
	}
	if (IsCommandActive(command, ACTION_CROUCH))
	{
		// B (Bottom - Yellow)
		Color pulseColor = Fade(YELLOW, 0.4f + 0.6f * pulse);
		DrawCircle(CONTROLLER_OVERLAY_OFFSET_X + 250, 310, 12, pulseColor);
	}
	if (IsCommandActive(command, ACTION_RUN))
	{
		// Y (Top - Green)
		Color pulseColor = Fade(DARKGREEN, 0.4f + 0.6f * pulse);
		DrawCircle(CONTROLLER_OVERLAY_OFFSET_X + 220, 285, 12, pulseColor);
	}
	if (IsCommandActive(command, ACTION_JUMP))
	{
		// A (Right - Red)
		Color pulseColor = Fade(RED, 0.4f + 0.6f * pulse);
		DrawCircle(CONTROLLER_OVERLAY_OFFSET_X + 280, 285, 12, pulseColor);
	}
#else
	// Xbox face buttons: Top = Y (Yellow), Bottom = A (Green), Left = X (Blue), Right = B (Red)
	if (IsCommandActive(command, ACTION_JUMP))
	{
		// A (Bottom - Green)
		Color pulseColor = Fade(DARKGREEN, 0.4f + 0.6f * pulse);
		DrawCircle(CONTROLLER_OVERLAY_OFFSET_X + 250, 310, 12, pulseColor);
	}
	if (IsCommandActive(command, ACTION_CROUCH))
	{
		// B (Right - Red)
		Color pulseColor = Fade(RED, 0.4f + 0.6f * pulse);
		DrawCircle(CONTROLLER_OVERLAY_OFFSET_X + 280, 285, 12, pulseColor);
	}
	if (IsCommandActive(command, ACTION_PICKUP))
	{
		// X (Left - Blue)
		Color pulseColor = Fade(DARKBLUE, 0.4f + 0.6f * pulse);
		DrawCircle(CONTROLLER_OVERLAY_OFFSET_X + 220, 285, 12, pulseColor);
	}
	if (IsCommandActive(command, ACTION_RUN))
	{
		// Y (Top - Yellow)
		Color pulseColor = Fade(YELLOW, 0.4f + 0.6f * pulse);
		DrawCircle(CONTROLLER_OVERLAY_OFFSET_X + 250, 260, 12, pulseColor);
	}
#endif

	// Center buttons
#if defined(PLATFORM_R36S)
	if (IsCommandActive(command, EXIT_COMMAND))
	{
		// FN (Exits Game)
		Color pulseColor = Fade(RED, 0.4f + 0.6f * pulse);
		DrawCircle(CONTROLLER_OVERLAY_OFFSET_X + 165, 310, 12, pulseColor);
	}
#endif // Centre Button R36S

	if (IsCommandActive(command, START_GAME))
	{
		// START
		Color pulseColor = Fade(DARKGREEN, 0.4f + 0.6f * pulse);
		DrawCircle(CONTROLLER_OVERLAY_OFFSET_X + 195, 340, 12, pulseColor);
	}
	if (IsCommandActive(command, MENU_TOGGLE))
	{
		// SELECT
		Color pulseColor = Fade(DARKGREEN, 0.4f + 0.6f * pulse);
		DrawCircle(CONTROLLER_OVERLAY_OFFSET_X + 135, 340, 12, pulseColor);
	}

	// Side buttons
#if defined(PLATFORM_R36S)
	if (IsCommandActive(command, POWER_COMMAND))
	{
		// Power
		Color pulseColor = Fade(RED, 0.4f + 0.6f * pulse);
		DrawCircle(CONTROLLER_OVERLAY_OFFSET_X + 10, 120, 12, pulseColor);
	}
	if (IsCommandActive(command, VOLUME_UP))
	{
		// Vol+
		Color pulseColor = Fade(RED, 0.4f + 0.6f * pulse);
		DrawCircle(CONTROLLER_OVERLAY_OFFSET_X + 320, 120, 12, pulseColor);
	}
	if (IsCommandActive(command, VOLUME_DOWN))
	{
		// Vol-
		Color pulseColor = Fade(RED, 0.4f + 0.6f * pulse);
		DrawCircle(CONTROLLER_OVERLAY_OFFSET_X + 320, 160, 12, pulseColor);
	}
#endif
}

void GameInputExit(void)
{
	// Close hardware input devices (R36S /dev/input fds)
	ExitInputManager();
}
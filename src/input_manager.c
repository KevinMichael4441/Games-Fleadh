#include <raylib.h>
#include <stdio.h>
#include <math.h>

#include "input_manager.h"
#include "constants.h"

// Platform-specific includes for R36S
#ifdef PLATFORM_R36S
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <errno.h>
#include <string.h>

// R36S-specific input device file descriptors
static int event_power_button = -1;
static int event_controller_button = -1;
static int event_volume_buttons = -1;

// R36S button states
static bool r36s_vol_up = false;
static bool r36s_vol_down = false;
static bool r36s_power = false;
static bool r36s_select = false;
static bool r36s_start = false;
static bool r36s_left_stick_btn = false;
static bool r36s_right_stick_btn = false;
static bool r36s_fn = false;

// Function to read events from R36S
static void ReadR36SInputEvents(int fd)
{
	struct input_event ev;

	while (read(fd, &ev, sizeof(ev)) == sizeof(ev))
	{
		if (ev.type == EV_KEY)
		{
			bool pressed = (ev.value == 1);

			switch (ev.code)
			{
			case 115:
				r36s_vol_up = pressed;
				break; // VOL_UP
			case 114:
				r36s_vol_down = pressed;
				break; // VOL_DOWN
			case 116:
				r36s_power = pressed;
				break; // POWER
			case 704:
				r36s_select = pressed;
				break; // SELECT
			case 705:
				r36s_start = pressed;
				break; // START
			case 706:
				r36s_left_stick_btn = pressed;
				break; // L3
			case 707:
				r36s_right_stick_btn = pressed;
				break; // R3
			case 708:
				r36s_fn = pressed;
				break; // FN
			}
		}
	}
}
#endif // PLATFORM_R36S

void InitInputManager()
{
#ifdef PLATFORM_R36S
	TraceLog(LOG_INFO, "Initializing R36S input manager");

	// Open R36S-specific input devices
	event_power_button = open("/dev/input/event0", O_RDONLY | O_NONBLOCK);
	if (event_power_button < 0)
		TraceLog(LOG_WARNING, "Failed to open /dev/input/event0: %s", strerror(errno));
	else
		TraceLog(LOG_INFO, "Opened /dev/input/event0 (Power button)");

	event_controller_button = open("/dev/input/event2", O_RDONLY | O_NONBLOCK);
	if (event_controller_button < 0)
		TraceLog(LOG_WARNING, "Failed to open /dev/input/event2: %s", strerror(errno));
	else
		TraceLog(LOG_INFO, "Opened /dev/input/event2 (Gamepad)");

	event_volume_buttons = open("/dev/input/event3", O_RDONLY | O_NONBLOCK);
	if (event_volume_buttons < 0)
		TraceLog(LOG_WARNING, "Failed to open /dev/input/event3: %s", strerror(errno));
	else
		TraceLog(LOG_INFO, "Opened /dev/input/event3 (Volume buttons)");
#else
	TraceLog(LOG_INFO, "Initializing standard input manager");
#endif // PLATFORM_R36S
}

Command PollInput()
{
	// Default command
	Command command = NONE;

#ifdef PLATFORM_R36S
	// Read R36S-specific input events
	if (event_power_button >= 0)
		ReadR36SInputEvents(event_power_button);
	if (event_controller_button >= 0)
		ReadR36SInputEvents(event_controller_button);
	if (event_volume_buttons >= 0)
		ReadR36SInputEvents(event_volume_buttons);

	if (r36s_power)
		command |= POWER_COMMAND; // Placeholder toggle save game
	if (r36s_fn)
		command |= EXIT_COMMAND; // FN Key
	if (r36s_vol_up)
		command |= VOLUME_UP; // Volume Up Key
	if (r36s_vol_down)
		command |= VOLUME_DOWN; // Volume Down Key
#endif							// PLATFORM_R36S

	// Check for gamepad input first
	if (IsGamepadAvailable(0))
	{
		// Supports one controller only for portability
		int gamepad = 0;

		// Get values for thumbstick
		float stick_Left_X = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X);
		float stick_Left_Y = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y);

		float stick_Right_X = GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_X);
		float stick_Right_Y = GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_Y);

#ifdef PLATFORM_R36S
		// Digital Triggers (R36S) (see command.h)
		bool trigger_L1 = IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_LEFT_TRIGGER_1);
		bool trigger_L2 = IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_LEFT_TRIGGER_2);
		bool trigger_R1 = IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_RIGHT_TRIGGER_1);
		bool trigger_R2 = IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_RIGHT_TRIGGER_2);
#else
		// Analog triggers (Xbox) (see command.h)
		float trigger_LT_Axis = GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_LEFT_TRIGGER);
		float trigger_RT_Axis = GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_RIGHT_TRIGGER);

		// Digital triggers (Xbox) (see command.h)
		bool trigger_LB = IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_LEFT_TRIGGER_1);
		bool trigger_RB = IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_RIGHT_TRIGGER_1);

		// Digital tumbstick buttons
		bool tumbstick_Left_Button = IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_LEFT_THUMB);
		bool tumbstick_Right_Button = IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_RIGHT_THUMB);
#endif

		// D-pad (see command.h)
		bool d_pad_UP = IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_LEFT_FACE_UP);
		bool d_pad_DOWN = IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_LEFT_FACE_DOWN);
		bool d_pad_LEFT = IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_LEFT_FACE_LEFT);
		bool d_pad_RIGHT = IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_LEFT_FACE_RIGHT);

		// Face buttons (see command.h)
#ifdef PLATFORM_R36S
		// R36S: Top = X (Blue), Bottom = B (Yellow), Left = Y (Green), Right = A (Red)
		bool button_A = IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT);
		bool button_B = IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_DOWN);
		bool button_X = IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_LEFT);
		bool button_Y = IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_UP);
#else
		// Xbox: Top = Y (Yellow), Bottom = A (Green), Left = X (Blue), Right = B (Red)
		bool button_A = IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_DOWN);
		bool button_B = IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT);
		bool button_X = IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_UP);
		bool button_Y = IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_RIGHT_FACE_LEFT);
#endif

		// Determine if any gamepad input exceeds threshold levels
		// add buttons too
		bool gamepadActive =
			fabsf(stick_Left_X) > TUMBSTICK_DEADZONE_THRESHOLD ||
			fabsf(stick_Left_Y) > TUMBSTICK_DEADZONE_THRESHOLD ||
			fabsf(stick_Right_X) > TUMBSTICK_DEADZONE_THRESHOLD ||
			fabsf(stick_Right_Y) > TUMBSTICK_DEADZONE_THRESHOLD ||
			d_pad_UP || d_pad_DOWN || d_pad_LEFT || d_pad_RIGHT ||
			button_A || button_B || button_X || button_Y ||
			IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_LEFT_THUMB) ||
			IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_RIGHT_THUMB) ||
			IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_MIDDLE) ||
			IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_MIDDLE_LEFT) ||
			IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_MIDDLE_RIGHT);

#ifdef PLATFORM_R36S
		// R36S: Add digital trigger and button checks
		if (trigger_L1 || trigger_L2 ||
			trigger_R1 || trigger_R2 ||
			r36s_left_stick_btn || r36s_right_stick_btn ||
			r36s_select || r36s_start || r36s_fn ||
			r36s_power || r36s_vol_up || r36s_vol_down)
		{
			gamepadActive = true;
		}
#else
		// Xbox: Add analog triggers and digital bumpers
		if ((trigger_LT_Axis > FIRING_TRIGGER_TRESHOLD) ||
			(trigger_RT_Axis > FIRING_TRIGGER_TRESHOLD) ||
			trigger_LB || trigger_RB || tumbstick_Left_Button || tumbstick_Right_Button)
		{
			gamepadActive = true;
		}
#endif

		// If the gamepad is active, determine specific command based on input
		if (gamepadActive)
		{
			// Check D-pad directional buttons for movement commands (see command.h)
			if (d_pad_UP)
				command |= MOVE_UP;
			if (d_pad_DOWN)
				command |= MOVE_DOWN;
			if (d_pad_LEFT)
				command |= MOVE_LEFT;
			if (d_pad_RIGHT)
				command |= MOVE_RIGHT;

			// Check thumbstick for directional input, prioritising vertical movement
			// Movement from left stick
			if (fabsf(stick_Left_Y) > TUMBSTICK_DEADZONE_THRESHOLD ||
				fabsf(stick_Left_X) > TUMBSTICK_DEADZONE_THRESHOLD)
			{
				if (stick_Left_Y < -MOVE_VERTICAL_THRESHOLD)
					command |= MOVE_UP;
				if (stick_Left_Y > MOVE_VERTICAL_THRESHOLD)
					command |= MOVE_DOWN;
				if (stick_Left_X < -MOVE_HORIZONTAL_THRESHOLD)
					command |= MOVE_LEFT;
				if (stick_Left_X > MOVE_HORIZONTAL_THRESHOLD)
					command |= MOVE_RIGHT;
			}

// Combat mapping (see command.h)
#ifdef PLATFORM_R36S
			// R36S: R1 = ATTACK_PRIMARY, R2 = ATTACK_SECONDARY
			if (trigger_R1)
				command |= ATTACK_PRIMARY;
			if (trigger_R2)
				command |= ATTACK_SECONDARY;
#else
			// Xbox: RT (analog) = ATTACK_PRIMARY, RB (digital) = ATTACK_SECONDARY
			if (trigger_RT_Axis > FIRING_TRIGGER_TRESHOLD)
				command |= ATTACK_PRIMARY;
			if (trigger_RB)
				command |= ATTACK_SECONDARY;
#endif

// Abilities (see command.h)
#ifdef PLATFORM_R36S
			// R36S: L1 = ACTION_SPECIAL_1, L2 = ACTION_SPECIAL_2
			if (trigger_L1)
				command |= ACTION_SPECIAL_1;
			if (trigger_L2)
				command |= ACTION_SPECIAL_2;
#else
			// Xbox: LT (analog) = ACTION_SPECIAL_1, LB (digital) = ACTION_SPECIAL_2
			if (trigger_LT_Axis > FIRING_TRIGGER_TRESHOLD)
				command |= ACTION_SPECIAL_1;
			if (trigger_LB)
				command |= ACTION_SPECIAL_2;
#endif

// Stick buttons (see command.h)
#ifdef PLATFORM_R36S
			// R36S: Left Stick Button = ACTION_SPECIAL_3, Right Stick Button = ACTION_SPECIAL_4
			if (r36s_left_stick_btn)
				command |= ACTION_SPECIAL_3;
			if (r36s_right_stick_btn)
				command |= ACTION_SPECIAL_4;
#else
			// Xbox: Left Stick Button = ACTION_SPECIAL_3, Right Stick Button = ACTION_SPECIAL_4
			if (tumbstick_Left_Button)
				command |= ACTION_SPECIAL_3;
			if (tumbstick_Right_Button)
				command |= ACTION_SPECIAL_4;
#endif

			// Face buttons (see command.h)
			if (button_A)
				command |= ACTION_JUMP;
			if (button_B)
				command |= ACTION_CROUCH;
			if (button_X)
				command |= ACTION_PICKUP;
			if (button_Y)
				command |= ACTION_RUN;

			// Aim (right stick placeholder for now)
			if (fabsf(stick_Right_X) > TUMBSTICK_DEADZONE_THRESHOLD ||
				fabsf(stick_Right_Y) > TUMBSTICK_DEADZONE_THRESHOLD)
			{
				command |= AIM;
			}

// Menu and Start (see command.h)
#ifdef PLATFORM_R36S
			// R36S Gamepad Select | Start Button Mapping
			if (r36s_select)
				command |= MENU_TOGGLE; // Select Key
			if (r36s_start)
				command |= START_GAME; // Start Key
#else
			// Xbox | PS controller Back | Start Button Mapping
			if (IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_MIDDLE_LEFT))
				command |= MENU_TOGGLE; // Back/View
			if (IsGamepadButtonDown(gamepad, GAMEPAD_BUTTON_MIDDLE_RIGHT))
				command |= START_GAME; // Start/Menu
#endif
		}
	}

	// If no gamepad input check keyboard
	// Movement
	if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))
		command |= MOVE_UP;
	if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))
		command |= MOVE_DOWN;
	if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))
		command |= MOVE_LEFT;
	if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT))
		command |= MOVE_RIGHT;

	// Attacks
	if (IsKeyPressed(KEY_SPACE))
		command |= ATTACK_PRIMARY;

	if (IsKeyPressed(KEY_LEFT_CONTROL) || IsKeyPressed(KEY_RIGHT_CONTROL))
		command |= ATTACK_SECONDARY;

	// Abilities
	if (IsKeyPressed(KEY_Z))
		command |= ACTION_SPECIAL_1;

	if (IsKeyPressed(KEY_C))
		command |= ACTION_SPECIAL_2;

	if (IsKeyPressed(KEY_V))
		command |= ACTION_SPECIAL_3;

	if (IsKeyPressed(KEY_B))
		command |= ACTION_SPECIAL_4;

	// Face buttons mapping (per your README)
	if (IsKeyPressed(KEY_P))
		command |= ACTION_JUMP;
	if (IsKeyPressed(KEY_L))
		command |= ACTION_CROUCH;
	if (IsKeyPressed(KEY_O))
		command |= ACTION_PICKUP;
	if (IsKeyDown(KEY_K))
		command |= ACTION_RUN;

	// Aim (numpad 8/6/2/4) here as a placeholder to simulate right tumbstick
	if (IsKeyDown(KEY_KP_8) || IsKeyDown(KEY_KP_6) || IsKeyDown(KEY_KP_2) || IsKeyDown(KEY_KP_4))
		command |= AIM;

	// Menu / start / exit
	if (IsKeyPressed(KEY_TAB))
		command |= MENU_TOGGLE;

	if (IsKeyPressed(KEY_ENTER))
		command |= START_GAME;

	if (IsKeyPressed(KEY_ESCAPE))
		command |= EXIT_COMMAND;

	if (IsKeyDown(KEY_VOLUME_UP))
		command |= VOLUME_UP;

	if (IsKeyDown(KEY_VOLUME_DOWN))
		command |= VOLUME_DOWN;

	if (IsKeyPressed(KEY_F1))
		command |= POWER_COMMAND;

	if (command != NONE)
	{
		// Debug trace
		char buffer[COMMAND_COUNT + 1]; // COUNT bits + null
		GetCommandBits(command, buffer);

		TraceLog(LOG_INFO, "Combined Command bits: %s", buffer);
	}

	return command; // NONE if nothing was set
}

void ExitInputManager()
{
#ifdef PLATFORM_R36S
	TraceLog(LOG_INFO, "Closing R36S input devices");
	if (event_power_button >= 0)
		close(event_power_button);
	if (event_controller_button >= 0)
		close(event_controller_button);
	if (event_volume_buttons >= 0)
		close(event_volume_buttons);
#else
	TraceLog(LOG_INFO, "Cleaning up standard input manager");
#endif // PLATFORM_R36S
}
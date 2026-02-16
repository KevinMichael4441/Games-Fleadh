//=================================================================
// Command Pattern Implementation
//=================================================================
#ifndef COMMAND_H
#define COMMAND_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

//=================================================================
// Define the Command enum
//=================================================================
typedef enum
{
	NONE 				= 0,		// No command (used to represent idle state)

	// Movement
	MOVE_UP 			= 1 << 0,	// Binary: 000000000000000000000001 Command to move up (Keyboard - W and Up Arrow Key | Xbox and R36S - D-Pad | Xbox and R36S - Left Tumbstick)
	MOVE_DOWN 			= 1 << 1,	// Binary: 000000000000000000000010 Command to move down (Keyboard - S and Down Arrow Key | Xbox and R36S - D-Pad | Xbox and R36S - Left Tumbstick)
	MOVE_LEFT 			= 1 << 2,	// Binary: 000000000000000000000100 Command to move left (Keyboard - A and Left Arrow Key | Xbox and R36S - D-Pad | Left Tumbstick)
	MOVE_RIGHT 			= 1 << 3,	// Binary: 000000000000000000001000 Command to move right (Keyboard - D and Right Arrow Key | Xbox and R36S - D-Pad | Xbox and R36S - Left Tumbstick)

	// Combat
	ATTACK_PRIMARY		= 1 << 4,	// Binary: 000000000000000000010000 Command to perform an attack (e.g., slash with sword) (Keyboard - Keyboard - Space | Xbox - RT | R36S - R1)
	ATTACK_SECONDARY	= 1 << 5,	// Binary: 000000000000000000100000 Command to perform an special attack (e.g. throw stone) (Keyboard - CTRL | Xbox - RB | R36S - R2)

	// Abilities / Actions
	ACTION_SPECIAL_1	= 1 << 6,	// Binary: 000000000000000001000000 Command to perform an special action (e.g. sweep kick) (Keyboard - Z | Xbox - LT | R36S - L1)
	ACTION_SPECIAL_2	= 1 << 7,	// Binary: 000000000000000010000000 Command to perform an special action (e.g. defend) (Keyboard - C | Xbox - LB | R36S - L2)
	ACTION_SPECIAL_3	= 1 << 8,	// Binary: 000000000000000100000000 Command to perform an special action (e.g. dash) (Keyboard - 1 | Xbox - Tumbstick Button Left | R36S - Tumbstick Button Left L3)
	ACTION_SPECIAL_4	= 1 << 9,	// Binary: 000000000000001000000000 Command to perform an special action (e.g. defend) (Keyboard - 3 | Xbox - Tumbstick Button Right | R36S - Tumbstick Button Right R3)

	// Face Buttons
	ACTION_JUMP			= 1 << 10,	// Binary: 000000000000010000000000 Command to perform an A Button action (e.g. Jump) (Keyboard - P | Xbox - A | R36S - A)
	ACTION_CROUCH		= 1 << 11,	// Binary: 000000000000100000000000 Command to perform an B Button action (e.g. Crouch) (Keyboard - L | Xbox - B | R36S - B)
	ACTION_PICKUP		= 1 << 12,	// Binary: 000000000001000000000000 Command to perform an X Button action (e.g. Pickup) (Keyboard - O | Xbox - X | R36S - X)
	ACTION_RUN 			= 1 << 13,	// Binary: 000000000010000000000000 Command to perform an Y Button action (e.g. Run) (Keyboard - K | Xbox - Y | R36S - Y)

	// Aiming
	AIM_UP 				= 1 << 14,	// Binary: 000000000100000000000000 Command to perform an Aim action (e.g. aim retinal) (Keyboard - NUM Pad[AIM_UP -> NUM 8,AIM_RIGHT -> NUM 6, AIM_DOWN -> NUM 2, AIM_LEFT -> NUM 4] | Xbox - Tumbstick Right | R36S - Tumbstick Right)
	AIM_DOWN			= 1 << 15,	// Binary: 000000001000000000000000 Command to perform an Aim action (e.g. aim retinal) (Keyboard - NUM Pad[AIM_UP -> NUM 8,AIM_RIGHT -> NUM 6, AIM_DOWN -> NUM 2, AIM_LEFT -> NUM 4] | Xbox - Tumbstick Right | R36S - Tumbstick Right)
	AIM_LEFT			= 1 << 16,	// Binary: 000000010000000000000000 Command to perform an Aim action (e.g. aim retinal) (Keyboard - NUM Pad[AIM_UP -> NUM 8,AIM_RIGHT -> NUM 6, AIM_DOWN -> NUM 2, AIM_LEFT -> NUM 4] | Xbox - Tumbstick Right | R36S - Tumbstick Right)
	AIM_RIGHT			= 1 << 17,	// Binary: 000000100000000000000000 Command to perform an Aim action (e.g. aim retinal) (Keyboard - NUM Pad[AIM_UP -> NUM 8,AIM_RIGHT -> NUM 6, AIM_DOWN -> NUM 2, AIM_LEFT -> NUM 4] | Xbox - Tumbstick Right | R36S - Tumbstick Right)


	// System (non - gameplay)
	VOLUME_UP			= 1 << 18,	// Binary: 000001000000000000000000 Command to perform an Volume Up (Keyboard - + | Xbox no Input | R36S - Volume Up) (Note: non-gameplay)
	VOLUME_DOWN			= 1 << 19,	// Binary: 000010000000000000000000 Command to perform an Volume Down (Keyboard - - | Xbox no Input | R36S - Volume Down) (Note: non-gameplay)
	MENU_TOGGLE			= 1 << 20,	// Binary: 000100000000000000000000 Menu Toggle (Keyboard - Tab | Controller - Back | Xbox - Back | R36S - Select) (Note: non-gameplay)
	START_GAME			= 1 << 21,	// Binary: 001000000000000000000000 Start Game (Keyboard - Enter | Xbox - Start | R36S - Start) (Note: non-gameplay)
	EXIT_COMMAND		= 1 << 22,	// Binary: 010000000000000000000000 Exit Game (Keyboard - Escape | Xbox no Input | R36S - FN) (Note: non-gameplay)
	POWER_COMMAND		= 1 << 23,	// Binary: 100000000000000000000000 Pause Game (Save Gamestate) (Keyboard no Input | Xbox no Input | R36S - Power button Press once to save, press again back to game) (Note: non-gameplay)

	COMMAND_COUNT 		= 24 // Total number of commands, useful for looping or limits
} Command;

//=================================================================
// Function to show active command bits (for debugging)
//=================================================================
void GetCommandBits(Command cmd, char *buffer);

//=================================================================
// Check if a specific command is active
//=================================================================
bool IsCommandActive(Command command, Command filter);


#ifdef __cplusplus
}
#endif

#endif // COMMAND_H
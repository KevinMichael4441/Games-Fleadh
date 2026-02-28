//=================================================================
// Game Input
//
// The game-layer wrapper around the platform input manager.
// Sits between raw hardware events and game logic.
//
// Responsibilities:
//	- Owns the full input lifecycle (wraps InitInputManager /
//	  ExitInputManager so callers never touch input_manager directly)
//	- Polls hardware once per frame via GameInputPoll
//	- Handles R36S volume button changes with edge detection
//	- Handles telemetry toggle (R36S / Linux)
//	- Builds the per-frame human-readable debug message
//	- Draws the controller overlay
//
// NOT responsible for:
//	- Reading /dev/input or gamepad hardware (that is input_manager)
//	- Game logic or scene management
//
//	Usage pattern:
//		Init()		-> GameInputInit()                  // after InitWindow
//		GameLoop()  -> GameInputPoll(&game_data)        // top of every frame
//		Draw()      -> GameInputDrawControllerOverlay()
//		Exit()      -> GameInputExit()
//=================================================================
#ifndef GAME_INPUT_H
#define GAME_INPUT_H

#include "core/command.h" // Command typedef
#include "game.h"		  // GameData

//=================================================================
// C linkage (C++ compiler flag)
//=================================================================
#ifdef __cplusplus
extern "C"
{
#endif

	//=================================================================
	// GameInputInit
	// Initialises the hardware input manager and game input layer.
	// Call once in Init() after InitWindow().
	//=================================================================
	void GameInputInit(void);

	//=================================================================
	// GameInputPoll
	// Poll hardware, write Command into game_data->command,
	// run volume / telemetry edge detection, build debug message.
	// Call once at the top of each frame (Input step).
	//=================================================================
	void GameInputPoll(GameData *game_data);

	//=================================================================
	// GameInputGetMessage
	// Returns the human-readable command string built by the most
	// recent GameInputPoll call.  Valid until the next poll.
	//=================================================================
	const char *GameInputGetMessage(void);

	//=================================================================
	// GameInputDrawControllerOverlay
	// Draws the R36S / Xbox button overlay using the last polled
	// command.  Call inside BeginDrawing / EndDrawing.
	//=================================================================
	void GameInputDrawControllerOverlay(void);

	//=================================================================
	// GameInputExit
	// Closes hardware devices and frees input resources.
	// Call once in Exit().
	//=================================================================
	void GameInputExit(void);

#ifdef __cplusplus
}
#endif
#endif // GAME_INPUT_H
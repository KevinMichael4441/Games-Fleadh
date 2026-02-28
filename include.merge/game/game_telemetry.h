#ifndef GAME_TELEMETRY_H
#define GAME_TELEMETRY_H

#include <stdbool.h>

//=================================================================
// C linkage (C++ compiler flag)
//=================================================================
#ifdef __cplusplus
extern "C"
{
#endif

	//=================================================================
	// Game Telemetry
	// Debug overlay showing FPS, frame time, GPU info.
	// R36S and Linux only - safely compiled out on other platforms.
	//
	// HOW TO USE IN YOUR PROJECT:
	//   1. Call GameTelemetryInit()   after InitWindow()
	//   2. Call GameTelemetryUpdate() once per frame in Update()
	//   3. Call GameTelemetryDraw()   inside BeginDrawing/EndDrawing
	//   4. Call GameTelemetryToggle() from a button press to show/hide
	//   5. Call GameTelemetryExit()   in your cleanup
	//=================================================================

	//=================================================================
	// Telemetry
	// Call Init after InitWindow() needs OpenGL context to be ready
	//=================================================================
	void GameTelemetryInit(void);

	//=================================================================
	// Per-frame update
	// Call once in Update() before Draw()
	//=================================================================
	void GameTelemetryUpdate(double now, float frame_time, int fps);

	//=================================================================
	// Draw the telemetry overlay
	// Call inside BeginDrawing / EndDrawing, after your scene
	//=================================================================
	void GameTelemetryDraw(void);

	//=================================================================
	// Toggle the overlay on/off
	// Hook this to whatever button / input e.g. ACTION_SPECIAL_2
	//=================================================================
	void GameTelemetryToggle(void);

	//=================================================================
	// Show / hide directly flag control
	//=================================================================
	void GameTelemetrySetVisible(bool visible);
	bool GameTelemetryIsVisible(void);

	//=================================================================
	// GameTelemetryExit
	// Call in Exit()
	//=================================================================
	void GameTelemetryExit(void);

#ifdef __cplusplus
}
#endif

#endif // GAME_TELEMETRY_H
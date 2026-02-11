#ifndef GAME_H
#define GAME_H

#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>
#include "constants.h"
#include "math.h"
#include "rlgl.h"

#include "gamestates.hpp"
#include "ooze.hpp"



#if defined(PLATFORM_R36S) || defined(PLATFORM_LINUX)
#include "telemetry.h"
#endif // Included for R36S and Linux only

#if defined(PLATFORM_R36S) || defined(PLATFORM_LINUX)
#if defined(GRAPHICS_API_OPENGL_ES3)
#include <GLES3/gl3.h> // GRAPHICS_API_OPENGL_ES3
#elif defined(GRAPHICS_API_OPENGL_ES2)
#include <GLES2/gl2.h> // GRAPHICS_API_OPENGL_ES2
#else
#include <GL/gl.h>
#endif // Default
#endif // R36S and Linux only



#if defined(PLATFORM_R36S) || defined(PLATFORM_LINUX)
	// Telemetry
	static Telemetry r36s_telemetry;
	static const char *glRendererStr = NULL;
	static const char *glVersionStr = NULL;
	static const char *glslVersionStr = NULL;
	static bool show_telemetry = true;
	static bool action_special_2_was_pressed = false;
#endif


class Game
{
public:
	Game();
	void Run();

private:

	void InitGame();
	void Update(float t_dt);
	void Draw();

	GameState gamestate;
	Ooze ooze;
};

#endif //game.h
#ifndef GAME_H
#define GAME_H

#include <stdio.h>
#include <stdlib.h>

#include <raylib.h>
#include <raymath.h>
#include "rlgl.h"

#include "cute_c2.h"
#include "math.h"

#include "constants.h"
#include "level_loader.h"
#include "cJSON.h"

#include "ui_manager.hpp"
#include "gamestates.hpp"

#include "command.h"
#include "input_manager.h"
#include "game_sound.h"

#include "camera_manager.hpp"
#include "ooze.hpp"
#include "supermech.h"

#include "collectibles.h"
#include "jump_pad.h"
#include "teleporter.h"
#include "laserdoor.h"
#include "security_system.h"

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
	static bool show_telemetry = false;
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
	void NonGameInputs();
	void Draw();
	
	void checkMechOozeCollision();
	void Respawn();

	UI_Manager ui_manager;
	GameState gamestate;
	Command m_activeCommand;
	int score;

	CameraManager camera;
	Ooze ooze;
	SuperMech mech;
	float mechRespawnCooldown = 0.0f;

	Collectibles_Manager m_collectibles_manager;
	JumpPad_Manager m_jumpPadd_manager;
	Teleporter_Manager m_teleporter_manager;
	LaserDoor_Manager m_laseDoor_manager;
	SecuritySystem m_securitySystem;

	LevelData m_level{};
	Texture2D temp_background;
};

#endif //game.h
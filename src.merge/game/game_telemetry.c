//=================================================================
// Game Telemetry
// Debug overlay: FPS, frame time, GPU renderer, OpenGL version.
// Compiled only on R36S and Linux - all other platforms get
// empty stubs so your code compiles everywhere unchanged.
//=================================================================
#include "game/game_telemetry.h"

//=================================================================
// R36S and Linux only - everything below is guarded
//=================================================================
#if defined(PLATFORM_R36S) || defined(PLATFORM_LINUX)

#include <stdbool.h>
#include <stddef.h>
#include <raylib.h>
#include "core/telemetry.h"

//---------------------------------------------------------------
// OpenGL headers (platform specific)
//---------------------------------------------------------------
#if defined(GRAPHICS_API_OPENGL_ES3)
#include <GLES3/gl3.h>
#elif defined(GRAPHICS_API_OPENGL_ES2)
#include <GLES2/gl2.h>
#else
#include <GL/gl.h>
#endif

//=================================================================
// Private state
//=================================================================
static Telemetry telemetry;
static const char *gl_renderer = NULL;
static const char *gl_version = NULL;
static const char *glsl_version = NULL;
static bool is_visible = false;

//=================================================================
// Lifecycle
//=================================================================
void GameTelemetryInit(void)
{
	// Must be called after InitWindow() - needs active OpenGL context
	gl_renderer = (const char *)glGetString(GL_RENDERER);
	gl_version = (const char *)glGetString(GL_VERSION);
	glsl_version = (const char *)glGetString(GL_SHADING_LANGUAGE_VERSION);
	InitTelemetry(&telemetry);

	TraceLog(LOG_INFO, "GAME_TELEMETRY: Initialised | %s | %s | %s",
			 gl_renderer, gl_version, glsl_version);
}

void GameTelemetryExit(void)
{
	// Nothing to free currently
}

//=================================================================
// Per-frame update
//=================================================================
void GameTelemetryUpdate(double now, float frame_time, int fps)
{
	UpdateTelemetryFrame(&telemetry, frame_time, now, fps);
	UpdateTelemetry(&telemetry, now);
}

//=================================================================
// Draw overlay
//=================================================================
void GameTelemetryDraw(void)
{
	if (!is_visible)
		return;

	DrawTelemetry(&telemetry, 8, 8, gl_renderer, gl_version, glsl_version);
}

//=================================================================
// Visibility controls
//=================================================================
void GameTelemetryToggle(void)
{
	is_visible = !is_visible;
	TraceLog(LOG_INFO, "GAME_TELEMETRY: %s", is_visible ? "ON" : "OFF");
}

void GameTelemetrySetVisible(bool visible)
{
	is_visible = visible;
}

bool GameTelemetryIsVisible(void)
{
	return is_visible;
}

//=================================================================
// All other platforms: empty stubs
// Call GameTelemetry* freely on Windows / Web
// without wrapping every call in #if defined blocks
//=================================================================
#else

void GameTelemetryInit(void) {}
void GameTelemetryExit(void) {}
void GameTelemetryUpdate(double now, float frame_time, int fps)
{
	(void)now;
	(void)frame_time;
	(void)fps; // suppress unused warnings
}
void GameTelemetryDraw(void) {}
void GameTelemetryToggle(void) {}
void GameTelemetrySetVisible(bool visible) { (void)visible; }
bool GameTelemetryIsVisible(void) { return false; }

#endif // PLATFORM_R36S || PLATFORM_LINUX
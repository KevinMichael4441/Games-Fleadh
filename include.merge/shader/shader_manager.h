#ifndef SHADER_MANAGER_H
#define SHADER_MANAGER_H

#include <raylib.h>
#include <stdbool.h>

#include "constants.h"

//=================================================================
// C linkage (C++ compiler flag)
//=================================================================
#ifdef __cplusplus
extern "C"
{
#endif

//=================================================================
// Compile-time Limits
// Override in constants.h before including this header
// e.g
// #include "constants.h"
// #include "shader_manager.h"
//=================================================================
#ifndef MAX_MANAGED_SHADERS
#define MAX_MANAGED_SHADERS 16
#endif

// Upper bound: each slot costs a registry entry (8 bytes) and a
// glGetUniformLocation call per frame. 64 is already excessive
// for r36s, if more are required, reconsider design.
#define MAX_MANAGED_SHADERS_LIMIT 64

	_Static_assert(
		MAX_MANAGED_SHADERS <= MAX_MANAGED_SHADERS_LIMIT,
		"MAX_MANAGED_SHADERS exceeds MAX_MANAGED_SHADERS_LIMIT (64). "
		"Reduce MAX_MANAGED_SHADERS in constants.h or raise MAX_MANAGED_SHADERS_LIMIT "
		"in shader_manager.h if you are certain you need this many shaders.");

//=================================================================
// Match in fragment shader code for auto-pushed uniforms
//=================================================================
#define SHADER_UNIFORM_TIME "time"					// float    : seconds, auto-pushed each frame
#define SHADER_UNIFORM_RESOLUTION "resolution"		// vec2     : render target size in pixels
#define SHADER_UNIFORM_INTENSITY "intensity"		// float    : effect strength 0.0 -> 1.0
#define SHADER_UNIFORM_TINT "tintColor"				// vec4     : rgba tint / glow colour
#define SHADER_UNIFORM_PIXEL_SIZE "pixelSize"		// vec2     : pixelate cell size in pixels
#define SHADER_UNIFORM_OUTLINE "outlineColor"		// vec4     : outline rgba
#define SHADER_UNIFORM_DISSOLVE "dissolveThreshold" // float    : 0.0 visible, 1.0 fully dissolved
#define SHADER_UNIFORM_CENTER "center"				// vec2     : focal point UV (0.0-1.0)
#define SHADER_UNIFORM_DEBUG "debugMode"			// int      : 0 normal, 1 visualise internals
#define SHADER_UNIFORM_DEBUG_COLOR "debugColor"		// vec4     : debug visualisation colour

	//=================================================================
	// ShaderParams
	// Pass to ShaderManagerSetParams()
	// Note only populated fields are pushed.
	// Uniforms that do not exist in a shader are silently skipped.
	//
	// Field names match GLSL uniform names exactly.
	// Struct order matches uniform define order above.
	//
	// Note: field order matters in C++ designated initialisers
	//   ShaderManagerSetParams(shader, (ShaderParams){
	//       .resolution       = (Vector2){ SCREEN_WIDTH, SCREEN_HEIGHT },
	//       .intensity        = 0.8f,
	//       .tintColor        = ORANGE,
	//       .pixelSize        = (Vector2){ 4.0f, 4.0f },
	//       .outlineColor     = YELLOW,
	//       .dissolveThreshold = 0.5f,
	//       .center           = (Vector2){ 0.5f, 0.5f },
	//       .debugMode        = false,
	//       .debugColor       = WHITE
	//   });
	//=================================================================
	typedef struct
	{
		Vector2 resolution;		 // render target size			(SHADER_UNIFORM_RESOLUTION)
		float intensity;		 // effect strength				(SHADER_UNIFORM_INTENSITY)
		Color tintColor;		 // tint / glow colour			(SHADER_UNIFORM_TINT)
		Vector2 pixelSize;		 // pixelate cell size			(SHADER_UNIFORM_PIXEL_SIZE)
		Color outlineColor;		 // outline colour				(SHADER_UNIFORM_OUTLINE)
		float dissolveThreshold; // dissolve threshold			(SHADER_UNIFORM_DISSOLVE)
		Vector2 center;			 // focal point UV				(SHADER_UNIFORM_CENTER)
		bool debugMode;			 // visualise internals			(SHADER_UNIFORM_DEBUG)
		Color debugColor;		 // debug colour, (WHITE)		(SHADER_UNIFORM_DEBUG_COLOR)
	} ShaderParams;

	//=================================================================
	// Shader Manager Stats (debugging / monitoring)
	//=================================================================
	typedef struct
	{
		int shaders_loaded; // Shaders currently tracked by the manager
		int max_shaders;	// MAX_MANAGED_SHADERS capacity
		float time_elapsed; // Internal time counter (seconds)
	} ShaderManagerStats;

	//=================================================================
	// Init Shader Manager
	//=================================================================
	void InitShaderManager(void);

	//=================================================================
	// Load a Fragment Shader
	// Raylibs built-in vertex passthrough is used automatically.
	// Returns the Shader handle - store it in main.
	// Returns a zeroed Shader on failure (check .id == 0).
	//=================================================================
	Shader ShaderManagerLoad(const char *fs_path);

	//=================================================================
	// Unload a Shader (free GPU memory, remove from manager)
	//=================================================================
	void ShaderManagerUnload(Shader shader);

	//=================================================================
	// Set Shader Parameters
	// Pushes only the uniforms that exist in the shader.
	// Safe to call every frame.
	// Make sure shader is not compute heavy review telemetry
	// debugColor defaults to WHITE if not set.
	//=================================================================
	void ShaderManagerSetParams(Shader shader, ShaderParams params);

	//=================================================================
	// Begin / End Shader Mode
	// Wrap DrawTexture / DrawText etc. calls between these.
	//=================================================================
	void ShaderManagerBegin(Shader shader);
	void ShaderManagerEnd(void);

	//=================================================================
	// Update Shader Manager (call once per frame, before drawing)
	// Pushes current time to SHADER_UNIFORM_TIME on all loaded shaders.
	//=================================================================
	void ShaderManagerUpdate(void);

	//=================================================================
	// Get Shader Manager Stats
	//=================================================================
	ShaderManagerStats ShaderManagerGetStats(void);

	//=================================================================
	// Cleanup Shader Manager
	//=================================================================
	void ExitShaderManager(void);

#ifdef __cplusplus
}
#endif

#endif // SHADER_MANAGER_H
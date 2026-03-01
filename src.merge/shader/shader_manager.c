#include <raylib.h>
#include <string.h>

#include "shader/shader_manager.h"

//=================================================================
// Internal Registry Entry
// Tracks each shader loaded through the manager so UpdateShaderManager
// can push the time uniform without the caller needing to do it.
//=================================================================
typedef struct
{
	unsigned int shader_id; // 0 = empty slot
	int time_loc;			// cached location of "time" uniform (-1 if not present)
} ManagedShaderEntry;

//=================================================================
// Shader Manager Internal State
//=================================================================
static ManagedShaderEntry registry[MAX_MANAGED_SHADERS];
static int shaders_loaded = 0;
static float time_elapsed = 0.0f;

//=================================================================
// Internal: Find registry slot by shader id
//=================================================================
static int FindSlot(unsigned int shader_id)
{
	for (int i = 0; i < MAX_MANAGED_SHADERS; i++)
	{
		if (registry[i].shader_id == shader_id)
			return i;
	}
	return -1;
}

//=================================================================
// Internal: Find a free registry slot
//=================================================================
static int FindFreeSlot(void)
{
	for (int i = 0; i < MAX_MANAGED_SHADERS; i++)
	{
		if (registry[i].shader_id == 0)
			return i;
	}
	return -1;
}

//=================================================================
// Internal: Push a vec4 colour uniform
//=================================================================
static void PushColor(Shader shader, int loc, Color color)
{
	if (loc == -1)
		return;
	float c[4] = {
		color.r / 255.0f,
		color.g / 255.0f,
		color.b / 255.0f,
		color.a / 255.0f};
	SetShaderValue(shader, loc, c, SHADER_UNIFORM_VEC4);
}

//=================================================================
// Init Shader Manager
//=================================================================
void InitShaderManager(void)
{
	TraceLog(LOG_INFO, "SHADER_MANAGER: Initialising");

	for (int i = 0; i < MAX_MANAGED_SHADERS; i++)
	{
		registry[i].shader_id = 0;
		registry[i].time_loc = -1;
	}

	shaders_loaded = 0;
	time_elapsed = 0.0f;

#ifdef PLATFORM_R36S
	TraceLog(LOG_INFO, "SHADER_MANAGER: Configured for R36S (GLSL 100 / Mali GPU)");
	TraceLog(LOG_INFO, "SHADER_MANAGER: Fragment shaders only with built-in vertex passthrough");
#else
	TraceLog(LOG_INFO, "SHADER_MANAGER: Configured for standard platform");
#endif

	TraceLog(LOG_INFO, "SHADER_MANAGER: Capacity: %d shader slots", MAX_MANAGED_SHADERS);
}

//=================================================================
// Load a Fragment Shader
//=================================================================
Shader ShaderManagerLoad(const char *fs_path)
{
	Shader empty = {0};

	if (!fs_path)
	{
		TraceLog(LOG_WARNING, "SHADER_MANAGER: ShaderManagerLoad: NULL fs_path");
		return empty;
	}

	if (shaders_loaded >= MAX_MANAGED_SHADERS)
	{
		TraceLog(LOG_ERROR, "SHADER_MANAGER: ShaderManagerLoad: MAX_MANAGED_SHADERS (%d) reached",
				 MAX_MANAGED_SHADERS);
		return empty;
	}

	int slot = FindFreeSlot();
	if (slot == -1)
	{
		TraceLog(LOG_ERROR, "SHADER_MANAGER: ShaderManagerLoad: No free slot");
		return empty;
	}

	// NULL vertex path = Raylib's built-in passthrough
	Shader shader = LoadShader(NULL, fs_path);

	if (shader.id == 0)
	{
		TraceLog(LOG_ERROR, "SHADER_MANAGER: Failed to compile: %s", fs_path);
		return empty;
	}

	registry[slot].shader_id = shader.id;
	registry[slot].time_loc = GetShaderLocation(shader, SHADER_UNIFORM_TIME);
	shaders_loaded++;

	// Push WHITE to debugColor immediately on load
	// GPU uniforms default to vec4(0,0,0,0) (black) until first push
	// This ensures debug mode shows WHITE before any ShaderManagerSetParams call
	{
		int debug_color_loc = GetShaderLocation(shader, SHADER_UNIFORM_DEBUG_COLOR);
		if (debug_color_loc != -1)
			PushColor(shader, debug_color_loc, WHITE);
	}

	TraceLog(LOG_INFO, "SHADER_MANAGER: Loaded [id=%d] %s (%d/%d)",
			 shader.id, fs_path, shaders_loaded, MAX_MANAGED_SHADERS);

	return shader;
}

//=================================================================
// Unload a Shader
//=================================================================
void ShaderManagerUnload(Shader shader)
{
	if (shader.id == 0)
		return;

	int slot = FindSlot(shader.id);
	if (slot == -1)
	{
		TraceLog(LOG_WARNING, "SHADER_MANAGER: ShaderManagerUnload: Shader [id=%d] not in registry",
				 shader.id);
		return;
	}

	UnloadShader(shader);
	registry[slot].shader_id = 0;
	registry[slot].time_loc = -1;
	shaders_loaded--;

	TraceLog(LOG_INFO, "SHADER_MANAGER: Unloaded [id=%d] (%d/%d remaining)",
			 shader.id, shaders_loaded, MAX_MANAGED_SHADERS);
}

//=================================================================
// Set Shader Parameters
// Looks up each uniform and pushes it if present in the shader.
// Locations are retrieved via Raylib, Raylib internally uses
// glGetUniformLocation which is fast for the small uniform counts
// used by these shaders.
//=================================================================
void ShaderManagerSetParams(Shader shader, ShaderParams params)
{
	if (shader.id == 0)
		return;

	// intensity
	{
		int loc = GetShaderLocation(shader, SHADER_UNIFORM_INTENSITY);
		if (loc != -1)
		{
			float v = params.intensity;
			if (v < 0.0f)
				v = 0.0f;
			if (v > 1.0f)
				v = 1.0f;
			SetShaderValue(shader, loc, &v, SHADER_UNIFORM_FLOAT);
		}
	}

	// tint colour
	{
		int loc = GetShaderLocation(shader, SHADER_UNIFORM_TINT);
		PushColor(shader, loc, params.tintColor);
	}

	// outline colour
	{
		int loc = GetShaderLocation(shader, SHADER_UNIFORM_OUTLINE);
		PushColor(shader, loc, params.outlineColor);
	}

	// dissolve threshold
	{
		int loc = GetShaderLocation(shader, SHADER_UNIFORM_DISSOLVE);
		if (loc != -1)
		{
			float v = params.dissolveThreshold;
			if (v < 0.0f)
				v = 0.0f;
			if (v > 1.0f)
				v = 1.0f;
			SetShaderValue(shader, loc, &v, SHADER_UNIFORM_FLOAT);
		}
	}

	// resolution
	{
		int loc = GetShaderLocation(shader, SHADER_UNIFORM_RESOLUTION);
		if (loc != -1)
		{
			float res[2] = {params.resolution.x, params.resolution.y};
			SetShaderValue(shader, loc, res, SHADER_UNIFORM_VEC2);
		}
	}

	// pixel size
	{
		int loc = GetShaderLocation(shader, SHADER_UNIFORM_PIXEL_SIZE);
		if (loc != -1)
		{
			float ps[2] = {params.pixelSize.x, params.pixelSize.y};
			SetShaderValue(shader, loc, ps, SHADER_UNIFORM_VEC2);
		}
	}

	// center
	{
		int loc = GetShaderLocation(shader, SHADER_UNIFORM_CENTER);
		if (loc != -1)
		{
			float c[2] = {params.center.x, params.center.y};
			SetShaderValue(shader, loc, c, SHADER_UNIFORM_VEC2);
		}
	}

	// debug mode
	{
		int loc = GetShaderLocation(shader, SHADER_UNIFORM_DEBUG);
		if (loc != -1)
		{
			int v = params.debugMode ? 1 : 0;
			SetShaderValue(shader, loc, &v, SHADER_UNIFORM_INT);
		}
	}

	// debug colour: default WHITE if not set
	{
		int loc = GetShaderLocation(shader, SHADER_UNIFORM_DEBUG_COLOR);
		if (loc != -1)
		{
			Color dc = params.debugColor;
			if (dc.r == 0 && dc.g == 0 && dc.b == 0 && dc.a == 0)
				dc = WHITE;
			PushColor(shader, loc, dc);
		}
	}
}

//=================================================================
// Begin Shader Mode
//=================================================================
void ShaderManagerBegin(Shader shader)
{
	if (shader.id == 0)
	{
		TraceLog(LOG_WARNING, "SHADER_MANAGER: ShaderManagerBegin: Invalid shader (id=0)");
		return;
	}
	BeginShaderMode(shader);
}

//=================================================================
// End Shader Mode
//=================================================================
void ShaderManagerEnd(void)
{
	EndShaderMode();
}

//=================================================================
// Update Shader Manager (call once per frame)
// Pushes time uniform to every registered shader.
//=================================================================
void ShaderManagerUpdate(void)
{
	time_elapsed = (float)GetTime();

	for (int i = 0; i < MAX_MANAGED_SHADERS; i++)
	{
		if (registry[i].shader_id != 0 && registry[i].time_loc != -1)
		{
			// Build a temporary Shader handle to call SetShaderValue
			Shader s = {0};
			s.id = registry[i].shader_id;
			SetShaderValue(s, registry[i].time_loc, &time_elapsed, SHADER_UNIFORM_FLOAT);
		}
	}
}

//=================================================================
// Get Shader Manager Stats
//=================================================================
ShaderManagerStats ShaderManagerGetStats(void)
{
	ShaderManagerStats stats = {
		.shaders_loaded = shaders_loaded,
		.max_shaders = MAX_MANAGED_SHADERS,
		.time_elapsed = time_elapsed};
	return stats;
}

//=================================================================
// Cleanup Shader Manager
//=================================================================
void ExitShaderManager(void)
{
	TraceLog(LOG_INFO, "SHADER_MANAGER: Shutting down (%d shaders still loaded)", shaders_loaded);

	// Warn about any shaders not explicitly unloaded by the caller
	for (int i = 0; i < MAX_MANAGED_SHADERS; i++)
	{
		if (registry[i].shader_id != 0)
		{
			TraceLog(LOG_WARNING, "SHADER_MANAGER: Shader [id=%d] was not explicitly unloaded",
					 registry[i].shader_id);
			registry[i].shader_id = 0;
			registry[i].time_loc = -1;
		}
	}

	shaders_loaded = 0;

	TraceLog(LOG_INFO, "SHADER_MANAGER: Shutdown complete");
}
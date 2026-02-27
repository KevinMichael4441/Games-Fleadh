//=================================================================
// Game Shader
// Owns shader manager lifecycle, shader loading, render texture
// allocation, and initial parameter configuration.
//
// game_shader hides the boilerplate of initialising and shutting
// down the shader system. All shader handles and render textures
// are stored in GameData so Draw() can reference them directly.
// Per-frame parameter updates driven by game state (e.g. damage
// flash, spotlight scan) remain in main.
//=================================================================
#ifndef GAME_SHADER_H
#define GAME_SHADER_H

#include <raylib.h>
#include "shader/shader_manager.h"

//=================================================================
// C linkage (C++ compiler flag)
//=================================================================
#ifdef __cplusplus
extern "C"
{
#endif

//=================================================================
// Typedef aliases, not new structs
//=================================================================

// Shader parameters (aliased from ShaderParams)
typedef ShaderParams GameShaderParams;

// Shader statistics (aliased from ShaderManagerStats)
typedef ShaderManagerStats GameShaderStats;

//=================================================================
// GameShaderInit
// Call in Init() after InitWindow().
// Initialises the shader manager, loads all shaders and render
// textures into game_data, and sets initial shader parameters.
//=================================================================
void GameShaderInit(GameData *game_data);

//=================================================================
// GameShaderUpdate
// Call every frame in Update() to advance shader time uniforms.
// Per-frame parameter updates driven by game state (damage_flash,
// spotlight position etc.) are set directly in Update().
//=================================================================
void GameShaderUpdate(void);

//=================================================================
// GameShaderExit
// Call in Exit() before CloseWindow().
// Unloads all shaders and render textures, shuts down the manager.
//=================================================================
void GameShaderExit(GameData *game_data);

//=================================================================
// GameShaderGetStats
// Returns shader statistics for debugging/display.
//
// Example:
//	GameShaderStats stats = GameShaderGetStats();
//	DrawText(TextFormat("Shaders: %d/%d", stats.shaders_loaded,
//		stats.max_shaders), 10, 10, 20, WHITE);
//=================================================================
GameShaderStats GameShaderGetStats(void);

//=================================================================
// GameShaderSetParams
// Set shader parameters (intensity, colors, etc.)
// Safe to call every frame.
//
// Example:
//	GameShaderSetParams(game_data->slime, (GameShaderParams){
//			.intensity = 0.5f,
//			.tintColor = GREEN
//	});
//=================================================================
void GameShaderSetParams(Shader shader, GameShaderParams params);

//=================================================================
// GameShaderBegin / GameShaderEnd
// Wrap drawing calls to apply shader effect.
//
// Example:
//	GameShaderBegin(game_data->slime);
//	DrawTexture(game_data->organism_texture, 0, 0, WHITE);
//	GameShaderEnd();
//=================================================================
void GameShaderBegin(Shader shader);
void GameShaderEnd(void);

#ifdef __cplusplus
}
#endif

#endif // GAME_SHADER_H
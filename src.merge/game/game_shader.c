//=================================================================
// Game Shader
// Owns shader manager lifecycle, shader loading, render texture
// allocation, and initial parameter configuration.
//
// Note: Read game_shader.h to understand what this module does.
//       Roam from here when you want to understand game_shader.
//
// Per-frame parameter updates driven by game state live in main:
//   - palette_tint intensity driven by game_data.damage_flash
//   - spotlight center.y driven by sinf(game_data.now)
//=================================================================

#include <raylib.h>

#include "game.h"
#include "game/game_shader.h"
#include "shader/shader_manager.h"
#include "constants.h"

//=================================================================
// Shader asset paths (game-specific configuration)
//=================================================================

//-------------------------------------------------------------
// Visual Effect Shaders
//-------------------------------------------------------------
#define SHADER_PATH_ARC "assets/shaders/arc.fs"
#define SHADER_PATH_BEACON "assets/shaders/beacon.fs"
#define SHADER_PATH_FIREBALL "assets/shaders/fireball.fs"
#define SHADER_PATH_SPARKS "assets/shaders/sparks.fs"
#define SHADER_PATH_DANGER_STROBE "assets/shaders/danger_strobe.fs"

//-------------------------------------------------------------
// Post-Processing Shaders
//-------------------------------------------------------------
#define SHADER_PATH_GRAYSCALE "assets/shaders/grayscale.fs"
#define SHADER_PATH_SCANLINES "assets/shaders/scanlines.fs"
#define SHADER_PATH_VIGNETTE "assets/shaders/vignette.fs"
#define SHADER_PATH_PIXELATE "assets/shaders/pixelate.fs"
#define SHADER_PATH_HEAT_HAZE "assets/shaders/heat_haze.fs"

//-------------------------------------------------------------
// Organism/Gameplay Shaders
//-------------------------------------------------------------
#define SHADER_PATH_SLIME "assets/shaders/slime.fs"
#define SHADER_PATH_OUTLINE "assets/shaders/outline.fs"
#define SHADER_PATH_DISSOLVE "assets/shaders/dissolve.fs"
#define SHADER_PATH_EDGE_BURN "assets/shaders/edge_burn.fs"

//-------------------------------------------------------------
// Lighting/Atmosphere Shaders
//-------------------------------------------------------------
#define SHADER_PATH_SPOTLIGHT "assets/shaders/spotlight.fs"
#define SHADER_PATH_PALETTE_TINT "assets/shaders/palette_tint.fs"

//=================================================================
// GameShaderInit
// Loads all shaders, render textures, and sets initial parameters
//=================================================================
void GameShaderInit(GameData *game_data)
{
	if (!game_data)
		return;

	// Initialise Shader Manager
	InitShaderManager();

	//-------------------------------------------------------------
	// Load shaders
	//-------------------------------------------------------------
	game_data->arc = ShaderManagerLoad(SHADER_PATH_ARC);
	game_data->beacon = ShaderManagerLoad(SHADER_PATH_BEACON);
	game_data->danger_strobe = ShaderManagerLoad(SHADER_PATH_DANGER_STROBE);
	game_data->dissolve = ShaderManagerLoad(SHADER_PATH_DISSOLVE);
	game_data->edge_burn = ShaderManagerLoad(SHADER_PATH_EDGE_BURN);
	game_data->fireball = ShaderManagerLoad(SHADER_PATH_FIREBALL);
	game_data->grayscale = ShaderManagerLoad(SHADER_PATH_GRAYSCALE);
	game_data->heat_haze = ShaderManagerLoad(SHADER_PATH_HEAT_HAZE);
	game_data->outline = ShaderManagerLoad(SHADER_PATH_OUTLINE);
	game_data->palette_tint = ShaderManagerLoad(SHADER_PATH_PALETTE_TINT);
	game_data->pixelate = ShaderManagerLoad(SHADER_PATH_PIXELATE);
	game_data->scanlines = ShaderManagerLoad(SHADER_PATH_SCANLINES);
	game_data->slime = ShaderManagerLoad(SHADER_PATH_SLIME);
	game_data->sparks = ShaderManagerLoad(SHADER_PATH_SPARKS);
	game_data->spotlight = ShaderManagerLoad(SHADER_PATH_SPOTLIGHT);
	game_data->vignette = ShaderManagerLoad(SHADER_PATH_VIGNETTE);

	//-------------------------------------------------------------
	// Initial shader parameters
	// See /assets/shaders for parameter details.
	//-------------------------------------------------------------

	ShaderManagerSetParams(game_data->arc, (ShaderParams){
											   .resolution = (Vector2){SCREEN_WIDTH, SCREEN_HEIGHT},
											   .intensity = 1.0f,
											   .tintColor = YELLOW});

	ShaderManagerSetParams(game_data->beacon, (ShaderParams){
												  .resolution = (Vector2){SCREEN_WIDTH, SCREEN_HEIGHT},
												  .intensity = 1.0f,
												  .tintColor = YELLOW});

	ShaderManagerSetParams(game_data->danger_strobe, (ShaderParams){
														 .resolution = (Vector2){SCREEN_WIDTH, SCREEN_HEIGHT},
														 .intensity = 1.0f,
														 .tintColor = RED});

	ShaderManagerSetParams(game_data->dissolve, (ShaderParams){
													.intensity = 0.5f,
													.dissolveThreshold = 0.5f});

	// resolution = background texture size in pixels
	// dissolveThreshold animates 0.0-1.0 to drive the burn effect
	ShaderManagerSetParams(game_data->edge_burn, (ShaderParams){
													 .resolution = (Vector2){SCREEN_WIDTH, SCREEN_HEIGHT},
													 .intensity = 1.0f,
													 .dissolveThreshold = 0.5f});

	// Lifecycle auto-repeats every 4s (grow 0.6s, hold 0.4s, fade 0.5s, pause 2.5s)
	// center UV = top-right area of play area
	ShaderManagerSetParams(game_data->fireball, (ShaderParams){
													.resolution = (Vector2){SCREEN_WIDTH, SCREEN_HEIGHT},
													.intensity = 1.0f,
													.tintColor = ORANGE,
													.center = (Vector2){0.75f, 0.25f}});

	ShaderManagerSetParams(game_data->grayscale, (ShaderParams){
													 .intensity = 1.0f});

	ShaderManagerSetParams(game_data->heat_haze, (ShaderParams){
													 .resolution = (Vector2){SCREEN_WIDTH, SCREEN_HEIGHT},
													 .intensity = 1.0f});

	ShaderManagerSetParams(game_data->outline, (ShaderParams){
												   .intensity = 1.0f,
												   .outlineColor = RED});

	ShaderManagerSetParams(game_data->palette_tint, (ShaderParams){
														.intensity = 1.0f,
														.tintColor = RED});

	ShaderManagerSetParams(game_data->pixelate, (ShaderParams){
													.intensity = 1.0f});

	ShaderManagerSetParams(game_data->scanlines, (ShaderParams){
													 .resolution = (Vector2){SCREEN_WIDTH, SCREEN_HEIGHT},
													 .intensity = 1.0f});

	// resolution = screen size - organisms move freely across the full screen
	// tintColor  = mid-green, lighter than the drawn cell body so the shader's
	// Voronoi egg dots (near-black) and membrane rings (bright)
	// have contrast to show against. Too dark = pattern invisible.
	ShaderManagerSetParams(game_data->slime, (ShaderParams){
												 .resolution = (Vector2){SCREEN_WIDTH, SCREEN_HEIGHT},
												 .intensity = 0.85f,
												 .tintColor = (Color){70, 130, 52, 255}});

	ShaderManagerSetParams(game_data->sparks, (ShaderParams){
												  .resolution = (Vector2){SCREEN_WIDTH, SCREEN_HEIGHT},
												  .intensity = 1.0f,
												  .tintColor = ORANGE});

	// dissolveThreshold repurposed as ambient darkness (0.05 = near black outside cone)
	ShaderManagerSetParams(game_data->spotlight, (ShaderParams){
													 .resolution = (Vector2){SCREEN_WIDTH, SCREEN_HEIGHT},
													 .intensity = 1.0f,
													 .tintColor = (Color){220, 210, 255, 255},
													 .dissolveThreshold = 0.05f,
													 .center = (Vector2){0.92f, 0.50f}});

	ShaderManagerSetParams(game_data->vignette, (ShaderParams){
													.intensity = 0.5f});

	//-------------------------------------------------------------
	// Load render textures
	// Screen-sized for full-scene shaders, entity-sized for sprites
	//-------------------------------------------------------------
	game_data->arc_render_texture = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);
	game_data->beacon_render_texture = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);
	game_data->danger_strobe_render_texture = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);
	game_data->dissolve_render_texture = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);
	game_data->edge_burn_render_texture = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);
	game_data->fireball_render_texture = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);
	game_data->grayscale_render_texture = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);
	game_data->heat_haze_render_texture = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);
	game_data->outline_render_texture = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);
	game_data->palette_tint_render_texture = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);
	game_data->pixelate_render_texture = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);
	game_data->scanlines_render_texture = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);
	game_data->slime_render_texture = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);
	game_data->sparks_render_texture = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);
	game_data->spotlight_render_texture = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);
	game_data->vignette_render_texture = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);

	TraceLog(LOG_INFO, "GAME_SHADER: Initialised");
}

//=================================================================
// GameShaderUpdate
// Updates shader time uniforms every frame
//=================================================================
void GameShaderUpdate(void)
{
	ShaderManagerUpdate();
}

//=================================================================
// GameShaderGetStats
// Returns shader statistics
// GameShaderStats is typedef alias for ShaderManagerStats (no conversion needed)
//=================================================================
GameShaderStats GameShaderGetStats(void)
{
	return ShaderManagerGetStats();
}

//=================================================================
// GameShaderSetParams
// Set shader parameters
// GameShaderParams is typedef alias for ShaderParams (no conversion needed)
//=================================================================
void GameShaderSetParams(Shader shader, GameShaderParams params)
{
	ShaderManagerSetParams(shader, params);
}

//=================================================================
// GameShaderBegin
// Begin shader mode (wrapper for student API)
//=================================================================
void GameShaderBegin(Shader shader)
{
	ShaderManagerBegin(shader);
}

//=================================================================
// GameShaderEnd
// End shader mode (wrapper for student API)
//=================================================================
void GameShaderEnd(void)
{
	ShaderManagerEnd();
}

//=================================================================
// GameShaderExit
// Cleanup: unload render textures, shaders, and shutdown manager
//=================================================================
void GameShaderExit(GameData *game_data)
{
	if (!game_data)
		return;

	//-------------------------------------------------------------
	// Unload render textures
	//-------------------------------------------------------------
	UnloadRenderTexture(game_data->arc_render_texture);
	UnloadRenderTexture(game_data->beacon_render_texture);
	UnloadRenderTexture(game_data->danger_strobe_render_texture);
	UnloadRenderTexture(game_data->dissolve_render_texture);
	UnloadRenderTexture(game_data->edge_burn_render_texture);
	UnloadRenderTexture(game_data->fireball_render_texture);
	UnloadRenderTexture(game_data->grayscale_render_texture);
	UnloadRenderTexture(game_data->heat_haze_render_texture);
	UnloadRenderTexture(game_data->outline_render_texture);
	UnloadRenderTexture(game_data->palette_tint_render_texture);
	UnloadRenderTexture(game_data->pixelate_render_texture);
	UnloadRenderTexture(game_data->scanlines_render_texture);
	UnloadRenderTexture(game_data->slime_render_texture);
	UnloadRenderTexture(game_data->sparks_render_texture);
	UnloadRenderTexture(game_data->spotlight_render_texture);
	UnloadRenderTexture(game_data->vignette_render_texture);

	//-------------------------------------------------------------
	// Unload shaders
	//-------------------------------------------------------------
	ShaderManagerUnload(game_data->arc);
	ShaderManagerUnload(game_data->beacon);
	ShaderManagerUnload(game_data->danger_strobe);
	ShaderManagerUnload(game_data->dissolve);
	ShaderManagerUnload(game_data->edge_burn);
	ShaderManagerUnload(game_data->fireball);
	ShaderManagerUnload(game_data->grayscale);
	ShaderManagerUnload(game_data->heat_haze);
	ShaderManagerUnload(game_data->outline);
	ShaderManagerUnload(game_data->palette_tint);
	ShaderManagerUnload(game_data->pixelate);
	ShaderManagerUnload(game_data->scanlines);
	ShaderManagerUnload(game_data->slime);
	ShaderManagerUnload(game_data->sparks);
	ShaderManagerUnload(game_data->spotlight);
	ShaderManagerUnload(game_data->vignette);

	ExitShaderManager();

	TraceLog(LOG_INFO, "GAME_SHADER: Exited");
}
#ifndef GAME_H
#define GAME_H

#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "constants.h"

#include "core/fast_math.h"
#include "game/game_telemetry.h"
#include "game/game_scene.h"
#include "game/game_gameplay.h"
#include "game/game_input.h"
#include "game/game_sound.h"
#include "game/game_achievements.h"
#include "game/game_spine.h"
#include "game/game_shader.h"
#include "game/game_splash.h"
#include "game/game_modal.h"

#include "simulation/organism.h"
#include "simulation/organism_membrane.h"

#include "core/collider.h"

//=================================================================
// C linkage (C++ compiler flag)
//=================================================================
#ifdef __cplusplus
extern "C"
{
#endif

	//=================================================================
	// Performance Profiling (CPU)
	//=================================================================
	typedef struct PerformanceSample
	{
		const char *label; // Under measurement (e.g. "ComputeOutline")
		double start;	   // seconds (GetTime())
		double end;		   // seconds (GetTime())
		double elapsed_ms; // milliseconds
		float budget_ms;   // threshold in ms for warning
	} PerformanceSample;

	static inline PerformanceSample PerformanceBegin(const char *label, float budget_ms)
	{
		PerformanceSample s = {0};
		s.label = label;
		s.budget_ms = budget_ms;
		s.start = GetTime();
		return s;
	}

	static inline void PerformanceEndAndLog(PerformanceSample *s)
	{
		s->end = GetTime();
		s->elapsed_ms = (s->end - s->start) * 1000.0;

		if (s->elapsed_ms <= (double)s->budget_ms)
		{
			TraceLog(LOG_INFO, "%s: %.2f ms", s->label, s->elapsed_ms);
		}
		else
		{
			TraceLog(LOG_WARNING, "%s: %.2f ms (CPU frame overrun: exceeded %.2f ms budget)",
					 s->label, s->elapsed_ms, s->budget_ms);
		}
	}

	//=================================================================
	// GameData
	//=================================================================
	typedef struct GameData
	{
		// Game Background (R36S controls overlay)
		Texture2D background;

		// Game Scene Backgrounds
		Texture2D background_gameplay;
		Texture2D background_level_complete;
		Texture2D background_game_over;
		Texture2D background_game_complete;

		// Player Data
		Vector2 player_position;
		float player_speed;
		float player_radius;
		Color player_color;
		SpineEntity *player;
		c2AABB player_collider;
		int player_points;

		// Reticle Data
		Vector2 reticle_position;
		float reticle_speed;
		float reticle_radius;
		Color reticle_color;
		c2Circle reticle_collider;

		// Min Max Game Area
		float min_X;
		float max_X;
		float min_Y;
		float max_Y;

		// Command from Input Manager
		Command command;

		// Time now
		double now;
		double frame_time;

		// Shaders
		Shader arc;
		Shader beacon;
		Shader danger_strobe;
		Shader dissolve;
		Shader edge_burn;
		Shader fireball;
		Shader grayscale;
		Shader heat_haze;
		Shader outline;
		Shader palette_tint;
		Shader pixelate;
		Shader scanlines;
		Shader slime;
		Shader sparks;
		Shader spotlight;
		Shader vignette;

		// Shader needs RenderTexture2D to sample alpha from
		// This can be a Generated Texture or Custom graphic designed texture
		// For the samples both are used
		RenderTexture2D arc_render_texture;			  // texture, entity-size or screen size
		RenderTexture2D beacon_render_texture;		  // texture or entity-size
		RenderTexture2D danger_strobe_render_texture; // texture, entity-size or screen size
		RenderTexture2D dissolve_render_texture;	  // texture or entity-size
		RenderTexture2D edge_burn_render_texture;	  // texture or entity-size
		RenderTexture2D fireball_render_texture;	  // texture, entity-size or screen size
		RenderTexture2D grayscale_render_texture;	  // texture, entity-size or screen size
		RenderTexture2D heat_haze_render_texture;	  // texture, entity-size or screen size
		RenderTexture2D outline_render_texture;		  // texture, entity-size
		RenderTexture2D palette_tint_render_texture;  // texture, entity-size
		RenderTexture2D pixelate_render_texture;	  // texture, entity-size or screen size
		RenderTexture2D scanlines_render_texture;	  // screen size
		RenderTexture2D slime_render_texture;		  // texture or entity-size
		RenderTexture2D sparks_render_texture;		  // texture, entity-size or screen size
		RenderTexture2D spotlight_render_texture;	  // texture, entity-size or screen size
		RenderTexture2D vignette_render_texture;	  // screen size

		// Shader Drivers
		float damage_flash;

		// Shader Manager Statistics
		GameShaderStats shader_stats;

		// Organisms
		Organism organism;
		OrganismMembrane membrane;

		// Achievements
		AchievementManager *manager;
		GameAchievements game_achievements;

		// Level Data
		int current_level;
		int total_kills;
		int kills_this_level;
		int total_coins;
		int secrets_found_this_level;
		int total_secrets_this_level;
		bool took_damage_this_level;
		bool is_running;

		// Exit Game
		bool exit_game;
		float exit_game_time;

		// Game Scene (Update/Draw)
		GameScene scene;

		// Level complete modal
		Modal level_modal;

	} GameData;

	//=================================================================
	// Game Function Prototypes
	//=================================================================
	void Init(void);
	void Input(void);
	void Update(void);
	void Draw(void);
	void Exit(void);
	void GameLoop(void);

#ifdef __cplusplus
}
#endif

#endif // GAME_H
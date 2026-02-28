#ifndef ORGANISM_H
#define ORGANISM_H

#include <raylib.h>

//=================================================================
// Organism
//
// Single-cell organisms floating in the background.
// Each organism follows an independent Lissajous path and spins
// at its own speed and direction, exposing internal structure
// (nucleus, vacuoles, cilia) that makes the rotation visible.
//
// The slime shader overlays frog-spawn bubble texture on top and
// adds a gelatinous outer membrane - it expects SCREEN_WIDTH x
// SCREEN_HEIGHT render texture because the organisms move freely
// across the full screen.
//
// Lifecycle:
//   OrganismInit(&game_data.organism);
//   OrganismUpdate(&game_data.organism, (float)game_data.now);
//   OrganismDraw(&game_data.organism);    // inside BeginShaderMode / EndShaderMode
//   OrganismExit(&game_data.organism);    // no-op - no heap allocations
//=================================================================

//=================================================================
// C linkage (C++ compiler flag)
//=================================================================
#ifdef __cplusplus
extern "C"
{
#endif

#define ORGANISM_COUNT 7

	typedef struct Organism
	{
		// Lissajous path parameters (harmonic motion - sine waves - set once at init, read-only after)
		float base_x[ORGANISM_COUNT];		// screen-space resting X
		float base_y[ORGANISM_COUNT];		// screen-space resting Y
		float amplitude_x[ORGANISM_COUNT];	// horizontal wander amplitude (px)
		float amplitude_y[ORGANISM_COUNT];	// vertical wander amplitude (px)
		float frequency_x[ORGANISM_COUNT];	// horizontal frequency (rad/s)
		float frequency_y[ORGANISM_COUNT];	// vertical frequency (rad/s)
		float phase_x[ORGANISM_COUNT];		// horizontal phase offset (rad)
		float phase_y[ORGANISM_COUNT];		// vertical phase offset (rad)

		// Current state (updated every frame)
		Vector2 position[ORGANISM_COUNT]; // current screen-space centre
		float radius[ORGANISM_COUNT];	  // cell radius in pixels

		// Spin (set once at init, angle updated every frame)
		float spin_speed[ORGANISM_COUNT]; // rotation speed  0.15 - 0.45 rad/s
		float spin_dir[ORGANISM_COUNT];	  // +1.0 CCW or -1.0 CW
		float spin_angle[ORGANISM_COUNT]; // current angle, derived from t (no drift)

		// Trig cache - computed ONCE in OrganismUpdate, read in OrganismDraw.
		// Eliminates 18 cosf/sinf calls per organism per frame in Draw.
		// Draw uses rotation identity: 	cos(a+b) = ca*cb - sa*sb
		// 									sin(a+b) = sa*cb + ca*sb
		float cos_angle[ORGANISM_COUNT]; // cosf(spin_angle[i])
		float sin_angle[ORGANISM_COUNT]; // sinf(spin_angle[i])
	} Organism;

	//=================================================================
	// Organism API
	//=================================================================
	void OrganismInit(Organism *organism);
	void OrganismUpdate(Organism *organism, float t);
	void OrganismDraw(const Organism *organism);
	void OrganismExit(Organism *organism);

#ifdef __cplusplus
}
#endif

#endif // ORGANISM_H
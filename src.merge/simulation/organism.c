#include <raylib.h>
#include <math.h>

#include "simulation/organism.h"
#include "core/fast_math.h"
#include "constants.h"

//=================================================================
// OrganismInit
//=================================================================
void OrganismInit(Organism *organism)
{
	if (!organism)
		return;

	for (int i = 0; i < ORGANISM_COUNT; i++)
	{
		organism->radius[i] = (float)GetRandomValue(20, 45);
		int spawn_r = (int)organism->radius[i];
		int spawn_x = GetRandomValue(40, 80);
		int spawn_y = GetRandomValue(30, 60);

		organism->base_x[i] = (float)GetRandomValue(spawn_r + spawn_x, SCREEN_WIDTH - spawn_r - spawn_x);
		organism->base_y[i] = (float)GetRandomValue(spawn_r + spawn_y, SCREEN_HEIGHT - spawn_r - spawn_y);
		organism->amplitude_x[i] = (float)spawn_x;
		organism->amplitude_y[i] = (float)spawn_y;
		organism->frequency_x[i] = (float)GetRandomValue(10, 25) * 0.01f;
		organism->frequency_y[i] = (float)GetRandomValue(10, 25) * 0.01f;
		organism->phase_x[i] = (float)GetRandomValue(0, 628) * 0.01f;
		organism->phase_y[i] = (float)GetRandomValue(0, 628) * 0.01f;

		organism->position[i] = (Vector2){organism->base_x[i], organism->base_y[i]};

		organism->spin_speed[i] = (float)GetRandomValue(15, 45) * 0.01f;
		organism->spin_dir[i] = (GetRandomValue(0, 1) == 0) ? 1.0f : -1.0f;
		organism->spin_angle[i] = 0.0f;
		organism->cos_angle[i] = 1.0f;
		organism->sin_angle[i] = 0.0f;
	}

	TraceLog(LOG_INFO, "ORGANISM: Initialised %d organisms", ORGANISM_COUNT);
}

//=================================================================
// OrganismUpdate
//
// All trig computed once here using fast_math lookup tables.
// cos_angle/sin_angle cached so OrganismDraw has zero trig calls.
//
// Uses FastSin for position oscillation (8-12x faster than sinf)
// and FastSinCos for rotation (computes both at once, more efficient).
//
// Expected performance: ~10-15% faster on R36S for 7 organisms.
//=================================================================
void OrganismUpdate(Organism *organism, float elapsed)
{
	if (!organism)
		return;

	for (int i = 0; i < ORGANISM_COUNT; i++)
	{
		// Position oscillation using fast sine lookup
		organism->position[i].x = organism->base_x[i] + organism->amplitude_x[i] * FastSin(organism->frequency_x[i] * elapsed + organism->phase_x[i]);
		organism->position[i].y = organism->base_y[i] + organism->amplitude_y[i] * FastSin(organism->frequency_y[i] * elapsed + organism->phase_y[i]);

		// Normalize spin angle to 0 to 2*PI
		organism->spin_angle[i] = fmodf(
			elapsed * organism->spin_speed[i] * organism->spin_dir[i],
			6.28318f);

		// Compute both sin and cos at once (more efficient than two separate calls)
		FastSinCos(organism->spin_angle[i], &organism->sin_angle[i], &organism->cos_angle[i]);
	}
}

//=================================================================
// OrganismDraw
//
// Draw call budget per organism (was 17, now 12):
//   4 pseudopods   (was 6,  -2 draws)
//   1 body
//   1 cytoplasm
//   2 vacuoles × 2 (rim + fill, no specular)   (was ×3, -2 draws)
//   1 nucleus  × 3 (rim + fill + specular kept, primary organelle)
//   Total: 4+1+1+4+3 = 12 per organism, 84 per frame (was 119)
//
// Specular removed from vacuoles: at R36S 480x320 the highlight
// dot is 2-3px wide and imperceptible. Kept on nucleus because it
// is larger and the specular reads as depth on the primary organelle.
//
// Pseudopods reduced 6->4: 90deg spacing still reads as organic.
// Saves 14 DrawCircleV/frame. Uses cardinal axes so the rotation
// identity simplifies to exact values (no floating point error).
//
// ZERO trig calls. Rotation identity from cached cosine_angle/sine_angle:
//   cos(angle+offset) = cosine_angle*cb - sine_angle*sb
//   sin(angle+offset) = sine_angle*cb + cosine_angle*sb
//=================================================================

// Pseudopods: 4 at 90deg spacing (0, 90, 180, 270)
// cos/sin of cardinal angles - exact values, no float error
static const float PSEUDO_CB[4] = {1.0f, 0.0f, -1.0f, 0.0f};
static const float PSEUDO_SB[4] = {0.0f, 1.0f, 0.0f, -1.0f};

// Vacuoles: +120deg and +240deg from nucleus
static const float VAC_CB[2] = {-0.5f, -0.5f};
static const float VAC_SB[2] = {0.8660f, -0.8660f};

void OrganismDraw(const Organism *organism)
{
	if (!organism)
		return;

	for (int i = 0; i < ORGANISM_COUNT; i++)
	{
		Vector2 position = organism->position[i];
		float spawn_r = organism->radius[i];
		float cosine_angle = organism->cos_angle[i];
		float sine_angle = organism->sin_angle[i];

		// ----------------------------------------------------------
		// Pseudopods: 4 lobes (was 6). Cardinal spacing means the
		// rotation identity collapses to simple sign flips at 0/180
		// and a swap at 90/270 which maans a fast possible evaluation.
		// ----------------------------------------------------------
		float pseudo_r = spawn_r * 0.18f;
		float pseudo_dist = spawn_r * 0.88f;
		for (int p = 0; p < 4; p++)
		{
			float position_x = cosine_angle * PSEUDO_CB[p] - sine_angle * PSEUDO_SB[p];
			float position_y = sine_angle * PSEUDO_CB[p] + cosine_angle * PSEUDO_SB[p];
			DrawCircleV(
				(Vector2){position.x + position_x * pseudo_dist, position.y + position_y * pseudo_dist},
				pseudo_r,
				(Color){72, 138, 52, 200});
		}

		// Body wall
		DrawCircleV(position, spawn_r, (Color){48, 92, 34, 225});
		// Cytoplasm
		DrawCircleV(position, spawn_r * 0.82f, (Color){85, 158, 62, 195});

		// ----------------------------------------------------------
		// Vacuoles: rim + fill only (no specular).
		// At R36S resolution the specular dot is 2 - 3px and invisible.
		// ----------------------------------------------------------
		float orb_orbit = spawn_r * 0.44f;
		float v_r = spawn_r * 0.17f;
		for (int v = 0; v < 2; v++)
		{
			float vx = cosine_angle * VAC_CB[v] - sine_angle * VAC_SB[v];
			float vy = sine_angle * VAC_CB[v] + cosine_angle * VAC_SB[v];
			Vector2 vp = {position.x + vx * orb_orbit, position.y + vy * orb_orbit};
			DrawCircleV(vp, v_r, (Color){42, 88, 28, 240});
			DrawCircleV(vp, v_r * 0.78f, (Color){128, 210, 88, 230});
		}

		// ----------------------------------------------------------
		// Nucleus: rim + fill + specular (kept - larger, reads depth).
		// Uses cosine_angle/sine_angle directly, no identity needed (offset = 0).
		// ----------------------------------------------------------
		float nucleus_r = spawn_r * 0.26f;
		Vector2 nucleus_pos = {position.x + cosine_angle * orb_orbit, position.y + sine_angle * orb_orbit};
		DrawCircleV(nucleus_pos, nucleus_r, (Color){42, 88, 28, 240});
		DrawCircleV(nucleus_pos, nucleus_r * 0.78f, (Color){128, 210, 88, 230});
		DrawCircleV(
			(Vector2){nucleus_pos.x - nucleus_r * 0.20f, nucleus_pos.y - nucleus_r * 0.20f},
			nucleus_r * 0.30f,
			(Color){195, 245, 148, 180});
	}
}

//=================================================================
// OrganismExit
//=================================================================
void OrganismExit(Organism *organism)
{
	// No heap allocations in Init....nothing to free.
	// Parameter kept for API consistency and future use.
	// Unused
	(void)organism;
	TraceLog(LOG_INFO, "ORGANISM: Exited");
}
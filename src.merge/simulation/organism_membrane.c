#include <math.h>
#include <string.h>

#include "simulation/organism_membrane.h"

//=================================================================
// Platform-specific ARM optimizations
// R36S uses Rockchip RK3326 (ARM Cortex-A35) with weak FPU
// NEON SIMD available on ARM platforms (R36S, ARM Linux, ARM Windows)
//=================================================================

// ARM NEON intrinsics for vectorization
// Check for ARM NEON support regardless of platform
#if defined(__ARM_NEON) || defined(__ARM_NEON__)
#include <arm_neon.h>
#define USE_ARM_NEON 1
#else
#define USE_ARM_NEON 0
#endif

//=================================================================
// Fast Math Lookup Tables (Platform-agnostic)
// Pre-computed sin/cos tables eliminate expensive trig calls
// 360 entries = 1 degree precision, total 2.8KB memory
// CRITICAL for R36S weak FPU but helps all platforms
//=================================================================
#define FAST_MATH_TABLE_SIZE 360
static float g_sin_table[FAST_MATH_TABLE_SIZE];
static float g_cos_table[FAST_MATH_TABLE_SIZE];
static bool g_tables_initialized = false;

static void InitFastMathTables(void)
{
	if (g_tables_initialized)
		return;

	for (int i = 0; i < FAST_MATH_TABLE_SIZE; i++)
	{
		// Calculate angle: 0 to 2*PI over 360 steps
		float angle = (2.0f * PI * i) / FAST_MATH_TABLE_SIZE;
		g_sin_table[i] = sinf(angle);
		g_cos_table[i] = cosf(angle);
	}
	g_tables_initialized = true;

#if defined(PLATFORM_R36S)
	TraceLog(LOG_INFO, "ORGANISM_MEMBRANE: Fast math tables initialised (R36S optimised)");
#elif defined(PLATFORM_LINUX)
	TraceLog(LOG_INFO, "ORGANISM_MEMBRANE: Fast math tables initialised (Linux)");
#elif defined(PLATFORM_WINDOWS)
	TraceLog(LOG_INFO, "ORGANISM_MEMBRANE: Fast math tables initialised (Windows)");
#elif defined(PLATFORM_WEB)
	TraceLog(LOG_INFO, "ORGANISM_MEMBRANE: Fast math tables initialised (Web)");
#endif
}

// Fast sin using lookup table (1 degree precision)
static inline float FastSin(float angle)
{
	// Normalize angle to 0 to 2*PI
	while (angle < 0.0f)
		angle += 2.0f * PI;
	while (angle >= 2.0f * PI)
		angle -= 2.0f * PI;

	// Convert to table index
	int index = (int)((angle * FAST_MATH_TABLE_SIZE) / (2.0f * PI)) % FAST_MATH_TABLE_SIZE;
	return g_sin_table[index];
}

// Fast cos using lookup table (1 degree precision)
static inline float FastCos(float angle)
{
	// Normalize angle to 0 to 2*PI
	while (angle < 0.0f)
		angle += 2.0f * PI;
	while (angle >= 2.0f * PI)
		angle -= 2.0f * PI;

	// Convert to table index
	int index = (int)((angle * FAST_MATH_TABLE_SIZE) / (2.0f * PI)) % FAST_MATH_TABLE_SIZE;
	return g_cos_table[index];
}

//=================================================================
// Fast inverse square root (Quake III algorithm)
// Much faster than 1.0f / sqrtf(x) on ARM and older x86
// Provides approx 3x speedup on R36S RK3326 Cortex-A35
//=================================================================
static inline float FastInvSqrt(float x)
{
	float half_x = 0.5f * x;
	int i = *(int *)&x;
	i = 0x5f3759df - (i >> 1);
	x = *(float *)&i;
	x = x * (1.5f - half_x * x * x); // One Newton iteration
	return x;
}

//=================================================================
// Helper functions
//=================================================================

static inline float Vec2LengthSq(Vector2 v)
{
	return v.x * v.x + v.y * v.y;
}

static inline float Vec2Length(Vector2 v)
{
	float len_sq = v.x * v.x + v.y * v.y;
	if (len_sq < 0.0001f)
		return 0.0f;

	// Use fast inverse sqrt: x * (1/sqrt(x)) = sqrt(x)
	return len_sq * FastInvSqrt(len_sq);
}

static inline float SmoothStep(float t)
{
	return t * t * (3.0f - 2.0f * t);
}

static Vector2 BezierCubicPoint(Vector2 p0, Vector2 p1, Vector2 p2, Vector2 p3, float t)
{
	float u = 1.0f - t;
	float tt = t * t;
	float uu = u * u;
	float uuu = uu * u;
	float ttt = tt * t;

	return (Vector2){
		uuu * p0.x + 3.0f * uu * t * p1.x + 3.0f * u * tt * p2.x + ttt * p3.x,
		uuu * p0.y + 3.0f * uu * t * p1.y + 3.0f * u * tt * p2.y + ttt * p3.y};
}

//=================================================================
// OrganismMembraneInit
//=================================================================
void OrganismMembraneInit(OrganismMembrane *membrane, Vector2 center)
{
	if (!membrane)
		return;

	// Initialize fast math tables on first use (all platforms)
	InitFastMathTables();

	memset(membrane, 0, sizeof(OrganismMembrane));

	membrane->center = center;
	membrane->velocity = (Vector2){0, 0};
	membrane->part_count = 0;
	membrane->currentShape = ORGANISM_SHAPE_IDLE;
	membrane->global_pulse = 0.0f;
	membrane->tension = 0.7f;
	membrane->viscosity = 0.6f;
	membrane->metaball_threshold = 35.0f;

	// Default shape parameters
	membrane->shapeParams.stretch_length = 2.5f;
	membrane->shapeParams.stretch_width = 0.2f;
	membrane->shapeParams.squash_width = 1.8f;
	membrane->shapeParams.squash_height = 0.15f;
	membrane->shapeParams.idle_breath_amount = 0.15f;
	membrane->shapeParams.idle_breath_speed = 1.5f;

	TraceLog(LOG_INFO, "ORGANISM_MEMBRANE: Initialized at (%.1f, %.1f)", center.x, center.y);
}

//=================================================================
// OrganismMembraneAddPart
//=================================================================
void OrganismMembraneAddPart(OrganismMembrane *membrane, Vector2 offset, float radius)
{
	if (!membrane || membrane->part_count >= ORGANISM_MEMBRANE_MAX_PARTS)
		return;

	OrganismPart *part = &membrane->parts[membrane->part_count];
	part->offset = offset;
	part->radius = radius;
	part->radius_squared = radius * radius; // Cache for performance
	part->active = true;

	membrane->part_count++;
}

//=================================================================
// OrganismMembraneClearParts
//=================================================================
void OrganismMembraneClearParts(OrganismMembrane *membrane)
{
	if (!membrane)
		return;
	membrane->part_count = 0;
}

//=================================================================
// OrganismMembraneUpdate
//=================================================================
void OrganismMembraneUpdate(OrganismMembrane *membrane, float dt, float elapsed)
{
	if (!membrane)
		return;

	// Update breathing pulse
	membrane->global_pulse = elapsed * 2.0f;

	// Apply velocity damping
	membrane->velocity.x *= 0.95f;
	membrane->velocity.y *= 0.95f;
	membrane->center.x += membrane->velocity.x * dt;
	membrane->center.y += membrane->velocity.y * dt;
}

//=================================================================
// OrganismMembraneSetShape
//=================================================================
void OrganismMembraneSetShape(OrganismMembrane *membrane, OrganismShapeType shape, float direction)
{
	if (!membrane)
		return;

	membrane->currentShape = shape;

	// Adjust tension/viscosity per shape
	switch (shape)
	{
	case ORGANISM_SHAPE_IDLE:
		membrane->tension = 0.6f;
		membrane->viscosity = 0.7f;
		break;

	case ORGANISM_SHAPE_STRETCH:
		membrane->tension = 0.95f;	// Very tight
		membrane->viscosity = 0.3f; // Low viscosity = slippery
		break;

	case ORGANISM_SHAPE_SQUASH:
		membrane->tension = 0.3f;	// Loose
		membrane->viscosity = 0.9f; // High viscosity = sticky
		break;

	default:
		break;
	}
}

//=================================================================
// Fast metaball field calculation
// OPTIMIZED: Uses cached radius_squared, fast inverse sqrt
// Platform-specific SIMD vectorization where available
//=================================================================
static inline float CalculateMetaballFieldFast(Vector2 point, const OrganismMembrane *membrane)
{
	float field = 0.0f;

#if USE_ARM_NEON
	// NEON vectorized version - process 4 parts at once
	// Available on: R36S (RK3326), ARM Linux, ARM Windows, Raspberry Pi
	// Automatically detected via __ARM_NEON compiler define
	float32x4_t field_vec = vdupq_n_f32(0.0f);
	float32x4_t point_x = vdupq_n_f32(point.x);
	float32x4_t point_y = vdupq_n_f32(point.y);
	float32x4_t center_x = vdupq_n_f32(membrane->center.x);
	float32x4_t center_y = vdupq_n_f32(membrane->center.y);

	// Process 4 parts at a time
	int i = 0;
	for (; i + 3 < membrane->part_count; i += 4)
	{
		// Load 4 parts
		float32x4_t offset_x = {
			membrane->parts[i].offset.x,
			membrane->parts[i + 1].offset.x,
			membrane->parts[i + 2].offset.x,
			membrane->parts[i + 3].offset.x};
		float32x4_t offset_y = {
			membrane->parts[i].offset.y,
			membrane->parts[i + 1].offset.y,
			membrane->parts[i + 2].offset.y,
			membrane->parts[i + 3].offset.y};
		float32x4_t rad_sq = {
			membrane->parts[i].radius_squared,
			membrane->parts[i + 1].radius_squared,
			membrane->parts[i + 2].radius_squared,
			membrane->parts[i + 3].radius_squared};

		// Calculate world positions
		float32x4_t part_x = vaddq_f32(center_x, offset_x);
		float32x4_t part_y = vaddq_f32(center_y, offset_y);

		// Calculate distance squared
		float32x4_t dx = vsubq_f32(point_x, part_x);
		float32x4_t dy = vsubq_f32(point_y, part_y);
		float32x4_t dist_sq = vaddq_f32(vmulq_f32(dx, dx), vmulq_f32(dy, dy));

		// Calculate field contribution: r_squared / sqrt(dist_squared)
		// Use reciprocal sqrt estimate for speed
		float32x4_t rsqrt = vrsqrteq_f32(dist_sq);
		float32x4_t dist = vmulq_f32(dist_sq, rsqrt);
		float32x4_t contrib = vmulq_f32(rad_sq, vrecpeq_f32(dist));

		field_vec = vaddq_f32(field_vec, contrib);
	}

	// Sum the 4 lanes
	float32x2_t sum = vadd_f32(vget_high_f32(field_vec), vget_low_f32(field_vec));
	field = vget_lane_f32(vpadd_f32(sum, sum), 0);

	// Process remaining parts (scalar fallback)
	for (; i < membrane->part_count; i++)
#else
	// Standard scalar version
	// Used on: x86/x64 (Windows/Linux), Web, or ARM without NEON
	for (int i = 0; i < membrane->part_count; i++)
#endif
	{
		const OrganismPart *part = &membrane->parts[i];
		if (!part->active)
			continue;

		// World position of part
		Vector2 part_pos = {
			membrane->center.x + part->offset.x,
			membrane->center.y + part->offset.y};

		// Distance squared from point to part center
		float dx = point.x - part_pos.x;
		float dy = point.y - part_pos.y;
		float dist_sq = dx * dx + dy * dy;

		// Metaball contribution: r_squared / sqrt(dist)
		if (dist_sq > 0.1f)
		{
			// Use fast inverse sqrt: dist = dist_sq * (1 / sqrt(dist_sq))
			float inv_sqrt_dist = FastInvSqrt(dist_sq);
			float dist = dist_sq * inv_sqrt_dist;
			field += part->radius_squared / dist;
		}
		else
		{
			// Very close to center
			field += part->radius * 10.0f;
		}
	}

	return field;
}

//=================================================================
// OrganismMembraneComputeOutline
// OPTIMIZED: Fewer raymarch steps, larger step size
// Uses fast sin/cos lookup tables (all platforms benefit)
//=================================================================
void OrganismMembraneComputeOutline(OrganismMembrane *membrane, float time)
{
	if (!membrane || membrane->part_count == 0)
		return;

	float threshold = membrane->metaball_threshold;

	// Pre-calculate angle step to avoid division in loop
	float angle_step = (2.0f * PI) / (float)ORGANISM_MEMBRANE_N_RAYS;

	// Raymarch from center outward to find membrane edge
	for (int ray_index = 0; ray_index < ORGANISM_MEMBRANE_N_RAYS; ray_index++)
	{
		float angle = angle_step * ray_index;

		// Use fast lookup tables instead of cosf/sinf
		// Major speedup on R36S (weak FPU), helps all platforms
		float cosine_angle = FastCos(angle);
		float sine_angle = FastSin(angle);

		float min_distance = 5.0f;
		float max_distance = 350.0f;
		float distance = min_distance;
		float step_size = 12.0f;

		// Coarse raymarch - march outward until field drops below threshold
		for (int step = 0; step < 40; step++)
		{
			Vector2 test_point = {
				membrane->center.x + cosine_angle * distance,
				membrane->center.y + sine_angle * distance};

			float field = CalculateMetaballFieldFast(test_point, membrane);

			if (field < threshold)
			{
				break; // Early exit saves CPU
			}

			distance += step_size;
			if (distance > max_distance)
			{
				distance = max_distance;
				break;
			}
		}

		// Binary search for precise edge
		float low = fmaxf(distance - step_size, min_distance);
		float high = distance;

		// 4 iterations (reduced from 5 for R36S performance)
		for (int refine = 0; refine < 4; refine++)
		{
			float mid = (low + high) * 0.5f;
			Vector2 test_point = {
				membrane->center.x + cosine_angle * mid,
				membrane->center.y + sine_angle * mid};

			float field = CalculateMetaballFieldFast(test_point, membrane);

			if (field > threshold)
				low = mid;
			else
				high = mid;
		}

		membrane->radii[ray_index] = (low + high) * 0.5f;
	}

	// Single-pass smoothing using Gaussian kernel
	float *radii = membrane->radii;
	float *smooth = membrane->radii_smoothing_buffer;

	for (int i = 0; i < ORGANISM_MEMBRANE_N_RAYS; i++)
	{
		int im2 = (i - 2 + ORGANISM_MEMBRANE_N_RAYS) % ORGANISM_MEMBRANE_N_RAYS;
		int im1 = (i - 1 + ORGANISM_MEMBRANE_N_RAYS) % ORGANISM_MEMBRANE_N_RAYS;
		int ip1 = (i + 1) % ORGANISM_MEMBRANE_N_RAYS;
		int ip2 = (i + 2) % ORGANISM_MEMBRANE_N_RAYS;

		// Gaussian kernel weights: 1,2,4,2,1 = 10
		smooth[i] = (radii[im2] + 2.0f * radii[im1] + 4.0f * radii[i] +
					 2.0f * radii[ip1] + radii[ip2]) *
					0.1f;
	}

	// Copy back - memcpy is platform-optimized
	memcpy(membrane->radii, membrane->radii_smoothing_buffer,
		   sizeof(float) * ORGANISM_MEMBRANE_N_RAYS);

	// Convert radii to anchor points for Bezier curve
	float ray_step = (float)ORGANISM_MEMBRANE_N_RAYS / (float)ORGANISM_MEMBRANE_N_ANCHORS;

	for (int i = 0; i < ORGANISM_MEMBRANE_N_ANCHORS; i++)
	{
		int ray_index = (int)(i * ray_step) % ORGANISM_MEMBRANE_N_RAYS;
		float angle = angle_step * ray_index;

		// Use fast lookup tables
		float cosine_angle = FastCos(angle);
		float sine_angle = FastSin(angle);

		membrane->anchors[i] = (Vector2){
			membrane->center.x + cosine_angle * membrane->radii[ray_index],
			membrane->center.y + sine_angle * membrane->radii[ray_index]};
	}
}

//=================================================================
// OrganismMembraneDraw
//=================================================================
void OrganismMembraneDraw(const OrganismMembrane *membrane, float thickness, Color color)
{
	if (!membrane)
		return;

	// Draw closed Bezier curve through anchor points
	for (int i = 0; i < ORGANISM_MEMBRANE_N_ANCHORS; i++)
	{
		int i0 = (i - 1 + ORGANISM_MEMBRANE_N_ANCHORS) % ORGANISM_MEMBRANE_N_ANCHORS;
		int i1 = i;
		int i2 = (i + 1) % ORGANISM_MEMBRANE_N_ANCHORS;
		int i3 = (i + 2) % ORGANISM_MEMBRANE_N_ANCHORS;

		Vector2 p0 = membrane->anchors[i1];
		Vector2 p3 = membrane->anchors[i2];
		Vector2 t1 = {
			membrane->anchors[i2].x - membrane->anchors[i0].x,
			membrane->anchors[i2].y - membrane->anchors[i0].y};
		Vector2 t2 = {
			membrane->anchors[i3].x - membrane->anchors[i1].x,
			membrane->anchors[i3].y - membrane->anchors[i1].y};

		float tension = 0.55f;
		Vector2 p1 = {p0.x + t1.x * tension / 3.0f, p0.y + t1.y * tension / 3.0f};
		Vector2 p2 = {p3.x - t2.x * tension / 3.0f, p3.y - t2.y * tension / 3.0f};

		Vector2 prev = BezierCubicPoint(p0, p1, p2, p3, 0.0f);
		float step_inv = 1.0f / (float)ORGANISM_MEMBRANE_BEZIER_STEPS;

		for (int s = 1; s <= ORGANISM_MEMBRANE_BEZIER_STEPS; s++)
		{
			float t = (float)s * step_inv;
			Vector2 cur = BezierCubicPoint(p0, p1, p2, p3, t);
			DrawLineEx(prev, cur, thickness, color);
			prev = cur;
		}
	}
}

//=================================================================
// OrganismMembraneDrawWithGlow
//=================================================================
void OrganismMembraneDrawWithGlow(const OrganismMembrane *membrane)
{
    if (!membrane)
        return;

    // Outer glow (dark soft green)
    OrganismMembraneDraw(membrane, 8.0f, (Color){20, 120, 60, 100});
    
    // Mid glow (vibrant green)
    OrganismMembraneDraw(membrane, 6.0f, (Color){60, 200, 120, 200});
    
    // Main outline (bright neon green)
    OrganismMembraneDraw(membrane, 4.0f, (Color){160, 255, 200, 255});
}

//=================================================================
// OrganismMembraneExit
//=================================================================
void OrganismMembraneExit(OrganismMembrane *membrane)
{
	// No heap allocations, nothing to free
	(void)membrane;
	TraceLog(LOG_INFO, "ORGANISM_MEMBRANE: Exited");
}

//=================================================================
// OrganismMembraneRebuildFromOrganism
//
// Helper function to rebuild membrane parts from ALL organisms.
// Instead of wrapping one organism, this creates a SINGLE membrane
// that encloses ALL 7 organisms on screen.
//
// Each organism's internal parts (pseudopods, vacuoles, nucleus)
// become "molecules" in the metaball field, creating one unified
// liquid membrane around everything.
//
// Call this each frame after OrganismUpdate.
//=================================================================
void OrganismMembraneRebuildFromOrganism(
	OrganismMembrane *membrane,
	const Organism *organism)
{
	if (!membrane || !organism)
		return;

	// Clear old parts
	OrganismMembraneClearParts(membrane);

	// Calculate centroid of all organisms for membrane center
	Vector2 centroid = {0, 0};
	for (int i = 0; i < ORGANISM_COUNT; i++)
	{
		centroid.x += organism->position[i].x;
		centroid.y += organism->position[i].y;
	}
	centroid.x /= ORGANISM_COUNT;
	centroid.y /= ORGANISM_COUNT;
	membrane->center = centroid;

	// Add ALL organisms' internal parts to the membrane
	for (int i = 0; i < ORGANISM_COUNT; i++)
	{
		// Offset from membrane center to this organism's center
		Vector2 organism_offset = {
			organism->position[i].x - membrane->center.x,
			organism->position[i].y - membrane->center.y};

		float spawn_r = organism->radius[i];
		float cosine_angle = organism->cos_angle[i];
		float sine_angle = organism->sin_angle[i];

		// ----------------------------------------------------------
		// Pseudopods: 4 lobes at cardinal angles
		// ----------------------------------------------------------
		float pseudo_r = spawn_r * 0.18f;
		float pseudo_dist = spawn_r * 0.88f;

		const float PSEUDO_CB[4] = {1.0f, 0.0f, -1.0f, 0.0f};
		const float PSEUDO_SB[4] = {0.0f, 1.0f, 0.0f, -1.0f};

		for (int p = 0; p < 4; p++)
		{
			float pos_x = cosine_angle * PSEUDO_CB[p] - sine_angle * PSEUDO_SB[p];
			float pos_y = sine_angle * PSEUDO_CB[p] + cosine_angle * PSEUDO_SB[p];

			Vector2 offset = {
				organism_offset.x + pos_x * pseudo_dist,
				organism_offset.y + pos_y * pseudo_dist};
			OrganismMembraneAddPart(membrane, offset, pseudo_r);
		}

		// ----------------------------------------------------------
		// Body (central mass)
		// ----------------------------------------------------------
		OrganismMembraneAddPart(membrane, organism_offset, spawn_r);

		// ----------------------------------------------------------
		// Vacuoles: 2 at +120 degrees and +240 degrees from nucleus
		// ----------------------------------------------------------
		float orb_orbit = spawn_r * 0.44f;
		float v_r = spawn_r * 0.17f;

		const float VAC_CB[2] = {-0.5f, -0.5f};
		const float VAC_SB[2] = {0.8660f, -0.8660f};

		for (int v = 0; v < 2; v++)
		{
			float vx = cosine_angle * VAC_CB[v] - sine_angle * VAC_SB[v];
			float vy = sine_angle * VAC_CB[v] + cosine_angle * VAC_SB[v];

			Vector2 offset = {
				organism_offset.x + vx * orb_orbit,
				organism_offset.y + vy * orb_orbit};
			OrganismMembraneAddPart(membrane, offset, v_r);
		}

		// ----------------------------------------------------------
		// Nucleus: at 0 degrees (directly uses cosine_angle/sine_angle)
		// ----------------------------------------------------------
		float nucleus_r = spawn_r * 0.26f;
		Vector2 nucleus_offset = {
			organism_offset.x + cosine_angle * orb_orbit,
			organism_offset.y + sine_angle * orb_orbit};
		OrganismMembraneAddPart(membrane, nucleus_offset, nucleus_r);
	}
}
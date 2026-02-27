#ifndef FAST_MATH_H
#define FAST_MATH_H

#include <raylib.h>
#include <math.h>
#include <stdbool.h>

//=================================================================
// Fast Math Library Enhanced for Raymath Compatibility
//
// Cross-platform optimised math functions.
// Provides drop-in replacements for <raymath.h> hot-path functions.
//
// Optimised for ARM platforms (R36S. R36XX, Raspberry Pi) but helps all.
//
// Usage:
//	#include <raymath.h>			// Use Raymath for most things
//	#include "core/fast_math.h"		// Use fast_math for hot paths
//
//	FastMathInit();					// Call once at startup
//
// Performance on R36S (Cortex-A35):
//	FastSin/FastCos:	8-12x faster than sinf/cosf
//	FastInvSqrt:		3x faster than 1.0f/sqrtf
//	FastVec2Length:		2-3x faster than Vector2Length
//
//=================================================================

#ifdef __cplusplus
extern "C"
{
#endif

//=================================================================
// Configuration
//=================================================================

// Lookup table size (360 = 1 degree precision)
// Memory usage: FAST_MATH_TABLE_SIZE * 2 * sizeof(float)
// 360 entries = 2.8KB total
#define FAST_MATH_TABLE_SIZE 360

	//=================================================================
	// Initialisation
	//=================================================================

	// Initialise lookup tables (call once at startup)
	void FastMathInit(void);

	// Check if tables are initialised
	bool FastMathIsInitialized(void);

	//=================================================================
	// Fast Trigonometry (1 degree precision)
	//=================================================================

	float FastSin(float angle);
	float FastCos(float angle);
	void FastSinCos(float angle, float *out_sin, float *out_cos); // Both at once (faster)

	//=================================================================
	// Fast Square Root Operations
	//=================================================================

	// Fast inverse square root (Quake III algorithm)
	// Approximately 3x faster than 1.0f / sqrtf(x)
	float FastInvSqrt(float x);

	// Fast square root (using fast inverse sqrt)
	// Approximately 2x faster than sqrtf(x)
	float FastSqrt(float x);

	//=================================================================
	// Fast Vector2 Operations (Raymath-compatible)
	// Drop-in replacements for hot paths
	//=================================================================

	// Length and distance (use sqrt internally - much faster than Raymath)
	float FastVec2Length(Vector2 v);			  // Replaces: Vector2Length(v)
	float FastVec2Distance(Vector2 a, Vector2 b); // Replaces: Vector2Distance(a, b)

	// Length squared (no sqrt - same speed as Raymath, included for API completeness)
	float FastVec2LengthSq(Vector2 v);				// Same as: Vector2LengthSqr(v)
	float FastVec2DistanceSq(Vector2 a, Vector2 b); // Same as: Vector2DistanceSqr(a, b)

	// Normalize (uses fast inverse sqrt)
	Vector2 FastVec2Normalize(Vector2 v); // Replaces: Vector2Normalize(v)

	// Rotation (uses lookup table for sin/cos)
	Vector2 FastVec2Rotate(Vector2 v, float angle); // Replaces: Vector2Rotate(v, angle)

	//=================================================================
	// Fast Vector3 Operations
	//=================================================================
	float FastVec3Length(Vector3 v);			  // Replaces: Vector3Length(v)
	float FastVec3Distance(Vector3 a, Vector3 b); // Replaces: Vector3Distance(a, b)
	float FastVec3LengthSq(Vector3 v);			  // Same as: Vector3LengthSqr(v)
	Vector3 FastVec3Normalize(Vector3 v);		  // Replaces: Vector3Normalize(v)

	//=================================================================
	// Angle Operations
	//=================================================================

	float FastAngleNormalize(float angle);		 // Normalize to 0 to 2*PI
	float FastAngleDifference(float a, float b); // Shortest angle between two angles

	//=================================================================
	// Interpolation
	//=================================================================

	float FastLerp(float a, float b, float t);
	float FastSmoothStep(float t);
	Vector2 FastVec2Lerp(Vector2 a, Vector2 b, float t);

//=================================================================
// Utility Macros (for distance and direction)
//=================================================================

// Calculate normalized direction and distance in one go
// More efficient than separate calls
#define FAST_VEC2_DIR_AND_DIST(from, to, out_dir, out_dist)                 \
	do                                                                      \
	{                                                                       \
		Vector2 delta = {(to).x - (from).x, (to).y - (from).y};             \
		float dist_sq = delta.x * delta.x + delta.y * delta.y;              \
		if (dist_sq < 0.0001f)                                              \
		{                                                                   \
			*(out_dir) = (Vector2){0.0f, 0.0f};                             \
			*(out_dist) = 0.0f;                                             \
		}                                                                   \
		else                                                                \
		{                                                                   \
			float inv_dist = FastInvSqrt(dist_sq);                          \
			*(out_dist) = dist_sq * inv_dist;                               \
			*(out_dir) = (Vector2){delta.x * inv_dist, delta.y * inv_dist}; \
		}                                                                   \
	} while (0)

	//=================================================================
	// Debug/Testing
	//=================================================================

	// Compare fast vs standard math (for testing accuracy)
	void FastMathPrintAccuracy(float angle);

#ifdef __cplusplus
}
#endif

#endif // FAST_MATH_H
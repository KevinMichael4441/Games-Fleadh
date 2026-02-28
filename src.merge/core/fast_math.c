#include "core/fast_math.h"

//=================================================================
// Fast Math Implementation
//
// Cross-platform optimised math library
// Major speedup on ARM platforms with weak FPUs
//=================================================================

//=================================================================
// Static lookup tables (shared across entire project)
//=================================================================
static float g_sin_table[FAST_MATH_TABLE_SIZE];
static float g_cos_table[FAST_MATH_TABLE_SIZE];
static bool g_tables_initialized = false;

//=================================================================
// FastMathInit
// Call once at startup
//=================================================================
void FastMathInit(void)
{
	if (g_tables_initialized)
		return;

	for (int i = 0; i < FAST_MATH_TABLE_SIZE; i++)
	{
		// Calculate angle: 0 to 2*PI over FAST_MATH_TABLE_SIZE steps
		float angle = (2.0f * PI * i) / FAST_MATH_TABLE_SIZE;
		g_sin_table[i] = sinf(angle);
		g_cos_table[i] = cosf(angle);
	}

	g_tables_initialized = true;

	// Platform-specific logging
#if defined(PLATFORM_R36S)
	TraceLog(LOG_INFO, "FAST_MATH: Lookup tables initialized (R36S optimized)");
#elif defined(PLATFORM_LINUX)
	TraceLog(LOG_INFO, "FAST_MATH: Lookup tables initialised (Linux)");
#elif defined(PLATFORM_WINDOWS)
	TraceLog(LOG_INFO, "FAST_MATH: Lookup tables initialised (Windows)");
#elif defined(PLATFORM_WEB)
	TraceLog(LOG_INFO, "FAST_MATH: Lookup tables initialised (Web)");
#else
	TraceLog(LOG_INFO, "FAST_MATH: Lookup tables initialised");
#endif
}

//=================================================================
// FastMathIsInitialized
//=================================================================
bool FastMathIsInitialized(void)
{
	return g_tables_initialized;
}

//=================================================================
// FastSin
// Fast sine using lookup table (1 degree precision)
//=================================================================
float FastSin(float angle)
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

//=================================================================
// FastCos
// Fast cosine using lookup table (1 degree precision)
//=================================================================
float FastCos(float angle)
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
// FastSinCos
// Compute both sin and cos at once (more efficient than two calls)
//=================================================================
void FastSinCos(float angle, float *out_sin, float *out_cos)
{
	// Normalize angle to 0 to 2*PI
	while (angle < 0.0f)
		angle += 2.0f * PI;
	while (angle >= 2.0f * PI)
		angle -= 2.0f * PI;

	// Convert to table index
	int index = (int)((angle * FAST_MATH_TABLE_SIZE) / (2.0f * PI)) % FAST_MATH_TABLE_SIZE;

	if (out_sin)
		*out_sin = g_sin_table[index];
	if (out_cos)
		*out_cos = g_cos_table[index];
}

//=================================================================
// FastInvSqrt
// Fast inverse square root (Quake III algorithm)
// Approximately 3x faster than 1.0f / sqrtf(x) on ARM
//=================================================================
float FastInvSqrt(float x)
{
	float half_x = 0.5f * x;
	int i = *(int *)&x;
	i = 0x5f3759df - (i >> 1);
	x = *(float *)&i;
	x = x * (1.5f - half_x * x * x); // One Newton iteration
	return x;
}

//=================================================================
// FastSqrt
// Fast square root using fast inverse sqrt
// sqrt(x) = x * (1 / sqrt(x))
//=================================================================
float FastSqrt(float x)
{
	if (x <= 0.0f)
		return 0.0f;
	return x * FastInvSqrt(x);
}

//=================================================================
// FastVec2Length
// Fast vector length using fast inverse sqrt
//=================================================================
float FastVec2Length(Vector2 v)
{
	float len_sq = v.x * v.x + v.y * v.y;
	if (len_sq < 0.0001f)
		return 0.0f;

	// Use fast inverse sqrt: x * (1/sqrt(x)) = sqrt(x)
	return len_sq * FastInvSqrt(len_sq);
}

//=================================================================
// FastVec2LengthSq
// Vector length squared (no sqrt needed, very fast)
//=================================================================
float FastVec2LengthSq(Vector2 v)
{
	return v.x * v.x + v.y * v.y;
}

//=================================================================
// FastVec2Normalize
// Fast vector normalization
//=================================================================
Vector2 FastVec2Normalize(Vector2 v)
{
	float len_sq = v.x * v.x + v.y * v.y;
	if (len_sq < 0.0001f)
		return (Vector2){0.0f, 0.0f};

	float inv_len = FastInvSqrt(len_sq);
	return (Vector2){v.x * inv_len, v.y * inv_len};
}

//=================================================================
// FastVec2Distance
// Fast distance between two points
//=================================================================
float FastVec2Distance(Vector2 a, Vector2 b)
{
	float dx = b.x - a.x;
	float dy = b.y - a.y;
	float dist_sq = dx * dx + dy * dy;

	if (dist_sq < 0.0001f)
		return 0.0f;

	return dist_sq * FastInvSqrt(dist_sq);
}

//=================================================================
// FastAngleNormalize
// Normalize angle to 0 to 2*PI range
//=================================================================
float FastAngleNormalize(float angle)
{
	while (angle < 0.0f)
		angle += 2.0f * PI;
	while (angle >= 2.0f * PI)
		angle -= 2.0f * PI;
	return angle;
}

//=================================================================
// FastAngleDifference
// Shortest angle between two angles
//=================================================================
float FastAngleDifference(float a, float b)
{
	float diff = FastAngleNormalize(b - a);
	if (diff > PI)
		diff -= 2.0f * PI;
	return diff;
}

//=================================================================
// FastLerp
// Linear interpolation
//=================================================================
float FastLerp(float a, float b, float t)
{
	return a + (b - a) * t;
}

//=================================================================
// FastSmoothStep
// Smooth interpolation (cubic)
//=================================================================
float FastSmoothStep(float t)
{
	return t * t * (3.0f - 2.0f * t);
}

//=================================================================
// FastVec2DistanceSq
// Distance squared (no sqrt - for comparisons)
//=================================================================
float FastVec2DistanceSq(Vector2 a, Vector2 b)
{
	float dx = b.x - a.x;
	float dy = b.y - a.y;
	return dx * dx + dy * dy;
}

//=================================================================
// FastVec2Rotate
// Rotate vector by angle (uses FastSinCos)
//=================================================================
Vector2 FastVec2Rotate(Vector2 v, float angle)
{
	float sin_angle, cos_angle;
	FastSinCos(angle, &sin_angle, &cos_angle);

	return (Vector2){
		v.x * cos_angle - v.y * sin_angle,
		v.x * sin_angle + v.y * cos_angle};
}

//=================================================================
// FastVec2Lerp
// Linear interpolation between two vectors
//=================================================================
Vector2 FastVec2Lerp(Vector2 a, Vector2 b, float t)
{
	return (Vector2){
		a.x + (b.x - a.x) * t,
		a.y + (b.y - a.y) * t};
}

//=================================================================
// FastVec3Length
// Fast Vector3 length using fast inverse sqrt
//=================================================================
float FastVec3Length(Vector3 v)
{
	float len_sq = v.x * v.x + v.y * v.y + v.z * v.z;
	if (len_sq < 0.0001f)
		return 0.0f;

	return len_sq * FastInvSqrt(len_sq);
}

//=================================================================
// FastVec3Distance
// Fast distance between two Vector3 points
//=================================================================
float FastVec3Distance(Vector3 a, Vector3 b)
{
	float dx = b.x - a.x;
	float dy = b.y - a.y;
	float dz = b.z - a.z;
	float dist_sq = dx * dx + dy * dy + dz * dz;

	if (dist_sq < 0.0001f)
		return 0.0f;

	return dist_sq * FastInvSqrt(dist_sq);
}

//=================================================================
// FastVec3LengthSq
// Vector3 length squared (no sqrt needed)
//=================================================================
float FastVec3LengthSq(Vector3 v)
{
	return v.x * v.x + v.y * v.y + v.z * v.z;
}

//=================================================================
// FastVec3Normalize
// Fast Vector3 normalization
//=================================================================
Vector3 FastVec3Normalize(Vector3 v)
{
	float len_sq = v.x * v.x + v.y * v.y + v.z * v.z;
	if (len_sq < 0.0001f)
		return (Vector3){0.0f, 0.0f, 0.0f};

	float inv_len = FastInvSqrt(len_sq);
	return (Vector3){v.x * inv_len, v.y * inv_len, v.z * inv_len};
}

//=================================================================
// FastMathPrintAccuracy
// Debug function to compare fast vs standard math
//=================================================================
void FastMathPrintAccuracy(float angle)
{
	float fast_sin = FastSin(angle);
	float std_sin = sinf(angle);
	float fast_cos = FastCos(angle);
	float std_cos = cosf(angle);

	float sin_error = fabsf(fast_sin - std_sin);
	float cos_error = fabsf(fast_cos - std_cos);

	TraceLog(LOG_INFO, "FAST_MATH: Angle=%.4f deg", angle * RAD2DEG);
	TraceLog(LOG_INFO, "  Sin: Fast=%.6f Std=%.6f Error=%.6f", fast_sin, std_sin, sin_error);
	TraceLog(LOG_INFO, "  Cos: Fast=%.6f Std=%.6f Error=%.6f", fast_cos, std_cos, cos_error);
}
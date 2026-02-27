// =============================================================
// Animated welding sparks
// Hash-driven particles with gravity
// Random direction and fade. Overlays on top of scene or texture.
//
// Uniforms:
//   resolution  vec2   render target size in pixels
//   intensity   float  spark density / brightness 0.0 - 1.0
//   tintColor   vec4   spark colour (e.g WHITE, YELLOW, ORANGE)
//   time        float  auto-pushed by ShaderManagerUpdate()
//   debugMode   int    1 = show raw particle field
//   debugColor  vec4   debug visualisation colour (default WHITE)
//
// Usage: continuous welding effect
//   ShaderManagerSetParams(shader, (ShaderParams){
//       .resolution = (Vector2){ SCREEN_WIDTH, SCREEN_HEIGHT },
//       .intensity  = 0.8f,
//       .tintColor  = ORANGE,
//   });
//
// Note: ShaderManagerSetParams order, must be in this order for C++
//
// typedef struct
// {
// 	Vector2 resolution;		 // render target size        (SHADER_UNIFORM_RESOLUTION)
// 	float intensity;		 // effect strength           (SHADER_UNIFORM_INTENSITY)
// 	Color tintColor;		 // tint / glow colour        (SHADER_UNIFORM_TINT)
// 	Vector2 pixelSize;		 // pixelate cell size        (SHADER_UNIFORM_PIXEL_SIZE)
// 	Color outlineColor;		 // outline colour            (SHADER_UNIFORM_OUTLINE)
// 	float dissolveThreshold; // dissolve threshold        (SHADER_UNIFORM_DISSOLVE)
// 	Vector2 center;			 // focal point UV            (SHADER_UNIFORM_CENTER)
// 	bool debugMode;			 // visualise internals       (SHADER_UNIFORM_DEBUG)
// 	Color debugColor;		 // debug colour(WHITE)       (SHADER_UNIFORM_DEBUG_COLOR)
// } ShaderParams;
//
// GLSL 100 / OpenGL ES 2.0 (R36S Mali GPU)
// =============================================================
precision mediump float;

varying vec2 fragTexCoord;
varying vec4 fragColor;

uniform sampler2D texture0;
uniform vec2 resolution;
uniform float intensity;
uniform vec4 tintColor;
uniform float time;
uniform int debugMode;
uniform vec4 debugColor;

// Hash for pseudo-random spark seeds
// Sin-free hash
// Safe on mediump Mali, no clustering
// Based on nested fract-multiply which avoids sin() precision loss
float hash(float p) {
	p = fract(p * 0.1031);
	p *= p + 33.33;
	p *= p + p;
	return fract(p);
}

// Each spark has a seed that drives its lifetime, direction and speed
float spark(vec2 uv, float seed) {
    // Wrap time to keep mediump precision on long sessions
	float t_wrapped = mod(time, 100.0);

    // Lifetime: each spark lives for 0.6 seconds, staggered by seed
	float lifetime = 0.6;
	float offset = hash(seed + 7.3) * 10.0;
	float t = mod(t_wrapped * (0.8 + hash(seed) * 0.8) + offset, lifetime);
	float age = t / lifetime; // 0 = born, 1 = dead

    // Birth position - cluster near bottom centre (weld point)
	float birth_x = 0.5 + (hash(seed + 1.1) - 0.5) * 0.12;
	float birth_y = 0.68 + (hash(seed + 2.2) - 0.5) * 0.06;

    // Random direction - upward bias with spread
    // Direction: s = horizontal spread, c = upward force
    // abs(c) guarantees all sparks travel upward
    // without this, half the sparks fall off-screen immediately
	float s = (hash(seed + 8.1) - 0.5) * 2.0;    // left/right  -1..+1
	float c = abs(hash(seed + 9.2) * 2.0 - 0.5);  // always upward > 0

    // Normalise to unit length
	float len = max(0.1, sqrt(s * s + c * c));
	s /= len;
	c /= len;

	float speed = 0.18 + hash(seed + 4.4) * 0.28;

    // Position with gravity pulling down
	float gravity = 0.5;

    // UV y increases downward so subtract c to move up, add gravity to pull down
	float px = birth_x + s * speed * t;
	float py = birth_y - c * speed * t + gravity * t * t;

    // Screen-space distance from spark centre
	vec2 aspect = vec2(resolution.x / resolution.y, 1.0);
	vec2 delta = (uv - vec2(px, py)) * aspect;
	float dist = length(delta);

    // Spark is a small bright dot that fades with age
    // Radius sized for visibility on R36S screen
	float radius = 0.014 + hash(seed + 5.5) * 0.008;
	float dot_glow = max(0.0, 1.0 - dist / radius);
	dot_glow = dot_glow * dot_glow;

    // Fade out as spark ages
	float fade = 1.0 - age * age;

	return dot_glow * fade;
}

void main() {
	vec2 uv = fragTexCoord;
	vec4 scene = texture2D(texture0, uv) * fragColor;

    // Accumulate 24 sparks
    // 24 sparks with well-separated seeds
	float total = 0.0;
	for(int i = 0; i < 24; i++) {
        // Seeds spaced by 17.0 - prime step avoids mediump clustering
		total += spark(uv, float(i) * 17.0 + 1.0);
	}
	total = min(total, 1.0);

    // DEBUG: show raw spark field tinted by debugColor
    // intensity not applied here so debug is visible even when intensity = 0
    // Black = no sparks in this region, bright = spark present
	if(debugMode == 1) {
		gl_FragColor = vec4(debugColor.rgb * total, 1.0);
		return;
	}

    // Apply intensity after debug branch so debug is always visible
	total = total * intensity;

    // Additive blend sparks over scene
	vec3 sparks_rgb = tintColor.rgb * total;
	gl_FragColor = vec4(scene.rgb + sparks_rgb, scene.a);
}
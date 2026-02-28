// =============================================================
// Electrical arc / lightning crawl overlay.
// Noise-based jagged lines that animate over time.
// Good for tesla coils, damaged equipment, forcefields,
// electric fences, or overloaded lab machinery.
//
// Uniforms:
//   resolution  vec2   render target size in pixels
//   tintColor   vec4   arc colour (WHITE, RED, PURPLE)
//   intensity   float  arc brightness / density 0.0 - 1.0
//   time        float  auto-pushed by ShaderManagerUpdate()
//   debugMode   int    1 = show raw arc field
//   debugColor  vec4   debug visualisation colour (default WHITE)
//
// Usage: damaged electric equipment
//   ShaderManagerSetParams(shader, (ShaderParams){
//       .resolution = (Vector2){ SCREEN_WIDTH, SCREEN_HEIGHT },
//       .tintColor  = PURPLE,
//       .intensity  = 0.8f
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
uniform vec4 tintColor;
uniform float intensity;
uniform float time;
uniform int debugMode;
uniform vec4 debugColor;

// Sin-free hash
// Safe on mediump Mali, no clustering
// Based on nested fract-multiply which avoids sin() precision loss
float hash(float p) {
    p = fract(p * 0.1031);
    p *= p + 33.33;
    p *= p + p;
    return fract(p);
}

float hash2(vec2 p) {
    p = fract(p * vec2(0.1031, 0.1030));
    p += dot(p, p.yx + 33.33);
    return fract((p.x + p.y) * p.x);
}

float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);

    float a = hash2(i);
    float b = hash2(i + vec2(1.0, 0.0));
    float c = hash2(i + vec2(0.0, 1.0));
    float d = hash2(i + vec2(1.0, 1.0));

    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

// Single arc bolt along the Y axis at a given X position
// Returns brightness at uv for that bolt
float arc_bolt(vec2 uv, float seed, float speed) {
    // Bolt x centre drifts with time and noise
    // Wrap time to preserve mediump precision on long sessions
    float t_wrapped = mod(time, 100.0);
    float t = t_wrapped * speed;

    // Bolt x centre shifts every ~1.5 seconds
    // Seeds spaced by 17.0 - prime step avoids mediump clustering
    float t_floor = floor(mod(t, 20.0) + seed * 3.7);
    float centre_x = 0.5 + (hash(t_floor + seed) - 0.5) * 0.6;

    // Jagged lateral displacement varies with y and time
    float jag = noise(vec2(uv.y * 12.0, t * 3.0 + seed)) - 0.5;
    jag += noise(vec2(uv.y * 24.0, t * 5.0 + seed)) * 0.5;

    float bolt_x = centre_x + jag * 0.07;

    // Use a fixed aspect ratio fallback if resolution not set
    float ar = (resolution.x > 0.0) ? resolution.x / resolution.y : 1.5;
    float dist_x = abs(uv.x - bolt_x) * ar;

    // Width tuned for R36S small screen
    float width = 0.010 + hash(seed + 2.7) * 0.006;
    float bolt = max(0.0, 1.0 - dist_x / width);
    bolt = bolt * bolt;

    // Flicker: each bolt randomly dims
    // Flicker using wrapped time
    float flicker = step(0.2, hash(seed + floor(mod(t_wrapped * 12.0, 89.0))));

    return bolt * flicker;
}

void main() {
    vec2 uv = fragTexCoord;
    vec4 scene = texture2D(texture0, uv) * fragColor;

    // Three independent arc bolts
    float arcs = 0.0;
    arcs += arc_bolt(uv, 0.0, 1.1);
    arcs += arc_bolt(uv, 17.0, 1.7);
    arcs += arc_bolt(uv, 34.0, 0.9);
    arcs = min(arcs, 1.0);

    // DEBUG: show raw arc field tinted by debugColor
    // intensity not applied here so debug is visible even when intensity = 0
    if(debugMode == 1) {
        gl_FragColor = vec4(debugColor.rgb * arcs, 1.0);
        return;
    }

    // Apply intensity after debug branch so debug is always visible
    arcs = arcs * intensity;

    // Inner core is white-hot, outer glow is tint colour
    vec3 core_color = mix(tintColor.rgb, vec3(1.0), arcs * 0.8);

    // Additive blend arcs over scene
    gl_FragColor = vec4(scene.rgb + core_color * arcs, scene.a);
}
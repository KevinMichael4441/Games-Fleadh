// =============================================================
// Noise-based sprite dissolve. No texture lookup needed -
// uses a hash function, so zero extra memory cost on R36S.
//
// Uniforms:
//   dissolveThreshold  float  0.0 = fully visible, 1.0 = fully gone
//   tintColor          vec4   edge glow colour (e.g. ORANGE)
//   intensity          float  edge glow width  (0.05 - 0.2 is good)
//   time               float  auto-pushed by ShaderManagerUpdate()
//   debugMode          int    1 = show raw noise
//   debugColor         vec4   debug visualisation colour (default WHITE)
//
// NOTE: A death_dissolve, powerup_pickup, teleport_fade, etc. variable is required to drive the intensity over time
//
// Usage: teleport_fade (in update loop)
//   teleport_fade += GetFrameTime() * 0.8f;
//   ShaderManagerSetParams(shader, (ShaderParams){
//       .intensity         = 0.15f,
//       .tintColor         = ORANGE,
//       .dissolveThreshold = teleport_fade
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
// 	Color debugColor;		 // debug colour, default WHITE (SHADER_UNIFORM_DEBUG_COLOR)
// } ShaderParams;
//
// GLSL 100 / OpenGL ES 2.0 (R36S Mali GPU)
// =============================================================
precision mediump float;

varying vec2 fragTexCoord;
varying vec4 fragColor;

uniform sampler2D texture0;
uniform float dissolveThreshold;
uniform vec4 tintColor;
uniform float intensity;
uniform float time;
uniform int debugMode;
uniform vec4 debugColor;

// Cheap pseudo-random hash, no texture, no branching
// Mediump-safe hash
// Wraps input to [0, PI] before sin() to preserve precision on Mali
float hash(vec2 p) {
    p = mod(p, vec2(289.0));
    float n = dot(p, vec2(127.1, 311.7));
    return fract(sin(mod(n, 3.14159265)) * 43758.5453);
}

// 2-octave value noise for a blobby dissolve edge
float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);

    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));

    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

void main() {
    vec4 texel = texture2D(texture0, fragTexCoord) * fragColor;
    float n = noise(fragTexCoord * 8.0) * 0.7 + noise(fragTexCoord * 16.0) * 0.3;

    // DEBUG: show raw noise tinted by debugColor
    // Dark = will dissolve first, bright = dissolves last
    if(debugMode == 1) {
        gl_FragColor = vec4(debugColor.rgb * n, 1.0);
        return;
    }

    if(n < dissolveThreshold)
        discard;

    // Edge glow near the dissolve boundary
    float edgeWidth = intensity * 0.15;
    float edge = 1.0 - smoothstep(dissolveThreshold, dissolveThreshold + edgeWidth, n);

    vec3 glow = tintColor.rgb * edge * tintColor.a;
    gl_FragColor = vec4(texel.rgb + glow, texel.a);
}
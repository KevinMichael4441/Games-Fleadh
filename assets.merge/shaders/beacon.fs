// =============================================================
// Pulsing emergency beacon glow.
// Simulates a rotating or flashing warning light.
// Good for hazard zones, alert states, alarm indicators.
//
// Uniforms:
//   resolution  vec2   render target size in pixels
//   tintColor   vec4   beacon colour (RED, ORANGE, YELLOW, BLUE)
//   intensity   float  glow brightness 0.0 - 1.0
//   center      vec2   beacon position in UV space (0.0-1.0)
//               default: top-right (0.9, 0.1) if not set
//   time        float  auto-pushed by ShaderManagerUpdate()
//   debugMode   int    1 = show raw glow mask
//   debugColor  vec4   debug visualisation colour (default WHITE)
//
// Usage: red alarm beacon in top-right corner
//   ShaderManagerSetParams(shader, (ShaderParams){
//       .resolution = (Vector2){ SCREEN_WIDTH, SCREEN_HEIGHT },
//       .intensity  = 0.9f,
//       .tintColor  = RED,
//       .center     = (Vector2){ 0.9f, 0.1f } OR (Vector2){ SCREEN_WIDTH - 40, 40 }
//   });
//
// Note: Flash rate: controlled by frequency constant below (flashes/sec)
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
uniform vec2 resolution;
uniform vec4 tintColor;
uniform float intensity;
uniform vec2 center;
uniform float time;
uniform int debugMode;
uniform vec4 debugColor;

void main() {
    vec4 scene = texture2D(texture0, fragTexCoord) * fragColor;

    // Beacon UV position - default top-right if center not set
    // Normalise beacon position to UV space
    vec2 beacon_uv = (center.x > 0.0 || center.y > 0.0) ? center : vec2(0.9, 0.1);

    // Aspect-correct distance from beacon centre
    float aspect_ratio = (resolution.x > 0.0) ? resolution.x / resolution.y : 1.5;
    vec2 aspect = vec2(aspect_ratio, 1.0);
    vec2 delta = (fragTexCoord - beacon_uv) * aspect;
    float dist = length(delta);

    // Radial glow falloff
    float glow_radius = 0.25;
    float glow = max(0.0, 1.0 - dist / glow_radius);
    glow = glow * glow * glow;

    // Tight bright core
    float core_radius = 0.018;
    float core = max(0.0, 1.0 - dist / core_radius);
    core = core * core;

    // Wrap time to keep mediump precision on long sessions
    // Flash pulse: sharp on, quick fade (like a real strobe beacon)
    float flash_rate = 1.5;  // flashes per second
    float t = mod(time, 100.0);
    float cycle = mod(t * flash_rate, 1.0);
    float strobe = pow(max(0.0, 1.0 - cycle * 2.0), 2.0); // fast flash, slow decay

    // Raw beacon value - intensity applied after debug branch
    // so debug is visible even when intensity = 0
    // Combine glow + core, modulated by strobe
    float beacon_raw = (glow * 0.6 + core) * strobe;

    // DEBUG: show raw beacon glow mask tinted by debugColor
    if(debugMode == 1) {
        gl_FragColor = vec4(debugColor.rgb * beacon_raw, 1.0);
        return;
    }

    // Additive blend beacon over scene
    float beacon = beacon_raw * intensity;
    gl_FragColor = vec4(scene.rgb + tintColor.rgb * beacon, scene.a);
}
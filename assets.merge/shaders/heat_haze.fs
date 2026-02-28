// =============================================================
// Thermal shimmer - UV distortion to simulate heat rising from
// hot equipment, furnaces, engines, or lab surfaces.
//
// Uniforms:
//   intensity   float  distortion strength 0.0 - 1.0
//                      (0.02 - 0.08 looks natural on R36S)
//   time        float  auto-pushed by ShaderManagerUpdate()
//   debugMode   int    1 = show raw distortion offset as red/green
//   debugColor  vec4   debug tint - mixed with UV offset channels
//                      WHITE shows clearest offset visualisation
//
// Usage: furnace heat shimmer
//   ShaderManagerSetParams(shader, (ShaderParams){
//       .intensity = 0.04f
//   });
//
// Note: wrap only the hot object or region, not the full screen,
// for a more targeted effect and lower fill-rate cost on R36S.
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
//
// GLSL 100 / OpenGL ES 2.0 (R36S Mali GPU)
// =============================================================
precision mediump float;

varying vec2 fragTexCoord;
varying vec4 fragColor;

uniform sampler2D texture0;
uniform float     intensity;
uniform float     time;
uniform int       debugMode;
uniform vec4      debugColor;

// Sin-free hash - safe on mediump Mali
float hash(vec2 p)
{
    p  = fract(p * vec2(0.1031, 0.1030));
    p += dot(p, p.yx + 33.33);
    return fract((p.x + p.y) * p.x);
}

float noise(vec2 p)
{
    vec2 i = floor(p);
    vec2 f = fract(p);
    f      = f * f * (3.0 - 2.0 * f);

    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));

    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

void main()
{
    vec2 uv = fragTexCoord;

    // Heat rises upward - distortion stronger toward bottom of region
    float heat_bias = 1.0 - uv.y;

    // Wrap time to keep mediump precision on long sessions
    float t    = mod(time, 100.0);

    // Two-layer noise at different speeds for organic shimmer
    float slow = noise(uv * 4.0  + vec2(0.0, -t * 0.3));
    float fast = noise(uv * 8.0  + vec2(0.0, -t * 0.7));
    float n    = slow * 0.7 + fast * 0.3;

    // Offset UV - horizontal wobble with slight vertical drift
    float offset_x = (n - 0.5) * 2.0 * intensity * heat_bias;
    float offset_y = (noise(uv * 6.0 + vec2(t * 0.2, 0.0)) - 0.5)
                   * intensity * heat_bias * 0.3;

    vec2 distorted_uv = uv + vec2(offset_x, offset_y);

    // DEBUG: show raw distortion offset as red/green tinted by debugColor
    // Red = horizontal offset, green = vertical offset
    if (debugMode == 1)
    {
        vec3 offset_vis = vec3(offset_x + 0.5, offset_y + 0.5, 0.0);
        gl_FragColor = vec4(offset_vis * debugColor.rgb, 1.0);
        return;
    }

    gl_FragColor = texture2D(texture0, distorted_uv) * fragColor;
}
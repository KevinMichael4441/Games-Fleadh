// =============================================================
// Organic fireball explosion - boiling connected mass of fire
// with cauliflower surface, not separate orbs.
// 5 overlapping lobes close together, heavy fbm distortion
// gives the lumpy organic shape from the reference.
//
// Lifecycle (4-second repeating cycle):
//   0.0 - 0.6s  grow      : expands outward from center
//   0.6 - 1.0s  hold      : full blaze
//   1.0 - 1.5s  dissipate : breaks apart and fades
//   1.5 - 4.0s  gone      : invisible
//
// Usage: fireball
//   ShaderManagerSetParams(shader, (ShaderParams){
//       .resolution        = (Vector2){ SCREEN_WIDTH, SCREEN_HEIGHT },
//       .intensity         = 1.0f,
//       .tintColor         = ORANGE,
//       .center            = (Vector2){ 0.5f, 0.5f } // screen centre in UV
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
uniform vec2      resolution;
uniform float     intensity;
uniform vec4      tintColor;
uniform vec2      center;
uniform float     time;
uniform int       debugMode;
uniform vec4      debugColor;

float hash(float p)
{
    p = fract(p * 0.1031);
    p *= p + 33.33;
    p *= p + p;
    return fract(p);
}

float hash2(vec2 p)
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
    return mix(mix(hash2(i),                  hash2(i + vec2(1.0, 0.0)), f.x),
               mix(hash2(i + vec2(0.0, 1.0)), hash2(i + vec2(1.0, 1.0)), f.x), f.y);
}

// 5-octave fbm for organic cauliflower surface
float fbm(vec2 p)
{
    float v  = noise(p)       * 0.500;
          v += noise(p * 2.1) * 0.250;
          v += noise(p * 4.3) * 0.125;
    return v;
}

float lifecycle(float cycle_t)
{
    if (cycle_t < 0.6) return smoothstep(0.0, 0.6, cycle_t);
    if (cycle_t < 1.0) return 1.0;
    if (cycle_t < 1.5) return 1.0 - smoothstep(1.0, 1.5, cycle_t);
    return 0.0;
}

void main()
{
    vec2  uv     = fragTexCoord;
    vec2  origin = (center.x > 0.0 || center.y > 0.0) ? center : vec2(0.5, 0.5);
    float ar     = (resolution.y > 0.0) ? resolution.x / resolution.y : 1.5;

    float t       = mod(time, 100.0);
    float cycle_t = mod(t, 4.0);
    float scale   = lifecycle(cycle_t);

    if (scale <= 0.0)
    {
        gl_FragColor = texture2D(texture0, uv) * fragColor;
        return;
    }

    // Aspect-correct UV relative to origin
    vec2 uv_a = (uv - origin) * vec2(ar, 1.0);

    // ----------------------------------------------------------
    // FBM turbulence field - this is the key to organic shape
    // Distorts the distance field so edges are lumpy/cauliflower
    // Amplitude grows during hold, breaks apart during dissipate
    // ----------------------------------------------------------
    float turb_scale  = 3.5;
    float turb_speed  = 0.55;
    float turb_amp    = 0.22 * scale
                      + 0.12 * (1.0 - scale); // more chaotic while fading

    vec2  turb_uv     = uv_a * turb_scale + vec2(t * turb_speed, t * 0.3);
    float distortion  = (fbm(turb_uv) - 0.5) * turb_amp;

    // Second fbm layer offset for surface detail (cheap second pass)
    vec2  detail_uv   = uv_a * 6.0 - vec2(t * 0.4, t * 0.7);
    float detail      = (fbm(detail_uv) - 0.5) * 0.10 * scale;

    float total_dist  = distortion + detail;

    // ----------------------------------------------------------
    // 3 overlapping lobes - close together so they merge
    // into one connected mass with visible bumps
    // Lobe positions: centre + two offset lobes
    // ----------------------------------------------------------
    float fire = 0.0;
    float core = 0.0;
    float mid  = 0.0;

    for (int i = 0; i < 3; i++)
    {
        float seed = float(i) * 17.0 + 1.0;

        // Lobes are CLOSE together (0.08-0.14 spread) so they merge
        // Large radius (0.30-0.45) so each lobe is a big blob
        float lobe_dir_x = hash(seed + 10.1) * 2.0 - 1.0;
        float lobe_dir_y = hash(seed + 11.2) * 2.0 - 1.0;
        float lobe_len   = max(sqrt(lobe_dir_x*lobe_dir_x + lobe_dir_y*lobe_dir_y), 0.001);
        lobe_dir_x      /= lobe_len;
        lobe_dir_y      /= lobe_len;

        float spread = (i == 0) ? 0.0 : (0.10 + hash(seed + 3.3) * 0.08) * scale;
        vec2  lobe_c = uv_a - vec2(lobe_dir_x, lobe_dir_y) * spread;

        // Radius: large, grows with scale
        float peak_r = 0.30 + hash(seed + 1.1) * 0.15; // 0.30 - 0.45
        float pulse  = 1.0 + 0.06 * noise(vec2(t * (2.5 + hash(seed)*2.0), seed * 7.0));
        float r      = peak_r * scale * pulse * intensity;

        // Distance with fbm distortion applied
        float d = length(lobe_c) - total_dist;

        float c = max(0.0, 1.0 - d / (r * 0.25));
        c       = c * c * c;

        float m = max(0.0, 1.0 - d / (r * 0.65));
        m       = m * m;

        float o = max(0.0, 1.0 - d / r);

        float flicker = 0.75 + 0.25 * noise(vec2(t * (7.0 + hash(seed)*4.0), seed * 13.0));

        fire += (o * 0.35 + m * 0.40 + c * 0.65) * flicker;
        core += c;
        mid  += m;
    }

    // Overall fade
    fire = min(fire * scale, 1.0);
    core = min(core * scale, 1.0);
    mid  = min(mid  * scale, 1.0);

    if (debugMode == 1)
    {
        gl_FragColor = vec4(debugColor.rgb * fire, 1.0);
        return;
    }

    float fire_f = fire * intensity;
    float core_f = core * intensity;

    vec4  scene    = texture2D(texture0, uv) * fragColor;
    vec3  dark_col = tintColor.rgb * vec3(0.3, 0.03, 0.0);
    vec3  fire_col = mix(dark_col,        tintColor.rgb,        mid);
    fire_col       = mix(fire_col,        vec3(1.0, 0.85, 0.2), core_f * 0.9);
    fire_col       = mix(fire_col,        vec3(1.0, 1.0,  0.9), core_f * core_f);

    gl_FragColor = vec4(fire_col * fire_f, fire_f);
}
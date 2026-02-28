// =============================================================
// Enemy / Platform fire shader (R36S).
//
// Two modes driven by dissolveThreshold:
//
//   Mode A (0.0 -> ~0.65):
//     "On fire" surface effect.
//     - Flames rise upward (directional)
//     - Hot core (yellow/white)
//     - Charring darkens sprite
//     - Sprite stays intact (no holes)
//     - Flames extend OUTSIDE silhouette (extruded tongues)
//
//   Mode B (~0.65 -> 1.0):
//     "Burn away" destruction.
//     - Edge-driven burn inward (silhouette first)
//     - Internal ignition pockets (hotspots)
//     - Pixels discard late
//
// Uniforms:
//   resolution         vec2   render target size
//   intensity          float  0.0 = none, 1.0 = strong
//   tintColor          vec4   flame mid colour (orange/yellow)
//   dissolveThreshold  float  0.0 - 1.0 mode + progression
//   time               float  seconds
//   debugMode          int    1 = show flame mask, 2 = show outside mask
//   debugColor         vec4   debug visualisation colour (default WHITE)
//
// Usage (enemy on fire, no destruction):
//   dissolveThreshold = 0.25 - 0.55
//
// Usage (burn away):
//   dissolveThreshold 0.65 -> 1.0 over time.
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
uniform float dissolveThreshold;
uniform float time;
uniform int debugMode;
uniform vec4 debugColor;

// -------------------------------------------------------------
// Mali-safe hash + value noise (no trig)
// -------------------------------------------------------------
float hash2(vec2 p) {
    p = fract(p * vec2(0.1031, 0.11369));
    p += dot(p, p.yx + 19.19);
    return fract((p.x + p.y) * p.x);
}

float noise2(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);

    float a = hash2(i);
    float b = hash2(i + vec2(1.0, 0.0));
    float c = hash2(i + vec2(0.0, 1.0));
    float d = hash2(i + vec2(1.0, 1.0));

    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

// 2-octave FBM (cheap)
float fbm2(vec2 p) {
    float n = noise2(p);
    n += 0.5 * noise2(p * 2.02 + 17.1);
    return n * (1.0 / 1.5);
}

// -------------------------------------------------------------
// Edge factor from alpha + neighbours (4 taps)
// Returns ~1.0 at alpha boundaries, ~0.0 on flat interior
// -------------------------------------------------------------
float edgeFactorFromA(float a, float n, float s, float e, float w) {
    float d = max(max(abs(a - n), abs(a - s)), max(abs(a - e), abs(a - w)));
    return clamp(d * 5.0, 0.0, 1.0);
}

// -------------------------------------------------------------
// Cross-ring minimum alpha (4 taps)
// Used as a cheap "depth from edge" estimate.
// -------------------------------------------------------------
float ringMinCross(vec2 uv, vec2 ts, float stepPx) {
    vec2 o = ts * stepPx;
    float m = 1.0;
    m = min(m, texture2D(texture0, uv + vec2(o.x, 0.0)).a);
    m = min(m, texture2D(texture0, uv + vec2(-o.x, 0.0)).a);
    m = min(m, texture2D(texture0, uv + vec2(0.0, o.y)).a);
    m = min(m, texture2D(texture0, uv + vec2(0.0, -o.y)).a);
    return m;
}

void main() {
    // Guard
    if(resolution.x <= 0.0 || resolution.y <= 0.0) {
        gl_FragColor = texture2D(texture0, fragTexCoord) * fragColor;
        return;
    }

    vec2 ts = 1.0 / resolution;
    float t = mod(time, 100.0);

    // Sample current + neighbour alpha (enables OUTSIDE flames)
    vec4 texel = texture2D(texture0, fragTexCoord) * fragColor;

    float aC = texel.a;
    float aN = texture2D(texture0, fragTexCoord + vec2(0.0, ts.y)).a;
    float aS = texture2D(texture0, fragTexCoord + vec2(0.0, -ts.y)).a;
    float aE = texture2D(texture0, fragTexCoord + vec2(ts.x, 0.0)).a;
    float aW = texture2D(texture0, fragTexCoord + vec2(-ts.x, 0.0)).a;

    float neighborMax = max(max(aN, aS), max(aE, aW));

    // Pixels we care about:
    //  - inside sprite (aC > 0)
    //  - OR just outside sprite but adjacent to it (neighborMax > 0)
    float coverage = max(aC, neighborMax);
    if(coverage < 0.02)
        discard;

    // Edge factor (works both inside and just outside)
    float edge = edgeFactorFromA(aC, aN, aS, aE, aW);

    // Outside mask: 1.0 just outside silhouette, 0.0 inside
    float outside = clamp((neighborMax - aC) * 6.0, 0.0, 1.0);

    // Destruction phase gate
    float destroyPhase = smoothstep(0.65, 0.98, dissolveThreshold);

    // ---------------------------------------------------------
    // Charring (only meaningful on sprite pixels)
    // ---------------------------------------------------------
    float charAmount = clamp(dissolveThreshold * 1.10, 0.0, 1.0);
    vec3 charred = mix(texel.rgb, texel.rgb * 0.22, charAmount);

    // ---------------------------------------------------------
    // Directional rising flame (surface)
    // ---------------------------------------------------------
    // If flames rise the wrong direction, flip Y:
    // float y = 1.0 - fragTexCoord.y;
    float y = fragTexCoord.y;

    // Flame UVs advected upward
    vec2 flameUV = fragTexCoord * vec2(6.0, 10.0);
    flameUV.y -= t * (2.6 + 1.6 * intensity);

    float body = fbm2(flameUV);
    float detail = fbm2(fragTexCoord * 18.0 + vec2(t * 2.5, -t * 3.2));
    float hotspot = fbm2(fragTexCoord * 9.0 + vec2(0.0, -t * 0.7));

    // Vertical bias (helps platform flames feel like they rise)
    float verticalBias = smoothstep(0.05, 0.95, y);

    // Surface flames (silhouette strong + subtle interior)
    float surface = edge * 1.8;
    surface += body * 0.35;
    surface += hotspot * 0.15;
    surface = clamp(surface, 0.0, 1.0);

    // Flicker
    float flicker = 0.70 + 0.60 * fbm2(fragTexCoord * 16.0 + vec2(t * 7.5, -t * 9.0));

    // ---------------------------------------------------------
    // Extruded tongues OUTSIDE silhouette
    // ---------------------------------------------------------
    // Direction away from sprite (approx via alpha gradient)
    // (Gradient points toward higher alpha; we want outward into lower alpha.)
    vec2 grad = vec2(aE - aW, aN - aS);
    float gradLen = length(grad);
    vec2 dir = gradLen > 0.0001 ? normalize(grad) : vec2(0.0, 1.0);
    dir = -dir;

    // Sample "outward" space a few texels away and advect upward
    vec2 flamePos = fragTexCoord + dir * ts * 3.0;
    flamePos.y -= t * (2.2 + intensity * 1.3);

    float tongue = fbm2(flamePos * vec2(6.0, 10.0));
    float tongueDetail = fbm2(flamePos * vec2(12.0, 20.0) + vec2(t * 1.2, -t * 1.6));

    // Make tongues tall & thin (shape)
    float shape = smoothstep(0.30, 0.92, tongue * (0.65 + 0.55 * tongueDetail));

    // Fade with edge proximity + vertical bias
    float outsideMask = outside * shape;
    outsideMask *= (0.35 + 0.65 * verticalBias);
    outsideMask *= (0.75 + 0.25 * edge);
    outsideMask *= flicker;
    outsideMask *= intensity;
    outsideMask = clamp(outsideMask, 0.0, 1.0);

    // ---------------------------------------------------------
    // Combine flame mask (inside + outside)
    // ---------------------------------------------------------
    float flameMask = surface;
    flameMask *= (0.55 + 0.45 * verticalBias);
    flameMask *= (0.75 + 0.45 * detail);
    flameMask *= flicker;
    flameMask *= intensity * 0.85;
    flameMask = clamp(flameMask, 0.0, 1.0);

    float combinedMask = max(flameMask, outsideMask);

    // Fire gradient: deep red -> orange -> yellow/white hot
    vec3 deep = vec3(0.35, 0.02, 0.01);
    vec3 mid = tintColor.rgb;
    vec3 hot = vec3(1.0, 0.95, 0.6);

    vec3 flameColor = mix(deep, mid, smoothstep(0.12, 0.70, combinedMask));
    flameColor = mix(flameColor, hot, pow(combinedMask, 2.0));

    // Inside: keep sprite readable, add flames where surface is high
    vec3 insideColor = mix(charred, charred + flameColor, clamp(flameMask, 0.0, 1.0));

    // Outside: no base sprite, only flames
    vec3 outsideColor = flameColor;

    // Final colour mix inside/outside
    vec3 finalColor = mix(insideColor, outsideColor, outside);

    // Alpha:
    // - inside: original alpha
    // - outside: flame alpha (soft)
    float outAlpha = aC;
    float outsideAlpha = outsideMask * 0.90; // tune: 0.65 - 1.0
    outAlpha = mix(outAlpha, max(outAlpha, outsideAlpha), outside);

    // ---------------------------------------------------------
    // Late destruction (burn away) - only affects sprite pixels
    // ---------------------------------------------------------
    if(destroyPhase > 0.001 && aC > 0.02) {
        // Cheap interior depth estimate (edge -> centre)
        float r1 = ringMinCross(fragTexCoord, ts, 1.0);
        float r2 = ringMinCross(fragTexCoord, ts, 4.0);
        float depth = clamp(r1 * 0.65 + r2 * 0.35, 0.0, 1.0);

        // Local threshold jitter (strong near edges)
        float edgeBias = pow(1.0 - depth, 0.6);
        float jitter = (fbm2(fragTexCoord * 12.0 + vec2(0.0, -t * 1.1)) - 0.5);
        float localT = dissolveThreshold + jitter * 0.22 * (0.35 + 0.65 * edgeBias);

        // Edge-in burn
        if(depth < localT)
            discard;

        // Internal ignition pockets (only in destruction phase)
        float pocketN = fbm2(fragTexCoord * 14.0 + vec2(t * 0.4, -t * 0.9));
        float pocketT = (dissolveThreshold - 0.62) * 1.25; // starts after ~0.62
        pocketT = clamp(pocketT, 0.0, 1.0);

        float pocketCut = pocketT - (pocketN * 0.55);
        if(pocketCut > 0.85)
            discard;

        // Boost flames slightly as it destroys
        finalColor += flameColor * (0.15 + 0.55 * destroyPhase);
        outAlpha = max(outAlpha, aC);
    }

    // ---------------------------------------------------------
    // DEBUG
    //   debugMode 1: combined fire mask (inside + outside)
    //   debugMode 2: outside mask (fringe area)
    // ---------------------------------------------------------
    if(debugMode == 1) {
        gl_FragColor = vec4(debugColor.rgb * combinedMask, 1.0);
        return;
    }
    if(debugMode == 2) {
        gl_FragColor = vec4(debugColor.rgb * outside, 1.0);
        return;
    }

    gl_FragColor = vec4(finalColor, outAlpha);
}
// =============================================================
// Darkens screen edges with a smooth radial gradient, 
// simulating a camera lens vignette.
//
// Uniforms:
//   intensity   float  0.0 = none, 1.0 = heavy
//   debugMode   int    1 = show raw vignette mask
//   debugColor  vec4   debug visualisation colour (default WHITE)
//
// Usage: Screen outline with fade
//   ShaderManagerSetParams(shader, (ShaderParams){
//       .intensity = 0.5f
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
uniform float intensity;
uniform int debugMode;
uniform vec4 debugColor;

void main() {
    vec4 texel = texture2D(texture0, fragTexCoord) * fragColor;

    // Distance from centre: corners normalised to ~1.0
    vec2 uv = fragTexCoord - 0.5;
    float dist = length(uv) * 1.4142;

    // Smooth quintic falloff
    float vign = dist * dist * (3.0 - 2.0 * dist);
    float dark = mix(0.0, vign, intensity);

    // DEBUG: show raw vignette mask tinted by debugColor
    // Bright centre fading to dark edges
    // DEBUG: show the raw vignette mask as greyscale
    if(debugMode == 1) {
        gl_FragColor = vec4(debugColor.rgb * (1.0 - vign), 1.0);
        return;
    }

    gl_FragColor = vec4(texel.rgb * (1.0 - dark), texel.a);
}
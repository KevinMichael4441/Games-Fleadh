// =============================================================
// Desaturates to greyscale.
// Good for game over screens, freeze effects, or fading out the world.
//
// Uniforms:
//   intensity   float  0.0 = full colour, 1.0 = full grey
//   debugMode   int    1 = show raw luminance value
//   debugColor  vec4   debug visualisation colour (default WHITE)
//
// Usage:
//   ShaderManagerSetParams(shader, (ShaderParams){
//       .intensity = 1.0f
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
uniform float     intensity;
uniform int       debugMode;
uniform vec4      debugColor;

void main()
{
    vec4  texel = texture2D(texture0, fragTexCoord) * fragColor;

    // Luminance-weighted greyscale (perceptual)
    float luma  = dot(texel.rgb, vec3(0.299, 0.587, 0.114));

    // DEBUG: show raw luminance tinted by debugColor
    if (debugMode == 1)
    {
        gl_FragColor = vec4(debugColor.rgb * luma, 1.0);
        return;
    }

    vec3 result  = mix(texel.rgb, vec3(luma), intensity);
    gl_FragColor = vec4(result, texel.a);
}
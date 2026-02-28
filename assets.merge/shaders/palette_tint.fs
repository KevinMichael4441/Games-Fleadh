// =============================================================
// Multiply-blends a colour over the sprite.
// Useful for damage flash (RED), poison (GREEN), buff (GOLD).
//
// Uniforms:
//   tintColor   vec4   rgba tint colour
//   intensity   float  0.0 = original colour, 1.0 = full tint
//   debugMode   int    1 = show tint colour directly, ignores scene
//   debugColor  vec4   debug visualisation colour (default WHITE)
//
// NOTE: A damage_flash variable is required to drive the intensity over time
//
// Usage: damage_flash (in update loop)
//   damage_flash -= GetFrameTime() * 2.0f;
//   ShaderManagerSetParams(shader, (ShaderParams){
//       .intensity = damage_flash,
//       .tintColor = RED
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
uniform vec4      tintColor;
uniform float     intensity;
uniform int       debugMode;
uniform vec4      debugColor;

void main()
{
    vec4 texel = texture2D(texture0, fragTexCoord) * fragColor;

    // DEBUG: show debugColor directly so debug is always visible
    // regardless of whether tintColor or intensity are set
    if (debugMode == 1)
    {
        gl_FragColor = vec4(debugColor.rgb, 1.0);
        return;
    }

    vec3 tinted  = texel.rgb * tintColor.rgb;
    vec3 result  = mix(texel.rgb, tinted, intensity);
    gl_FragColor = vec4(result, texel.a);
}
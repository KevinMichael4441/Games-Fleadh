// =============================================================
// Single-pass sprite silhouette outline.
// Samples 4 cardinal neighbours; draws outlineColor on
// transparent pixels that border an opaque one.
//
// Uniforms:
//   resolution    vec2   SPRITE texture size in pixels (not screen size)
//   outlineColor  vec4   rgba outline colour
//   intensity     float  outline alpha multiplier 0.0 - 1.0
//   debugMode     int    1 = show raw edge detection
//   debugColor    vec4   debug visualisation colour (default WHITE)
//
// Note: resolution here is the sprite sheet / texture size, not
// the screen. e.g. (Vector2){ sprite.width, sprite.height }
//
// Usage:
//   ShaderManagerSetParams(shader, (ShaderParams){
//       .resolution   = (Vector2){ sprite.width, sprite.height },
//       .intensity    = 1.0f,
//       .outlineColor = YELLOW
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
uniform vec2      resolution;
uniform vec4      outlineColor;
uniform float     intensity;
uniform int       debugMode;
uniform vec4      debugColor;

void main()
{
    vec4 texel = texture2D(texture0, fragTexCoord) * fragColor;

    // Guard against resolution = (0,0) - shows debugColor in debug,
    // passthrough in normal mode
    if (resolution.x <= 0.0 || resolution.y <= 0.0)
    {
        if (debugMode == 1)
        {
            gl_FragColor = vec4(debugColor.rgb, 1.0);
            return;
        }
        gl_FragColor = texel;
        return;
    }

    vec2 texelSize = 1.0 / resolution;

    // Sample 4 cardinal neighbours for edge detection
    float n = texture2D(texture0, fragTexCoord + vec2( 0.0,         texelSize.y)).a;
    float s = texture2D(texture0, fragTexCoord + vec2( 0.0,        -texelSize.y)).a;
    float e = texture2D(texture0, fragTexCoord + vec2( texelSize.x,  0.0       )).a;
    float w = texture2D(texture0, fragTexCoord + vec2(-texelSize.x,  0.0       )).a;

    float edge = max(max(n, s), max(e, w));

    // DEBUG: show raw edge detection tinted by debugColor
    if (debugMode == 1)
    {
        gl_FragColor = vec4(debugColor.rgb * edge, 1.0);
        return;
    }

    // Draw outline only on transparent pixels adjacent to opaque ones
    if (texel.a < 0.1)
    {
        vec4 outline = outlineColor;
        outline.a   *= edge * intensity;
        gl_FragColor = outline;
        return;
    }

    gl_FragColor = texel;
}
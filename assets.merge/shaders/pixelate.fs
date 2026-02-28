// =============================================================
// Snaps to a coarser pixel grid for a chunky lo-fi look.
//
// Uniforms:
//   resolution  vec2   render target size in pixels
//   pixelSize   vec2   cell size in pixels (e.g. 4.0, 4.0)
//   debugMode   int    1 = show snapped UV coords as red/green grid
//   debugColor  vec4   debug visualisation colour (default WHITE)
//                      Note: debug mixes debugColor with UV grid,
//                      WHITE shows clearest grid pattern
//
// Usage: pixelate screen or texture
//   ShaderManagerSetParams(shader, (ShaderParams){
//       .resolution = (Vector2){ SCREEN_WIDTH, SCREEN_HEIGHT },
//       .pixelSize  = (Vector2){ 4.0f, 4.0f }
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
uniform vec2      pixelSize;
uniform int       debugMode;
uniform vec4      debugColor;

void main()
{
    // Guard against pixelSize = (0,0) - shows debugColor solid in debug,
    // falls through to unmodified UV in normal mode
    if (pixelSize.x <= 0.0 || pixelSize.y <= 0.0)
    {
        if (debugMode == 1)
        {
            gl_FragColor = vec4(debugColor.rgb, 1.0);
            return;
        }
        gl_FragColor = texture2D(texture0, fragTexCoord) * fragColor;
        return;
    }

    vec2 screenCoord = fragTexCoord * resolution;
    vec2 snapped     = floor(screenCoord / pixelSize) * pixelSize;
    vec2 snappedUV   = snapped / resolution;

    // DEBUG: show snapped UV as red/green grid tinted by debugColor
    // Red = U axis, green = V axis, confirms cell size on device
    if (debugMode == 1)
    {
        vec3 grid = vec3(snappedUV.x, snappedUV.y, 0.0);
        gl_FragColor = vec4(grid * debugColor.rgb, 1.0);
        return;
    }

    gl_FragColor = texture2D(texture0, snappedUV) * fragColor;
}
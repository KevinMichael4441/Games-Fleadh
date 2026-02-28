// =============================================================
// Horizontal Scanline overlay, CRT-style, for a retro pixel art look.
// Horizontal CRT scanline overlay.
//
// Uniforms:
//   resolution  vec2   render target size in pixels (e.g. 640, 480)
//   intensity   float  0.0 = invisible, 1.0 = heavy
//   debugMode   int    1 = show raw scanline pattern
//   debugColor  vec4   debug visualisation colour (default WHITE)
//
// Tip for R36S (640x480 native):
//   params.resolution = (Vector2){ 640, 480 };
//   params.intensity  = 0.35f;
// Usage (R36S native resolution):
//   ShaderManagerSetParams(shader, (ShaderParams){
//       .resolution = (Vector2){ SCREEN_WIDTH, SCREEN_HEIGHT },
//       .intensity  = 0.35f
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
uniform vec2 resolution;
uniform float intensity;
uniform int debugMode;
uniform vec4 debugColor;

void main() {
    vec4 texel = texture2D(texture0, fragTexCoord) * fragColor;
    float screenY = fragTexCoord.y * resolution.y;
    float scanline = 0.5 + 0.5 * sin(screenY * 3.14159);

    // DEBUG: show raw scanline pattern tinted by debugColor
    if(debugMode == 1) {
        gl_FragColor = vec4(debugColor.rgb * scanline, 1.0);
        return;
    }

    float dark = mix(1.0, scanline, intensity);
    gl_FragColor = vec4(texel.rgb * dark, texel.a);
}
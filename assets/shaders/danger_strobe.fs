// =============================================================
// Red/dark warning strobe flash.
// Good for alarm states, critical system failure,
// hazard zones, low health warning.
//
// Uniforms:
//   tintColor   vec4   strobe colour (RED for danger, BLUE for police)
//   intensity   float  strobe strength 0.0 - 1.0
//   time        float  auto-pushed by ShaderManagerUpdate()
//   debugMode   int    1 = show raw strobe value
//   debugColor  vec4   debug visualisation colour (default WHITE)
//
// Usage@ critical alarm state
//   ShaderManagerSetParams(shader, (ShaderParams){
//       .intensity = 0.7f,
//       .tintColor = RED
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
// Flash rate: 2 flashes/sec. 
// Edit STROBE_RATE below to change.
//
// GLSL 100 / OpenGL ES 2.0 (R36S Mali GPU)
// =============================================================
precision mediump float;

varying vec2 fragTexCoord;
varying vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 tintColor;
uniform float intensity;
uniform float time;
uniform int debugMode;
uniform vec4 debugColor;

#define STROBE_RATE 2.0

void main() {
    vec4 scene = texture2D(texture0, fragTexCoord) * fragColor;

    // Sharp square-ish pulse - on for first half of cycle, off for second
    // Wrap time before scaling to preserve mediump precision on long sessions
    // e.g. at t=684, mod(684, 50.0)=34, then 34*2.0=68, mod(68,1.0) works fine
    float t = mod(time, 50.0);
    float cycle = mod(t * STROBE_RATE, 1.0);
    // fast rise
    float strobe = smoothstep(0.0, 0.08, cycle) *
        (1.0 - smoothstep(0.45, 0.55, cycle)); // fast fall

    // Darken scene when strobe is off for a more dramatic effect
    float dark = 1.0 - (1.0 - strobe) * intensity * 0.4;

    // Colour tint when strobe is on
    float tint = strobe * intensity;

    // DEBUG: show raw strobe value tinted by debugColor
    if(debugMode == 1) {
        gl_FragColor = vec4(debugColor.rgb * strobe, 1.0);
        return;
    }

    vec3 result = scene.rgb * dark;
    result = mix(result, result * tintColor.rgb * 1.4, tint);

    gl_FragColor = vec4(result, scene.a);
}
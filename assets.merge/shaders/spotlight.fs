// =============================================================
// Hard-edged radial light source cutting through darkness.
// Designed for TraI - the creature's eye that illuminates any
// room it's in, making the unobservable TraI observable and
// therefore dangerous.
//
// Everything outside the light radius is darkened to near-black.
// Inside the cone is fully lit. The edge has a slight bloom.
// A slow sweep arc simulates the creature scanning the room.
//
// Uniforms:
//   resolution  vec2   render target size in pixels
//   intensity   float  light brightness 0.0 = off, 1.0 = full
//   tintColor   vec4   light colour (WHITE for harsh lab light,
//                      YELLOW for warm eye glow, BLUE for eerie)
//   center      vec2   light source position in UV space (0.0-1.0)
//               default: right edge centre (0.95, 0.5) - creature eye
//   dissolveThreshold  float  ambient darkness 0.0 = pitch black outside
//                             0.3 = dim ambient, 1.0 = no darkness effect
//   time        float  auto-pushed by ShaderManagerUpdate()
//   debugMode   int    1 = show raw light cone field
//   debugColor  vec4   debug visualisation colour (default WHITE)
//
// Usage - creature eye scanning left:
//   ShaderManagerSetParams(shader, (ShaderParams){
//       .resolution        = (Vector2){ SCREEN_WIDTH, SCREEN_HEIGHT },
//       .intensity         = 1.0f,
//       .tintColor         = WHITE,
//       .center            = (Vector2){ creature_uv.x, creature_uv.y },
//       .dissolveThreshold = 0.05f    // nearly pitch black outside cone
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
// Tip: set dissolveThreshold = 0.0 for pure darkness outside the eye.
// Set to 0.3 for dimly lit corridor feel.
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
uniform vec2 center;
uniform float dissolveThreshold; // repurposed: ambient light level outside cone
uniform float time;
uniform int debugMode;
uniform vec4 debugColor;

// Sin-free hash - safe on mediump Mali
float hash(float p) {
	p = fract(p * 0.1031);
	p *= p + 33.33;
	p *= p + p;
	return fract(p);
}

float noise1(float p) {
	float i = floor(p);
	float f = fract(p);
	f = f * f * (3.0 - 2.0 * f);
	return mix(hash(i), hash(i + 1.0), f);
}

void main() {
	vec2 uv = fragTexCoord;

    // Light source position - default right-centre (creature eye coming from edge)
	vec2 eye = (center.x > 0.0 || center.y > 0.0) ? center : vec2(0.95, 0.5);

    // Aspect-correct delta and distance from eye
	float ar = (resolution.y > 0.0) ? resolution.x / resolution.y : 1.5;
	vec2 delta = (uv - eye) * vec2(ar, 1.0);
	float dist = length(delta);

    // Light cone radius
	float cone_radius = 0.55 * intensity;

    // Smooth hard edge - tight falloff gives searchlight feel
    // Inner full-bright zone, then rapid falloff at the edge
	float inner = cone_radius * 0.75;
	float lit = 1.0 - smoothstep(inner, cone_radius, dist);

    // Wrap time to keep mediump precision on long sessions
	float t = mod(time, 100.0);

    // Bloom at the light source itself - bright hotspot at the eye
	float bloom_r = 0.04;
	float bloom = max(0.0, 1.0 - dist / bloom_r);
	bloom = bloom * bloom;

    // Slight flicker - the eye is organic, not a perfect lamp
	float flicker = 0.93 + 0.07 * noise1(t * 7.3);
	float light = (lit + bloom * 0.8) * flicker;
	light = min(light, 1.0);

    // Ambient level outside the cone - controls how dark the darkness is
    // dissolveThreshold repurposed: 0.0 = pitch black, 0.3 = dim ambient
	float ambient = dissolveThreshold;

    // Darkness factor applied outside the cone
	float brightness = mix(ambient, 1.0, light);

    // DEBUG: show raw light cone field tinted by debugColor
	if(debugMode == 1) {
		gl_FragColor = vec4(debugColor.rgb * light, 1.0);
		return;
	}

	vec4 scene = texture2D(texture0, uv) * fragColor;

    // Inside cone: scene lit with tintColor cast
    // Outside cone: scene darkened to near-black
	vec3 lit_col = scene.rgb * brightness;

    // Add slight tint cast inside the cone (cold harsh light, warm eye glow, etc.)
	lit_col = mix(lit_col, lit_col * tintColor.rgb * 1.1, light * intensity * 0.4);

    // Bloom hotspot at the source - tint colour burns bright
	lit_col += tintColor.rgb * bloom * intensity * 0.6;

	gl_FragColor = vec4(lit_col, scene.a);
}
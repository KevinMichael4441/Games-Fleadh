precision mediump float;

varying vec2 fragTexCoord;
varying vec4 fragColor;

uniform sampler2D texture0;
uniform vec2 resolution;   // RT size in pixels
uniform float intensity;   // 0..1

void main()
{
    vec4 texel = texture2D(texture0, fragTexCoord) * fragColor;

    // Keep RT background truly transparent (no RGB garbage)
    if (texel.a <= 0.001) {
        gl_FragColor = vec4(0.0);
        return;
    }

    vec2 ts = 1.0 / resolution;

    // 4 alpha taps (1px). We only use them to create an INNER rim.
    float aR = texture2D(texture0, fragTexCoord + vec2( ts.x, 0.0)).a;
    float aL = texture2D(texture0, fragTexCoord + vec2(-ts.x, 0.0)).a;
    float aU = texture2D(texture0, fragTexCoord + vec2(0.0,  ts.y)).a;
    float aD = texture2D(texture0, fragTexCoord + vec2(0.0, -ts.y)).a;

    float aMin = min(min(aR, aL), min(aU, aD));

    // Inner rim: near edge aMin drops toward 0, inside stays ~1
    float inner = clamp((texel.a - aMin) * 4.5, 0.0, 1.0);
    inner *= inner;

    vec3 col = texel.rgb;

    // Green punch
    col.g = min(1.0, col.g * (1.0 + 0.20 * intensity));
    col.r *= (1.0 - 0.08 * intensity);
    col.b *= (1.0 - 0.05 * intensity);

    // Subtle rim light (only inside slime)
    col += inner * vec3(0.70, 1.00, 0.55) * (0.30 * intensity);

    // Preserve alpha EXACTLY (so RT stays transparent where it should)
    gl_FragColor = vec4(col, texel.a);
}
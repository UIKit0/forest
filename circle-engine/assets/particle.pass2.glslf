#version 120

uniform sampler2D firstPass;
uniform float brightness;
varying vec2 texcoord;

void main() {
    vec4 texel = texture2D(firstPass, texcoord);
    vec4 c = vec4(brightness * texel.rgb / texel.w, 1.0);
    gl_FragColor = c * c * c;
}
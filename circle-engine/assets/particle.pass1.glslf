#version 120

varying vec4 varying_color;
uniform sampler2D texture;

void main() {
    gl_FragColor = varying_color * texture2D(texture, gl_PointCoord).r;
}
#version 120

varying vec4 sprite_color;
uniform sampler2D texture;

void main() {
    gl_FragColor = sprite_color * texture2D(texture, gl_PointCoord).r;
}
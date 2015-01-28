#version 120

attribute vec2 position;
attribute vec4 color;
varying vec4 varying_color;

void main() {
    gl_Position = gl_ModelViewProjectionMatrix * vec4(position.xy, 0.0, 1.0);
    varying_color = color;
}

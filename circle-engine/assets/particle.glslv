uniform float radius;
attribute vec2 position;
attribute vec4 color;
varying vec4 varying_color;

void main() {
    gl_Position = gl_ModelViewProjectionMatrix * vec4(position.xy, 0.0, 1.0);
    gl_PointSize = 2.0 * radius;
    varying_color = color;
}

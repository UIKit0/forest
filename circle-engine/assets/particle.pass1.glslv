#version 120

attribute vec2 position;
attribute vec4 color;
attribute vec4 expiration;  // uint32 packed into a color

uniform int currentTime;
uniform int fadeDuration;

varying vec4 sprite_color;

void main()
{
    // Unpack expiration time
    int expirationInt =
        int(expiration.a * 16777216) +
        int(expiration.b * 65536) +
        int(expiration.g * 256) +
        int(expiration.r);
    int timeRemaining = expirationInt - currentTime;

    // Particle brightness
    float brightness = 1.0;
    if (timeRemaining <= fadeDuration) {
        brightness = max(0.0, timeRemaining / float(fadeDuration));
    }

    gl_Position = gl_ModelViewProjectionMatrix * vec4(position.xy, 0.0, 1.0);
    sprite_color = brightness * color;
}

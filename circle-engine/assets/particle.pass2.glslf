#version 120

uniform sampler2D firstPass;
uniform float brightness;
varying vec2 v_position;

uniform sampler2D feedback, forceGrid;
uniform vec2 forceGridUpperLeft, forceGridLowerRight;
uniform vec2 feedbackUpperLeft, feedbackLowerRight;
uniform float feedbackControl;
uniform vec2 texelSize;

void main()
{
    // Sampling and color nonlinearity for particle field
    vec4 texel = texture2D(firstPass, vec2(v_position.x, 1.0 - v_position.y));
    vec3 c = brightness * texel.rgb / texel.w;
    c = c * c * c;

    // Sample location perturbed by force grid, for optical flow
    vec2 forceCoord = (v_position - forceGridUpperLeft) / (forceGridLowerRight - forceGridUpperLeft);
    vec2 force = texture2D(forceGrid, forceCoord).rg * 1e-6 * feedbackControl;
    vec2 samplePos = v_position - force;

    // Sampled feedback from previous frame, with box diffusion
    vec3 feedbackSample = vec3(0,0,0);
    for (int i = -3; i <= 3; i++) {
        for (int j = -3; j <= 3; j++) {
            feedbackSample += texture2D(feedback, samplePos + texelSize * vec2(i, j)).rgb;
        }
    }
    feedbackSample *= 1.0 / (7.0*7.0);
    feedbackSample = feedbackSample * vec3(feedbackControl);

    // Feedback mask test
    if (v_position.x > feedbackUpperLeft.x && v_position.x < feedbackLowerRight.x) {
        float t = (v_position.y - feedbackLowerRight.y) / (feedbackUpperLeft.y - feedbackLowerRight.y);
        if (t > 0.0 && t < 1.0) {

            vec3 blend = feedbackSample * feedbackControl + c;
            if (blend.r > 1.0) {
                blend /= blend.r;
            }
            if (blend.g > 1.0) {
                blend /= blend.g;
            }
            if (blend.b > 1.0) {
                blend /= blend.b;
            }

            // Blend along a nonlinear vertical gradient
            c = mix(c, blend, 1.0 - 1.0 / (1.0 + t));
        }
    }

    gl_FragColor = vec4(c.rgb, 1.0);
}
#version 120

uniform sampler2D firstPass;
uniform float brightness;
varying vec2 v_position;

uniform sampler2D feedback, forceGrid, feedbackMask;
uniform vec2 forceGridUpperLeft, forceGridLowerRight;
uniform float feedbackGain, feedbackExp, feedbackFlow;
uniform vec2 texelSize;

void main()
{
    // Sampling and color nonlinearity for particle field
    vec4 texel = texture2D(firstPass, vec2(v_position.x, 1.0 - v_position.y));
    vec3 c = brightness * texel.rgb / texel.w;
    c = c * c * c;

    // Feedback mask test
    vec4 mask = texture2D(feedbackMask, v_position);
    if (mask.r > 0.5) {

        // Sample location perturbed by force grid, for optical flow
        vec2 forceCoord = (v_position - forceGridUpperLeft) / (forceGridLowerRight - forceGridUpperLeft);
        vec2 force = texture2D(forceGrid, forceCoord).rg * 2e-6 * feedbackFlow;
        vec2 samplePos = v_position - force;

        // Sampled feedback from previous frame, with box diffusion
        vec3 feedbackSample = vec3(0,0,0);
        for (int i = -3; i <= 3; i++) {
            for (int j = -3; j <= 3; j++) {
                feedbackSample += texture2D(feedback, samplePos + texelSize * vec2(i, j)).rgb;
            }
        }
        feedbackSample *= 1.0 / (7.0*7.0);

        float t = 0.7 - v_position.y + feedbackGain;
        c = pow(c + pow(feedbackSample * t, vec3(feedbackExp)), vec3(1.0f/feedbackExp));
    }

    gl_FragColor = vec4(c.rgb, 1.0);
}
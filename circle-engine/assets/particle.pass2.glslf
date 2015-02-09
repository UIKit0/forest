#version 120

uniform sampler2D firstPass;
uniform float brightness;
varying vec2 v_position;

uniform sampler2D feedback;
uniform vec2 feedbackUpperLeft, feedbackLowerRight;
uniform float feedbackGain;

void main()
{
    // Sampling and color nonlinearity for particle field
    vec4 texel = texture2D(firstPass, vec2(v_position.x, 1.0 - v_position.y));
    vec4 c = vec4(brightness * texel.rgb / texel.w, 1.0);
    c = c * c * c;

    // Feedback mask test
    if (v_position.x > feedbackUpperLeft.x && v_position.x < feedbackLowerRight.x) {
        float t = (v_position.y - feedbackLowerRight.y) / (feedbackUpperLeft.y - feedbackLowerRight.y);
        if (t > 0.0 && t < 1.0) {

            vec4 fb = texture2D(feedback, v_position);

            c += (feedbackGain * t) / length(fb) * fb;
        }
    }

    gl_FragColor = c;
}
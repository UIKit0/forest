#version 120

varying vec2 texcoord;
uniform sampler2D sourceTexture;
uniform sampler2D modelPoints;

void main() {
    vec3 sampleLoc = texture2D(modelPoints, texcoord).rgb;
    gl_FragColor = texture2D(sourceTexture, sampleLoc.rg) * sampleLoc.b;
}

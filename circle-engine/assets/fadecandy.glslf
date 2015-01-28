varying vec2 texcoord;
uniform sampler2D sourceTexture;
uniform sampler2D modelPoints;

void main() {
    vec2 sampleLoc = texture2D(modelPoints, texcoord).xy;
    gl_FragColor = texture2D(sourceTexture, sampleLoc);
}

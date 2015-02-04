#!/usr/bin/env python
import random

NUM_SAMPLES = 128

print """#version 120

varying vec2 texcoord;
uniform sampler2D sourceTexture;
uniform sampler2D modelTexture;
uniform mat3 sourceTransform;
uniform float sampleRadius;

void main()
{
    // Sample location from our LED model is stored in a texture
    vec3 sampleLoc = sourceTransform * texture2D(modelTexture, texcoord).rgb;

    vec4 color = vec4(0,0,0,0);
"""

for i in range(NUM_SAMPLES):
	while True:
		x = random.random() * 2 - 1;
		y = random.random() * 2 - 1;
		if (x*x + y*y) <= 1:
			break
	print "    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(%f,%f));" % (x,y)

print """
    gl_FragColor = color * sampleLoc.z * %f;
}
""" % (1.0 / NUM_SAMPLES)

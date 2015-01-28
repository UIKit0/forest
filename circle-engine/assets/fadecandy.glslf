#version 120

varying vec2 texcoord;
uniform sampler2D sourceTexture;
uniform sampler2D modelTexture;
uniform mat3 sourceTransform;
uniform float sampleRadius;

void main()
{
    // Sample location from our LED model is stored in a texture
    vec3 sampleLoc = sourceTransform * texture2D(modelTexture, texcoord).rgb;

    // Center of sampling circle
    vec4 color = texture2D(sourceTexture, sampleLoc.xy);

    // Also combine samples from the circle circumference, using max() to spread small points

    color = max(color, texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(0.0, 1.0)));
    color = max(color, texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(0.6427876096865393, 0.7660444431189781)));
    color = max(color, texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(0.984807753012208, 0.17364817766693041)));
    color = max(color, texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(0.8660254037844388, -0.4999999999999998)));
    color = max(color, texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(0.3420201433256689, -0.9396926207859083)));
    color = max(color, texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(-0.34202014332566866, -0.9396926207859084)));
    color = max(color, texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(-0.8660254037844384, -0.5000000000000004)));
    color = max(color, texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(-0.9848077530122081, 0.17364817766692997)));
    color = max(color, texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(-0.6427876096865396, 0.7660444431189779)));

    gl_FragColor = color * sampleLoc.z;
}

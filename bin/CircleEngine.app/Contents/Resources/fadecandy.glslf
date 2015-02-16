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

    vec4 color = vec4(0,0,0,0);

    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(-0.172515,-0.667990));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(-0.784588,-0.572691));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(0.183401,-0.721674));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(-0.264739,0.782797));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(0.828717,0.135628));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(-0.158197,-0.797185));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(0.100039,-0.082638));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(0.907780,-0.355063));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(-0.895095,0.018989));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(0.429536,-0.565571));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(-0.193875,0.302376));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(-0.865202,0.359807));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(0.163513,-0.332716));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(-0.579317,0.779255));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(0.340996,-0.429929));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(0.349328,0.298927));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(0.709044,0.589994));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(-0.158065,0.377054));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(-0.861210,-0.498775));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(-0.377008,0.375714));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(-0.387151,-0.507292));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(-0.294382,-0.951896));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(-0.546638,-0.700188));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(0.373958,-0.223620));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(-0.041202,0.210406));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(0.844433,-0.444307));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(-0.208568,-0.940568));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(0.132472,0.370955));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(0.043716,-0.153531));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(-0.539540,-0.361642));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(-0.817942,-0.575019));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(-0.411279,-0.779489));

    gl_FragColor = color * sampleLoc.z * 0.031250;
}


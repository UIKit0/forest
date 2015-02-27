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

    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(-0.103451,-0.002042));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(-0.312103,0.870058));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(0.135691,-0.050944));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(0.871135,-0.411072));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(-0.199277,-0.227168));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(-0.284814,0.386091));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(0.302186,0.446740));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(-0.125158,0.193029));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(-0.693195,0.648559));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(0.746811,-0.315656));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(0.007798,0.574525));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(0.267081,-0.118305));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(-0.636142,-0.214997));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(-0.087023,0.543680));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(0.451879,0.240402));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(-0.054215,-0.371866));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(0.261204,-0.944166));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(-0.446322,0.118899));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(-0.089401,0.615895));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(0.699072,-0.094601));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(0.324978,0.753693));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(0.496590,0.747568));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(0.539751,0.070183));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(-0.007601,-0.829158));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(-0.769536,-0.581130));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(-0.630047,-0.410931));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(-0.571188,-0.714043));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(0.291954,-0.387872));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(-0.157998,-0.310368));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(0.189187,-0.744466));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(-0.759701,0.138867));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(-0.579396,-0.184614));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(0.778092,-0.624957));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(-0.644683,-0.544555));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(0.992596,-0.033690));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(0.610510,-0.298218));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(-0.314519,-0.913558));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(-0.209055,0.303477));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(-0.072559,-0.082492));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(0.517765,-0.324064));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(-0.652588,0.533406));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(0.564032,-0.029016));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(0.351569,0.817482));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(0.686162,0.222775));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(0.839839,-0.292104));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(-0.544023,0.021056));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(0.573101,0.298696));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(-0.737224,0.577882));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(0.730326,-0.462111));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(0.132183,-0.661863));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(-0.932652,0.100705));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(-0.284076,0.143271));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(-0.546303,-0.600148));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(0.779473,0.370131));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(0.757808,0.073962));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(0.303163,0.260238));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(-0.616118,0.101440));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(0.795693,0.010139));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(0.445676,-0.422130));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(0.492192,-0.752044));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(0.666085,0.034845));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(-0.229798,-0.147845));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(-0.643731,0.746227));
    color += texture2D(sourceTexture, sampleLoc.xy + sampleRadius * vec2(0.906645,-0.296480));

    gl_FragColor = color * sampleLoc.z * 0.031250;
}


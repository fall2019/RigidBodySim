#version 460

layout(location = 0) in vec3 iPosition;
layout(location = 1) in vec2 iTexCoord;
layout(location = 2) in vec3 iNormal;
layout(location = 3) in vec3 iTangent;
layout(location = 4) in vec3 iBitangent;
layout(location = 5) in vec3 iInstancePosition;
layout(location = 6) in uint iAlbedoTextureIndex;
layout(location = 7) in uint iModelIndex;

out gl_PerVertex
{
    vec4 gl_Position;
};

layout(set = 0, binding = 1) uniform uModelBuffer
{
    mat4 uModel[2];
};

layout(set = 0, binding = 2) uniform uLightBuffer
{
    mat4 uLightProjection;
    vec4 uLightColorPadding;
    vec3 uLightDirection;
};

void main()
{
    vec3 worldSpacePosition = ((uModel[iModelIndex]  * vec4(iPosition,1.0)) + vec4(iInstancePosition,0.0)).xyz;
    gl_Position = uLightProjection * vec4(worldSpacePosition, 1.0);
}
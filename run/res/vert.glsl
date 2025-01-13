#version 460 core

layout(location = 0) in vec4 posIn;
layout(location = 1) in vec4 colorIn;
layout(location = 2) in vec4 normalIn;
layout(location = 3) in vec2 texCoordIn;

out vec4 fColor;
out vec3 fNormal;
out vec2 fTexCoord;
out vec4 fPosition;

uniform mat4 uViewProjection;
uniform mat4 uModel;
uniform mat3 uModelNormal;

void main() {
    gl_Position = uViewProjection * uModel * posIn;
    fPosition = gl_Position;
    fColor = colorIn;
    fNormal = uModelNormal * normalIn.xyz;
    fTexCoord = texCoordIn;
}

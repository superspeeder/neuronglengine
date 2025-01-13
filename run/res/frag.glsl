#version 460 core

in vec4 fColor;
in vec3 fNormal;
in vec2 fTexCoord;
in vec4 fPosition;

out vec4 colorOut;

uniform vec3 uSunDirection;
uniform vec3 uSunLight;
uniform vec3 uAmbientLight;
uniform vec3 uEyePosition;
uniform float uSpecularStrength;

vec3 light(vec3 lightDir, vec3 lightColor, vec3 normal, vec3 eyePosition, vec3 fragPosition, float specularStrength) {
    float diff = max(dot(normal, -lightDir), 0.0);
    vec3 diffuse = diff * lightColor * 0.7;

    vec3 viewDir = normalize(eyePosition - fragPosition);
    vec3 reflectDir = reflect(lightDir, -normal);

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;

    return vec3(specular + diffuse);
}

void main() {
    vec3 normal = normalize(fNormal);
    vec3 sunDir = normalize(uSunDirection);
    vec3 sunlight = light(sunDir, uSunLight, normal, uEyePosition, fPosition.xyz, uSpecularStrength);

    vec3 combined = (uAmbientLight + sunlight) * fColor.rgb;

    colorOut = vec4(combined, 1.0);
}
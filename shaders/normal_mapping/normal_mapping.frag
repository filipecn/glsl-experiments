#version 440 core
layout(location = 0) out vec4 fragColor;

in vec3 fNormal;
in vec3 fPosition;
in vec2 fUV;
in vec3 tLightPos;
in vec3 tViewPos;
in vec3 tFPos;

struct Light {
    vec3 direction;// directional light direction
    vec3 point;// point light position
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct Material {
    vec3 kAmbient;
    vec3 kDiffuse;
    vec3 kSpecular;
    float shininess;
};

layout(location = 1) uniform Light light;
layout(location = 6) uniform Material material;
layout(location = 10) uniform vec3 cameraPosition;
layout(location = 14) uniform vec2 screenResolution;

uniform sampler2D channel0;
uniform sampler2D channel1;

void main() {
    // obtain normal from normal map in range [0,1]
    vec3 normal = texture(channel1, fUV).rgb;
    // transform normal vector to range [-1,1]
    normal = normalize(normal * 2.0 - 1.0);
    // Blinn-Phong
    // N - surface normal
    // L - light direction
    // V - view direction
    // H - half-way between V and L
    // R - direction of reflection (ignored)
    // (all vectors leave the surface)
    vec3 N = normalize(normal);
    vec3 L = normalize(tLightPos - tFPos);
    vec3 V = normalize(tViewPos - tFPos);
    vec3 H =  reflect(-L, N);
//    H =  normalize(L + V);
    // cosine of the angle between N and L is proportional to light intensity
    float lambertian = max(dot(N, L), 0.0);
    // specular reflection is proportional to V an H alignment
    // material shininess property controls specular size
    float specular = pow(max(dot(V, H), 0.0), material.shininess);
    // final color is then calculated mixing all contributions
    vec3 lightIntensity = (light.ambient * material.kAmbient +
    light.diffuse * texture(channel0, fUV).rgb * lambertian +
    light.specular * material.kSpecular * specular) *
    vec3(1, 1, 1);
    fragColor = vec4(lightIntensity, 1.0);
}

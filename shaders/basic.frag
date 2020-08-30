#version 440 core
layout(location = 0) out vec4 fragColor;

in vec3 fNormal;
in vec3 fPosition;

struct Light {
  vec3 position;
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

layout(location = 2) uniform Light light;
layout(location = 6) uniform Material material;
layout(location = 10) uniform vec3 cameraPosition;

void main() {
  vec3 norm = normalize(fNormal);
  vec3 lightDirection = normalize(light.position - fPosition);
  float diff = max(dot(norm, lightDirection), 0.0);
  vec3 viewDir = normalize(cameraPosition - fPosition);
  vec3 halfwayDirection = normalize(lightDirection + viewDir);
  float spec = pow(max(dot(viewDir, halfwayDirection), 0.0), material.shininess);
  vec3 lightIntensity = (light.ambient * material.kAmbient +
                         light.diffuse * material.kDiffuse * diff +
                         light.specular * material.kSpecular * spec) *
                        vec3(1, 1, 1);
  fragColor = vec4(lightIntensity, 1.0);
}
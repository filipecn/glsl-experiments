#version 440 core
layout(location = 0) out vec4 fragColor;

in vec3 fNormal;
in vec3 fPosition;
in vec3 fUV;

struct Light {
  vec3 direction; // directional light direction
  vec3 point; // point light position
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

void main() {
  vec3 N = normalize(fNormal);
  vec3 L = normalize(light.direction);
  vec3 V = normalize(cameraPosition - fPosition);
  vec3 H = reflect(-L,V);
  // AMBIENT
  vec3 ambient = light.ambient;
  // DIFFUSE
  float lambertian = max(dot(N, L), 0.0);
  // let apply a smooth transition close to interface of light and dark areas
  lambertian = smoothstep(0.0,0.01,lambertian);
  vec3 diffuse = light.diffuse * lambertian;  
  // SPECULAR
  float spec = pow(max(dot(V, H), 0.0) * lambertian, material.shininess * material.shininess);
  // lets also change the specular boundary
  spec = smoothstep(0.005, 0.01, spec);
  vec3 specular = light.specular * spec;
  // RIM LIGHT
  float rimThreshold = 0.1;
  float rimAmount = 0.716;
  vec3 rimColor = vec3(1.0,1.0,1.0);
  float rimDot = 1 - dot(V, N);
  float rimIntensity = rimDot * pow(dot(N, L), rimThreshold);
  rimIntensity = smoothstep(rimAmount - 0.01, rimAmount + 0.01, rimIntensity);
  vec3 rim = rimIntensity * rimColor;
  // FINAL COLOR
  fragColor = vec4(diffuse * material.kDiffuse + 
  				   ambient * material.kAmbient +
  				   specular * material.kSpecular +
  				   rim, 1.0);
}







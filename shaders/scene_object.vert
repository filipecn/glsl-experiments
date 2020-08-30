#version 440 core
// regular vertex attributes
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texcoord;
// per instance attributes
layout(location = 3) uniform mat4 projectionMatrix;
layout(location = 4) uniform mat4 modelMatrix;
// output to fragment shader
out VERTEX {
  //  vec4 color;
  //       vec3 normal;
  vec2 uv;
}
vertex;
void main() {
  gl_Position = projectionMatrix * modelMatrix * vec4(position, 1);
  //       vertex.normal = normalize(model * vec4(normal,
  //   0)).xyz;     float intensity = max(dot(vertex.normal,
  //   (model * vec4(ldir,0)).xyz), 0.0);
  //     vertex.color =   vec4(0,1,1,0.5) * intensity;
  // vertex.uv = texcoord;
};




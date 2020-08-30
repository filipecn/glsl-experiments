#version 440 core
uniform sampler2D tex;
in VERTEX {
  //  vec4 color;
  // vec3 normal;
  vec2 uv;
}
vertex;
out vec4 outColor;
void main() {
  outColor = vec4(0.0, 1.0, 0.0, 0.5);
  // outColor = texture2D(tex, vertex.uv);
  // outColor = vec4(vertex.uv.x,0,vertex.uv.y,1);
};
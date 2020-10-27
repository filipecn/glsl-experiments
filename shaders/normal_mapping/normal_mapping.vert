#version 440 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texcoord;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 bitangent;

out vec3 fNormal;
out vec3 fPosition;
out vec2 fUV;
out vec3 tLightPos;
out vec3 tViewPos;
out vec3 tFPos;

struct Light {
    vec3 direction;// directional light direction
    vec3 point;// point light position
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

layout(location = 1) uniform Light light;
layout(location = 10) uniform vec3 cameraPosition;
layout(location = 11) uniform mat4 model;
layout(location = 12) uniform mat4 view;
layout(location = 13) uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(position, 1.0);
    fPosition = vec3(model * vec4(position, 1.0));
    fNormal = mat3(transpose(inverse(model))) * normal;
    fUV = texcoord;

    vec3 T = normalize(vec3(model * vec4(tangent, 0.0)));
    vec3 N = normalize(vec3(model * vec4(normal, 0.0)));
    // re-orthogonalize T with respect to N
    T = normalize(T - dot(T, N) * N);
    // then retrieve perpendicular vector B with the cross product of T and N
    vec3 B = cross(N, T);
    mat3 TBN = transpose(mat3(T, B, N));
    tLightPos = TBN * light.point;
    tViewPos  = TBN * cameraPosition;
    tFPos  = TBN * vec3(model * vec4(position, 1.0));

}

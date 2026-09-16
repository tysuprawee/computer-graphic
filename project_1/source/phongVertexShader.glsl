#version 330 core

// Input vertex attributes (from VBO)
layout(location = 0) in vec3 position; // Vertex position
layout(location = 1) in vec3 normal;   // Vertex normal

// Uniforms
uniform mat4 MVP; // Combined Model-View-Projection matrix
uniform mat4 M;   // Model matrix alone, to take position and normal to world space

out vec3 fragPosition;
out vec3 fragNormal;

void main() {
    // Transform the vertex position
    gl_Position = MVP * vec4(position, 1.0);

    // P1bTask5 - The lighting is computed in world space.
    fragPosition = vec3(M * vec4(position, 1.0));

    // Every transform used here is a rotation, a translation or a uniform
    // scale, so the upper 3x3 rotates the normal correctly once normalized.
    fragNormal = mat3(M) * normal;
}

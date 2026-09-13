#version 330 core

// Input vertex attributes (from VBO)
layout(location = 0) in vec3 position; // Vertex position
// P1bTask5 - Input Normals for lighting
layout(location = 1) in vec3 normal;   // Vertex normal

// Uniforms
uniform mat4 MVP; // Combined Model-View-Projection matrix
uniform mat4 M;   // Model matrix on its own, to put the normal in world space

out vec3 fragNormal;

void main() {
    // Transform the vertex position
    gl_Position = MVP * vec4(position, 1.0);

    // The model matrix is only translations and rotations here, so its upper
    // 3x3 rotates the normal correctly. A non-uniform scale would need the
    // inverse transpose instead.
    fragNormal = mat3(M) * normal;
}

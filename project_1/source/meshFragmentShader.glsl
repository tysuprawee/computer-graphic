#version 330 core

// P1bTask5 - Modify shader to use position, normal and light positions to compute lighting.
in vec3 fragNormal;

// P1bTask4 - 1.0 when this piece is the selected one, 0.0 otherwise.
uniform float highlight;

// Output color
out vec4 color;

void main() {
    // A single fixed directional light, enough to tell the pieces apart.
    // P1bTask5 replaces this with a light position passed in from main.cpp.
    vec3 lightDir = normalize(vec3(0.4, 1.0, 0.6));
    vec3 n = normalize(fragNormal);
    float diffuse = max(dot(n, lightDir), 0.0);

    // P1bTask4 - Draw the selected part in a brighter color.
    vec3 albedo = mix(vec3(0.62, 0.66, 0.72), vec3(1.0, 0.80, 0.30), highlight);
    color = vec4(albedo * (0.25 + 0.75 * diffuse), 1.0);
}

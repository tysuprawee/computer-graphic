#version 330 core

// P1bTask5 - Phong shading from two positional lights.
struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

const int MAX_LIGHTS = 2;

in vec3 fragPosition;
in vec3 fragNormal;

uniform Light lights[MAX_LIGHTS];
uniform int lightCount;

uniform vec3 materialColor;  // The object's own color
uniform vec3 eyePosition;    // Camera position, for the specular term
uniform float highlight;     // P1bTask4 - 1.0 when this piece is selected

// Output color
out vec4 color;

void main() {
    // P1bTask4 - The selected piece is drawn in a brighter version of its own
    // color, so it stands out without losing which part it is.
    vec3 baseColor = mix(materialColor, vec3(1.0), 0.55 * highlight);

    // P1bTask5 - Diffuse and ambient material are the color of the object, and
    // specular is one tenth of that.
    vec3 materialAmbient  = baseColor;
    vec3 materialDiffuse  = baseColor;
    vec3 materialSpecular = 0.1 * baseColor;
    float shininess = 32.0;

    vec3 n = normalize(fragNormal);
    vec3 viewDir = normalize(eyePosition - fragPosition);

    vec3 result = vec3(0.0);
    for (int i = 0; i < lightCount; ++i) {
        vec3 lightDir = normalize(lights[i].position - fragPosition);
        vec3 reflectDir = reflect(-lightDir, n);

        vec3 ambient = lights[i].ambient * materialAmbient;
        vec3 diffuse = lights[i].diffuse * materialDiffuse * max(dot(n, lightDir), 0.0);
        vec3 specular = lights[i].specular * materialSpecular
                      * pow(max(dot(viewDir, reflectDir), 0.0), shininess);

        result += ambient + diffuse + specular;
    }

    color = vec4(result, 1.0);
}

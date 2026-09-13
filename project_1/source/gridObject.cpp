#include "gridObject.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>


gridObject::gridObject() { // Assign current value of nextId to id and increment it
    
    // Initialize the model matrix
    modelMatrix = glm::mat4(1.0f);

    // Generate and bind VAO, VBO, and EBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // P1aTask1 - Grid on the Y = 0 plane plus the positive coordinate axes.
    std::vector<GLfloat> vertices;
    std::vector<GLuint> indices;

    // Append one vertex (position + color) and return its index.
    auto addVertex = [&vertices](const glm::vec3& p, const glm::vec3& c) -> GLuint {
        GLuint index = (GLuint)(vertices.size() / 6);
        vertices.insert(vertices.end(), { p.x, p.y, p.z, c.r, c.g, c.b });
        return index;
    };
    // Append a line segment. Each endpoint gets its own vertex so that lines
    // meeting at the origin can carry different colors.
    auto addLine = [&](const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) {
        indices.push_back(addVertex(a, c));
        indices.push_back(addVertex(b, c));
    };

    const float extent = 5.0f;      // grid spans (-5,0,-5) to (+5,0,+5)
    const float axisLength = 5.0f;  // positive portion of each axis
    const glm::vec3 gridColor(0.7f, 0.7f, 0.7f);
    const glm::vec3 origin(0.0f);

    // Integer grid lines. The half of the line at z == 0 (resp. x == 0) that runs
    // along the positive X (resp. Z) axis is left out, so the colored axis lines
    // below do not z-fight with a gray grid line lying in the same place.
    for (int i = -(int)extent; i <= (int)extent; ++i) {
        const float f = (float)i;

        // Lines parallel to the X axis
        if (i == 0)
            addLine(glm::vec3(-extent, 0.0f, 0.0f), origin, gridColor);
        else
            addLine(glm::vec3(-extent, 0.0f, f), glm::vec3(extent, 0.0f, f), gridColor);

        // Lines parallel to the Z axis
        if (i == 0)
            addLine(glm::vec3(0.0f, 0.0f, -extent), origin, gridColor);
        else
            addLine(glm::vec3(f, 0.0f, -extent), glm::vec3(f, 0.0f, extent), gridColor);
    }

    // Positive portion of each axis, length 5: X red, Y green, Z blue.
    addLine(origin, glm::vec3(axisLength, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    addLine(origin, glm::vec3(0.0f, axisLength, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    addLine(origin, glm::vec3(0.0f, 0.0f, axisLength), glm::vec3(0.0f, 0.0f, 1.0f));

    numIndices = (GLsizei)indices.size();
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    // Vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    // Load and compile shaders
    shaderProgram = LoadShaders("gridVertexShader.glsl", "gridFragmentShader.glsl");
}

gridObject::~gridObject() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);
    
}

void gridObject::draw(const glm::mat4& view, const glm::mat4& projection) {
    glUseProgram(shaderProgram);
    
    // P1aTask1 - Compute the MVP matrix and send it to the shader as a uniform.
    glm::mat4 MVP = projection * view * modelMatrix;
    GLuint matrixID = glGetUniformLocation(shaderProgram, "MVP");
    glUniformMatrix4fv(matrixID, 1, GL_FALSE, glm::value_ptr(MVP));

    // P1aTask1 - Draw the grid and axis lines.
    glBindVertexArray(VAO);
    glDrawElements(GL_LINES, numIndices, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
}

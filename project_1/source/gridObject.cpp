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

    std::vector<GLfloat> vertices;
    std::vector<GLuint> indices;

    auto addVertex = [&vertices](const glm::vec3& p, const glm::vec3& c) -> GLuint {
        GLuint index = (GLuint)(vertices.size() / 6);
        vertices.insert(vertices.end(), { p.x, p.y, p.z, c.r, c.g, c.b });
        return index;
    };
    auto addLine = [&](const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) {
        indices.push_back(addVertex(a, c));
        indices.push_back(addVertex(b, c));
    };

    const float extent = 5.0f;
    const float axisLength = 5.0f;
    const glm::vec3 gridColor(0.7f, 0.7f, 0.7f);
    const glm::vec3 origin(0.0f);

    for (int i = -(int)extent; i <= (int)extent; ++i) {
        const float f = (float)i;

        if (i == 0)
            addLine(glm::vec3(-extent, 0.0f, 0.0f), origin, gridColor);
        else
            addLine(glm::vec3(-extent, 0.0f, f), glm::vec3(extent, 0.0f, f), gridColor);

        if (i == 0)
            addLine(glm::vec3(0.0f, 0.0f, -extent), origin, gridColor);
        else
            addLine(glm::vec3(f, 0.0f, -extent), glm::vec3(f, 0.0f, extent), gridColor);
    }

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
    
    glm::mat4 MVP = projection * view * modelMatrix;
    GLuint matrixID = glGetUniformLocation(shaderProgram, "MVP");
    glUniformMatrix4fv(matrixID, 1, GL_FALSE, glm::value_ptr(MVP));

    glBindVertexArray(VAO);
    glDrawElements(GL_LINES, numIndices, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
}

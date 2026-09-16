#include "meshObject.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <common/objloader.hpp>

// Initialize static member
int meshObject::nextId = 1;
std::map<int, meshObject*> meshObject::meshObjectMap;

meshObject::meshObject(const std::string& objPath,
                       const std::string& vertexShader,
                       const std::string& fragmentShader) : id(nextId++) { // Assign current value of nextId to id and increment it

    // Add this object to the map
    meshObjectMap[id] = this;

    // Initialize the model matrix
    modelMatrix = glm::mat4(1.0f);

    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;
    std::vector<unsigned int> indices;
    if (!loadOBJ(objPath.c_str(), positions, uvs, normals, indices)) {
        std::cerr << "Failed to load " << objPath << std::endl;
    }

    // P1bTask5 - Interleave the normals with the positions so the lighting pass
    // reads both out of the same buffer. The unlit Project 1a shader declares
    // only attribute 0 and simply ignores attribute 1.
    std::vector<GLfloat> vertices;
    vertices.reserve(positions.size() * 6);
    for (size_t i = 0; i < positions.size(); ++i) {
        vertices.push_back(positions[i].x);
        vertices.push_back(positions[i].y);
        vertices.push_back(positions[i].z);
        vertices.push_back(normals[i].x);
        vertices.push_back(normals[i].y);
        vertices.push_back(normals[i].z);
    }

    numIndices = (GLsizei)indices.size();

    // Generate and bind VAO, VBO, and EBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    // Load and compile shaders
    shaderProgram = LoadShaders(vertexShader.c_str(), fragmentShader.c_str());
    pickingShaderProgram = LoadShaders("pickingVertexShader.glsl", "pickingFragmentShader.glsl");
}

meshObject::~meshObject() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);

    // Remove this object from the map
    meshObjectMap.erase(id);
}

void meshObject::draw(const glm::mat4& view, const glm::mat4& projection) {
    glUseProgram(shaderProgram);

    // Compute the MVP matrix
    glm::mat4 MVP = projection * view * modelMatrix;
    GLuint matrixID = glGetUniformLocation(shaderProgram, "MVP");
    glUniformMatrix4fv(matrixID, 1, GL_FALSE, glm::value_ptr(MVP));

    // Draw the object
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

// P1bTask4 - Draw this piece under its parent's transform, then recurse into
// the children so one call on the root draws the whole arm.
// P1bTask5 - Hand the lighting info to the shader.
void meshObject::draw(const glm::mat4& parentTransform, const glm::mat4& view, const glm::mat4& projection,
                      const Light* lights, int lightCount, const glm::vec3& eyePosition) {
    glm::mat4 world = parentTransform * modelMatrix;

    glUseProgram(shaderProgram);

    glm::mat4 MVP = projection * view * world;
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "MVP"), 1, GL_FALSE, glm::value_ptr(MVP));

    // The lighting works in world space, so the model matrix is needed on its
    // own as well as folded into MVP.
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "M"), 1, GL_FALSE, glm::value_ptr(world));

    glUniform3fv(glGetUniformLocation(shaderProgram, "eyePosition"), 1, glm::value_ptr(eyePosition));
    glUniform3fv(glGetUniformLocation(shaderProgram, "materialColor"), 1, glm::value_ptr(color));

    // P1bTask4 - Draw the selected part in a brighter color.
    glUniform1f(glGetUniformLocation(shaderProgram, "highlight"), selected ? 1.0f : 0.0f);

    glUniform1i(glGetUniformLocation(shaderProgram, "lightCount"), lightCount);
    for (int i = 0; i < lightCount; ++i) {
        std::string prefix = "lights[" + std::to_string(i) + "].";
        glUniform3fv(glGetUniformLocation(shaderProgram, (prefix + "position").c_str()), 1, glm::value_ptr(lights[i].position));
        glUniform3fv(glGetUniformLocation(shaderProgram, (prefix + "ambient").c_str()),  1, glm::value_ptr(lights[i].ambient));
        glUniform3fv(glGetUniformLocation(shaderProgram, (prefix + "diffuse").c_str()),  1, glm::value_ptr(lights[i].diffuse));
        glUniform3fv(glGetUniformLocation(shaderProgram, (prefix + "specular").c_str()), 1, glm::value_ptr(lights[i].specular));
    }

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    for (meshObject* child : children) {
        child->draw(world, view, projection, lights, lightCount, eyePosition);
    }
}

void meshObject::translate(const glm::vec3& translation) {
    // Apply translation to the model matrix
    modelMatrix = glm::translate(modelMatrix, translation);
}

void meshObject::rotate(float angle, const glm::vec3& axis) {
    // Apply rotation to the model matrix
    modelMatrix = glm::rotate(modelMatrix, glm::radians(angle), axis);
}

void meshObject::scale(float factor) {
    modelMatrix = glm::scale(modelMatrix, glm::vec3(factor));
}

void meshObject::resetTransform() {
    // The joint chain is rebuilt every frame, so each frame starts clean
    // instead of piling onto the previous frame's transform.
    modelMatrix = glm::mat4(1.0f);
}

void meshObject::drawPicking(const glm::mat4& view, const glm::mat4& projection) {
    drawPicking(glm::mat4(1.0f), view, projection);
}

// P1bBonus - Draw the piece flat-shaded in a color that encodes its id, so the
// id can be read straight back out of the framebuffer under the cursor.
void meshObject::drawPicking(const glm::mat4& parentTransform, const glm::mat4& view, const glm::mat4& projection) {
    glm::mat4 world = parentTransform * modelMatrix;

    glUseProgram(pickingShaderProgram); // Use the picking shader

    glUniform1f(glGetUniformLocation(pickingShaderProgram, "objectID"), (float)id);

    glm::mat4 MVP = projection * view * world;
    GLuint matrixID = glGetUniformLocation(pickingShaderProgram, "MVP");
    glUniformMatrix4fv(matrixID, 1, GL_FALSE, glm::value_ptr(MVP));

    // Draw the object
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glUseProgram(0); // Unbind the shader program

    for (meshObject* child : children) {
        child->drawPicking(world, view, projection);
    }
}

meshObject* meshObject::getMeshObjectById(int id) {
    // Look up the object by ID in the map
    auto it = meshObjectMap.find(id);
    if (it != meshObjectMap.end()) {
        return it->second;
    }
    return nullptr; // Return nullptr if ID not found
}

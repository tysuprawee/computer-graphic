#include "meshObject.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <common/objloader.hpp>

// Initialize static member
int meshObject::nextId = 1;
std::map<int, meshObject*> meshObject::meshObjectMap;

// P1aTask3 - Build the object from a triangulated .obj file.
meshObject::meshObject(const std::string& objPath) : id(nextId++) { // Assign current value of nextId to id and increment it
    
    // Add this object to the map
    meshObjectMap[id] = this;

    // Initialize the model matrix
    modelMatrix = glm::mat4(1.0f);

    // P1aTask3 - Read the geometry from the obj file. The supplied loader already
    // welds each unique position/uv/normal triple and hands back an index buffer.
    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;
    std::vector<unsigned int> indices;
    if (!loadOBJ(objPath.c_str(), positions, uvs, normals, indices)) {
        std::cerr << "Failed to load " << objPath << std::endl;
    }

    // P1bTask5 - Interleave the normals with the positions so the lighting pass
    // can read them straight out of the same buffer.
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

    // Vertex attributes: 0 = position, 1 = normal, 6 floats per vertex.
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    // Load and compile shaders
    shaderProgram = LoadShaders("meshVertexShader.glsl", "meshFragmentShader.glsl");
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

//TODO: P1bTask5 - Modify to accept lighiting info as arguement.
void meshObject::draw(const glm::mat4& view, const glm::mat4& projection) {
    glUseProgram(shaderProgram);
    
    // Compute the MVP matrix
    glm::mat4 MVP = projection * view * modelMatrix;
    GLuint matrixID = glGetUniformLocation(shaderProgram, "MVP");
    glUniformMatrix4fv(matrixID, 1, GL_FALSE, glm::value_ptr(MVP));
    
    // P1bTask5 - The lighting needs the model matrix on its own to rotate the
    // normals, since MVP also carries the view and projection.
    GLuint modelID = glGetUniformLocation(shaderProgram, "M");
    glUniformMatrix4fv(modelID, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    // P1bTask4 - Tell the shader whether this is the selected piece.
    GLuint highlightID = glGetUniformLocation(shaderProgram, "highlight");
    glUniform1f(highlightID, selected ? 1.0f : 0.0f);

    // Draw the object
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void meshObject::translate(const glm::vec3& translation) {
    // Apply translation to the model matrix
    modelMatrix = glm::translate(modelMatrix, translation);
}

void meshObject::resetTransform() {
    // Rebuilding the joint chain every frame needs a clean slate, otherwise
    // each frame's rotations would pile on top of the previous frame's.
    modelMatrix = glm::mat4(1.0f);
}

void meshObject::rotate(float angle, const glm::vec3& axis) {
    // Apply rotation to the model matrix
    modelMatrix = glm::rotate(modelMatrix, glm::radians(angle), axis);
}

void meshObject::drawPicking(const glm::mat4& view, const glm::mat4& projection) {
    glUseProgram(pickingShaderProgram); // Use the picking shader

    //TODO: P1bBonus2 - Send the value of the id variable to the shader as a uniform. Use glUniform1f

    glm::mat4 MVP = projection * view * modelMatrix;
    GLuint matrixID = glGetUniformLocation(pickingShaderProgram, "MVP");
    glUniformMatrix4fv(matrixID, 1, GL_FALSE, glm::value_ptr(MVP));

    // Draw the object
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glUseProgram(0); // Unbind the shader program
}

meshObject* meshObject::getMeshObjectById(int id) {
    // Look up the object by ID in the map
    auto it = meshObjectMap.find(id);
    if (it != meshObjectMap.end()) {
        return it->second;
    }
    return nullptr; // Return nullptr if ID not found
}

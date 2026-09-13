#ifndef meshObject_hpp
#define meshObject_hpp

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <common/shader.hpp>
#include <map>
#include <string>

class meshObject {
public:
    // P1aTask3 - Build the object from a triangulated .obj file. The path is
    // relative to the source directory, which is the program's working directory.
    meshObject(const std::string& objPath);
    ~meshObject();

    void draw(const glm::mat4& view, const glm::mat4& projection);
    void drawPicking(const glm::mat4& view, const glm::mat4& projection);
    void translate(const glm::vec3& translation); // Translate the object
    void rotate(float angle, const glm::vec3& axis); // Rotate the object
    void resetTransform(); // Clear the model matrix back to the identity

    // P1bTask4 - Highlight the piece the user currently has selected.
    void setSelected(bool isSelected) { selected = isSelected; }

    int getId() const { return id; } // Getter for the ID
    
    static meshObject* getMeshObjectById(int id); // Retrieve object by ID
    
    // TODO: P1bTask4 - Create a list of children.

private:
    GLuint VAO, VBO, EBO;
    GLuint shaderProgram;
    GLuint pickingShaderProgram;
    glm::mat4 modelMatrix;

    static int nextId; // Static counter for unique IDs
    int id;            // ID for this specific object
    bool selected = false; // P1bTask4 - drawn brighter while true
    
    GLsizei numIndices;
    
    static std::map<int, meshObject*> meshObjectMap; // Static map of ID to Object
};

#endif

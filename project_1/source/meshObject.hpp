#ifndef meshObject_hpp
#define meshObject_hpp

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <common/shader.hpp>
#include <map>
#include <string>
#include <vector>

// P1bTask5 - One light. The scene is lit by two of these.
struct Light {
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

class meshObject {
public:
    // P1aTask3 - Build the object from a triangulated .obj file. The path is
    // relative to the source directory, which is the program's working
    // directory. The shader pair defaults to the unlit one used by Project 1a;
    // Project 1b passes the Phong pair instead.
    meshObject(const std::string& objPath,
               const std::string& vertexShader = "meshVertexShader.glsl",
               const std::string& fragmentShader = "meshFragmentShader.glsl");
    ~meshObject();

    void draw(const glm::mat4& view, const glm::mat4& projection);
    void drawPicking(const glm::mat4& view, const glm::mat4& projection);

    // P1bTask4 - Hierarchical draw. modelMatrix is the transform relative to
    // the parent, so calling this on the root draws the whole tree.
    void draw(const glm::mat4& parentTransform, const glm::mat4& view, const glm::mat4& projection,
              const Light* lights, int lightCount, const glm::vec3& eyePosition);
    void drawPicking(const glm::mat4& parentTransform, const glm::mat4& view, const glm::mat4& projection);

    void translate(const glm::vec3& translation); // Translate the object
    void rotate(float angle, const glm::vec3& axis); // Rotate the object
    void scale(float factor); // Uniform scale, so normals survive rotation by mat3(M)
    void resetTransform(); // Back to the identity, for rebuilding each frame

    const glm::mat4& getLocalTransform() const { return modelMatrix; }

    int getId() const { return id; } // Getter for the ID

    static meshObject* getMeshObjectById(int id); // Retrieve object by ID

    // P1bTask4 - A list of children, so each piece carries the ones it moves.
    void addChild(meshObject* child) { children.push_back(child); }

    void setColor(const glm::vec3& c) { color = c; }
    void setSelected(bool isSelected) { selected = isSelected; }

private:
    GLuint VAO, VBO, EBO;
    GLuint shaderProgram;
    GLuint pickingShaderProgram;
    glm::mat4 modelMatrix;

    static int nextId; // Static counter for unique IDs
    int id;            // ID for this specific object

    GLsizei numIndices;

    std::vector<meshObject*> children;
    glm::vec3 color = glm::vec3(0.75f);
    bool selected = false;

    static std::map<int, meshObject*> meshObjectMap; // Static map of ID to Object
};

#endif

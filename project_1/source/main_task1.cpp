#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "gridObject.hpp"
#include "window.hpp"

int main() {
    if (initWindow("Pongpeeradech,Suprawee(34287548) - Task 1: Grid") != 0) return -1;

    glm::mat4 projectionMatrix = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);

    glm::mat4 viewMatrix = glm::lookAt(
        glm::vec3(9.8f, 8.0f, 9.8f), // Camera position
        glm::vec3(0.0f),             // Look at the origin
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    gridObject grid;

    do {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        grid.draw(viewMatrix, projectionMatrix);

        glfwSwapBuffers(window);
        glfwPollEvents();

    } // Check if the ESC key was pressed or the window was closed
    while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0);

    glfwTerminate();
    return 0;
}

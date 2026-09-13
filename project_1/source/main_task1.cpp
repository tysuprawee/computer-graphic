// main_task1.cpp - P1aTask1: the integer grid on the Y = 0 plane and the
// positive X/Y/Z axes. A fixed viewpoint; Task 2 is what makes it movable.
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "gridObject.hpp"
#include "window.hpp"

int main() {
    if (initWindow("Pongpeeradech,Suprawee(34287548) - Task 1: Grid") != 0) return -1;

    // Projection matrix : 45 degree Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    glm::mat4 projectionMatrix = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);

    // P1aTask1 - One fixed camera, looking down at the grid from the +X +Y +Z
    // octant so all three coloured axes are visible at once.
    glm::mat4 viewMatrix = glm::lookAt(
        glm::vec3(9.8f, 8.0f, 9.8f), // Camera position
        glm::vec3(0.0f),             // Look at the origin
        glm::vec3(0.0f, 1.0f, 0.0f)  // Up direction
    );

    gridObject grid;

    do {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // P1aTask1 - Draw the grid and the coordinate axes.
        grid.draw(viewMatrix, projectionMatrix);

        glfwSwapBuffers(window);
        glfwPollEvents();

    } // Check if the ESC key was pressed or the window was closed
    while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0);

    glfwTerminate();
    return 0;
}

// main_task2.cpp - P1aTask2: the orbiting camera, drawn against the Task 1 grid.
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <cstdio>
#include "gridObject.hpp"
#include "window.hpp"

// The object that is currently selected. 0 is the camera.
int currSelected = 0;

int main() {
    if (initWindow("Pongpeeradech,Suprawee(34287548) - Task 2: Camera Rotations") != 0) return -1;

    // Projection matrix : 45 degree Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    glm::mat4 projectionMatrix = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);

    gridObject grid;

    // P1aTask2 - Camera angles. The camera orbits the origin on a sphere of fixed
    // radius: theta runs along the blue circle parallel to the equator, phi runs
    // along the red circle orthogonal to it.
    float cameraRadius = 16.0f;
    float cameraTheta = glm::radians(45.0f);
    float cameraPhi = glm::radians(30.0f);
    const float cameraSpeed = glm::radians(90.0f); // radians per second

    double lastTime = glfwGetTime();
    double lastFrameTime = glfwGetTime();
    int nbFrames = 0;
    do {
        // Timing
        double currentTime = glfwGetTime();
        // Seconds since the previous frame, so camera motion is frame rate independent.
        float deltaTime = float(currentTime - lastFrameTime);
        lastFrameTime = currentTime;
        nbFrames++;
        if (currentTime - lastTime >= 1.0){ // If last prinf() was more than 1sec ago
            printf("%f ms/frame\n", 1000.0 / double(nbFrames));
            nbFrames = 0;
            lastTime += 1.0;
        }

        // P1aTask2 - Press C to select the camera.
        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
            currSelected = 0;
        }

        // P1aTask2 - Left/Right walk the camera along the blue equatorial circle.
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS && currSelected == 0) {
            cameraTheta -= cameraSpeed * deltaTime;
        }
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS && currSelected == 0) {
            cameraTheta += cameraSpeed * deltaTime;
        }

        // P1aTask2 - Up/Down rotate the camera along the red orthogonal circle.
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS && currSelected == 0) {
            cameraPhi += cameraSpeed * deltaTime;
        }
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS && currSelected == 0) {
            cameraPhi -= cameraSpeed * deltaTime;
        }

        // P1aTask2 - Create the view matrix based on camera angles.
        glm::vec3 cameraPosition(
            cameraRadius * std::cos(cameraPhi) * std::sin(cameraTheta),
            cameraRadius * std::sin(cameraPhi),
            cameraRadius * std::cos(cameraPhi) * std::cos(cameraTheta)
        );
        // The up vector is the tangent to the red orbit at the camera's position.
        // It is perpendicular to the view direction for every phi, so the camera
        // keeps pointing at the origin even when it passes over the poles, where a
        // fixed (0,1,0) up would collapse and make lookAt degenerate.
        glm::vec3 cameraUp(
            -std::sin(cameraPhi) * std::sin(cameraTheta),
             std::cos(cameraPhi),
            -std::sin(cameraPhi) * std::cos(cameraTheta)
        );
        glm::mat4 viewMatrix = glm::lookAt(
            cameraPosition,   // Camera position
            glm::vec3(0.0f),  // Look at the origin
            cameraUp          // Up direction, tangent to the orbit
        );

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

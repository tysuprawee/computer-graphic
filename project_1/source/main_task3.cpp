#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <cmath>
#include <cstdio>
#include "meshObject.hpp"
#include "gridObject.hpp"
#include "window.hpp"

// Function prototypes
int getPickedId();

// The object that is currently selected for P1bTask4
int currSelected = 0;

int main() {
    // ATTN: REFER TO https://learnopengl.com/Getting-started/Creating-a-window
    // AND https://learnopengl.com/Getting-started/Hello-Window to familiarize yourself with the initialization of a window in OpenGL
    
    if (initWindow("Pongpeeradech,Suprawee(34287548) - Task 3: Robot Arm") != 0) return -1;

    glm::mat4 projectionMatrix = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);
    // Or, for Project 2, use an ortho camera :
    // gProjectionMatrix = glm::ortho(-4.0f, 4.0f, -3.0f, 3.0f, 0.0f, 100.0f); // In world coordinates

    
    gridObject grid;
    meshObject base("base.obj");
    meshObject arm1("arm1.obj");
    meshObject joint("joint.obj");
    meshObject arm2("arm2.obj");

    const float baseTop = 1.462f;
    const float arm1Len = 2.082f;

    const float j1 = -43.0f;
    const float j2 = -46.9f;
    const float j3 = -80.8f;

    const glm::vec3 xAxis(1.0f, 0.0f, 0.0f);
    const glm::vec3 yAxis(0.0f, 1.0f, 0.0f);

    base.rotate(j1, yAxis);

    arm1.rotate(j1, yAxis);
    arm1.translate(glm::vec3(0.0f, baseTop, 0.0f));
    arm1.rotate(j2, xAxis);

    joint.rotate(j1, yAxis);
    joint.translate(glm::vec3(0.0f, baseTop, 0.0f));
    joint.rotate(j2, xAxis);
    joint.translate(glm::vec3(0.0f, arm1Len, 0.0f));

    arm2.rotate(j1, yAxis);
    arm2.translate(glm::vec3(0.0f, baseTop, 0.0f));
    arm2.rotate(j2, xAxis);
    arm2.translate(glm::vec3(0.0f, arm1Len, 0.0f));
    arm2.rotate(j3, xAxis);

    // TODO: P1bTask4 - Create a hierarchical structure and adjust the relative translations.

    float cameraRadius = 16.0f;
    float cameraTheta = glm::radians(45.0f);
    float cameraPhi = glm::radians(30.0f);
    const float cameraSpeed = glm::radians(90.0f);
    
    //TODO: P1bTask5 - Create variables to store lighting info.
    
    
    double lastTime = glfwGetTime();
    double lastFrameTime = glfwGetTime();
    int nbFrames = 0;
    do {
        
        // Timing
        double currentTime = glfwGetTime();
        float deltaTime = float(currentTime - lastFrameTime);
        lastFrameTime = currentTime;
        nbFrames++;
        if (currentTime - lastTime >= 1.0){ // If last prinf() was more than 1sec ago
            printf("%f ms/frame\n", 1000.0 / double(nbFrames));
            nbFrames = 0;
            lastTime += 1.0;
        }
        
        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
            currSelected = 0;
        }

        //TODO: P1bTask4 - On key press set currSelected to the id of the robot piece to select.
        
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS && currSelected == 0) {
            cameraTheta -= cameraSpeed * deltaTime;
        }
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS && currSelected == 0) {
            cameraTheta += cameraSpeed * deltaTime;
        }
        
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS && currSelected == 0) {
            cameraPhi += cameraSpeed * deltaTime;
        }
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS && currSelected == 0) {
            cameraPhi -= cameraSpeed * deltaTime;
        }
        
        //TODO: P1bTask4 - On key press, based on currSelected, make appropriate transformation.

        glm::vec3 cameraPosition(
            cameraRadius * std::cos(cameraPhi) * std::sin(cameraTheta),
            cameraRadius * std::sin(cameraPhi),
            cameraRadius * std::cos(cameraPhi) * std::cos(cameraTheta)
        );
        glm::vec3 cameraUp(
            -std::sin(cameraPhi) * std::sin(cameraTheta),
             std::cos(cameraPhi),
            -std::sin(cameraPhi) * std::cos(cameraTheta)
        );
        glm::mat4 viewMatrix = glm::lookAt(
            cameraPosition,   // Camera position
            glm::vec3(0.0f),  // Look at the origin
            cameraUp
        );
        
        // Draw picking for P1bBonus2
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT)){
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            //TODO: P1bBonus2 draw all robort arm pieces using drawPicking function
            currSelected = getPickedId();
            
            std::cout << "Picked id: " << currSelected << std::endl;
            
            //TODO: P1bBonus2 - meshObject::getMeshObjectById can be used to get the picked object.
        }
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // DRAWING the SCENE
        
        grid.draw(viewMatrix, projectionMatrix);
        base.draw(viewMatrix, projectionMatrix);
        arm1.draw(viewMatrix, projectionMatrix);
        joint.draw(viewMatrix, projectionMatrix);
        arm2.draw(viewMatrix, projectionMatrix);
        
        //TODO: P1bTask4 - Draw the robot arm pieces using the hierachy instead. Call the draw function on the root node. The remeaining pieces will be drawn using recursive calls.

        //TODO: P1bTask5 - Pass the lighting info to the draw function.
        
        glfwSwapBuffers(window);
        glfwPollEvents();

    } // Check if the ESC key was pressed or the window was closed
    while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
    glfwWindowShouldClose(window) == 0);

    glfwTerminate();
    return 0;
}

// Initialize GLFW and create a window

int getPickedId(){
    glFlush();
    // --- Wait until all the pending drawing commands are really done.
    // Ultra-mega-over slow !
    // There are usually a long time between glDrawElements() and
    // all the fragments completely rasterized.
    glFinish();
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    unsigned char data[4] = {0, 0, 0, 0};

    //TODO: P1bBonus2 - Use glfwGetCursorPos to get the x and y value of the cursor.
    
    //TODO: P1bBonus2 - Use glfwGetFramebufferSize and glfwGetWindowSize to get the frame buffer size and window size. On high resolution displays, these sizes might be different.
    
    
    //TODO: P1bBonus2 - Use glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data) to read the pixel data.
    // Note that y position has to be flipped as glfwGetCursorPos gives the cursor position relative to top left of the screen. The read location must also be multiplied by (buffer size / windowSize) for some displays.
    
    int pickedId = data[0];
    return pickedId;
}

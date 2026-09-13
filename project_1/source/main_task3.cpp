// main_task3.cpp - P1aTask3: the robot arm, built on the Task 1 grid and
// the Task 2 orbiting camera.
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

    // Projection matrix : 45 degree Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    // Note: glm::perspective expects the FoV in radians.
    glm::mat4 projectionMatrix = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);
    // Or, for Project 2, use an ortho camera :
    // gProjectionMatrix = glm::ortho(-4.0f, 4.0f, -3.0f, 3.0f, 0.0f, 100.0f); // In world coordinates

    
    // P1aTask3 - Create each robot arm piece. Every .obj is modelled in its rest
    // pose: standing along +Y with the object origin at the pivot end, so the
    // joint angles below are pure rotations about that pivot.
    gridObject grid;
    meshObject base("base.obj");
    meshObject arm1("arm1.obj");
    meshObject joint("joint.obj");
    meshObject arm2("arm2.obj");

    // Dimensions measured from the .obj files (obj space is Y up).
    const float baseTop = 1.462f; // top face of the truncated tetrahedron
    const float arm1Len = 2.082f; // hinge to far end of arm 1

    // Joint angles in degrees, matching J1/J2/J3 in the assignment figure.
    // P1bTask4 drives these from the keyboard, so they are rebuilt every frame.
    float j1 = -43.0f; // yaw of the whole arm about the vertical axis
    float j2 = -46.9f; // shoulder, hinged at the centre of the base
    float j3 = -80.8f; // elbow, hinged at the centre of the joint
    const float jointSpeed = 60.0f; // degrees per second

    const glm::vec3 xAxis(1.0f, 0.0f, 0.0f);
    const glm::vec3 yAxis(0.0f, 1.0f, 0.0f);

    // TODO: P1bTask4 - Create a hierarchical structure and adjust the relative translations.

    // P1aTask2 - Camera angles. The camera orbits the origin on a sphere of fixed
    // radius: theta runs along the blue circle parallel to the equator, phi runs
    // along the red circle orthogonal to it.
    float cameraRadius = 16.0f;
    float cameraTheta = glm::radians(45.0f);
    float cameraPhi = glm::radians(30.0f);
    const float cameraSpeed = glm::radians(90.0f); // radians per second
    
    //TODO: P1bTask5 - Create variables to store lighting info.
    
    
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

        // P1bTask4 - On key press set currSelected to the id of the robot piece
        // to select. Each selectable piece owns one of the three joint angles;
        // the joint sphere itself has no degree of freedom of its own.
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) currSelected = base.getId();
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) currSelected = arm1.getId();
        if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) currSelected = arm2.getId();

        base.setSelected(currSelected == base.getId());
        arm1.setSelected(currSelected == arm1.getId());
        arm2.setSelected(currSelected == arm2.getId());
        
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
        
        // P1bTask4 - Based on currSelected, make the appropriate transformation.
        // The same arrow keys steer the camera or a joint depending on the
        // selection, which is why the camera blocks above test currSelected == 0.
        const bool keyLeft  = glfwGetKey(window, GLFW_KEY_LEFT)  == GLFW_PRESS;
        const bool keyRight = glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS;
        const bool keyUp    = glfwGetKey(window, GLFW_KEY_UP)    == GLFW_PRESS;
        const bool keyDown  = glfwGetKey(window, GLFW_KEY_DOWN)  == GLFW_PRESS;

        if (currSelected == base.getId()) {        // J1 - yaw the whole arm
            if (keyLeft)  j1 -= jointSpeed * deltaTime;
            if (keyRight) j1 += jointSpeed * deltaTime;
        } else if (currSelected == arm1.getId()) { // J2 - shoulder
            if (keyUp)   j2 += jointSpeed * deltaTime;
            if (keyDown) j2 -= jointSpeed * deltaTime;
        } else if (currSelected == arm2.getId()) { // J3 - elbow
            if (keyUp)   j3 += jointSpeed * deltaTime;
            if (keyDown) j3 -= jointSpeed * deltaTime;
        }

        // P1aTask3 - Translate each robot arm piece to its appropriate location.
        // Each matrix is built as T(pivot) * R * T(offset), so a rotation always
        // happens about its own pivot rather than about the world origin. The
        // repeated prefixes are exactly the parent transforms that P1bTask4 will
        // factor out into a hierarchy. Rebuilt each frame from the live angles.
        base.resetTransform();
        arm1.resetTransform();
        joint.resetTransform();
        arm2.resetTransform();

        // base: J1 spins the base itself about the vertical axis. Everything
        // else stands on the base, so Ry(j1) is the shared prefix of every
        // chain below and the whole arm turns with it.
        base.rotate(j1, yAxis);

        // arm1: hinged at the centre of the base.
        arm1.rotate(j1, yAxis);
        arm1.translate(glm::vec3(0.0f, baseTop, 0.0f));
        arm1.rotate(j2, xAxis);

        // joint: rides on arm1, parked at arm1's far end.
        joint.rotate(j1, yAxis);
        joint.translate(glm::vec3(0.0f, baseTop, 0.0f));
        joint.rotate(j2, xAxis);
        joint.translate(glm::vec3(0.0f, arm1Len, 0.0f));

        // arm2: connected to the centre of the joint, hinged there.
        arm2.rotate(j1, yAxis);
        arm2.translate(glm::vec3(0.0f, baseTop, 0.0f));
        arm2.rotate(j2, xAxis);
        arm2.translate(glm::vec3(0.0f, arm1Len, 0.0f));
        arm2.rotate(j3, xAxis);

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
        
        // P1aTask3 - Draw all robot arm pieces.
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

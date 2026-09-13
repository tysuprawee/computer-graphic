// main.cpp
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <cmath>
#include "meshObject.hpp"
#include "gridObject.hpp"

// Function prototypes
int initWindow(void);
static void mouseCallback(GLFWwindow*, int, int, int);
int getPickedId();


const GLuint windowWidth = 1024, windowHeight = 768;
GLFWwindow* window;

// The object that is currently selected for P1bTask4
int currSelected = 0;


int main() {
    // ATTN: REFER TO https://learnopengl.com/Getting-started/Creating-a-window
    // AND https://learnopengl.com/Getting-started/Hello-Window to familiarize yourself with the initialization of a window in OpenGL
    
    if (initWindow() != 0) return -1;

    // Projection matrix : 45 degree Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    // Note: glm::perspective expects the FoV in radians.
    glm::mat4 projectionMatrix = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);
    // Or, for Project 2, use an ortho camera :
    // gProjectionMatrix = glm::ortho(-4.0f, 4.0f, -3.0f, 3.0f, 0.0f, 100.0f); // In world coordinates

    
    //TODO: P1aTask3 - Create each robot arm piece by creating instances class meshObject.
    gridObject grid;
    // TODO: P1aTask3 - Translate each robot arm piece to its approapriate location
    
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
        
        // Draw picking for P1bBonus2
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT)){
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            //TODO: P1bBonus2 draw all robort arm pieces using drawPicking function
            currSelected = getPickedId();
            
            std::cout << "Picked id: " << currSelected << std::endl;
            
            //TODO: P1bBonus2 - meshObject::getMeshObjectById can be used to get the picked object.
        }
        
        //TODO: P1bTask4 - On key press set currSelected to the id of the robot piece to select.
        
        //TODO: P1bTask4 - On key press, based on currSelected, make appropriate transformation.
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // DRAWING the SCENE
        
        //TODO: P1aTask3 - Draw all robot arm pieces.
        grid.draw(viewMatrix, projectionMatrix);
        
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
int initWindow() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // FOR MAC

    window = glfwCreateWindow(windowWidth, windowHeight, "Pongpeeradech,Suprawee(34287548)", NULL, NULL);
    if (window == NULL) {
        fprintf(stderr, "Failed to open GLFW window.\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }
    
    // Set up inputs
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_FALSE);
    glfwSetCursorPos(window, windowWidth / 2, windowHeight / 2);
    glfwSetMouseButtonCallback(window, mouseCallback);
    
    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);
    // Cull triangles which normal is not towards the camera
    glEnable(GL_CULL_FACE);
    
    return 0;
}

static void mouseCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        std::cout << "Left mouse button pressed" << std::endl;
    }
}

int getPickedId(){
    glFlush();
    // --- Wait until all the pending drawing commands are really done.
    // Ultra-mega-over slow !
    // There are usually a long time between glDrawElements() and
    // all the fragments completely rasterized.
    glFinish();
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    unsigned char data[4];

    //TODO: P1bBonus2 - Use glfwGetCursorPos to get the x and y value of the cursor.
    
    //TODO: P1bBonus2 - Use glfwGetFramebufferSize and glfwGetWindowSize to get the frame buffer size and window size. On high resolution displays, these sizes might be different.
    
    
    //TODO: P1bBonus2 - Use glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data) to read the pixel data.
    // Note that y position has to be flipped as glfwGetCursorPos gives the cursor position relative to top left of the screen. The read location must also be multiplied by (buffer size / windowSize) for some displays.
    
    int pickedId = data[0];
    return pickedId;
}

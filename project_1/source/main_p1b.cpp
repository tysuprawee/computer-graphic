// main_p1b.cpp - Project 1b: hierarchical transformations and lighting.
//   Task 4  keyboard interaction   Task 5  two-light Phong shading
//   Task 6  teleporting            Bonus   picking and reaching
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

// Measured from the .obj files, which are modelled standing along +Y with the
// object origin at the pivot end.
static const float baseTop = 1.462f; // top face of the base, where arm 1 hinges
static const float arm1Len = 2.082f; // arm 1 hinge to the centre of the joint
static const float arm2Len = 1.414f; // joint centre to the tip of arm 2

// P1bTask6 - Constant downward acceleration, so the projectile follows a
// Newtonian arc. The launch speed is fixed by the assignment at the length of
// arm 2, which makes gravity the only thing that sets how far the solid can be
// thrown. At this value the reach is 7.01 units, so every grid point is within
// range, and the flight lasts about three seconds.
static const float gravity = 1.0f;

static const float solidScale = 0.55f;
static const float solidRestY = 0.476f * solidScale; // dodecahedron circumradius, scaled

// P1bTask6 - Where the solid lands, measured along the arm's own forward
// direction, for a given shoulder and elbow angle. Everything below happens in
// the base's local frame, where the arm swings in the y-z plane and "forward"
// is +z; the base's own Ry(J1) is what points that plane at the target.
static float landingDistance(float j2deg, float j3deg) {
    const float a = glm::radians(j2deg);
    const float b = glm::radians(j2deg + j3deg);

    const float jointH = arm1Len * std::sin(a);
    const float jointY = baseTop + arm1Len * std::cos(a);

    const float tipH = jointH + arm2Len * std::sin(b);
    const float tipY = jointY + arm2Len * std::cos(b);
    if (tipY < solidRestY) return -1.0e9f; // arm is through the floor

    // P1bTask6 - The solid leaves the tip along arm 2, at a speed equal to the
    // length of arm 2.
    const float velH = arm2Len * std::sin(b);
    const float velY = arm2Len * std::cos(b);

    // Solve tipY + velY t - g t^2 / 2 = solidRestY for the later root.
    const float A = -0.5f * gravity;
    const float B = velY;
    const float C = tipY - solidRestY;
    const float disc = B * B - 4.0f * A * C;
    if (disc < 0.0f) return -1.0e9f;

    const float root = std::sqrt(disc);
    const float t1 = (-B + root) / (2.0f * A);
    const float t2 = (-B - root) / (2.0f * A);
    const float t = std::max(t1, t2);
    if (t <= 0.0f) return -1.0e9f;

    return tipH + velH * t;
}

// P1bBonus - Reaching. Inverse dynamics: search the shoulder and elbow angles
// for a pose whose launch lands the solid at the requested distance. One
// equation in two unknowns has a family of solutions, so the scan keeps the
// closest and breaks ties towards the smaller elbow bend.
static void solveReach(float targetDistance, float& j2, float& j3) {
    float bestCost = 1.0e9f;
    float bestJ2 = j2, bestJ3 = j3;

    float j2Lo = -80.0f, j2Hi = 80.0f, j3Lo = -170.0f, j3Hi = 170.0f, step = 2.0f;
    for (int pass = 0; pass < 4; ++pass) {
        for (float a = j2Lo; a <= j2Hi; a += step) {
            for (float b = j3Lo; b <= j3Hi; b += step) {
                const float d = landingDistance(a, b);
                if (d < -1.0e8f) continue;
                const float cost = std::fabs(d - targetDistance) + 1.0e-4f * std::fabs(b);
                if (cost < bestCost) { bestCost = cost; bestJ2 = a; bestJ3 = b; }
            }
        }
        // Tighten the window around the best pose so far and scan again.
        j2Lo = bestJ2 - step; j2Hi = bestJ2 + step;
        j3Lo = bestJ3 - step; j3Hi = bestJ3 + step;
        step *= 0.2f;
    }

    j2 = bestJ2;
    j3 = bestJ3;
}

int main() {
    if (initWindow("Pongpeeradech,Suprawee(34287548) - Project 1b") != 0) return -1;

    glm::mat4 projectionMatrix = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);

    gridObject grid;

    const std::string vs = "phongVertexShader.glsl";
    const std::string fs = "phongFragmentShader.glsl";
    meshObject base("base.obj", vs, fs);
    meshObject arm1("arm1.obj", vs, fs);
    meshObject joint("joint.obj", vs, fs);
    meshObject arm2("arm2.obj", vs, fs);

    // P1bTask6 - The projectile is a regular dodecahedron, so the same mesh the
    // joint uses serves as the Platonic solid.
    meshObject solid("joint.obj", vs, fs);
    meshObject marker("joint.obj", vs, fs);

    // P1bTask4 - Each piece carries the pieces mounted on it.
    base.addChild(&arm1);
    arm1.addChild(&joint);
    joint.addChild(&arm2);

    // P1bTask5 - Every object has its own colour, which is also its diffuse and
    // ambient material.
    base.setColor(glm::vec3(0.78f, 0.22f, 0.20f));
    arm1.setColor(glm::vec3(0.82f, 0.68f, 0.26f));
    joint.setColor(glm::vec3(0.44f, 0.54f, 0.70f));
    arm2.setColor(glm::vec3(0.35f, 0.64f, 0.46f));
    solid.setColor(glm::vec3(0.88f, 0.84f, 0.36f));
    marker.setColor(glm::vec3(0.90f, 0.35f, 0.75f));

    // P1bTask4 - Joint angles, driven from the keyboard.
    float j1 = -35.0f; // base yaw
    float j2 =  28.0f; // shoulder, hinged at the centre of the base
    float j3 = -62.0f; // elbow, hinged at the centre of the joint
    const float jointSpeed = 60.0f; // degrees per second

    // P1bTask4 - Where the whole model stands, slid on the XZ plane.
    glm::vec3 basePosition(0.0f);
    const float slideSpeed = 4.0f; // units per second

    // Arrow keys drive the base's translation when it was selected with T, and
    // its rotation when it was selected with R.
    bool baseTranslates = false;

    const glm::vec3 xAxis(1.0f, 0.0f, 0.0f);
    const glm::vec3 yAxis(0.0f, 1.0f, 0.0f);

    float cameraRadius = 16.0f;
    float cameraTheta = glm::radians(45.0f);
    float cameraPhi = glm::radians(30.0f);
    const float cameraSpeed = glm::radians(90.0f);

    // P1bTask6 - Projectile state.
    bool solidFlying = false;
    bool solidVisible = false;
    float solidSettle = 0.0f; // short pause between the landing and the teleport
    glm::vec3 solidPosition(0.0f);
    glm::vec3 solidVelocity(0.0f);

    // P1bBonus - The grid point the arm is asked to reach.
    bool markVisible = false;
    int markX = 3, markZ = 2;

    bool prevS = false, prevJ = false, prevK = false, prevI = false, prevM = false;
    bool prevMouse = false;

    double lastTime = glfwGetTime();
    double lastFrameTime = glfwGetTime();
    int nbFrames = 0;
    do {
        double currentTime = glfwGetTime();
        float deltaTime = float(currentTime - lastFrameTime);
        lastFrameTime = currentTime;
        nbFrames++;
        if (currentTime - lastTime >= 1.0){
            printf("%f ms/frame\n", 1000.0 / double(nbFrames));
            nbFrames = 0;
            lastTime += 1.0;
        }

        // P1bTask4 - On key press set currSelected to the id of the robot piece
        // to select. T and R both select the base, and decide whether the arrow
        // keys slide it or turn it.
        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) currSelected = 0;
        if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) { currSelected = base.getId(); baseTranslates = true;  }
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) { currSelected = base.getId(); baseTranslates = false; }
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) currSelected = arm1.getId();
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) currSelected = arm2.getId();

        base.setSelected(currSelected == base.getId());
        arm1.setSelected(currSelected == arm1.getId());
        joint.setSelected(currSelected == joint.getId());
        arm2.setSelected(currSelected == arm2.getId());

        const bool keyLeft  = glfwGetKey(window, GLFW_KEY_LEFT)  == GLFW_PRESS;
        const bool keyRight = glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS;
        const bool keyUp    = glfwGetKey(window, GLFW_KEY_UP)    == GLFW_PRESS;
        const bool keyDown  = glfwGetKey(window, GLFW_KEY_DOWN)  == GLFW_PRESS;

        // P1bTask4 - On key press, based on currSelected, make the appropriate
        // transformation. The same arrow keys drive the camera or one joint,
        // depending on what is selected.
        if (currSelected == 0) {
            if (keyLeft)  cameraTheta -= cameraSpeed * deltaTime;
            if (keyRight) cameraTheta += cameraSpeed * deltaTime;
            if (keyUp)    cameraPhi   += cameraSpeed * deltaTime;
            if (keyDown)  cameraPhi   -= cameraSpeed * deltaTime;
        } else if (currSelected == base.getId()) {
            if (baseTranslates) {
                // The whole model slides on the XZ plane.
                if (keyLeft)  basePosition.x -= slideSpeed * deltaTime;
                if (keyRight) basePosition.x += slideSpeed * deltaTime;
                if (keyUp)    basePosition.z -= slideSpeed * deltaTime;
                if (keyDown)  basePosition.z += slideSpeed * deltaTime;
            } else {
                // J1 - the base, joint and arms all turn about Y.
                if (keyLeft)  j1 -= jointSpeed * deltaTime;
                if (keyRight) j1 += jointSpeed * deltaTime;
            }
        } else if (currSelected == arm1.getId()) {
            if (keyUp)   j2 += jointSpeed * deltaTime; // J2
            if (keyDown) j2 -= jointSpeed * deltaTime;
        } else if (currSelected == arm2.getId()) {
            if (keyUp)   j3 += jointSpeed * deltaTime; // J3
            if (keyDown) j3 -= jointSpeed * deltaTime;
        }

        // P1bBonus - Reaching. j/k/i/m walk a mark over the grid points, and
        // each move re-aims the arms so that S will land the solid on it. Only
        // the arms and the base's rotation move; the body stays where it is.
        const bool keyJ = glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS;
        const bool keyK = glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS;
        const bool keyI = glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS;
        const bool keyM = glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS;
        bool markMoved = false;
        if (keyJ && !prevJ) { markX = std::max(markX - 1, -5); markMoved = true; }
        if (keyK && !prevK) { markX = std::min(markX + 1,  5); markMoved = true; }
        if (keyI && !prevI) { markZ = std::max(markZ - 1, -5); markMoved = true; }
        if (keyM && !prevM) { markZ = std::min(markZ + 1,  5); markMoved = true; }
        prevJ = keyJ; prevK = keyK; prevI = keyI; prevM = keyM;

        if (markMoved) {
            markVisible = true;
            const float dx = float(markX) - basePosition.x;
            const float dz = float(markZ) - basePosition.z;
            j1 = glm::degrees(std::atan2(dx, dz)); // point the arm's plane at the mark
            solveReach(std::sqrt(dx * dx + dz * dz), j2, j3);
        }

        // P1bTask4 - Rebuild the hierarchy's local transforms from the current
        // angles. Each one is relative to its parent, so a piece automatically
        // carries everything mounted on it.
        base.resetTransform();
        base.translate(basePosition);
        base.rotate(j1, yAxis);

        arm1.resetTransform();
        arm1.translate(glm::vec3(0.0f, baseTop, 0.0f));
        arm1.rotate(j2, xAxis);

        joint.resetTransform();
        joint.translate(glm::vec3(0.0f, arm1Len, 0.0f));

        arm2.resetTransform();
        arm2.rotate(j3, xAxis);

        // The tip of arm 2, and the direction it points, in world space.
        const glm::mat4 baseWorld  = base.getLocalTransform();
        const glm::mat4 arm1World  = baseWorld * arm1.getLocalTransform();
        const glm::mat4 jointWorld = arm1World * joint.getLocalTransform();
        const glm::mat4 arm2World  = jointWorld * arm2.getLocalTransform();
        const glm::vec3 tip = glm::vec3(arm2World * glm::vec4(0.0f, arm2Len, 0.0f, 1.0f));
        const glm::vec3 tipDirection = glm::normalize(glm::vec3(arm2World * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f)));

        // P1bTask6 - S sends the solid out of the tip of arm 2, along arm 2, at
        // a speed equal to the length of arm 2.
        const bool keyS = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
        if (keyS && !prevS) {
            solidPosition = tip;
            solidVelocity = tipDirection * arm2Len;
            solidFlying = true;
            solidVisible = true;
        }
        prevS = keyS;

        // P1bTask6 - Newton's law under gravity, until it hits the grid.
        if (solidFlying) {
            solidVelocity.y -= gravity * deltaTime;
            solidPosition += solidVelocity * deltaTime;
            if (solidPosition.y <= solidRestY) {
                solidPosition.y = solidRestY;
                solidFlying = false;
                solidSettle = 0.45f;
            }
        } else if (solidSettle > 0.0f) {
            // On impact, move the robot arm to the impact location. Holding off
            // for a moment leaves the landing visible before the arm covers it.
            solidSettle -= deltaTime;
            if (solidSettle <= 0.0f) {
                basePosition.x = solidPosition.x;
                basePosition.z = solidPosition.z;
            }
        }

        solid.resetTransform();
        solid.translate(solidPosition);
        solid.scale(solidScale);

        marker.resetTransform();
        marker.translate(glm::vec3(float(markX), solidRestY * 0.5f, float(markZ)));
        marker.scale(0.22f);

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

        // P1bTask5 - Two lights, held beside the camera so one comes from the
        // left and the other from the right.
        const glm::vec3 viewDirection = glm::normalize(-cameraPosition);
        const glm::vec3 cameraRight = glm::normalize(glm::cross(viewDirection, cameraUp));
        Light lights[2];
        lights[0].position  = cameraPosition - cameraRight * 7.0f + cameraUp * 2.0f;
        lights[0].ambient   = glm::vec3(0.09f, 0.09f, 0.11f);
        lights[0].diffuse   = glm::vec3(0.58f, 0.52f, 0.44f); // warm, from the left
        lights[0].specular  = glm::vec3(1.0f);
        lights[1].position  = cameraPosition + cameraRight * 7.0f + cameraUp * 2.0f;
        lights[1].ambient   = glm::vec3(0.07f, 0.08f, 0.11f);
        lights[1].diffuse   = glm::vec3(0.36f, 0.44f, 0.58f); // cool, from the right
        lights[1].specular  = glm::vec3(1.0f);

        // P1bBonus - Picking. Draw every piece in a colour that encodes its id,
        // read the pixel under the cursor, then throw the frame away and draw
        // the scene properly, so none of this is ever seen.
        const bool mouseDown = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
        if (mouseDown && !prevMouse){
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            base.drawPicking(glm::mat4(1.0f), viewMatrix, projectionMatrix);
            const int picked = getPickedId();
            if (meshObject::getMeshObjectById(picked) != nullptr) {
                currSelected = picked;
                std::cout << "Picked id: " << currSelected << std::endl;
            } else {
                currSelected = 0; // clicked the background, back to the camera
            }
        }
        prevMouse = mouseDown;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // DRAWING the SCENE

        grid.draw(viewMatrix, projectionMatrix);

        // P1bTask4 - Draw the robot arm pieces using the hierarchy. One call on
        // the root draws the rest through recursive calls.
        // P1bTask5 - Pass the lighting info to the draw function.
        base.draw(glm::mat4(1.0f), viewMatrix, projectionMatrix, lights, 2, cameraPosition);

        if (markVisible)  marker.draw(glm::mat4(1.0f), viewMatrix, projectionMatrix, lights, 2, cameraPosition);
        if (solidVisible) solid.draw(glm::mat4(1.0f), viewMatrix, projectionMatrix, lights, 2, cameraPosition);

        glfwSwapBuffers(window);
        glfwPollEvents();

    } // Check if the ESC key was pressed or the window was closed
    while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
    glfwWindowShouldClose(window) == 0);

    glfwTerminate();
    return 0;
}

int getPickedId(){
    glFlush();
    // --- Wait until all the pending drawing commands are really done.
    // Ultra-mega-over slow !
    // There are usually a long time between glDrawElements() and
    // all the fragments completely rasterized.
    glFinish();

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    unsigned char data[4] = {0, 0, 0, 0};

    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    // On a high resolution display the framebuffer is larger than the window,
    // so the cursor position has to be scaled by the ratio between them.
    int fbWidth, fbHeight, winWidth, winHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    glfwGetWindowSize(window, &winWidth, &winHeight);
    const double scaleX = double(fbWidth) / double(winWidth);
    const double scaleY = double(fbHeight) / double(winHeight);

    // glfwGetCursorPos measures from the top left and glReadPixels from the
    // bottom left, so the y position has to be flipped.
    const int readX = int(xpos * scaleX);
    const int readY = int((double(winHeight) - ypos) * scaleY);

    glReadPixels(readX, readY, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);

    int pickedId = data[0];
    return pickedId;
}

#ifndef window_hpp
#define window_hpp

#include <GL/glew.h>
#include <GLFW/glfw3.h>

// Shared by every task executable so the boilerplate lives in one place.
extern const GLuint windowWidth, windowHeight;
extern GLFWwindow* window;

// Creates the window and GL context. Returns 0 on success.
int initWindow(const char* title);

#endif

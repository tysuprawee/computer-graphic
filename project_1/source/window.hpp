#ifndef window_hpp
#define window_hpp

#include <GL/glew.h>
#include <GLFW/glfw3.h>

extern const GLuint windowWidth, windowHeight;
extern GLFWwindow* window;

int initWindow(const char* title);

#endif

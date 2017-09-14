// Include standard headers
#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <glm/gtc/matrix_transform.hpp>
#include "../src/uniforms.h"

#include "../src/program.h"
#include "../src/args.h"
#include "../src/window.h"
#include "../src/camera.h"

static GLfloat vCoordinates[] = {
    1.0f, -1.0f, 0.0f,
    1.0f, 1.0f, 0.0f,
    -1.0f, 1.0f, 0.0f,
    -1.0f, -1.0f, 0.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, 1.0f, -1.0f,
    -1.0f, 1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f
};

static GLfloat vColors[] = {
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f
};

static GLubyte vIndices[] = {
    0, 1, 2,
    2, 3, 0,
    4, 6, 5,
    4, 7, 6,
    2, 7, 3,
    7, 6, 2,
    0, 4, 1,
    4, 1, 5,
    6, 2, 1,
    1, 6, 5,
    0, 3, 7,
    0, 7, 4
};

int main() {
  gl::Window window("Cube", 640, 480);

  gl::Camera camera(window.width(), window.height());
  camera.SwitchInteraction(true);
  gl::Program program("../shader/cube_vertex.glsl",
                      "../shader/cube_fragment.glsl");
  gl::Uniforms uniforms;
  uniforms.GetLocation(program.id(), "mvp", gl::kMatrix4f);

  gl::Args args(3);
  args.BindBuffer(0, {GL_ARRAY_BUFFER, sizeof(float), 3, GL_FLOAT},
                  sizeof(vCoordinates) / sizeof(vCoordinates[0]),
                  vCoordinates);
  args.BindBuffer(1, {GL_ARRAY_BUFFER, sizeof(float), 3, GL_FLOAT},
                  sizeof(vColors) / sizeof(vColors[0]),
                  vColors);
  args.BindBuffer(2, {GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned char),
                       1, GL_UNSIGNED_BYTE},
                  sizeof(vIndices) / sizeof(vIndices[0]),
                  vIndices);

  // Set the mouse at the center of the screen
  glfwPollEvents();
  glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  do {
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    camera.SetView(window);
    glm::mat4 mvp = camera.mvp();

    glUseProgram(program.id());
    uniforms.Bind("mvp", &mvp, 1);
    glBindVertexArray(args.vao());
    glDrawElements(GL_TRIANGLES, sizeof(vIndices) / sizeof(vIndices[0]),
                   GL_UNSIGNED_BYTE, 0);
    glBindVertexArray(0);

    window.swap_buffer();
  } while( window.get_key(GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
           window.should_close() == 0);

  // Close OpenGL window and terminate GLFW
  glfwTerminate();

  return 0;
}


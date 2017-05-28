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

#include "utils/context.h"
#include "utils/control.h"
#include "utils/shader.h"

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

int main( void ) {
  // GLFW window initialization, GLEW initialization
  Context context("Test");

  // Mouse and Key initialzation
  Control control(context.window());

  // Create and compile our GLSL program from the shaders
  GLuint program_id = LoadShaders("vertex_vntf.glsl", "fragment_vntf.glsl");
  GLint matrix_id = glGetUniformLocation(program_id, "mvp");

  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  GLuint vbos[3];
  glGenBuffers(3, vbos);

  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vCoordinates), vCoordinates,
               GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);

  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, vbos[1]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vColors), vColors,
               GL_STATIC_DRAW);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos[2]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(vIndices), vIndices,
               GL_STATIC_DRAW);


  // Set the mouse at the center of the screen
  glfwPollEvents();
  glfwSetCursorPos(context.window(), 1024/2, 768/2);

  glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  do {
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(program_id);
    control.ComputeMatricesFromInputs();
    glm::mat4 MVP = control.projection_mat() * control.view_mat() * glm::mat4(1.0f);
    glUniformMatrix4fv(matrix_id, 1, GL_FALSE, &MVP[0][0]);
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, sizeof(vIndices) / sizeof(vIndices[0]),
                   GL_UNSIGNED_BYTE, 0);

    // Swap buffers
    glfwSwapBuffers(context.window());
    glfwPollEvents();

  } while( glfwGetKey(context.window(), GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
           glfwWindowShouldClose(context.window()) == 0 );

  // Cleanup VBO
  glDeleteBuffers(3, vbos);
  glDeleteVertexArrays(1, &vao);
  glDeleteProgram(program_id);

  // Close OpenGL window and terminate GLFW
  glfwTerminate();

  return 0;
}


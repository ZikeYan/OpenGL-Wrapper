//
// Created by Neo on 16/7/29.
//

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

#include "GLObjects/plane.h"
#include "GLObjects/camera.h"

int main( void ) {
  // Context and control init
  Context context("F-16");
  Control control(context.window());

  // Foreach GLObject do Init()
  Plane f16 = Plane("../shader/vertex_plane.glsl",
                "../shader/fragment_plane.glsl",
                "../obj/f16.obj", "../obj/f16.bmp");
  f16.Init();

  Camera camera = Camera("../shader/vertex_camera.glsl",
                         "../shader/fragment_camera.glsl");
  camera.Init();

  // Additional settings
  glfwPollEvents();
  glfwSetCursorPos(context.window(), 1024/2, 768/2);

  glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  do {
    // Update control
    control.UpdateCameraPose();
    glm::mat4 mvp = control.projection_mat() *
        control.view_mat() *
        glm::mat4(1.0f);
    glm::mat4 v = control.view_mat();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Foreach GLObject do data updating and Render();
    f16.set_mvp(&mvp[0][0]);
    f16.set_v(&v[0][0]);
    f16.Render();

    camera.set_mvp(&mvp[0][0]);
    camera.UpdateData(v);
    camera.Render();

    // Additional operations
    glfwSwapBuffers(context.window());
    glfwPollEvents();

  } while( glfwGetKey(context.window(), GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
           glfwWindowShouldClose(context.window()) == 0 );

  // Close OpenGL window and terminate GLFW
  glfwTerminate();
  return 0;
}


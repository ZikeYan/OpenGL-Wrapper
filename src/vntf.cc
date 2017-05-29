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
#include "utils/model.h"
#include "utils/texture.h"

#include "gl_objects/gl_object.h"
#include "gl_objects/vntf_demo.h"

const int kWidth  = 640;
const int kHeight = 480;
int main( void ) {
  // Context and control init
  Context context("F-16", kWidth, kHeight);
  VNTFDemo vntf_mesh = VNTFDemo(&context);

  // Additional settings
  glfwPollEvents();

  glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  do
  {
    // Update control
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    vntf_mesh.Render();
    glfwSwapBuffers(context.window());
    glfwPollEvents();

    cv::Mat m = context.CaptureDepth();
    cv::flip(m, m, 0);
    cv::imshow("m", m);
  } while( glfwGetKey(context.window(), GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
           glfwWindowShouldClose(context.window()) == 0 );

  // Close OpenGL window and terminate GLFW
  glfwTerminate();
  return 0;
}
//
// Created by Neo on 28/05/2017.
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
#include "gl_objects/v_demo.h"

const int kWidth  = 640;
const int kHeight = 480;

void SimpleLoadPLYVertices(std::string path, std::vector<glm::vec3>& vertices) {
  std::ifstream in(path);
  int size; char line[100]; std::string dummy;
  for (int i = 0; i < 3; ++i) {
    in.getline(line, 100);
    std::cout << line << std::endl;
  }
  in >> dummy >> dummy >> size;
  std::cout << "Size: " << size << std::endl;
  for (int i = 0; i < 7; ++i) {
    in.getline(line, 100);
    std::cout << i << " " << line << std::endl;
  }
  vertices.resize(size);
  for (int i = 0; i < size; ++i) {
    in >> vertices[i].x >> vertices[i].y >> vertices[i].z;
  }
  in.close();
}

int main( void ) {
  // Context and control init
  std::vector<glm::vec3> vertices;
  SimpleLoadPLYVertices("/Users/Neo/Desktop/fused.ply", vertices);

  Context context("Mesh", kWidth, kHeight);
  VDemo v_mesh = VDemo(&context, vertices.size());

  do {
    cv::Mat I = cv::Mat::eye(cv::Size(4, 4), CV_32F);
    // If you want to control with your kbd, set true;
    // If you want input from data, set false
    v_mesh.Render((float*)vertices.data(), vertices.size(), I, true);

    cv::Mat m = context.CaptureDepth();
    cv::imshow("im", m);
  } while( glfwGetKey(context.window(), GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
           glfwWindowShouldClose(context.window()) == 0 );

  // Close OpenGL window and terminate GLFW
  glfwTerminate();
  return 0;
}
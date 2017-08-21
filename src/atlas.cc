//
// Created by Neo on 08/08/2017.
//

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <opencv2/opencv.hpp>
#include <utils/shader.h>
#include <program.h>
#include "utils/model.h"
#include "utils/texture.h"
#include "utils/context.h"
#include "utils/control.h"
#include "window.h"
#include "texture.h"
#include "args.h"

int main() {

  std::vector<glm::vec3> vertices_;
  std::vector<glm::vec3> normals_;
  std::vector<glm::vec2> uvs_;
  std::vector<unsigned int> indices_;

  LoadModel("../obj/f16.obj", vertices_, uvs_, normals_, indices_);
  std::cerr << vertices_.size() << " " << indices_.size()/3 << std::endl;

  gl::Texture texture;
  texture.Load("../obj/f16.bmp");

  // Context and control init
  gl::Window window("F-16", texture.width()/2, texture.height()/2);
  texture.Init();

  gl::Program program("../shader/vertex_atlas.glsl",
                      "../shader/fragment_atlas.glsl");
  gl::Args args(4);
  std::cout << uvs_.data() << std::endl;
  args.BindBuffer(0, {GL_ARRAY_BUFFER, sizeof(float), 2, GL_FLOAT},
                  uvs_.size(), uvs_.data());
  args.BindBuffer(1, {GL_ARRAY_BUFFER, sizeof(float), 3, GL_FLOAT},
                  vertices_.size(), vertices_.data());
  args.BindBuffer(2, {GL_ARRAY_BUFFER, sizeof(float), 3, GL_FLOAT},
                  normals_.size(), normals_.data());
  args.BindBuffer(3, {GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int),
                      1, GL_UNSIGNED_INT},
                  indices_.size(), indices_.data());

  // Additional settings
  glfwPollEvents();

  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  cv::Mat pixels;
  do {
    // Update control
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    texture.Bind(0);
    glUseProgram(program.id());
    glBindVertexArray(args.vao());
    glDrawElements(GL_TRIANGLES, indices_.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    window.swap_buffer();
    pixels = window.CaptureRGB();
  } while( window.get_key(GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
           window.should_close() == 0 );
  // Close OpenGL window and terminate GLFW
  glfwTerminate();

  return 0;
}
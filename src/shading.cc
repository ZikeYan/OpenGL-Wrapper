//
// Created by Neo on 08/08/2017.
//

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <opencv2/opencv.hpp>
#include <utils/shader.h>
#include <program.h>
#include <uniform.h>
#include "utils/model.h"
#include "utils/texture.h"
#include "utils/context.h"
#include "utils/control.h"
#include "window.h"
#include "texture.h"
#include "args.h"
#include "model.h"

int main() {
  gl::Model model;
  model.LoadObj("../obj/f16.obj");

  gl::Texture texture;
  texture.Load("../obj/f16.bmp");

  // Context and control init
  gl::Window window("F-16", texture.width()/2, texture.height()/2);
  texture.Init();

  gl::Program program("../shader/shading_vertex.glsl",
                      "../shader/shading_fragment.glsl");
  gl::Uniform uniform_light(program.id(), "light", gl::kVector3f);
  gl::Args args(4);
  args.BindBuffer(0, {GL_ARRAY_BUFFER, sizeof(float), 2, GL_FLOAT},
                  model.uvs().size(), model.uvs().data());
  args.BindBuffer(1, {GL_ARRAY_BUFFER, sizeof(float), 3, GL_FLOAT},
                  model.positions().size(), model.positions().data());
  args.BindBuffer(2, {GL_ARRAY_BUFFER, sizeof(float), 3, GL_FLOAT},
                  model.normals().size(), model.normals().data());
  args.BindBuffer(3, {GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int),
                      1, GL_UNSIGNED_INT},
                  model.indices().size(), model.indices().data());

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

    glm::vec3 light(2, 4, 2);
    uniform_light.Bind(&light);
    glBindVertexArray(args.vao());
    glDrawElements(GL_TRIANGLES, model.indices().size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    window.swap_buffer();
    pixels = window.CaptureRGB();
  } while( window.get_key(GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
           window.should_close() == 0 );
  // Close OpenGL window and terminate GLFW
  glfwTerminate();

  return 0;
}
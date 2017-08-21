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
#include <camera.h>
#include <uniform.h>
#include <program.h>

#include "utils/context.h"
#include "utils/control.h"
#include "utils/model.h"
#include "utils/texture.h"

#include "gl_objects/gl_object.h"
#include "gl_objects/vntf_demo.h"

#include "window.h"
#include "texture.h"
#include "args.h"
#include <glm/gtx/string_cast.hpp>

const int kWidth  = 640;
const int kHeight = 480;
int main( void ) {
  std::vector<glm::vec3> vertices_;
  std::vector<glm::vec3> normals_;
  std::vector<glm::vec2> uvs_;
  std::vector<unsigned int> indices_;
  LoadModel("../obj/f16.obj", vertices_, uvs_, normals_, indices_);
  std::cerr << vertices_.size() << " " << indices_.size()/3 << std::endl;

  gl::Texture texture;
  texture.Load("../obj/f16.bmp");

  // Context and control init
  gl::Window window("F-16", kWidth, kHeight);
  texture.Init();

  gl::Camera camera(45.0f, window.width(), window.height(), 0.01f, 1000.0f);
  camera.SwitchInteraction(true);
  glm::mat4 model = glm::mat4(1.0f);
  camera.set_model(model);

  gl::Program program("../shader/vertex_vntf.glsl",
                      "../shader/fragment_vntf.glsl");
  gl::Uniform uniform_mvp(program.id(), "mvp", gl::kMatrix4f);
  gl::Uniform uniform_view(program.id(), "c_T_w", gl::kMatrix4f);
  gl::Uniform uniform_tex(program.id(), "textureSampler", gl::kTexture2D);

  gl::Args args(4);
  std::cout << uvs_.data() << std::endl;
  args.BindBuffer(0, {GL_ARRAY_BUFFER, sizeof(float), 3, GL_FLOAT},
                  vertices_.size(), vertices_.data());
  args.BindBuffer(1, {GL_ARRAY_BUFFER, sizeof(float), 3, GL_FLOAT},
                  normals_.size(), normals_.data());
  args.BindBuffer(2, {GL_ARRAY_BUFFER, sizeof(float), 2, GL_FLOAT},
                  uvs_.size(), uvs_.data());
  args.BindBuffer(3, {GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int),
                      1, GL_UNSIGNED_INT},
                  indices_.size(), indices_.data());

  // Additional settings
  glfwPollEvents();
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  float y = 0.0f;
  float z = 0.0f;
  do {
    // Update control
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    camera.SetView(window);

    glUseProgram(program.id());
    texture.Bind(0);

    glm::mat4 projection = camera.projection();
    glm::mat4 mvp = camera.mvp();
    glm::mat4 view = camera.view();

    int tex_idx = 0;
    uniform_mvp.Bind(&mvp[0][0]);
    uniform_view.Bind(&view[0][0]);
    uniform_tex.Bind(&tex_idx);

    glBindVertexArray(args.vao());
    glDrawElements(GL_TRIANGLES, indices_.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    window.swap_buffer();
  } while( window.get_key(GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
           window.should_close() == 0 );

  // Close OpenGL window and terminate GLFW
  glfwTerminate();
  return 0;
}
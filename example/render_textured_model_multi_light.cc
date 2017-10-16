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

#include "../include/glwrapper.h"

const int kWidth  = 1280;
const int kHeight = 960;

int main() {
  std::vector<glm::vec3> light_src_positions = {
      glm::vec3(-10, 0, 10),
      glm::vec3(10, 0, 10),
      glm::vec3(0, 0, 20),
      glm::vec3(0, 0, -20),
      glm::vec3(0, 10, 0),
      glm::vec3(0, -10, 0),
      glm::vec3(-10, 0, 0),
      glm::vec3(10, 0, 0),
      glm::vec3(-15, 0, 5),
      glm::vec3(15, 0, 5),
      glm::vec3(0, 15, 5),
      glm::vec3(0, -15, 5),
  };
  std::vector<glm::vec3> light_src_colors = {
      glm::vec3(1, 1, 1),
      glm::vec3(1, 1, 1),
      glm::vec3(1, 1, 1),
      glm::vec3(1, 1, 1),
      glm::vec3(1, 1, 1),
      glm::vec3(1, 1, 1),
      glm::vec3(1, 1, 1),
      glm::vec3(1, 1, 1),
      glm::vec3(1, 1, 1),
      glm::vec3(1, 1, 1),
      glm::vec3(1, 1, 1),
      glm::vec3(1, 1, 1)
  };

  std::vector<glm::vec3> axises = {
      glm::vec3(0, 0, 0),
      glm::vec3(20, 0, 0),
      glm::vec3(0, 0, 0),
      glm::vec3(0, 20, 0),
      glm::vec3(0, 0, 0),
      glm::vec3(0, 0, 20)
  };

  std::vector<glm::vec3> axis_colors = {
      glm::vec3(1, 0, 0),
      glm::vec3(1, 0, 0),
      glm::vec3(0, 1, 0),
      glm::vec3(0, 1, 0),
      glm::vec3(0, 0, 1),
      glm::vec3(0, 0, 1)
  };

  float light_power = 40;
  glm::vec3 light_color = glm::vec3(1, 1, 1);
  std::stringstream ss;
  ss << light_src_positions.size();
  int light_cnt = light_src_colors.size();

  /// Load model and texture
  gl::Model model;
  model.LoadObj("../model/beethoven/beethoven.obj");
  gl::Texture texture;
  texture.Load("../model/beethoven/beethoven.png");

  // Context and control init
  gl::Window window("F16", kWidth, kHeight);
  gl::Camera camera(window.width(), window.height());
  camera.SwitchInteraction(true);

  glm::mat4 model_mat = glm::mat4(1.0f);
  //model_mat[1][1] = model_mat[2][2] = -1;
  camera.set_model(model_mat);

  gl::Program program;
  program.Load("../shader/textured_model_multi_light_vertex.glsl", gl::kVertexShader);
  program.ReplaceMacro("LIGHT_COUNT", ss.str(), gl::kVertexShader);
  program.Load("../shader/textured_model_multi_light_fragment.glsl", gl::kFragmentShader);
  program.ReplaceMacro("LIGHT_COUNT", ss.str(), gl::kFragmentShader);
  program.Build();

  texture.Init("../model/beethoven/beethoven.png");
  gl::Uniforms uniforms;
  uniforms.GetLocation(program.id(), "mvp", gl::kMatrix4f);
  uniforms.GetLocation(program.id(), "c_T_w", gl::kMatrix4f);
  uniforms.GetLocation(program.id(), "texture_sampler", gl::kTexture2D);
  uniforms.GetLocation(program.id(), "light", gl::kVector3f);
  uniforms.GetLocation(program.id(), "light_cnt", gl::kInt);
  uniforms.GetLocation(program.id(), "light_power", gl::kFloat);
  uniforms.GetLocation(program.id(), "light_color", gl::kVector3f);

  gl::Args args(4);
  args.InitBuffer(0, {GL_ARRAY_BUFFER, sizeof(float), 3, GL_FLOAT},
                  model.positions().size());
  args.InitBuffer(1, {GL_ARRAY_BUFFER, sizeof(float), 3, GL_FLOAT},
                  model.normals().size());
  args.InitBuffer(2, {GL_ARRAY_BUFFER, sizeof(float), 2, GL_FLOAT},
                  model.uvs().size());
  args.InitBuffer(3, {GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int),
                      1, GL_UNSIGNED_INT},
                  model.indices().size());

  gl::Program primitive_program;
  primitive_program.Load("../shader/primitive_vertex.glsl", gl::kVertexShader);
  primitive_program.Load("../shader/primitive_fragment.glsl", gl::kFragmentShader);
  primitive_program.Build();
  gl::Args axis_args(2);
  axis_args.BindBuffer(0, {GL_ARRAY_BUFFER, sizeof(float), 3, GL_FLOAT},
                       axises.size(), axises.data());
  axis_args.BindBuffer(1, {GL_ARRAY_BUFFER, sizeof(float), 3, GL_FLOAT},
                       axis_colors.size(), axis_colors.data());
  gl::Uniforms light_uniforms;
  light_uniforms.GetLocation(primitive_program.id(), "mvp", gl::kMatrix4f);

  // Additional settings
  glfwPollEvents();
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  cv::Mat capture;
  do {
    // Update control
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /// Update viewpoint
    camera.UpdateView(window);

    /// Bind uniform data
    glm::mat4 mvp = camera.mvp();
    glm::mat4 view = camera.view();

    glUseProgram(program.id());
    texture.Bind(0);
    uniforms.Bind("mvp", &mvp, 1);
    uniforms.Bind("c_T_w", &view, 1);
    uniforms.Bind("texture_sampler", GLuint(0));
    uniforms.Bind("light", light_src_positions.data(), light_src_positions.size());
    uniforms.Bind("light_cnt", &light_cnt, 1);
    uniforms.Bind("light_power", &light_power, 1);
    uniforms.Bind("light_color", &light_color, 1);

    /// Bind vertex data
    glBindVertexArray(args.vao());
    args.BindBuffer(0, {GL_ARRAY_BUFFER, sizeof(float), 3, GL_FLOAT},
                    model.positions().size(), model.positions().data());
    args.BindBuffer(1, {GL_ARRAY_BUFFER, sizeof(float), 3, GL_FLOAT},
                    model.normals().size(), model.normals().data());
    args.BindBuffer(2, {GL_ARRAY_BUFFER, sizeof(float), 2, GL_FLOAT},
                    model.uvs().size(), model.uvs().data());
    args.BindBuffer(3, {GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int),
                        1, GL_UNSIGNED_INT},
                    model.indices().size(), model.indices().data());

    /// Render
    glDrawElements(GL_TRIANGLES, model.indices().size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glUseProgram(primitive_program.id());
    light_uniforms.Bind("mvp", &mvp, 1);

    glBindVertexArray(axis_args.vao());
    glEnable(GL_PROGRAM_POINT_SIZE);
    glDrawArrays(GL_LINES, 0, axises.size());
    glBindVertexArray(0);

    window.swap_buffer();
  } while( window.get_key(GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
           window.should_close() == 0 );

//  if (capture.rows > 0) {
//    cv::Mat depths = camera.ConvertDepthBuffer(capture, 5000);
//    cv::imshow("depth", depths);
//    cv::waitKey(-1);
//  }

  // Close OpenGL window and terminate GLFW
  glfwTerminate();

  return 0;
}
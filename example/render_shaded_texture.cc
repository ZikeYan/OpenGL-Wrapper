//
// Created by Neo on 08/08/2017.
//

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <opencv2/opencv.hpp>

#include "../include/glwrapper.h"

int main() {
  std::vector<glm::vec3> lights = {
      glm::vec3(-10, 0, 10),
      glm::vec3(10, 0, 10),
      glm::vec3(0, 0, 20),
      glm::vec3(0, 0, -20),
      glm::vec3(0, 10, 0),
      glm::vec3(0, -10, 0),
      glm::vec3(-10, 0, 0),
      glm::vec3(10, 0, 0)
  };
  float light_power = 40;
  glm::vec3 light_color = glm::vec3(1, 1, 1);
  std::stringstream ss;
  ss << lights.size();

  gl::Model model;
  model.LoadObj("../model/beethoven/beethoven.obj");
  gl::Texture texture;
  texture.Load("../model/beethoven/beethoven.png");

  // Context and control init
  gl::Window window("Shading", texture.width(), texture.height());
  texture.Init("../model/beethoven/beethoven.png");
  /// Finally output shaded texture
  gl::Program program;
  program.Load("../shader/shading_vertex.glsl",
                       gl::kVertexShader);
  program.Load("../shader/shading_fragment.glsl",
                       gl::kFragmentShader);
  program.ReplaceMacro("LIGHT_COUNT", ss.str(), gl::kFragmentShader);
  program.Build();
  gl::Uniforms uniforms;
  uniforms.GetLocation(program.id(), "lights", gl::kVector3f);
  uniforms.GetLocation(program.id(), "light_power", gl::kFloat);
  uniforms.GetLocation(program.id(), "light_color", gl::kVector3f);
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
  gl::FrameBuffer fbo_shaded_texture(GL_RGB, texture.width(), texture.height());

  /// divided by 2 depends on Resolution: refactor it later
  fbo_shaded_texture.Bind();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glUseProgram(program.id());
  texture.Bind(0);
  uniforms.Bind("lights", lights.data(), lights.size());
  uniforms.Bind("light_power", &light_power, 1);
  uniforms.Bind("light_color", &light_color, 1);
  glBindVertexArray(args.vao());
  glDrawElements(GL_TRIANGLES, model.indices().size(), GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);

  window.swap_buffer();
  cv::imwrite("shaded-texture.png", fbo_shaded_texture.Capture());
  return 0;
}
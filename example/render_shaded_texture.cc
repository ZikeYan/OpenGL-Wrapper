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
  gl::Program shading_program;
  shading_program.Load("../shader/shading_vertex.glsl", gl::kVertexShader);
  shading_program.Load("../shader/shading_fragment.glsl", gl::kFragmentShader);
  shading_program.ReplaceMacro("LIGHT_COUNT", ss.str(), gl::kFragmentShader);
  shading_program.Build();

  gl::Uniforms shading_uniforms;
  shading_uniforms.GetLocation(shading_program.id(), "lights", gl::kVector3f);
  shading_uniforms.GetLocation(shading_program.id(), "light_power", gl::kFloat);
  shading_uniforms.GetLocation(shading_program.id(), "light_color", gl::kVector3f);

  gl::Args shading_args(4);
  shading_args.BindBuffer(0, {GL_ARRAY_BUFFER, sizeof(float), 2, GL_FLOAT},
                          model.uvs().size(), model.uvs().data());
  shading_args.BindBuffer(1, {GL_ARRAY_BUFFER, sizeof(float), 3, GL_FLOAT},
                          model.positions().size(), model.positions().data());
  shading_args.BindBuffer(2, {GL_ARRAY_BUFFER, sizeof(float), 3, GL_FLOAT},
                          model.normals().size(), model.normals().data());
  shading_args.BindBuffer(3, {GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int),
                              1, GL_UNSIGNED_INT},
                          model.indices().size(), model.indices().data());

  /// divided by 2 depends on Resolution: refactor it later
  window.Resize(texture.width(), texture.height());
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  texture.Bind(0);
  glUseProgram(shading_program.id());

  shading_uniforms.Bind("lights",
                        lights.data(),
                        lights.size());
  shading_uniforms.Bind("light_power", &light_power, 1);
  shading_uniforms.Bind("light_color", &light_color, 1);
  glBindVertexArray(shading_args.vao());
  glDrawElements(GL_TRIANGLES, model.indices().size(), GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);

  window.swap_buffer();
  cv::Mat pixel = window.CaptureRGB();

  cv::imwrite("shaded-texture.png", pixel);
  return 0;
}
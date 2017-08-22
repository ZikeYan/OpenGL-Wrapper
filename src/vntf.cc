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
#include "model.h"

const int kWidth  = 640;
const int kHeight = 480;
int main() {
  /// Load model and texture
  gl::Model model;
  model.LoadObj("../obj/f16.obj");
  gl::Texture texture;
  texture.Load("../obj/f16.bmp");

  // Context and control init
  gl::Window window("F-16", kWidth, kHeight);
  gl::Camera camera(45.0f, window.width(), window.height(), 0.1f, 1000.0f);
  camera.SwitchInteraction(true);
  glm::mat4 model_mat = glm::mat4(1.0f);
  camera.set_model(model_mat);

  gl::Program program("../shader/vertex_vntf.glsl",
                      "../shader/fragment_vntf.glsl");
  texture.Init();
  gl::Uniform uniform_mvp(program.id(), "mvp", gl::kMatrix4f);
  gl::Uniform uniform_view(program.id(), "c_T_w", gl::kMatrix4f);
  gl::Uniform uniform_tex(program.id(), "texture_sampler", gl::kTexture2D);
  gl::Uniform uniform_light(program.id(), "light", gl::kVector3f);

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

  // Additional settings
  glfwPollEvents();
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  cv::Mat capture;
  float t = 0;
  do {
    // Update control
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    camera.SetView(window);
    glm::mat4 projection = camera.projection();
    glm::mat4 mvp = camera.mvp();
    glm::mat4 view = camera.view();
    glm::vec3 light = glm::vec3(0, cos(t), sin(t));
    t += 0.01f;
    int tex_idx = 0;

    glUseProgram(program.id());
    texture.Bind(0);
    uniform_mvp.Bind(&mvp[0][0]);
    uniform_view.Bind(&view[0][0]);
    uniform_tex.Bind(&tex_idx);
    uniform_light.Bind(&light);

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

    glDrawElements(GL_TRIANGLES, model.indices().size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    capture = window.CaptureDepth();
    window.swap_buffer();
  } while( window.get_key(GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
           window.should_close() == 0 );

//  cv::Mat depths = camera.ConvertDepthBuffer(capture, 5000);
//  cv::imshow("depth", depths);
//  cv::waitKey(-1);
//  cv::imwrite("depth.png", depths);

  // Close OpenGL window and terminate GLFW
  glfwTerminate();
  return 0;
}
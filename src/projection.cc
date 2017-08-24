//
// Created by Neo on 23/08/2017.
//

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <opencv2/opencv.hpp>
#include <utils/shader.h>
#include <program.h>
#include <uniform.h>
#include <camera.h>
#include "utils/model.h"
#include "utils/texture.h"
#include "utils/context.h"
#include "utils/control.h"
#include "window.h"
#include "texture.h"
#include "args.h"
#include "model.h"

int main() {
  static const GLfloat vertices[] = {
      -1.0f, -1.0f, 0.0f,
      1.0f, -1.0f, 0.0f,
      -1.0f,  1.0f, 0.0f,
      -1.0f,  1.0f, 0.0f,
      1.0f, -1.0f, 0.0f,
      1.0f,  1.0f, 0.0f,
  };

  gl::Model model;
  model.LoadObj("../obj/f16.obj");

  // Context and control init
  gl::Window window("F-16", 640, 480);
  gl::Camera camera(45.0f, window.width(), window.height());
  camera.SwitchInteraction(true);

  gl::Program program1("../shader/proj_vertex_pass1.glsl",
                       "../shader/proj_fragment_pass1.glsl");
  gl::Uniform uniform1_mvp(program1.id(), "mvp", gl::kMatrix4f);
  gl::Args args1(2);
  args1.BindBuffer(0, {GL_ARRAY_BUFFER, sizeof(float), 3, GL_FLOAT},
                  model.positions().size(), model.positions().data());
  args1.BindBuffer(1, {GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int),
                      1, GL_UNSIGNED_INT},
                  model.indices().size(), model.indices().data());

  gl::Program program2("../shader/proj_vertex_pass2.glsl",
                       "../shader/proj_fragment_pass2.glsl");
  gl::Uniform uniform2_tex(program2.id(), "z_buffer", gl::kTexture2D);
  gl::Uniform uniform2_mvp(program2.id(), "mvp", gl::kMatrix4f);
  gl::Args args2(3);
  args2.BindBuffer(0, {GL_ARRAY_BUFFER, sizeof(float), 2, GL_FLOAT},
                   model.uvs().size(), model.uvs().data());
  args2.BindBuffer(1, {GL_ARRAY_BUFFER, sizeof(float), 3, GL_FLOAT},
                   model.positions().size(), model.positions().data());
  args2.BindBuffer(2, {GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int),
                       1, GL_UNSIGNED_INT},
                   model.indices().size(), model.indices().data());

  // Create framebuffer
  GLuint FramebufferName = 0;
  glGenFramebuffers(1, &FramebufferName);
  glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);

  // Generate depth texture
  GLuint depthTexture;
  glGenTextures(1, &depthTexture);
  glBindTexture(GL_TEXTURE_2D, depthTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, 1280, 960, 0,
               GL_DEPTH_COMPONENT, GL_FLOAT, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
  // Bind depth texture to the framebuffer
  glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTexture, 0);

  // Output depth into texture, don't draw color
  glDrawBuffer(GL_NONE);

  // Always check that our framebuffer is ok
  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    return -1;

  // Additional settings
  glfwPollEvents();

  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  // Enable depth test
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glEnable(GL_CULL_FACE);

  cv::Mat pixel = cv::Mat(960, 1280, CV_32FC1);
  do {
    // Control
    camera.SetView(window);
    glm::mat4 mvp = camera.mvp();
    glm::mat4 view = camera.view();

    // Pass 1:
    // Render to framebuffer, into depth texture
    glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTexture, 0);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(program1.id());
    uniform1_mvp.Bind(&mvp);
    glBindVertexArray(args1.vao());
    glDrawElements(GL_TRIANGLES, model.indices().size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // Pass 2:
    // Test rendering texture
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_PROGRAM_POINT_SIZE);

    glUseProgram(program2.id());
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthTexture);

    glGetTexImage(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, GL_FLOAT, pixel.data);
    cv::flip(pixel, pixel, 0);
    cv::Mat render;
    cv::resize(pixel, render, cv::Size(pixel.cols / 2, pixel.rows / 2));
    cv::imshow("depth", render);
    int tex_idx = 0;
    uniform2_mvp.Bind(&mvp);
    uniform2_tex.Bind(&tex_idx);
    glBindVertexArray(args2.vao());
    glDrawElements(GL_TRIANGLES, model.indices().size(), GL_UNSIGNED_INT, 0);
    //glDrawArrays(GL_POINTS, 0, model.positions().size());
    glBindVertexArray(0);

    window.swap_buffer();
  } while( window.get_key(GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
           window.should_close() == 0 );
  // Close OpenGL window and terminate GLFW
  glfwTerminate();
  return 0;
}
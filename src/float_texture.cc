//
// Created by Neo on 24/08/2017.
//

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <opencv2/opencv.hpp>
#include <utils/shader.h>
#include <program.h>
#include <uniform.h>
#include <camera.h>
#include <fstream>
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

  gl::Program program1("../shader/float_tex_vertex.glsl",
                       "../shader/float_tex_fragment.glsl");
  gl::Texture texture("../obj/f16.bmp");

  gl::Uniform uniform1_mvp(program1.id(), "mvp", gl::kMatrix4f);
  gl::Args args1(3);
  args1.BindBuffer(0, {GL_ARRAY_BUFFER, sizeof(float), 3, GL_FLOAT},
                   model.positions().size(), model.positions().data());
  args1.BindBuffer(1, {GL_ARRAY_BUFFER, sizeof(float), 2, GL_FLOAT},
                   model.uvs().size(), model.uvs().data());
  args1.BindBuffer(2, {GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int),
                       1, GL_UNSIGNED_INT},
                   model.indices().size(), model.indices().data());

  gl::Program program2("../shader/tex_vertex.glsl",
                       "../shader/tex_fragment.glsl");
  gl::Uniform uniform2_tex(program2.id(), "z_buffer", gl::kTexture2D);
  gl::Args args2(2);
  args2.BindBuffer(0, {GL_ARRAY_BUFFER, sizeof(float), 3, GL_FLOAT},
                   6, (void*)vertices);
  args2.BindBuffer(1, {GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int),
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
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 1280, 960, 0,
               GL_RGBA, GL_FLOAT, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  GLuint depthrenderbuffer;
  glGenRenderbuffers(1, &depthrenderbuffer);
  glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 1280, 960);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);

  // Bind depth texture to the framebuffer
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, depthTexture, 0);

  // Set the list of draw buffers.
  GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
  glDrawBuffers(1, DrawBuffers); //

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

  cv::Mat pixel = cv::Mat(960, 1280, CV_32FC4);
  do {
    // Control
    camera.SetView(window);
    glm::mat4 mvp = camera.mvp();
    glm::mat4 view = camera.view();

    // Pass 1:
    // Render to framebuffer, into depth texture
    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);
    // Cull triangles which normal is not towards the camera
    glEnable(GL_CULL_FACE);

    glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(program1.id());
    texture.Bind(0);

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

    glUseProgram(program2.id());
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, pixel.data);
    int tex1_idx = 0;
    uniform2_tex.Bind(&tex1_idx);
    glBindVertexArray(args2.vao());
    glDrawArrays(GL_TRIANGLES, 0, 6);
    //glDrawElements(GL_TRIANGLES, model.indices().size(), GL_UNSIGNED_INT, 0);
    //glDrawArrays(GL_POINTS, 0, model.positions().size());
    glBindVertexArray(0);

    window.swap_buffer();
  } while( window.get_key(GLFW_KEY_ESCAPE) != GLFW_PRESS &&
           window.should_close() == 0 );

  // Close OpenGL window and terminate GLFW
  cv::resize(pixel, pixel, cv::Size(pixel.cols/2, pixel.rows/2));
  cv::flip(pixel, pixel, 0);
  cv::imshow("pixel", pixel);
  cv::waitKey(-1);
  std::ofstream out("map.txt");
  for (int i = 0; i < pixel.rows; ++i)
    for (int j = 0; j < pixel.cols; ++j) {
      cv::Vec4f v = pixel.at<cv::Vec4f>(i, j);
        if (v[0] != 0 || v[1] != 0)
          out << i << " " << j << " "
              << v[0] << " " << v[1] << std::endl;
    }

  glfwTerminate();
  return 0;
}
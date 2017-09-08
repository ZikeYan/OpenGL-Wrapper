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
#include <trajectory.h>
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

  int width = 640;
  int height= 480;
  // Context and control init
  gl::Window window("F-16", width, height);
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
  GLuint frame_buffer = 0;
  glGenFramebuffers(1, &frame_buffer);
  glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);

  // Generate depth texture
  gl::Texture output_texture;
  output_texture.Init(GL_RGBA32F, width, height);

  GLuint depth_render_buffer;
  glGenRenderbuffers(1, &depth_render_buffer);
  glBindRenderbuffer(GL_RENDERBUFFER, depth_render_buffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT,
                        width * 2, height * 2);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                            GL_RENDERBUFFER, depth_render_buffer);

  // Bind depth texture to the framebuffer
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                       output_texture.id(), 0);

  // Set the list of draw buffers.
  GLenum draw_buffers[1] = {GL_COLOR_ATTACHMENT0};
  glDrawBuffers(1, draw_buffers); //

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

  gl::Trajectory traj;
  //traj.Load("test.traj");
  int idx = 0;

  cv::Mat pixel = cv::Mat(height * 2, width * 2, CV_32FC4);
  do {
    // Control
    camera.SetView(window);
    glm::mat4 mvp = camera.mvp();
    glm::mat4 view = camera.view();
    //glm::mat4 projection = camera.projection();
    //glm::mat4 mvp = camera.mvp();
    //glm::mat4 view = camera.view();
    //glm::mat4 view = traj.poses()[idx++];
    //if (idx > traj.poses().size()) break;
    //glm::mat4 mvp = projection * view;

    // Pass 1:
    // Render to framebuffer, into depth texture

    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /// Choose shader
    glUseProgram(program1.id());
    /// Bind texture
    texture.Bind(0);
    /// Bind uniform
    uniform1_mvp.Bind(&mvp);
    /// Bind args
    glBindVertexArray(args1.vao());
    /// Run
    glDrawElements(GL_TRIANGLES, model.indices().size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // Pass 2:
    // Test rendering texture
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(program2.id());
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, output_texture.id());
    int tex1_idx = 0;
    uniform2_tex.Bind(&tex1_idx);
    glBindVertexArray(args2.vao());
    glDrawArrays(GL_TRIANGLES, 0, 6);
    //glDrawElements(GL_TRIANGLES, model.indices().size(), GL_UNSIGNED_INT, 0);
    //glDrawArrays(GL_POINTS, 0, model.positions().size());
    glBindVertexArray(0);


    if (window.get_key(GLFW_KEY_ENTER)) {
      glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, pixel.data);
      std::stringstream proj;
      proj << "Proj" << idx - 1 << ".txt";
      cv::flip(pixel, pixel, 0);

      std::ofstream out("map.txt");
      for (int i = 0; i < pixel.rows; ++i)
        for (int j = 0; j < pixel.cols; ++j) {
          cv::Vec4f v = pixel.at<cv::Vec4f>(i, j);
          if (v[0] != 0 || v[1] != 0)
            out << i << " " << j << " "
                << v[0] << " " << v[1] << std::endl;
        }
    }

    window.swap_buffer();
  } while( window.get_key(GLFW_KEY_ESCAPE) != GLFW_PRESS &&
           window.should_close() == 0 );

  // Close OpenGL window and terminate GLFW
//  cv::resize(pixel, pixel, cv::Size(pixel.cols/2, pixel.rows/2));
//  cv::imshow("pixel", pixel);
//  cv::waitKey(-1);


  glfwTerminate();
  return 0;
}
//
// Created by Neo on 24/08/2017.
//

#include <fstream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <opencv2/opencv.hpp>

#include "../src/program.h"
#include "../src/uniform.h"
#include "../src/camera.h"
#include "../src/trajectory.h"
#include "../src/window.h"
#include "../src/texture.h"
#include "../src/args.h"
#include "../src/model.h"

int kWidth = 640;
int kHeight= 480;

static const GLfloat kVertices[] = {
    -1.0f, -1.0f, 0.0f,
    1.0f, -1.0f, 0.0f,
    -1.0f,  1.0f, 0.0f,
    -1.0f,  1.0f, 0.0f,
    1.0f, -1.0f, 0.0f,
    1.0f,  1.0f, 0.0f,
};

int main() {


  gl::Model model;
  model.LoadObj("../obj/f16.obj");

  // Context and control init
  gl::Window window("F-16", kWidth, kHeight);
  gl::Camera camera(window.width(), window.height());
  camera.SwitchInteraction(true);

  gl::Program program1("../shader/write_texture_vertex.glsl",
                       "../shader/write_texture_fragment.glsl");
  gl::Texture read_texture("../obj/f16.bmp");
  gl::Uniform uniform1_mvp(program1.id(), "mvp", gl::kMatrix4f);
  gl::Args args1(3);
  args1.BindBuffer(0, {GL_ARRAY_BUFFER, sizeof(float), 3, GL_FLOAT},
                   model.positions().size(), model.positions().data());
  args1.BindBuffer(1, {GL_ARRAY_BUFFER, sizeof(float), 2, GL_FLOAT},
                   model.uvs().size(), model.uvs().data());
  args1.BindBuffer(2, {GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int),
                       1, GL_UNSIGNED_INT},
                   model.indices().size(), model.indices().data());

  gl::Program program2("../shader/simple_texture_vertex.glsl",
                       "../shader/simple_texture_fragment.glsl");
  gl::Uniform uniform2_tex(program2.id(), "tex", gl::kTexture2D);
  gl::Args args2(2);
  args2.BindBuffer(0, {GL_ARRAY_BUFFER, sizeof(float), 3, GL_FLOAT},
                   6, (void*)kVertices);
  args2.BindBuffer(1, {GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int),
                       1, GL_UNSIGNED_INT},
                   model.indices().size(), model.indices().data());

  ////////////////////////////////////
  // Not encapsulated now
  // Create framebuffer
  GLuint frame_buffer = 0;
  glGenFramebuffers(1, &frame_buffer);
  glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
  // Color -> texture, to output
  gl::Texture write_texture;
  write_texture.Init(GL_RGBA32F, kWidth, kHeight);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                       write_texture.id(), 0);
  // Depth -> render buffer, for depth test
  GLuint render_buffer;
  glGenRenderbuffers(1, &render_buffer);
  glBindRenderbuffer(GL_RENDERBUFFER, render_buffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT,
                        write_texture.width(), write_texture.height());
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                            GL_RENDERBUFFER, render_buffer);
  // Set the list of draw buffers.
  GLenum draw_buffers[1] = {GL_COLOR_ATTACHMENT0};
  glDrawBuffers(1, draw_buffers);
  // Always check that our framebuffer is ok
  if(glCheckFramebufferStatus(GL_FRAMEBUFFER)
     != GL_FRAMEBUFFER_COMPLETE)
    return -1;
  ////////////////////////////////////

  // Additional settings
  glfwPollEvents();

  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  // Enable depth test
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  gl::Trajectory traj;

  cv::Mat pixel = cv::Mat(write_texture.height(),
                          write_texture.width(),
                          CV_32FC4);
  do {
    // Control
    camera.SetView(window);
    glm::mat4 mvp = camera.mvp();
    glm::mat4 view = camera.view();

    // Pass 1:
    // Render to framebuffer, into depth texture
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /// Choose shader
    glUseProgram(program1.id());
    read_texture.Bind(0);
    uniform1_mvp.Bind(&mvp);
    glBindVertexArray(args1.vao());
    glDrawElements(GL_TRIANGLES, model.indices().size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // Pass 2:
    // Test rendering texture
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(program2.id());
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, write_texture.id());
    uniform2_tex.Bind(GLuint(0));
    glBindVertexArray(args2.vao());
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    if (window.get_key(GLFW_KEY_ENTER)) {
      glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, pixel.data);
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
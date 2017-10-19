//
// Created by Neo on 24/08/2017.
//

#include <fstream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <opencv2/opencv.hpp>
#include "glwrapper.h"

// Actual window size: 1280x960
int kWindowWidth = 640;
int kWindowHeight= 480;

int kTextureWidth = 1280;
int kTextureHeight = 960;

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
  model.LoadObj("../model/face/face.obj");

  // Context and control init
  gl::Window window("F-16", kWindowWidth, kWindowHeight);
  gl::Camera camera(window.width(), window.height());
  camera.set_intrinsic("../model/face/face-intrinsic.txt");
  camera.SwitchInteraction(false);

  gl::Program program1;
  program1.Load("../shader/render_to_fbo_vertex.glsl", gl::kVertexShader);
  program1.Load("../shader/render_to_fbo_fragment.glsl", gl::kFragmentShader);
  program1.Build();

  gl::Texture read_texture;
  read_texture.Init("../model/face/face.png");
  gl::Uniforms uniforms1;
  uniforms1.GetLocation(program1.id(), "mvp", gl::kMatrix4f);
  uniforms1.GetLocation(program1.id(), "tex", gl::kTexture2D);
  gl::Args args1(3);
  args1.BindBuffer(0, {GL_ARRAY_BUFFER, sizeof(float), 3, GL_FLOAT},
                   model.positions().size(), model.positions().data());
  args1.BindBuffer(1, {GL_ARRAY_BUFFER, sizeof(float), 2, GL_FLOAT},
                   model.uvs().size(), model.uvs().data());
  args1.BindBuffer(2, {GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int),
                       1, GL_UNSIGNED_INT},
                   model.indices().size(), model.indices().data());

  gl::Program program2;
  program2.Load("../shader/simple_texture_vertex.glsl", gl::kVertexShader);
  program2.Load("../shader/simple_texture_fragment.glsl", gl::kFragmentShader);
  program2.Build();

  gl::Uniforms uniforms2;
  uniforms2.GetLocation(program2.id(), "tex", gl::kTexture2D);
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
  write_texture.Init(GL_RGBA, kTextureWidth, kTextureHeight);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                       write_texture.id(), 0);
  // Depth -> render buffer, for depth example
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
  // Enable depth example
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  gl::Trajectory traj;
  traj.Load("../model/face/face-extrinsic.txt");
  glm::mat4 model_mat = glm::mat4(1.0);
  model_mat[1][1] = model_mat[2][2] = -1;
  camera.set_model(model_mat);
  for (auto& T : traj.poses()) {
    T = model_mat * glm::transpose(T) * model_mat;
  }

  cv::Mat pixel = cv::Mat(write_texture.height(),
                          write_texture.width(),
                          CV_8UC4);

  for (int i = 0; i < traj.poses().size(); ++i) {
    std::cout << i << " / " << traj.poses().size() << std::endl;
    glm::mat4 mvp = camera.projection() * traj.poses()[i] * camera.model();

    // Control
    camera.UpdateView(window);

    // Pass 1:
    // Render to framebuffer, into depth texture
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
    glViewport(0, 0, kTextureWidth*2, kTextureHeight*2); // retina
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /// Choose shader
    glUseProgram(program1.id());
    read_texture.Bind(0);
    uniforms1.Bind("mvp", &mvp, 1);
    uniforms1.Bind("tex", GLuint(0));
    glBindVertexArray(args1.vao());
    glDrawElements(GL_TRIANGLES, model.indices().size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // Pass 2:
    // Test rendering texture
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, kWindowWidth*2, kWindowHeight*2); // 2x retina
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(program2.id());
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, write_texture.id());
    uniforms2.Bind("tex", GLuint(1));
    glBindVertexArray(args2.vao());
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glGetTexImage(GL_TEXTURE_2D, 0, GL_BGRA, GL_UNSIGNED_BYTE, pixel.data);
    cv::flip(pixel, pixel, 0);
    std::stringstream ss;
    ss.str("");
    ss << "pixel_" << i << ".png";
    cv::imwrite(ss.str(), pixel);

    window.swap_buffer();
  }


  // Close OpenGL window and terminate GLFW
//  cv::resize(pixel, pixel, cv::Size(pixel.cols/2, pixel.rows/2));
//  cv::imshow("pixel", pixel);
//  cv::waitKey(-1);


  glfwTerminate();
  return 0;
}
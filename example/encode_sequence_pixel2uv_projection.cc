//
// Created by Neo on 21/09/2017.
//

/// Input: mesh(.obj), texture(.png)
///        intrinsic (fx, cx, fy, cy, w, h)
///        extrinsics (4x4 mat)
/// Output: rendered image(.png)
///         pixel-wise texture coordinate map(.txt)
///         => further interpreted by MATLAB

#include <fstream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <opencv2/opencv.hpp>

#include "../src/program.h"
#include "../src/uniforms.h"
#include "../src/camera.h"
#include "../src/trajectory.h"
#include "../src/window.h"
#include "../src/texture.h"
#include "../src/args.h"
#include "../src/model.h"

int kWidth = 1280;
int kHeight= 960;

/// Wrap for light sources
/// Show axis and light sources
int main() {
  //////////////////////////////////
  /// Load data at the first stage
  gl::Model model;
  model.LoadObj("../model/face/face.obj");

  /// Context and control init
  gl::Window window("Projection mapping", kWidth, kHeight);

  gl::Camera camera(window.width(), window.height());
  camera.set_intrinsic("../model/face/face-intrinsic.txt");
  camera.SwitchInteraction(false);

  glm::mat4 model_mat = glm::mat4(1.0);
  model_mat[1][1] = model_mat[2][2] = -1;
  camera.set_model(model_mat);

  gl::Trajectory traj;
  traj.Load("../model/face/face-extrinsic.txt");
  /// Convert to OpenGL coordinate system
  /// x1' = T1 x1
  /// x2' = T2 x2
  /// x2 = 2M1 x1
  /// x2' = 2M1 x1'
  /// => 2M1 (T1 x1) = T2 (2M1 x1)
  /// => T2 =  2M1 T1 2M1.inv()
  for (auto& T : traj.poses()) {
    T = model_mat * glm::transpose(T) * model_mat;
  }

  gl::Texture input_texture;
  input_texture.Init("../model/face/face.png");

  ////////////////////////////////////
  /// Prepare for writing uv coordinates into texture
  gl::Program texture_writer_program;
  texture_writer_program.Load("../shader/write_uv_vertex.glsl",
                              gl::kVertexShader);
  texture_writer_program.Load("../shader/write_uv_fragment.glsl",
                              gl::kFragmentShader);
  texture_writer_program.Build();
  gl::Uniforms texture_writer_uniforms;
  texture_writer_uniforms.GetLocation(texture_writer_program.id(),
                                      "mvp", gl::kMatrix4f);
  gl::Args texture_writer_args(3);
  texture_writer_args.BindBuffer(0, {GL_ARRAY_BUFFER, sizeof(float), 3, GL_FLOAT},
                                 model.positions().size(), model.positions().data());
  texture_writer_args.BindBuffer(1, {GL_ARRAY_BUFFER, sizeof(float), 2, GL_FLOAT},
                                 model.uvs().size(), model.uvs().data());
  texture_writer_args.BindBuffer(2, {GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int),
                                     1, GL_UNSIGNED_INT},
                                 model.indices().size(), model.indices().data());

  // Not encapsulated now: set context for writing to renderbuffer
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
                        kWidth, kHeight);
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

  ////////////////////////////////////
  /// Render the model
  gl::Program render_program;
  render_program.Load("../shader/simple_model_vertex.glsl",
                      gl::kVertexShader);
  render_program.Load("../shader/simple_model_fragment.glsl",
                      gl::kFragmentShader);
  render_program.Build();

  gl::Uniforms render_uniforms;
  render_uniforms.GetLocation(render_program.id(), "mvp", gl::kMatrix4f);
  render_uniforms.GetLocation(render_program.id(), "texture_sampler", gl::kTexture2D);

  gl::Args render_args(3);
  render_args.InitBuffer(0, {GL_ARRAY_BUFFER, sizeof(float), 3, GL_FLOAT},
                         model.positions().size());
  render_args.InitBuffer(1, {GL_ARRAY_BUFFER, sizeof(float), 2, GL_FLOAT},
                         model.uvs().size());
  render_args.InitBuffer(2, {GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int),
                             1, GL_UNSIGNED_INT},
                         model.indices().size());

  // Additional settings
  glfwPollEvents();
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  // Buffer to collect written texture
  cv::Mat pixel = cv::Mat(write_texture.height(),
                          write_texture.width(),
                          CV_32FC4);

  for (int i = 0; i < traj.poses().size(); ++i) {
    std::cout << i << " / " << traj.poses().size() << std::endl;
    glm::mat4 mvp = camera.projection() * traj.poses()[i] * camera.model();

    // Pass 1:
    // Render to framebuffer, into texture
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /// Choose shader
    glUseProgram(texture_writer_program.id());
    input_texture.Bind(0);
    texture_writer_uniforms.Bind("mvp", &mvp, 1);
    glBindVertexArray(texture_writer_args.vao());
    glDrawElements(GL_TRIANGLES, model.indices().size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // Pass 2:
    // Render the scene for visualization
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(render_program.id());
    input_texture.Bind(0);
    render_uniforms.Bind("mvp", &mvp, 1);
    render_uniforms.Bind("texture_sampler", GLuint(0));

    /// Bind vertex data
    glBindVertexArray(render_args.vao());
    render_args.BindBuffer(0, {GL_ARRAY_BUFFER, sizeof(float), 3, GL_FLOAT},
                           model.positions().size(), model.positions().data());
    render_args.BindBuffer(1, {GL_ARRAY_BUFFER, sizeof(float), 2, GL_FLOAT},
                           model.uvs().size(), model.uvs().data());
    render_args.BindBuffer(2, {GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int),
                               1, GL_UNSIGNED_INT},
                           model.indices().size(), model.indices().data());

    /// Render
    glDrawElements(GL_TRIANGLES, model.indices().size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    /// Output: rendered image
    ///         coordinate map (x y -> tex uv)
    write_texture.Bind(0);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, pixel.data);
    cv::flip(pixel, pixel, 0);

    std::stringstream ss;
    ss << "im_gl" << i << ".png";
    cv::imwrite(ss.str(), window.CaptureRGB());
    ss.str("");
    ss << "map" << i << ".txt";
    std::ofstream out(ss.str());
    for (int i = 0; i < pixel.rows; ++i) {
      for (int j = 0; j < pixel.cols; ++j) {
        cv::Vec4f v = pixel.at<cv::Vec4f>(i, j);
        if (v[0] != 0 || v[1] != 0) {
          out << i << " " << j << " "
              << v[0] << " " << v[1] << std::endl;
        }
      }
    }
    window.swap_buffer();

    if (window.get_key(GLFW_KEY_ESCAPE) == GLFW_PRESS) break;
  }

  glfwTerminate();

  return 0;
}
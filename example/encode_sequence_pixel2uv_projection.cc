//
// Created by Neo on 21/09/2017.
//

/// Input: mesh(.obj), texture(.png)
///        intrinsic (fx, cx, fy, cy, w, h)
///        extrinsics (4x4 mat)
/// Output: rendered image(.png)
///         pixel-wise texture coordinate map(.txt)
///         => further interpreted by MATLAB
//
// Created by Neo on 24/08/2017.
//

#include <fstream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <opencv2/opencv.hpp>
#include "glwrapper.h"
#include "encode_pixel2uv.h"

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
  // Context and control init
  gl::Window window("Face", kWindowWidth, kWindowHeight);
  gl::Camera camera(window.width(), window.height());
  camera.set_intrinsic("../model/face/face-intrinsic.txt");
  camera.SwitchInteraction(false);
  gl::Trajectory traj;
  traj.Load("../model/face/face-extrinsic.txt");
  glm::mat4 model_mat = glm::mat4(1.0);
  model_mat[1][1] = model_mat[2][2] = -1;
  camera.set_model(model_mat);
  for (auto& T : traj.poses()) {
    T = model_mat * glm::transpose(T) * model_mat;
  }

  gl::Model model;
  model.LoadObj("../model/face/face.obj");
  gl::Texture input_texture;
  input_texture.Init("../model/face/face.png");

  /// 0: uv coordinate->texture to encode projection matrix
  gl::Program program0;
  program0.Load("../shader/pixel_uv_fbo_vertex.glsl", gl::kVertexShader);
  program0.Load("../shader/pixel_uv_fbo_fragment.glsl", gl::kFragmentShader);
  program0.Build();
  gl::Uniforms uniforms0;
  uniforms0.GetLocation(program0.id(), "mvp", gl::kMatrix4f);
  gl::Args args0(3);
  args0.BindBuffer(0, {GL_ARRAY_BUFFER, sizeof(float), 3, GL_FLOAT},
                   model.positions().size(), model.positions().data());
  args0.BindBuffer(1, {GL_ARRAY_BUFFER, sizeof(float), 2, GL_FLOAT},
                   model.uvs().size(), model.uvs().data());
  args0.BindBuffer(2, {GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int),
                       1, GL_UNSIGNED_INT},
                   model.indices().size(), model.indices().data());
  gl::FrameBuffer fbo_uv(GL_RGBA32F, kTextureWidth, kTextureHeight);

  /// 1: high-resolution image->texture
  gl::Program program1;
  program1.Load("../shader/textured_model_fbo_vertex.glsl", gl::kVertexShader);
  program1.Load("../shader/textured_model_fbo_fragment.glsl", gl::kFragmentShader);
  program1.Build();
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
  gl::FrameBuffer fbo_image(GL_RGB, kTextureWidth, kTextureHeight);

  /// 2: render texture->window
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

  // Additional settings
  glfwPollEvents();
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  for (int i = 0; i < traj.poses().size(); ++i) {
    std::cout << i << " / " << traj.poses().size() << std::endl;
    glm::mat4 mvp = camera.projection() * traj.poses()[i] * camera.model();

    // Control
    camera.UpdateView(window);

    // Pass 0:
    fbo_uv.Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(program0.id());
    uniforms0.Bind("mvp", &mvp, 1);
    glBindVertexArray(args0.vao());
    glDrawElements(GL_TRIANGLES, model.indices().size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // Write uv map
    std::stringstream ss("");
    ss << "map_" << i << ".txt";
    EncodePixelToUV(ss.str(), fbo_uv.Capture());

    // Pass 1:
    fbo_image.Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(program1.id());
    input_texture.Bind(0);
    uniforms1.Bind("mvp", &mvp, 1);
    uniforms1.Bind("tex", GLuint(0));
    glBindVertexArray(args1.vao());
    glDrawElements(GL_TRIANGLES, model.indices().size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // Write image
    ss.str("");
    ss << "pixel_" << i << ".png";
    cv::imwrite(ss.str(), fbo_image.Capture());

    // Pass 2: simple rendering
    window.Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(program2.id());
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, fbo_image.texture().id());
    uniforms2.Bind("tex", GLuint(1));
    glBindVertexArray(args2.vao());
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    window.swap_buffer();
  }

  glfwTerminate();
  return 0;
}
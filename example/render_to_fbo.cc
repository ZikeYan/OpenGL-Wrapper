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
  gl::Model model;
  model.LoadObj("../model/f16/f16.obj");

  // Context and control init
  gl::Window window("F16", kWindowWidth, kWindowHeight);
  gl::Camera camera(window.width(), window.height());
  camera.SwitchInteraction(true);

  gl::Texture input_texture;
  input_texture.Init("../model/f16/f16-1024.png");
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
  gl::FrameBuffer fbo_image(GL_RGBA, kTextureWidth, kTextureHeight);

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

  glfwPollEvents();
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  cv::Mat image_mat = cv::Mat(fbo_image.texture().height(),
                              fbo_image.texture().width(),
                              CV_8UC4);
  do {
    camera.UpdateView(window);
    glm::mat4 mvp = camera.mvp();

    // Pass 1:
    // Render to framebuffer, into depth texture
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_image.fbo());
    glViewport(0, 0, kTextureWidth * 2, kTextureHeight * 2); // retina
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /// Choose shader
    glUseProgram(program1.id());
    input_texture.Bind(0);
    uniforms1.Bind("mvp", &mvp, 1);
    uniforms1.Bind("tex", GLuint(0));
    glBindVertexArray(args1.vao());
    glDrawElements(GL_TRIANGLES, model.indices().size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, fbo_image.texture().id());
    if (window.get_key(GLFW_KEY_END) == GLFW_PRESS) {
      glGetTexImage(GL_TEXTURE_2D, 0, GL_BGRA, GL_UNSIGNED_BYTE, image_mat.data);
      cv::flip(image_mat, image_mat, 0);
      cv::imwrite("texture.png", image_mat);
    }

    // Pass 2:
    // Test rendering texture
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, kWindowWidth * 2, kWindowHeight * 2); // 2x retina
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(program2.id());
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, fbo_image.texture().id());
    uniforms2.Bind("tex", GLuint(1));
    glBindVertexArray(args2.vao());
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    window.swap_buffer();
  } while( window.get_key(GLFW_KEY_ESCAPE) != GLFW_PRESS &&
           window.should_close() == 0 );

  glfwTerminate();
  return 0;
}
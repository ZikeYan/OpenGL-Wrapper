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
/// Pixel-unit
int kWindowWidth = 1280;
int kWindowHeight= 960;

/// Pixel-unit
int kTextureWidth = 2560;
int kTextureHeight = 1920;

static const GLfloat kVertices[] = {
    -1.0f, -1.0f, 0.0f,
    1.0f, -1.0f, 0.0f,
    -1.0f,  1.0f, 0.0f,
    -1.0f,  1.0f, 0.0f,
    1.0f, -1.0f, 0.0f,
    1.0f,  1.0f, 0.0f,
};
bool kUnitVisual = false; // -> kUnitPixel

int main() {
  // Context and control init
  gl::Window window("F16", kWindowWidth, kWindowHeight, kUnitVisual);
  gl::Camera camera(window.visual_width(), window.visual_height());
  camera.SwitchInteraction(true);
  gl::Model model;
  model.LoadObj("../model/f16/f16.obj");
  gl::Texture input_texture;
  input_texture.Init("../model/f16/f16-1024.png");

  gl::Program program1;
  program1.Load("../shader/textured_model_fbo_vertex.glsl",
                gl::kVertexShader);
  program1.Load("../shader/textured_model_fbo_fragment.glsl",
                gl::kFragmentShader);
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
  gl::FrameBuffer fbo_image(GL_RGBA, kTextureWidth, kTextureHeight, kUnitVisual);

  gl::Program program2;
  program2.Load("../shader/simple_texture_vertex.glsl",
                gl::kVertexShader);
  program2.Load("../shader/simple_texture_fragment.glsl",
                gl::kFragmentShader);
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

  do {
    camera.UpdateView(window);
    glm::mat4 mvp = camera.mvp();

    // Pass 1:
    // Render to framebuffer, into depth texture
    fbo_image.Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(program1.id());
    input_texture.Bind(0);
    uniforms1.Bind("mvp", &mvp, 1);
    uniforms1.Bind("tex", GLuint(0));
    glBindVertexArray(args1.vao());
    glDrawElements(GL_TRIANGLES, model.indices().size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    if (window.get_key(GLFW_KEY_ENTER) == GLFW_PRESS) {
      cv::imwrite("texture.png", fbo_image.Capture());
    }

    // Pass 2:
    // Test rendering texture
    window.Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(program2.id());
    fbo_image.texture().Bind(1);
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
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
#include "../src/camera.h"
#include "../src/uniforms.h"
#include "../src/program.h"

#include "../src/window.h"
#include "../src/texture.h"
#include "../src/args.h"
#include "../src/model.h"
#include "../src/trajectory.h"

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
  gl::Camera camera(window.width(), window.height());
  camera.SwitchInteraction(true);

  glm::mat4 model_mat = glm::mat4(1.0f);
  camera.set_model(model_mat);

  gl::Program program("../shader/model_vertex.glsl",
                      "../shader/model_fragment.glsl");

  texture.Init();
  gl::Uniforms uniforms;
  uniforms.GetLocation(program.id(), "mvp", gl::kMatrix4f);
  uniforms.GetLocation(program.id(), "c_T_w", gl::kMatrix4f);
  uniforms.GetLocation(program.id(), "texture_sampler", gl::kTexture2D);
  uniforms.GetLocation(program.id(), "light", gl::kVector3f);

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
  gl::Trajectory traj;

  int capture_cnt = 0;

  do {
    // Update control
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /// Update viewpoint
    camera.SetView(window);

    /// Bind uniform data
    glm::mat4 mvp = camera.mvp();
    glm::mat4 view = camera.view();
    std::vector<glm::vec3> lights = {
        glm::vec3(0, 3, 0), glm::vec3(0, -3, 0), glm::vec3(-3, 3, 1)
    };
    glUseProgram(program.id());
    texture.Bind(0);
    uniforms.Bind("mvp", &mvp, 1);
    uniforms.Bind("c_T_w", &view, 1);
    uniforms.Bind("texture_sampler", GLuint(0));
    uniforms.Bind("light", lights.data(), lights.size());

    /// Bind vertex data
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

    /// Render
    glDrawElements(GL_TRIANGLES, model.indices().size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    /// Test capture utils
    if (window.get_key(GLFW_KEY_ENTER) == GLFW_PRESS) {
      capture = window.CaptureDepth();

      std::stringstream ss;
      ss << "Image" << capture_cnt << ".png";
      cv::imwrite(ss.str(), capture);
      traj.poses().push_back(view);
      capture_cnt ++;
    }
    window.swap_buffer();
  } while( window.get_key(GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
           window.should_close() == 0 );

  if (capture.rows > 0) {
    cv::Mat depths = camera.ConvertDepthBuffer(capture, 5000);
    cv::imshow("depth", depths);
    cv::waitKey(-1);
  }

  // Close OpenGL window and terminate GLFW
  glfwTerminate();

  return 0;
}
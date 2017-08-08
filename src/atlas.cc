//
// Created by Neo on 08/08/2017.
//

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <opencv2/opencv.hpp>
#include <utils/shader.h>
#include "utils/model.h"
#include "utils/texture.h"
#include "utils/context.h"
#include "utils/control.h"

int main() {

  std::vector<glm::vec3> vertices_;
  std::vector<glm::vec3> normals_;
  std::vector<glm::vec2> uvs_;
  std::vector<unsigned int> indices_;

  unsigned char* texture_data_;
  int texture_width_;
  int texture_height_;

  LoadModel("../obj/f16.obj", vertices_, uvs_, normals_, indices_);
  std::cerr << vertices_.size() << " " << indices_.size()/3 << std::endl;
  LoadTexture("../obj/f16.bmp", texture_data_,
              texture_width_, texture_height_);

  cv::Mat shading = cv::Mat(texture_height_, texture_width_, CV_8UC3);

  for (int i = 0; i < indices_.size() / 3; ++i) {
    int i0 = indices_[3 * i + 0],
        i1 = indices_[3 * i + 1],
        i2 = indices_[3 * i + 2];

    cv::Point vertices[1][3];
    vertices[0][0] = cv::Point(uvs_[i0].x * texture_width_,
                              (1 - uvs_[i0].y) *texture_height_);
    vertices[0][1] = cv::Point(uvs_[i1].x * texture_width_,
                               (1 - uvs_[i1].y) * texture_height_);
    vertices[0][2] = cv::Point(uvs_[i2].x * texture_width_,
                               (1 - uvs_[i2].y) * texture_height_);
    glm::vec3 light = glm::vec3(2, 2, 1);
    glm::vec3 position = vertices_[i0];
    glm::vec3 normal = normals_[i0];
    float color = 255 * glm::dot(glm::normalize(position - light),
                                 glm::normalize(normal));
    //std::cout << color << std::endl;

    const cv::Point *ppt[1] = {vertices[0]};
    int npt[] = {3};

    cv::fillPoly(shading, ppt, npt, 1,
                 cv::Scalar(color, color, color));
  }
  //cv::imshow("shading", shading);
  //cv::waitKey(-1);

  // Context and control init
  Context context("F-16", texture_width_/2, texture_height_/2);

  GLuint program_id;
  LoadShaders("../shader/vertex_atlas.glsl",
              "../shader/fragment_atlas.glsl",
              program_id);

  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  GLuint vbos[4];
  glGenBuffers(4, vbos);

  // Vertices: uv version
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
  glBufferData(GL_ARRAY_BUFFER,
               sizeof(float) * 2 * uvs_.size(),
               uvs_.data(),
               GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

  // Vertices: real 3d vertex position
  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, vbos[1]);
  glBufferData(GL_ARRAY_BUFFER,
               sizeof(float) * 3 * vertices_.size(),
               vertices_.data(),
               GL_STATIC_DRAW);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

  // Normals:
  glEnableVertexAttribArray(2);
  glBindBuffer(GL_ARRAY_BUFFER, vbos[2]);
  glBufferData(GL_ARRAY_BUFFER,
               sizeof(float) * 3 * normals_.size(),
               normals_.data(),
               GL_STATIC_DRAW);
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);

  // Indices
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos[3]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               sizeof(unsigned int) * indices_.size(),
               indices_.data(),
               GL_STATIC_DRAW);

  // Additional settings
  glfwPollEvents();

  glClearColor(74.0 / 255.0f, 117.0f / 255.0f, 140.0f / 255.0f, 0.0f);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  cv::Mat pixels;
  do
  {
    // Update control
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(program_id);
    glDrawElements(GL_TRIANGLES, indices_.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(vao);
    pixels = context.Capture();
    glfwSwapBuffers(context.window());
    glfwPollEvents();

  } while( glfwGetKey(context.window(), GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
           glfwWindowShouldClose(context.window()) == 0 );

  // Close OpenGL window and terminate GLFW
  glfwTerminate();
  //cv::resize(pixels, pixels, cv::Size(texture_width_, texture_width_));
  //cv::flip(pixels, pixels, 0);

  cv::imwrite("shading.bmp", pixels);
  return 0;
}
//
// Created by Neo on 16/7/17.
// GLFW and GLEW initialization
// Provides @window() as an interaction interface
//

#ifndef RAYTRACING_CONTEXT_H
#define RAYTRACING_CONTEXT_H

#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <opencv2/opencv.hpp>

class Context {
public:
  Context(std::string window_name, int width, int height,
          float z_near = 0.1f, float z_far = 1000.0f);
  cv::Mat CaptureDepth();
  cv::Mat Capture();

  GLFWwindow *window() const {
    return window_;
  }

  void intrinsic_mat(float& fx, float& fy) {
    fx = width_ / (2 * tan(M_PI/8));
    fy = height_ / (2 * tan(M_PI/8));
  }
  const glm::mat4& projection_mat() const {
    return projection_mat_;
  }

private:
  const float kFactor = 5000.0;
  const float kMaxDepth = 40000.0;

  GLFWwindow *window_;
  glm::mat4 projection_mat_;

  cv::Mat color_mat_;
  cv::Mat depth_mat_float_;
  cv::Mat depth_mat_16u_;

  int width_;
  int height_;
  float z_near_;
  float z_far_;
};


#endif //RAYTRACING_CONTEXT_H

//
// Created by Neo on 16/7/17.
//

#include "utils/context.h"

#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

Context::Context(std::string window_name, int width, int height,
                 float z_near, float z_far) {
  // Initialise GLFW
  if( !glfwInit() ) {
    std::cerr << "Failed to initialize GLFW." << std::endl;
    exit(1);
  }

  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // Open a window and create its OpenGL context
  width_ = width;
  height_ = height;
  window_ = glfwCreateWindow(width, height, window_name.c_str(), NULL, NULL);
  if( window_ == NULL ){
    std::cerr << "Failed to open GLFW window." << std::endl;
    glfwTerminate();
    exit(1);
  }
  glfwMakeContextCurrent(window_);

  // Ensure we can capture the escape key being pressed below
  glfwSetInputMode(window_, GLFW_STICKY_KEYS, GL_TRUE);

  // Initialize GLEW
  glewExperimental = GL_TRUE; // Needed for core profile
  if (glewInit() != GLEW_OK) {
    std::cerr << "Failed to initialize GLEW." << std::endl;
    glfwTerminate();
    exit(1);
  }

  /// TODO: K -> perspective
  z_near_ = z_near;
  z_far_ = z_far;
  projection_mat_ = glm::perspective(45.0f, (float)width/(float)height,
                                     z_near, z_far);

#ifdef __APPLE__
  width_ *= 2;
  height_ *= 2;
#endif
  depth_mat_float_ = cv::Mat(height_, width_, CV_32F);
  depth_mat_16u_   = cv::Mat(height_, width_, CV_16U);
}

cv::Mat Context::CaptureDepth() {
  glReadBuffer(GL_BACK);
  glReadPixels(0, 0, width_, height_,
               GL_DEPTH_COMPONENT,
               GL_FLOAT, depth_mat_float_.data);

  for (int i = 0; i < height_; ++i) {
    for (int j = 0; j < width_; ++j) {
      float z = depth_mat_float_.at<float>(i, j);
      if (isnan(z) || isinf(z)) {
        depth_mat_16u_.at<unsigned short>(i, j) = 0;
      } else {
        float clip_z = 2 * z - 1; // [0,1] -> [-1,1]
        // [-(n+f)/(n-f)] + [2nf/(n-f)] / w_z = clip_z
        GLfloat world_z = 2*z_near_*z_far_/(clip_z*(z_near_-z_far_)+
            (z_near_+z_far_));
        float d = world_z * kFactor;
        d = (d > kMaxDepth) ? kMaxDepth : d;
        depth_mat_16u_.at<unsigned short>(i, j) = d;
      }
    }
  }

  cv::Mat ret;
#ifdef __APPLE__
  cv::resize(depth_mat_16u_, ret, cv::Size(depth_mat_16u_.cols/2,
                                           depth_mat_16u_.rows/2),
             CV_INTER_NN);
#else
  ret = depth_mat_16u_.clone();
#endif
  cv::flip(ret, ret, 0);
  return ret;
}
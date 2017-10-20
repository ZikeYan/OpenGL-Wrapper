//
// Created by Neo on 14/08/2017.
//

#include "window.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

namespace gl {
Window::Window(std::string window_name, int width, int height,
               bool unit_visual) {
  // Initialise GLFW
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW." << std::endl;
    exit(1);
  }
  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // Open a window and create its OpenGL context
  pixel_width_  = visual_width_  = width;
  pixel_height_ = visual_height_ = height;

#ifdef __APPLE__
  if (unit_visual) {
    pixel_width_ *= 2;
    pixel_height_ *= 2;
  } else {
    visual_width_ /= 2;
    visual_height_ /= 2;
  }
#endif
  window_ = glfwCreateWindow(visual_width_, visual_height_,
                             window_name.c_str(),
                             NULL, NULL);
  if (window_ == NULL) {
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

  rgb_   = cv::Mat(pixel_height_, pixel_width_, CV_8UC3);
  rgba_  = cv::Mat(pixel_height_, pixel_width_, CV_8UC4);
  depth_ = cv::Mat(pixel_height_, pixel_width_, CV_32F);
}

void Window::Resize(int width, int height, bool unit_visual) {
  pixel_width_ = visual_width_ = width;
  pixel_height_ = visual_height_ = height;
#ifdef __APPLE__
  if (unit_visual) {
    pixel_width_ *= 2;
    pixel_height_ *= 2;
  } else {
    visual_width_ /= 2;
    visual_height_ /= 2;
  }
#endif
  glfwSetWindowSize(window_, visual_width_, visual_height_);
  glfwPollEvents();
  glViewport(0, 0, pixel_width_, pixel_height_);

  rgb_   = cv::Mat(pixel_height_, pixel_width_, CV_8UC3);
  rgba_  = cv::Mat(pixel_height_, pixel_width_, CV_8UC4);
  depth_ = cv::Mat(pixel_height_, pixel_width_, CV_32F);
}

void Window::Bind() {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, pixel_width_, pixel_height_);
}

cv::Mat Window::CaptureRGB() {
  glReadPixels(0, 0, pixel_width_, pixel_height_,
               GL_BGR, GL_UNSIGNED_BYTE, rgb_.data);
  cv::Mat ret;
  cv::flip(rgb_, ret, 0);

  return ret;
}

cv::Mat Window::CaptureRGBA() {
  glReadPixels(0, 0, pixel_width_, pixel_height_,
               GL_BGRA, GL_UNSIGNED_BYTE, rgba_.data);
  cv::Mat ret;
  cv::flip(rgba_, ret, 0);

  return ret;
}

cv::Mat Window::CaptureDepth() {
  glReadBuffer(GL_BACK);
  glReadPixels(0, 0, pixel_width_, pixel_height_,
               GL_DEPTH_COMPONENT, GL_FLOAT, depth_.data);
  cv::Mat ret;
  cv::flip(depth_, ret, 0);

  return ret;
}
}
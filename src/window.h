//
// Created by Neo on 14/08/2017.
//

#ifndef OPENGL_SNIPPET_WINDOW_H
#define OPENGL_SNIPPET_WINDOW_H

#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <opencv2/opencv.hpp>

// set window size
// get screen capture
namespace gl {
class Window {
public:
  // If you want a 1024x768 window,
  // 1. LOOK like 1024x768,      => unit_visual = true
  // 2. STORE PIXELS in 1024x768 => unit_visual = false,
  //    it would look like 512x384 on macOS
  // !unit_visual => unit_pixel
  Window(std::string window_name, int width, int height,
         bool unit_visual = false);
  void Bind();

  /// GLFW operations
  void swap_buffer() {
    glfwSwapBuffers(window_);
    glfwPollEvents();
  }
  int should_close() {
    return glfwWindowShouldClose(window_);
  }
  int get_key(int key) {
    return glfwGetKey(window_, key);
  }

  /// Properties
  const int visual_width() const {
    return visual_width_;
  }
  const int visual_height() const {
    return visual_height_;
  }
  const int pixel_width() const {
    return pixel_width_;
  }
  const int pixel_height() const {
    return pixel_height_;
  }
  void Resize(int width, int height,
              bool unit_visual = false);

  /// Screenshot utilities. A temporary solution.
  /// To take screenshots reliably, render to a fbo and use fbo.Capture().
  cv::Mat CaptureRGB();
  cv::Mat CaptureRGBA();
  cv::Mat CaptureDepth();

private:
  GLFWwindow *window_;
  int visual_width_;
  int visual_height_;
  int pixel_width_;
  int pixel_height_;

  cv::Mat rgb_;
  cv::Mat rgba_;
  cv::Mat depth_;
};
}

#endif //OPENGL_SNIPPET_WINDOW_H

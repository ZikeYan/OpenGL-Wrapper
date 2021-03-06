//
// Created by Neo on 19/10/2017.
//

#ifndef OPENGL_WRAPPER_FRAME_BUFFER_H
#define OPENGL_WRAPPER_FRAME_BUFFER_H

#include <GL/glew.h>
#include <opencv2/opencv.hpp>

#include "texture.h"

namespace gl {
class FrameBuffer {
public:
  FrameBuffer() = default;
  /// !unit_visual => unit_pixel
  FrameBuffer(int internal_type, int width, int height,
              bool unit_visual = false);
  void Bind();

  // pixel unit
  int& height() {
    return height_;
  }
  int& width() {
    return width_;
  }
  GLuint& fbo() {
    return fbo_;
  }
  Texture& texture() {
    return texture_;
  }
  cv::Mat& Capture();

private:
  int height_;
  int width_;

  Texture texture_;
  GLuint fbo_;
  GLuint render_buffer_;

  cv::Mat capture_;
  GLenum format_;
  GLenum type_;
};
}


#endif //OPENGL_WRAPPER_FRAME_BUFFER_H

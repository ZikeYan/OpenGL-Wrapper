//
// Created by Neo on 19/10/2017.
//

#include "framebuffer.h"

namespace gl {
FrameBuffer::FrameBuffer(int internal_format, int width, int height) {
  fbo_ = 0;
  glGenFramebuffers(1, &fbo_);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo_);

  width_ = width;
  height_ = height;
  texture_.Init(internal_format, width, height);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                       texture_.id(), 0);

  // Depth -> render buffer, for depth example
  render_buffer_ = 0;
  glGenRenderbuffers(1, &render_buffer_);
  glBindRenderbuffer(GL_RENDERBUFFER, render_buffer_);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT,
                        texture_.width(), texture_.height());
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                            GL_RENDERBUFFER, render_buffer_);

  // TODO: check this
  GLenum draw_buffers[1] = {GL_COLOR_ATTACHMENT0};
  glDrawBuffers(1, draw_buffers);

  if(glCheckFramebufferStatus(GL_FRAMEBUFFER)
     != GL_FRAMEBUFFER_COMPLETE) {
    std::cout << "Unable to create frame buffer!\n";
    exit(-1);
  }

  /// Decide type of the pixel buffer
  switch (internal_format) {
    case GL_RGBA32F: // Remain channels (value passing)
      format_ = GL_RGBA;
      type_ = GL_FLOAT;
      capture_ = cv::Mat(texture_.height(), texture_.width(), CV_32FC4);
      break;
    case GL_RGBA: // RGB->BGR in opencv2
      format_ = GL_BGRA;
      type_ = GL_UNSIGNED_BYTE;
      capture_ = cv::Mat(texture_.height(), texture_.width(), CV_8UC4);
      break;
    case GL_RGB: // RGB->BGR in opencv2
    default:
      format_ = GL_BGR;
      type_ = GL_UNSIGNED_BYTE;
      capture_ = cv::Mat(texture_.height(), texture_.width(), CV_8UC3);
      break;
  }
}

void FrameBuffer::Bind() {
  glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
  glViewport(0, 0, texture_.width(), texture_.height());
}

cv::Mat& FrameBuffer::Capture() {
  texture_.Bind(0);
  glGetTexImage(GL_TEXTURE_2D, 0, format_, type_, capture_.data);
  cv::flip(capture_, capture_, 0);
  return capture_;
}
}
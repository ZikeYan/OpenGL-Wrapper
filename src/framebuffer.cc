//
// Created by Neo on 19/10/2017.
//

#include "framebuffer.h"

namespace gl {
FrameBuffer::FrameBuffer(int internal_type, int width, int height) {
  fbo_ = 0;
  glGenFramebuffers(1, &fbo_);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo_);

  width_ = width;
  height_ = height;
  texture_.Init(internal_type, width, height);
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
}

}
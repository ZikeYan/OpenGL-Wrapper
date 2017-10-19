//
// Created by Neo on 19/10/2017.
//

#ifndef OPENGL_WRAPPER_FRAME_BUFFER_H
#define OPENGL_WRAPPER_FRAME_BUFFER_H

#include <GL/glew.h>
#include "texture.h"

namespace gl {
class FrameBuffer {
public:
  FrameBuffer() = default;
  FrameBuffer(int internal_type, int width, int height);

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

private:
  int height_;
  int width_;
  Texture texture_;
  GLuint fbo_;
  GLuint render_buffer_;
};
}


#endif //OPENGL_WRAPPER_FRAME_BUFFER_H

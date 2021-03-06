//
// Created by Neo on 14/08/2017.
//

#include "texture.h"

namespace gl {

Texture::~Texture() {
  if (texture_gened_)
    glDeleteTextures(1, &texture_id_);
}

void Texture::Load(std::string texture_path) {
  texture_ = cv::imread(texture_path);
  cv::flip(texture_, texture_, 0);

  width_  = texture_.cols;
  height_ = texture_.rows;
}

void Texture::Load(cv::Mat &texture) {
  texture.copyTo(texture_);
  cv::flip(texture_, texture_, 0);

  width_  = texture_.cols;
  height_ = texture_.rows;
}

void Texture::Init(std::string texture_path) {
  Load(texture_path);
  ConfigTexture();
}

void Texture::Init(cv::Mat& texture) {
  Load(texture);
  ConfigTexture();
}

/// Specifically used in Framebuffer
void Texture::Init(GLint internal_format,
                   int width, int height,
                   bool unit_visual) {
  width_ = width;
  height_ = height;
#ifdef __APPLE__
  if (unit_visual) {
    width_ *= 2;
    height_ *= 2;
  }
#endif

  // TODO: complete the switch table
  GLenum output_format, type;
  switch (internal_format) {
    case GL_RGBA32F:
      output_format = GL_RGBA;
      type = GL_FLOAT;
      break;
    case GL_RGBA:
      output_format = GL_RGBA;
      type = GL_UNSIGNED_BYTE;
      break;
    case GL_RGB:
    default:
      output_format = GL_RGB;
      type = GL_UNSIGNED_BYTE;
      break;
  }

  glGenTextures(1, &texture_id_);
  glBindTexture(GL_TEXTURE_2D, texture_id_);
  glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width_, height_, 0,
               output_format, type, 0);
  // Clamp to edge, since we are writing to it
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  texture_gened_ = true;
}

void Texture::Bind(int texture_idx) {
  GLenum texture_macro = GL_TEXTURE0 + texture_idx;
  glActiveTexture(texture_macro);
  glBindTexture(GL_TEXTURE_2D, texture_id_);
}

void Texture::ConfigTexture() {
  glGenTextures(1, &texture_id_);
  glBindTexture(GL_TEXTURE_2D, texture_id_);
  // TODO: change format according to loaded texture
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_, height_, 0,
               GL_BGR, GL_UNSIGNED_BYTE, texture_.data);

  // ... nice trilinear filtering.
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glGenerateMipmap(GL_TEXTURE_2D);

  texture_gened_ = true;
}
}

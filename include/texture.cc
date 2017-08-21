//
// Created by Neo on 14/08/2017.
//

#include "texture.h"

namespace gl {
Texture::Texture(std::string texture_path){
  Load(texture_path);
  Init();
}

void Texture::Load(std::string texture_path) {
  texture_ = cv::imread(texture_path);
  width_  = texture_.cols;
  height_ = texture_.rows;
}

void Texture::Init() {
  glGenTextures(1, &texture_id_);
  glBindTexture(GL_TEXTURE_2D, texture_id_);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_, height_, 0, GL_BGR,
               GL_UNSIGNED_BYTE, texture_.data);

  // ... nice trilinear filtering.
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glGenerateMipmap(GL_TEXTURE_2D);
}

void Texture::Bind(int texture_idx) {
  GLenum texture_macro = GL_TEXTURE0 + texture_idx;
  glActiveTexture(texture_macro);
  glBindTexture(texture_id_, texture_macro);
}
}

//
// Created by Neo on 14/08/2017.
//

#ifndef OPENGL_SNIPPET_TEXTURE_H
#define OPENGL_SNIPPET_TEXTURE_H

#include <string>
#include <GL/glew.h>
#include <opencv2/opencv.hpp>

namespace gl {
class Texture {
public:
  Texture() = default;
  void Load(std::string texture_path);

  /// Init for reading
  void Init();
  /// Init for writing. Note differences in filtering
  void Init(GLint internal_format,
            int width, int height);

  explicit
  Texture(std::string texture_path);

  void Bind(int texture_idx);

  const GLuint id() const {
    return texture_id_;
  }
  const int width() const {
    return width_;
  }
  const int height() const {
    return height_;
  }

private:

  cv::Mat texture_;
  int width_;
  int height_;
  GLuint texture_id_;
};
}


#endif //OPENGL_SNIPPET_TEXTURE_H

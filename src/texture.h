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
  /// Texture t = Texture(); t.Init( /* for read or write */);
  Texture() = default;
  ~Texture();

  void Load(std::string texture_path);
  void Load(cv::Mat& texture);

  /// Init for reading. Load from file.
  void Init(std::string texture_path);
  void Init(cv::Mat& texture);

  /// Init for writing. Note differences in filtering
  // !unit_visual => unit_pixel
  void Init(GLint internal_format,
            int width, int height,
            bool unit_visual = false);

  void Bind(int texture_idx);

  cv::Mat &image() {
    return texture_;
  }
  const GLuint id() const {
    return texture_id_;
  }
  // pixel unit
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

  bool texture_gened_ = false;

  void ConfigTexture();
};
}


#endif //OPENGL_SNIPPET_TEXTURE_H

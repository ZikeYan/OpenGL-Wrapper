//
// Created by Neo on 16/8/2.
//

#ifndef OPENGL_TEMPLATE_GLOBJECT_H
#define OPENGL_TEMPLATE_GLOBJECT_H

#include <string>
#include <vector>
#include <GL/glew.h>

enum UniformType {
  kTexture,
  kMatrix4f
};

/// I classify OpenGL objects into two kinds:
/// type 1. MeshObject,  for traditional mesh rendering
/// type 2. FrameObject, only for efficient image displaying
/// Actually, type 2 is a specific instance of type 1;
/// as it is widely used, I make it special

class GLObject {
public:
  GLObject();
  ~GLObject();

  /// Part initialization
  /// Shared: do not override
  void InitShader(std::string vert_shader_path,
                  std::string frag_shader_path,
                  std::vector<std::pair<UniformType, std::string> >& uniforms);

  virtual void InitVAO(std::vector<int>& count_of_objects) = 0;
  virtual void InitTexture(int width, int height) = 0;

  /// Part argument passing
  /// Shared: do not override
  void SetUniforms(std::vector<void*> uniform_vals);
  void SetTexture(unsigned char* data, int width, int height);

  /// Part rendering
  virtual void Render() = 0;

protected:
  // These are setted by inherited classes
  bool is_vao_inited_ = false;
  bool is_texture_inited_ = false;

  // This is setted by the base class
  bool is_shader_inited_ = false;

  GLuint vao_;
  GLuint program_;

  int     vbo_count_;
  GLuint* vbos_; // glGenBuffer uses array
  std::vector<int> count_of_objects_;

  /// At current support 1 texture only
  GLuint  texture_;

  int     uniform_count_;
  std::vector<std::pair<UniformType, GLint> > uniforms_;
  std::vector<void*> uniform_values_;
};


#endif //OPENGL_TEMPLATE_GLOBJECT_H

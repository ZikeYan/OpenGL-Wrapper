//
// Created by Neo on 14/08/2017.
//

#ifndef OPENGL_SNIPPET_SHADER_H
#define OPENGL_SNIPPET_SHADER_H

#include <string>
#include <GL/glew.h>
namespace gl {
enum ShaderType {
  kVertexShader,
  kFragmentShader
};

class Program {
public:
  /// load & replace => Build (compile & link)
  Program() = default;
  ~Program();

  void Load(std::string shader_path, ShaderType type);
  void Build();

  const GLuint id() const {
    return program_id_;
  }

private:
  std::string vert_shader_path_;
  std::string vert_shader_str_;

  std::string frag_shader_path_;
  std::string frag_shader_str_;

  GLint Compile(const std::string& shader_str, GLuint &shader_id);
  GLint Link(GLuint &program_id,
             GLuint &vert_shader_id,
             GLuint &frag_shader_id);

  bool program_built_ = false;
  GLuint program_id_;
};
}


#endif //OPENGL_SNIPPET_SHADER_H

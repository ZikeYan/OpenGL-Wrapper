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

enum MeshType {
  kVertex,
  kVertexFace,
  kVertexNormalFace,
  kVertexNormalUVFace,
  kImage // Special case
};

class GLObject {
public:
  GLObject(MeshType mesh_type);
  ~GLObject();

  /// Part 1: initialization
  void InitShader(std::string vert_shader_path,
                  std::string frag_shader_path,
                  std::vector<std::pair<UniformType, std::string> >& uniforms);
  void InitVAO(std::vector<int>& count_of_objects);
  void InitTexture(int width, int height);

  /// Part 2: data passing
  void SetUniforms(std::vector<void*> uniform_values);

  // kImage
  void SetMesh();
  // kVertex
  void SetMesh(float* vertices);
  // kVertexFace
  void SetMesh(float* vertices,
               unsigned int* faces);
  // kVertexNormalFace
  void SetMesh(float* vertices, float* normals,
               unsigned int* faces);
  // kVertexNormalUVFace
  void SetMesh(float* vertices, float* normals, float* uvs,
               unsigned int* faces);

  void SetTexture(unsigned char* data, int width, int height);

  /// Part 3: rendering
  void Render();

protected:
  MeshType mesh_type_;

  bool is_shader_inited_  = false;
  bool is_vao_inited_     = false;
  bool is_texture_inited_ = false;

  /// Shaders and uniforms
  GLuint  program_;
  int     uniform_count_;
  // Correspondence is required
  // TODO: use unordered_map to manage
  // uniform_name -> (type, GLint) and
  // uniform_name -> (value)
  std::vector<std::pair<UniformType, GLint> > uniforms_;
  std::vector<void*>                          uniform_values_;

  /// Mesh data buffers
  GLuint  vao_;
  int     vbo_count_;
  // glGenBuffer uses raw array, otherwise I prefer use vector
  GLuint* vbos_;
  std::vector<int> count_of_objects_;

  /// Texture, only 1 texture is supported
  GLuint  texture_;
};


#endif //OPENGL_TEMPLATE_GLOBJECT_H

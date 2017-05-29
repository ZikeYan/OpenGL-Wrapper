//
// Created by Neo on 16/8/2.
//

#ifndef OPENGL_TEMPLATE_GLOBJECT_H
#define OPENGL_TEMPLATE_GLOBJECT_H

#include <string>
#include <vector>
#include <unordered_map>
#include <GL/glew.h>

enum UniformType {
  kTexture,
  kMatrix4f,
  kNull
};

enum MeshType {
  kVertex,
  kVertexFace,
  kVertexNormalFace,
  kVertexNormalUVFace,
  kImage // Special case
};

struct Uniform {
  Uniform() : type(kNull), id(-1), data(NULL) {}
  Uniform(UniformType _type, GLint _id, void* _data)
  : type(_type), id(_id), data(_data) {}

  UniformType type;
  GLint id;
  void* data;
};

class GLObject {
public:
  GLObject(MeshType mesh_type);
  ~GLObject();

  /// Part 1: initialization
  void InitShader(std::string vert_shader_path,
                  std::string frag_shader_path,
                  std::unordered_map<std::string, UniformType >& uniform_types);
  void InitVAO(std::vector<int>& max_count_of_objects);
  void InitTexture(int width, int height);

  /// Part 2: data passing
  void SetUniforms(std::unordered_map<std::string, void*>& uniform_values);

  // kImage
  void SetMesh();
  // kVertex
  void SetMesh(float* vertices, int vertex_count);
  // kVertexFace
  void SetMesh(float* vertices, int vertex_count,
               unsigned int* faces, int face_count);
  // kVertexNormalFace
  void SetMesh(float* vertices, int vertex_count,
               float* normals, int normal_count,
               unsigned int* faces, int face_count);
  // kVertexNormalUVFace
  void SetMesh(float* vertices, int vertex_count,
               float* normals, int normal_count,
               float* uvs, int uv_count,
               unsigned int* faces, int face_count);

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
  // uniform_name -> (type, GLint) and
  // uniform_name -> (value)
  std::unordered_map<std::string, Uniform> uniforms_;

  /// Mesh data buffers
  GLuint  vao_;
  int     vbo_count_;
  // glGenBuffer uses raw array, otherwise I prefer use vector
  GLuint* vbos_;
  std::vector<int> max_count_of_objects_;
  std::vector<int> curr_count_of_objects_;

  /// Texture, only 1 texture is supported
  GLuint  texture_;
};


#endif //OPENGL_TEMPLATE_GLOBJECT_H

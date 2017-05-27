//
// Created by Neo on 26/05/2017.
//

#ifndef OPENGL_SNIPPET_MESH_OBJECT_H
#define OPENGL_SNIPPET_MESH_OBJECT_H

#include "gl_object.h"

enum MeshType {
  kVertex,
  kVertexFace,
  kVertexNormalFace,
  kVertexNormalUVFace
};

class MeshObject : public GLObject {
public:
  MeshObject(MeshType mesh_type);
  ~MeshObject();

  /// Vertex: sizeof(float) * 3 * vertex_count
  /// Normal: sizeof(float) * 3 * normal_count
  /// UV:     sizeof(float) * 2 * uv_count
  /// face:   sizeof(unsigned int) * 3 * face_count
  void InitVAO(std::vector<int>& count_of_objects);
  void InitTexture(int width, int height);

  // void LoadMesh(std::string obj_path)
  void LoadMesh(float* vertices);
  void LoadMesh(float* vertices,
                unsigned int* indices);
  void LoadMesh(float* vertices, float* normals,
                unsigned int* indices);
  void LoadMesh(float* vertices, float* normals, float* uvs,
                unsigned int* indices);

  void Render();

protected:
  MeshType mesh_type_;

  // float3
  std::vector<float> vertices;
  std::vector<float> normals;

  // float2
  std::vector<float> uvs;

  // uint3
  std::vector<unsigned int> faces;
};


#endif //OPENGL_SNIPPET_MESH_OBJECT_H

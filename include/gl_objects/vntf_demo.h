//
// Created by Neo on 28/05/2017.
//

#ifndef OPENGL_SNIPPET_VNTF_DEMO_H
#define OPENGL_SNIPPET_VNTF_DEMO_H

#include "gl_objects/gl_object.h"
#include "utils/context.h"
#include "utils/control.h"
#include "utils/model.h"
#include "utils/texture.h"

/// Mesh of vertex, normal, texture and faces
class VNTFDemo {
public:
  VNTFDemo(Context* context) {
    context_ = context;
    /// Load all data
    LoadModel("../obj/f16.obj", vertices_, uvs_, normals_, indices_);
    LoadTexture("../obj/f16.bmp", texture_data_,
                texture_width_, texture_height_);

    object_  = new GLObject(kVertexNormalUVFace);
    control_ = new Control(context->window());

    /// Prepare shader
    std::unordered_map<std::string, UniformType> uniform_names;
    uniform_names["mvp"]            = kMatrix4f;
    uniform_names["c_T_w"]          = kMatrix4f;
    uniform_names["texture_sampler"] = kTexture;
    object_->InitShader("../shader/vntf_vertex.glsl",
                        "../shader/vntf_fragment.glsl",
                        uniform_names);

    /// Prepare data buffer
    std::vector<int> count_of_objects;
    count_of_objects.push_back(vertices_.size());
    count_of_objects.push_back(normals_.size());
    count_of_objects.push_back(uvs_.size());
    count_of_objects.push_back(indices_.size() / 3);
    object_->InitVAO(count_of_objects);

    /// Prepare texture
    object_->InitTexture(texture_width_, texture_height_);
  }

  void Render() {
    control_->UpdateCameraPose();

    glm::mat4 transform = glm::mat4(1.0f);
    transform[1][1] = transform[2][2] = -1;
    glm::mat4 model = glm::mat4(1.0f);
    model[0][0] = model[1][1] = -1;
    glm::mat4 mvp = context_->projection_mat() *
                    control_->view_mat() * transform *
                    model;
    glm::mat4 v = control_->view_mat();

    object_->SetMesh((float*)vertices_.data(), vertices_.size(),
                     (float*)normals_.data(), normals_.size(),
                     (float*)uvs_.data(), uvs_.size(),
                     indices_.data(), indices_.size()/3);
    object_->SetTexture(texture_data_,
                        texture_width_, texture_height_);

    std::unordered_map<std::string, void*> uniform_values;
    uniform_values["mvp"]            = (void*)&mvp[0][0];
    uniform_values["c_T_w"]          = (void*)&v[0][0];
    uniform_values["texture_sampler"] = NULL;

    object_->SetUniforms(uniform_values);
    object_->Render();
  }

  ~VNTFDemo() {
    delete object_;
    delete control_;
  }

private:
  Context* context_;
  GLObject *object_;
  Control *control_;

  std::vector<glm::vec3> vertices_;
  std::vector<glm::vec3> normals_;
  std::vector<glm::vec2> uvs_;
  std::vector<unsigned int> indices_;

  unsigned char* texture_data_;
  int texture_width_;
  int texture_height_;
};


#endif //OPENGL_SNIPPET_VNTF_DEMO_H

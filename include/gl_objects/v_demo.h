//
// Created by Neo on 28/05/2017.
//

#ifndef OPENGL_SNIPPET_V_DEMO_H
#define OPENGL_SNIPPET_V_DEMO_H

#include "gl_objects/gl_object.h"
#include "utils/context.h"
#include "utils/control.h"
#include "utils/model.h"
#include "utils/texture.h"

class VDemo {
public:
  VDemo(Context* context, int vertex_count) {
    context_ = context;

    /// Load all data
    object_  = new GLObject(kVertex);
    control_ = new Control(context->window());

    /// Prepare shader
    std::unordered_map <std::string, UniformType> uniform_names;
    uniform_names["mvp"] = kMatrix4f;
    object_->InitShader("../shader/vertex_v.glsl",
                        "../shader/fragment_v.glsl",
                        uniform_names);

    /// Prepare data buffer
    std::vector<int> count_of_objects;
    count_of_objects.push_back(vertex_count);
    count_of_objects.push_back(vertex_count);
    object_->InitVAO(count_of_objects);

    glfwPollEvents();

    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
  }

  void Render(float* vertices, int vertex_count, cv::Mat c_T_w,
              bool use_input = false) {
    // between world coordinate system -> OpenGL
    glm::mat4 transform = glm::mat4(1.0f);
    transform[1][1] = transform[2][2] = -1.0f;
    glm::mat4 view_mat = glm::mat4(1.0f);
    for (int i = 0; i < 4; ++i)
      for (int j = 0; j < 4; ++j)
        view_mat[i][j] = c_T_w.at<float>(j, i);
    view_mat = transform * view_mat;

    if (use_input) {
      control_->UpdateCameraPose();
      view_mat = control_->view_mat() * transform;
    }

    glm::mat4 mvp = context_->projection_mat() *
                    view_mat *
                    glm::mat4(1.0f);

    object_->SetMesh(vertices, vertex_count);

    std::unordered_map<std::string, void *> uniform_values;
    uniform_values["mvp"] = (void *) &mvp[0][0];

    object_->SetUniforms(uniform_values);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    object_->Render();
    glfwSwapBuffers(context_->window());
    glfwPollEvents();
  }

  ~VDemo() {
    delete object_;
    delete control_;
  }

private:
  Context *context_;
  GLObject *object_;
  Control *control_;
};

#endif
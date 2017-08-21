//
// Created by Neo on 14/08/2017.
//

#ifndef OPENGL_SNIPPET_CAMERA_H
#define OPENGL_SNIPPET_CAMERA_H

// set K, P
// conversions regarding captured image from Window
// TODO: add an assistance class for trajectory
#include <GL/glew.h>
#include <glm/glm.hpp>

namespace gl {
class Camera {
public:
  Camera() = default;
  // Should be coincide with the window

  Camera(float fov, int width, int height,
         float z_near, float z_far);

  void set_perspective(float fov, int width, int height,
                       float z_near, float z_far) {
    projection_ = glm::perspective(fov, (float)width/(float)height,
                                   z_near, z_far);
  }
  void set_view(glm::mat4 view) {
    view_ = view;
  }
  void set_model(glm::mat4 model) {
    model_ = model;
  }

  /// Usually used for uniforms
  const glm::mat4 mvp() {
    return projection_ * view_ * model_;
  }
  const glm::mat4 projection() const {
    return projection_;
  }
  const glm::mat4 view() const {
    return view_;
  }
  const glm::mat4 model() const {
    return model_;
  }

private:
  glm::mat4 projection_; // K
  glm::mat4 view_;       // [R | t]
  glm::mat4 model_;      // e.g. {x, -y, -z}
};
}


#endif //OPENGL_SNIPPET_CAMERA_H

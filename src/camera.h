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
#include <glm/gtc/matrix_transform.hpp>

#include <opencv2/opencv.hpp>

#include "window.h"

namespace gl {
class Camera {
public:
  /// Camera c = Camera(); c.set_perspective( ... )
  /// or Camera c = Camera( ... )
  Camera() = default;
  // Should be coincide with the window

  Camera(int width, int height,
         float fov = 45.0f,
         float z_near = 0.1f,
         float z_far = 100.0f);

  cv::Mat ConvertDepthBuffer(cv::Mat& depthf, float factor);

  void set_perspective(int width, int height,
                       float fov, float z_near, float z_far) {
    z_near_ = z_near;
    z_far_ = z_far;
    projection_ = glm::perspective(fov, (float)width/(float)height,
                                   z_near, z_far);
  }
  // Manually set view from input (short version)
  void set_view(glm::mat4 view) {
    view_ = view;
  }
  // ... Or set it with interactions (long version)
  void SwitchInteraction(bool enable_interaction);
  void UpdateView(Window &window);

  void set_model(glm::mat4 model) {
    model_ = model;
  }

  /// Usually used for uniforms
  glm::mat4 mvp() {
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
  float z_near_;
  float z_far_;

  glm::mat4 projection_; // K
  glm::mat4 view_;       // [R | t]
  glm::mat4 model_;      // e.g. {x, -y, -z}

  // Polar-system: interaction from the keyboard
  bool interaction_enabled_;
  glm::vec3 position_;
  float elevation_;
  float azimuth_;

  float kMoveSpeed = 3.0f;
  float kRotateSpeed = 0.5f;
};
}


#endif //OPENGL_SNIPPET_CAMERA_H

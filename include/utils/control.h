//
// Created by Neo on 16/7/17.
//

#ifndef RAYTRACING_CONTROL_H
#define RAYTRACING_CONTROL_H

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class Control {
public:
  glm::mat4 view_mat();
  glm::mat4 projection_mat();
  void UpdateCameraPose();
  Control(GLFWwindow *window);

private:
  // Descartes-system
  glm::mat4 view_mat_;
  glm::mat4 projection_mat_;
  glm::vec3 position_;

  // Polar-system parameters
  float horizontal_angle_;
  float vertical_angle_;
  float fov_;

  // Interaction parameters
  float rotate_speed_;
  float move_speed_;
  GLFWwindow *window_;

};

#endif //RAYTRACING_CONTROL_H

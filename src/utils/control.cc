//
// Created by Neo on 16/7/17.
//

#include "utils/control.h"

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

Control::Control(GLFWwindow *window) {
  window_ = window;

  position_ = glm::vec3( 0, 0, 2 );
  horizontal_angle_ = 3.14f;
  vertical_angle_ = 0.0f;
  fov_ = 45.0f;
  move_speed_ = 3.0f;
  rotate_speed_ = 0.005f;

  projection_mat_ = glm::perspective(fov_, 4.0f / 3.0f, 0.01f, 10000.0f);
}

glm::mat4 Control::view_mat() {
  return view_mat_;
}
glm::mat4 Control::projection_mat() {
  return projection_mat_;
}

void Control::UpdateCameraPose(){
  // glfwGetTime is called only once, the first time this function is called
  static double last_time = glfwGetTime();

  double current_time = glfwGetTime();
  float delta_time = float(current_time - last_time);

  // Get mouse position
  double xpos, ypos;
  glfwGetCursorPos(window_, &xpos, &ypos);

  int width, height;
  glfwGetFramebufferSize(window_, &width, &height);
  glfwSetCursorPos(window_, width/2, height/2);
#ifdef __APPLE__
#endif

  // Compute new orientation
  horizontal_angle_ += rotate_speed_ * float(width/2 - xpos );
  vertical_angle_   += rotate_speed_ * float(height/2 - ypos );

  glm::vec3 look_direction(
      cos(vertical_angle_) * sin(horizontal_angle_),
      sin(vertical_angle_),
      cos(vertical_angle_) * cos(horizontal_angle_));

  glm::vec3 move_direction(
      cos(vertical_angle_) * sin(horizontal_angle_),
      0,
      cos(vertical_angle_) * cos(horizontal_angle_));

  glm::vec3 right = glm::vec3(
      sin(horizontal_angle_ - 3.14f/2.0f),
      0,
      cos(horizontal_angle_ - 3.14f/2.0f));

  glm::vec3 up = glm::cross(right, look_direction );

  if (glfwGetKey( window_, GLFW_KEY_W ) == GLFW_PRESS) {
    position_ += move_direction * move_speed_ * delta_time;
  }
  if (glfwGetKey( window_, GLFW_KEY_S ) == GLFW_PRESS) {
    position_ -= move_direction * move_speed_ * delta_time;
  }
  if (glfwGetKey( window_, GLFW_KEY_D ) == GLFW_PRESS) {
    position_ += right * move_speed_ * delta_time;
  }
  if (glfwGetKey( window_, GLFW_KEY_A ) == GLFW_PRESS) {
    position_ -= right * move_speed_ * delta_time;
  }
  if (glfwGetKey(window_, GLFW_KEY_SPACE) == GLFW_PRESS) {
    position_.y += move_speed_ * delta_time;
  }
  if (glfwGetKey(window_, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
    position_.y -= move_speed_ * delta_time;
  }

  // Camera matrix
  view_mat_       = glm::lookAt(
      position_,
      position_ + look_direction,
      up);

  last_time = current_time;
}
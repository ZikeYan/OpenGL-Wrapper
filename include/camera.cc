//
// Created by Neo on 14/08/2017.
//

#include <glm/gtc/matrix_transform.hpp>
#include "camera.h"

namespace gl {
Camera::Camera(float fov, int width, int height,
               float z_near, float z_far) {
  set_perspective(fov, width, height, z_near, z_far);
}
}
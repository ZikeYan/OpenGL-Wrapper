//
// Created by Neo on 22/08/2017.
//

#ifndef OPENGL_SNIPPET_MODEL_H
#define OPENGL_SNIPPET_MODEL_H

#include <string>
#include <vector>
#include <glm/glm.hpp>

namespace gl {
class Model {
public:
  Model() = default;
  void LoadObj(std::string path);

  explicit
  Model(std::string path);

  std::vector<glm::vec3>& positions() {
    return positions_;
  }

  std::vector<glm::vec3>& normals() {
    return normals_;
  }

  std::vector<glm::vec2>& uvs() {
    return uvs_;
  }

  std::vector<unsigned int>& indices() {
    return indices_;
  }

private:
  std::vector<glm::vec3> positions_;
  std::vector<glm::vec3> normals_;
  std::vector<glm::vec2> uvs_;
  std::vector<unsigned int> indices_;
};
}


#endif //OPENGL_SNIPPET_MODEL_H

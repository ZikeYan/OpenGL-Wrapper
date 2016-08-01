//
// Created by Neo on 16/7/29.
//

#ifndef RAYTRACING_MODEL_H
#define RAYTRACING_MODEL_H

#include <string>
#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>

int LoadModel(std::string model_path,
              std::vector<glm::vec3> &vertices,
              std::vector<glm::vec2> &uvs,
              std::vector<glm::vec3> &normals,
              std::vector<unsigned int> &indices);

#endif //RAYTRACING_MODEL_H

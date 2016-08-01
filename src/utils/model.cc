//
// Created by Neo on 16/7/29.
//

#include "utils/model.h"

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>

int LoadModel(std::string model_path,
              std::vector<glm::vec3> &vertices,
              std::vector<glm::vec2> &uvs,
              std::vector<glm::vec3> &normals,
              std::vector<unsigned int> &indices) {
  // Assuming it is .obj
  std::ifstream obj_stream(model_path, std::ios::in);

  std::vector<glm::vec3> raw_vertices;
  std::vector<glm::vec2> raw_uvs;
  std::vector<glm::vec3> raw_normals;
  std::vector<std::string> raw_indices;

  // Load data into vectors
  for (std::string line; std::getline(obj_stream, line); ) {
    std::stringstream line_stream(line);
    std::string tag;
    line_stream >> tag;
    if (tag == "v") { // vertex coordinate
      glm::vec3 vertex;
      line_stream >> vertex.x >> vertex.y >> vertex.z;
      raw_vertices.push_back(vertex);
    } else if (tag == "vt") { // uv coordinate
      glm::vec2 uv;
      line_stream >> uv.x >> uv.y;
      raw_uvs.push_back(uv);
    } else if (tag == "vn") { // normal
      glm::vec3 normal;
      line_stream >> normal.x >> normal.y >> normal.z;
      raw_normals.push_back(normal);
    } else if (tag == "f") { // indices
      std::string slashed_index;
      for (int i = 0; i < 3; ++i) {
        line_stream >> slashed_index;
        raw_indices.push_back(slashed_index);
      }
    }
  }

  // Assign new indices for "vi/uvi/ni" -> i
  std::unordered_map<std::string, unsigned int> new_indices;
  unsigned int new_index = 0;
  for (auto i = raw_indices.begin(); i != raw_indices.end(); ++i) {
    if (new_indices.find(*i) == new_indices.end()) {
      new_indices[*i] = new_index;
      new_index ++;
    }
  }

  // Generate new vectors
  for (auto i = raw_indices.begin(); i != raw_indices.end(); ++i) {
    //std::cout << *i << " | "
    //<< new_indices[*i] << "/" << vertices.size() << std::endl;
    indices.push_back(new_indices[*i]);

    // Existed index
    if (new_indices[*i] < vertices.size()) continue;

    std::stringstream indice_group_stream(*i);
    char slash;
    unsigned int vertex_index, uv_index, normal_index;
    indice_group_stream
    >> vertex_index >> slash >> uv_index >> slash >> normal_index;

    vertices.push_back(raw_vertices[vertex_index - 1]);
    uvs.push_back(raw_uvs[uv_index - 1]);
    normals.push_back(raw_normals[normal_index - 1]);
  }
  return 0;
}
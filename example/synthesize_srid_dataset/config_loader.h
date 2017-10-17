//
// Created by Neo on 16/10/2017.
//

#ifndef OPENGL_WRAPPER_CONFIG_LOADER_H
#define OPENGL_WRAPPER_CONFIG_LOADER_H

#include <iostream>
#include <string>
#include <vector>

#include <opencv2/opencv.hpp>
#include <glm/glm.hpp>

class ConfigLoader {
public:
  /// Properties
  float light_power;
  std::vector<glm::vec3> light_positions;
  std::vector<glm::vec3> light_colors;

  std::string input_texture_path;
  std::string input_model_path;
  int downsample_factor;

  std::string output_path;
  int output_width_original;
  int output_height_original;

  float radius_min;
  float radius_max;
  float radius_step;
  float azimuth_step;
  float elevation_step;
  std::vector<glm::vec3> camera_positions;
  std::vector<glm::ivec3> camera_polars;

  bool show_light_sources;
  bool use_preset_path;
  /// Methods
  void LoadParams(std::string path) {
    cv::FileStorage fs(path, cv::FileStorage::READ);

    show_light_sources = (int)fs["show_light_sources"];
    use_preset_path = (int)fs["use_preset_path"];
    input_texture_path = (std::string)fs["input_texture_path"];
    input_model_path = (std::string)fs["input_model_path"];
    downsample_factor = (int)fs["downsample_factor"];
    std::cout << input_texture_path << std::endl;

    std::cout << downsample_factor << std::endl;

    output_path = (std::string)fs["output_path"];
    output_width_original = (int)fs["output_width_original"];
    output_height_original = (int)fs["output_height_original"];
    std::cout << output_width_original << std::endl;
  }

  void LoadLights(std::string path) {
    cv::FileStorage fs(path, cv::FileStorage::READ);
    cv::FileNode fs_lights = fs["lights"];

    for (auto iter = fs_lights.begin(); iter != fs_lights.end(); ++iter) {
      light_positions.push_back(glm::vec3((float)(*iter)["x"],
                                          (float)(*iter)["y"],
                                          (float)(*iter)["z"]));
      light_colors.push_back(glm::vec3((float)(*iter)["r"],
                                       (float)(*iter)["g"],
                                       (float)(*iter)["b"]));
    }

    light_power = (float)fs["light_power"];
  }

  void LoadPositions(std::string path) {
    cv::FileStorage fs(path, cv::FileStorage::READ);

    radius_min = (float)fs["radius_min"];
    radius_max = (float)fs["radius_max"];
    radius_step = (float)fs["radius_step"];

    azimuth_step = (float)fs["azimuth_step"];
    elevation_step = (float)fs["elevation_step"];

    cv::FileNode fs_positions = fs["camera_positions"];

    for (auto iter = fs_positions.begin(); iter != fs_positions.end(); ++iter) {
      camera_positions.push_back(glm::vec3((float)(*iter)["x"],
                                           (float)(*iter)["y"],
                                           (float)(*iter)["z"]));
      camera_polars.push_back(glm::vec3((int)(*iter)["r"],
                                        (int)(*iter)["e"],
                                        (int)(*iter)["a"]));
    }
  }


};
#endif //OPENGL_WRAPPER_CONFIG_LOADER_H

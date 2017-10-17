//
// Created by Neo on 17/10/2017.
//

#include <cmath>
#include <string>
#include <opencv2/opencv.hpp>

void GenerateSphericalPositions(
    std::string output_path,
    float elevation_step,
    float azimuth_step,
    float radius_min,
    float radius_max,
    float radius_step) {
  cv::FileStorage fs(output_path, cv::FileStorage::WRITE);

  fs << "radius_min" << radius_min;
  fs << "radius_max" << radius_max;
  fs << "radius_step" << radius_step;

  fs << "azimuth_step" << azimuth_step;
  fs << "elevation_step" << elevation_step;

  fs << "camera_positions" << "[";
  int r = 0;
  for (float radius = radius_min;
       radius < radius_max;
       radius += radius_step, ++r) {
    int e = 0;
    for (float elevation = -M_PI_2;
         elevation <= M_PI_2;
         elevation += elevation_step, ++e) {
      int a = 0;
      for (float azimuth = 0;
           azimuth < 2*M_PI;
           azimuth += azimuth_step, ++a) {
        fs << "{:"
           << "x" << radius * cos(elevation) * cos(azimuth)
           << "y" << radius * cos(elevation) * sin(azimuth)
           << "z" << radius * sin(elevation)
           << "r" << r << "e" << e << "a" << a
           << "}";
      }
    }
  }
  fs << "]";
}

int main() {
  GenerateSphericalPositions("positions.yaml", M_PI/6, M_PI/5, 3, 3.1, 1);
  return 0;
}
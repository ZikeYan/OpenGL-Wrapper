//
// Created by Neo on 17/10/2017.
//

#ifndef OPENGL_WRAPPER_ENCODE_PIXEL2UV_H
#define OPENGL_WRAPPER_ENCODE_PIXEL2UV_H

#include <fstream>
#include <string>
#include <vector>
#include <opencv2/opencv.hpp>

void EncodePixelToUV(std::string path, cv::Mat pixel2uv) {
  std::fstream out(path, std::ios::out);
  for (int i = 0; i < pixel2uv.rows; ++i) {
    for (int j = 0; j < pixel2uv.cols; ++j) {
      cv::Vec4f v = pixel2uv.at<cv::Vec4f>(i, j);
      if (v[0] != 0 || v[1] != 0) {
        out << i << " " << j << " " << v[0] << " " << v[1] << "\n";
      }
    }
  }
}

#endif //OPENGL_WRAPPER_ENCODE_PIXEL2UV_H

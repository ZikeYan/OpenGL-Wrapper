//
// Created by Neo on 16/7/31.
//

#include "utils/texture.h"

#include <stdio.h>
#include <iostream>

void SetTexture(std::string texture_path,
                 unsigned char* &data,
                 int &width,
                 int &height) {
  const int kBMPHeaderSize = 54;
  unsigned char header[kBMPHeaderSize];
  unsigned int data_pos;
  unsigned int image_size;

  //////////////////////////////////////////////////
  // Open the file
  FILE * file = fopen(texture_path.c_str(),"rb");
  if (!file) {
    std::cout << "Invalid path: " << texture_path << std::endl;
    exit(1);
  }
  if ( fread(header, 1, 54, file) != 54
       || header[0] != 'B' || header[1] != 'M'
       || *(int*)&(header[0x1E]) != 0
       || *(int*)&(header[0x1C]) != 24) {
    std::cout << "Invalid BMP header." << std::endl;
    exit(1);
  }

  // Read the information about the image
  data_pos   = *(unsigned int*)&(header[0x0A]);
  image_size = *(unsigned int*)&(header[0x22]);
  width      = *(unsigned int*)&(header[0x12]);
  height     = *(unsigned int*)&(header[0x16]);

  image_size = image_size > 0 ? image_size : width * height * 3;
  data_pos = data_pos > 0 ? data_pos : 54;

  // Create a buffer
  data = new unsigned char[image_size];
  fread(data, 1, image_size, file);
  fclose(file);
}

GLint SetTexture(std::string texture_path,
                  GLuint &texture_id){
  const int kBMPHeaderSize = 54;
  unsigned char header[kBMPHeaderSize];
  unsigned int data_pos;
  unsigned int image_size;
  unsigned int width, height;
  unsigned char *data;

  //////////////////////////////////////////////////
  // Open the file
  FILE * file = fopen(texture_path.c_str(),"rb");
  if (!file) {
    std::cout << "Invalid path: " << texture_path << std::endl;
    return -1;
  }
  if ( fread(header, 1, 54, file) != 54
       || header[0] != 'B' || header[1] != 'M'
       || *(int*)&(header[0x1E]) != 0
       || *(int*)&(header[0x1C]) != 24) {
    std::cout << "Invalid BMP header." << std::endl;
    return -1;
  }

  // Read the information about the image
  data_pos   = *(unsigned int*)&(header[0x0A]);
  image_size = *(unsigned int*)&(header[0x22]);
  width      = *(unsigned int*)&(header[0x12]);
  height     = *(unsigned int*)&(header[0x16]);

  image_size = image_size > 0 ? image_size : width * height * 3;
  data_pos = data_pos > 0 ? data_pos : 54;

  // Create a buffer
  data = new unsigned char[image_size];
  fread(data, 1, image_size, file);
  fclose(file);

  //////////////////////////////////////////////////
  // Texture manipulation
  glGenTextures(1, &texture_id);
  glBindTexture(GL_TEXTURE_2D, texture_id);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR,
               GL_UNSIGNED_BYTE, data);
  delete[] data;

  // ... nice trilinear filtering.
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  // add GL_TEXTURE_WRAP_R for 3D texture
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glGenerateMipmap(GL_TEXTURE_2D);

  return 0;
}
//
// Created by Neo on 16/7/31.
//

#ifndef RAYTRACING_TEXTURE_H
#define RAYTRACING_TEXTURE_H

#include <string>
#include <GL/glew.h>

GLint LoadTexture(std::string texture_path,
                  GLuint &texture_id);

void LoadTexture(std::string texture_path,
                 unsigned char* &data,
                 int &width,
                 int &height);
#endif //RAYTRACING_TEXTURE_H

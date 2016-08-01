//
// Created by Neo on 16/7/17.
//

#ifndef RAYTRACING_SHADER_H
#define RAYTRACING_SHADER_H

#include <string>
#include <GL/glew.h>

GLint LoadShaders(std::string vertex_shader_path,
                  std::string fragment_shader_path,
                  GLuint &program_id);

#endif //RAYTRACING_SHADER_H

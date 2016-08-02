//
// Created by Neo on 16/8/2.
//

#ifndef OPENGL_TEMPLATE_GLOBJECT_H
#define OPENGL_TEMPLATE_GLOBJECT_H

#include <string>
#include <GL/glew.h>

class GLObject {
public:
  virtual void Init() = 0;
  virtual void Render() = 0;

protected:
  GLuint vao_;
  GLuint program_id_;
  GLuint texture_id_;
};


#endif //OPENGL_TEMPLATE_GLOBJECT_H

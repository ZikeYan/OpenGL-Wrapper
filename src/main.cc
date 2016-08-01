//
// Created by Neo on 16/7/29.
//

#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <glm/gtc/matrix_transform.hpp>

#include "utils/context.h"
#include "utils/control.h"
#include "utils/model.h"
#include "utils/shader.h"
#include "utils/texture.h"

int main( void ) {
  Context context("F-16");
  Control control(context.window());

  //////////
  // Load Model-Texture-Shader
  std::vector<glm::vec3> vertices;
  std::vector<glm::vec2> uvs;
  std::vector<glm::vec3> normals;
  std::vector<unsigned int> indices;
  GLuint texture, program_id;
  GLint result;
  result = LoadModel("/Users/Neo/Desktop/f16.obj", vertices, uvs, normals, indices);
  result = LoadTexture("/Users/Neo/Desktop/F16s.bmp", texture);
  result = LoadShaders("vertex.glsl", "fragment.glsl", program_id);

  GLint matrix_id = glGetUniformLocation(program_id, "mvp");
  GLint view_id = glGetUniformLocation(program_id, "viewMat");
  GLint texture_id = glGetUniformLocation(program_id, "textureSampler");

  ////////////
  // Initialize Model vao-vbo with data
  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  GLuint vbo[4];
  glGenBuffers(4, vbo);

  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]),
               vertices.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
  glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(uvs[0]),
               uvs.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

  glEnableVertexAttribArray(2);
  glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
  glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(normals[0]),
               normals.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[3]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(indices[0]),
               indices.data(), GL_STATIC_DRAW);

  //////////
  // Additional OpenGL setting
  glfwPollEvents();
  glfwSetCursorPos(context.window(), 1024/2, 768/2);

  glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  do {
    ////////////////
    // General updating
    control.UpdateCameraPose();
    glm::mat4 MVP = control.projection_mat() * control.view_mat() * glm::mat4(1.0f);
    glm::mat4 viewMat = control.view_mat();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ///////////////
    // Render with Shader, texture, vao
    glUseProgram(program_id);

    glUniformMatrix4fv(matrix_id, 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(view_id, 1, GL_FALSE, &viewMat[0][0]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(texture_id, 0);

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

    // Swap buffers
    glfwSwapBuffers(context.window());
    glfwPollEvents();

  } while( glfwGetKey(context.window(), GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
           glfwWindowShouldClose(context.window()) == 0 );

  // Cleanup VBO
  glDeleteBuffers(3, vbo);
  glDeleteVertexArrays(1, &vao);
  glDeleteProgram(program_id);

  // Close OpenGL window and terminate GLFW
  glfwTerminate();
  return 0;
}


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
#include "utils/texture.h"

#include "gl_objects/mesh_object.h"

int main( void ) {
  // Context and control init
  Context context("F-16");
  Control control(context.window());

  MeshObject plane(kVertexNormalUVFace);

  /// Compile shaders
  std::vector<std::pair<UniformType, std::string> > uniform_names;
  uniform_names.clear();
  uniform_names.push_back(std::make_pair(kMatrix4f, "mvp"));
  uniform_names.push_back(std::make_pair(kMatrix4f, "c_T_w"));
  uniform_names.push_back(std::make_pair(kTexture, "textureSampler"));
  plane.InitShader("../shader/vertex_plane.glsl",
                   "../shader/fragment_plane.glsl",
                   uniform_names);

  /// Prepare mesh data buffer
  std::vector<glm::vec3> vertices;
  std::vector<glm::vec2> uvs;
  std::vector<glm::vec3> normals;
  std::vector<unsigned int> indices; /// Not a face ! do it better !
  LoadModel("../obj/f16.obj",
            vertices, uvs, normals, indices);
  std::vector<int> count_of_objects;
  count_of_objects.push_back(vertices.size());
  count_of_objects.push_back(normals.size());
  count_of_objects.push_back(uvs.size());
  std::cout << indices.size() << std::endl;
  count_of_objects.push_back(indices.size() / 3);
  plane.InitVAO(count_of_objects);

  /// Prepare mesh texture
  unsigned char *ptr;
  int width, height;
  SetTexture("../obj/f16.bmp", ptr, width, height);
  plane.InitTexture(width, height);

  std::cout << "texture loaded";

  // Additional settings
  glfwPollEvents();
  glfwSetCursorPos(context.window(), 1024/2, 768/2);

  glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  do {
    // Update control
    control.UpdateCameraPose();
    glm::mat4 mvp = control.projection_mat() *
        control.view_mat() *
        glm::mat4(1.0f);
    glm::mat4 v = control.view_mat();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    std::cout << "Cleared" << std::endl;
    std::vector<void*> uniforms;
    int texture = 0;
    uniforms.push_back((void*)&mvp[0][0]);
    uniforms.push_back((void*)&v[0][0]);
    uniforms.push_back((void*)&texture);
    std::cout << "Pushed back" << std::endl;

    plane.LoadMesh((float*) vertices.data(),
                   (float*) normals.data(),
                   (float*) uvs.data(),
                   (unsigned int*) indices.data());
    plane.SetTexture(ptr, width, height);
    plane.SetUniforms(uniforms);
    std::cout << "Render" << std::endl;
    plane.Render();
    // Foreach GLObject do data updating and Render();
//    f16.set_mvp(&mvp[0][0]);
//    f16.set_v(&v[0][0]);
//    f16.Render();
//
//    camera.set_mvp(&mvp[0][0]);
//    camera.UpdateData(v);
//    camera.Render();

    // Additional operations
    glfwSwapBuffers(context.window());
    glfwPollEvents();

  } while( glfwGetKey(context.window(), GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
           glfwWindowShouldClose(context.window()) == 0 );

  // Close OpenGL window and terminate GLFW
  glfwTerminate();
  return 0;
}


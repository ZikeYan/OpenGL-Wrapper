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

#include "gl_objects/gl_object.h"

#include <opencv2/opencv.hpp>

const int kWidth = 640;
const int kHeight= 480;
int main( void ) {
  // Context and control init
  int frame_width = kWidth, frame_height = kHeight;
#ifdef __APPLE__
  frame_width *= 2;
  frame_height *= 2;
#endif
  Context context("F-16", kWidth, kHeight);
  Control control(context.window());

  GLObject plane(kVertexNormalUVFace);

  /// Compile shaders
  std::vector<std::pair<UniformType, std::string> > uniform_names;
  uniform_names.clear();
  uniform_names.push_back(std::make_pair(kMatrix4f, "mvp"));
  uniform_names.push_back(std::make_pair(kMatrix4f, "c_T_w"));
  uniform_names.push_back(std::make_pair(kTexture, "textureSampler"));
  plane.InitShader("../shader/vertex_vntf.glsl",
                   "../shader/fragment_vntf.glsl",
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
  LoadTexture("../obj/f16.bmp", ptr, width, height);
  plane.InitTexture(width, height);

  std::cout << "texture loaded";

  // Additional settings
  glfwPollEvents();

  glfwSetCursorPos(context.window(), frame_width / 2, frame_height / 2);

  glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  cv::Mat im = cv::Mat(frame_height, frame_width, CV_32F);
  cv::Mat sh = cv::Mat(frame_height, frame_width, CV_16U);
  do
  {
    // Update control
    control.UpdateCameraPose();
    for (int i = 0; i < 4; ++i) {
      for (int j = 0; j < 4; ++j) {
        std::cout << control.projection_mat()[i][j] << " ";
      }
      std::cout << std::endl;
    }

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

    plane.SetMesh((float*) vertices.data(),
                  (float*) normals.data(),
                  (float*) uvs.data(),
                  (unsigned int*) indices.data());
    plane.SetTexture(ptr, width, height);
    plane.SetUniforms(uniforms);
    plane.Render();

    glReadBuffer(GL_BACK);
    glReadPixels(0, 0, frame_width, frame_height,
                 GL_DEPTH_COMPONENT,
                 GL_FLOAT, im.data);
#ifdef __APPLE__
    //cv::resize(im, im, cv::Size(frame_width/2, frame_height/2));
#endif
//    GLfloat depth = 0, max_depth = 0;
    for (int i = 0; i < frame_height; ++i) {
      for (int j = 0; j < frame_width; ++j) {
        float d = im.at<float>(i, j);
        if (isnan(d) || isinf(d)) {
          sh.at<unsigned short>(i, j) = 0;
        } else {
          float clip_z = 2 * d - 1; // [0,1] -> [-1,1]
          // [-(n+f)/(n-f)] + [2nf/(n-f)] / w_z = clip_z
          float f = 10000.0, n = 0.01f;
          GLfloat world_z = 2*n*f/(clip_z*(n-f)+(n+f));
          float depth = world_z * 5000;
          if (depth > 40000) depth = 40000;
          sh.at<unsigned short>(i, j) = depth;
        }
      }
    }

    cv::Mat show;
    cv::resize(sh, show, cv::Size(frame_width/2, frame_height/2), CV_INTER_NN);
    cv::imshow("im", show);
    if (cv::waitKey(10) == 27) break;

    glfwSwapBuffers(context.window());
    glfwPollEvents();
    //break;

    //cv::waitKey(-1);
    // Foreach GLObject do data updating and Render();
//    f16.set_mvp(&mvp[0][0]);
//    f16.set_v(&v[0][0]);
//    f16.Render();
//
//    camera.set_mvp(&mvp[0][0]);
//    camera.UpdateData(v);
//    camera.Render();

    // Additional operations

  } while( glfwGetKey(context.window(), GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
           glfwWindowShouldClose(context.window()) == 0 );

  // Close OpenGL window and terminate GLFW
  glfwTerminate();
  return 0;
}


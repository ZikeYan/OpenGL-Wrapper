//
// Created by Neo on 21/09/2017.
//

/// Run the program twice.
/// Step 1: high-res texture -> high-res rendered image
/// Step 2: low-res texture -> low-res rendered image
/// TODO: add configurations for camera movement and texture resolutions
/// Input: mesh(.obj), texture(high-res, low-res .png)
///        navigate with mouse and kbd
/// Output: rendered image(high-res.png, low-res.png)
///         pixel-wise texture coordinate map(.txt)
///         => further interpreted by MATLAB


#include <fstream>
#include <chrono>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <opencv2/opencv.hpp>

#include "../src/program.h"
#include "../src/uniforms.h"
#include "../src/camera.h"
#include "../src/trajectory.h"
#include "../src/window.h"
#include "../src/texture.h"
#include "../src/args.h"
#include "../src/model.h"

std::string kBasePath = "../../../data/TorusGlass";
std::string kResPostfix = "1024";
int kWidth = 1280;
int kHeight= 960;

bool kShowILightSrc = false;
bool kIsCameraPathSet = true;

std::vector<glm::vec3> axises = {
    glm::vec3(0, 0, 0),
    glm::vec3(20, 0, 0),
    glm::vec3(0, 0, 0),
    glm::vec3(0, 20, 0),
    glm::vec3(0, 0, 0),
    glm::vec3(0, 0, 20)
};

std::vector<glm::vec3> axis_colors = {
    glm::vec3(1, 0, 0),
    glm::vec3(1, 0, 0),
    glm::vec3(0, 1, 0),
    glm::vec3(0, 1, 0),
    glm::vec3(0, 0, 1),
    glm::vec3(0, 0, 1)
};

/// Wrap for light sources
/// Show axis and light sources
int main() {
  /// Light sources
  std::vector<glm::vec3> light_src_positions = {
      glm::vec3(-10, 0, 10),
      glm::vec3(10, 0, 10),
      glm::vec3(0, 0, 20),
      glm::vec3(0, 0, -20),
      glm::vec3(0, 10, 0),
      glm::vec3(0, -10, 0),
      glm::vec3(-10, 0, 0),
      glm::vec3(10, 0, 0),
      glm::vec3(-15, 0, 5),
      glm::vec3(15, 0, 5),
      glm::vec3(0, 15, 5),
      glm::vec3(0, -15, 5),
  };
  std::vector<glm::vec3> light_src_colors = {
      glm::vec3(1, 1, 1),
      glm::vec3(1, 1, 1),
      glm::vec3(1, 1, 1),
      glm::vec3(1, 1, 1),
      glm::vec3(1, 1, 1),
      glm::vec3(1, 1, 1),
      glm::vec3(1, 1, 1),
      glm::vec3(1, 1, 1),
      glm::vec3(1, 1, 1),
      glm::vec3(1, 1, 1),
      glm::vec3(1, 1, 1),
      glm::vec3(1, 1, 1)
  };
  float light_power = 30;
  glm::vec3 light_color = glm::vec3(1, 1, 1);
  std::stringstream ss;
  ss << light_src_positions.size();

  //////////////////////////////////
  /// Load data at the first stage
  gl::Model model;
  model.LoadObj("../model/torus/torus.obj");

  /// Context and control init
  gl::Window window("Generate dataset", kWidth, kHeight);
  gl::Camera camera(window.width(), window.height());
  camera.SwitchInteraction(true);

  gl::Texture input_texture;
  input_texture.Init("../model/torus/glass-" + kResPostfix + ".png");

  ////////////////////////////////////
  /// Prepare for writing uv coordinates into texture
  gl::Program texture_writer_program;
  texture_writer_program.Load("../shader/write_uv_vertex.glsl",
                              gl::kVertexShader);
  texture_writer_program.Load("../shader/write_uv_fragment.glsl",
                              gl::kFragmentShader);
  texture_writer_program.Build();
  gl::Uniforms texture_writer_uniforms;
  texture_writer_uniforms.GetLocation(texture_writer_program.id(),
                                      "mvp", gl::kMatrix4f);
  gl::Args texture_writer_args(3);
  texture_writer_args.BindBuffer(0, {GL_ARRAY_BUFFER, sizeof(float), 3, GL_FLOAT},
                                 model.positions().size(), model.positions().data());
  texture_writer_args.BindBuffer(1, {GL_ARRAY_BUFFER, sizeof(float), 2, GL_FLOAT},
                                 model.uvs().size(), model.uvs().data());
  texture_writer_args.BindBuffer(2, {GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int),
                                     1, GL_UNSIGNED_INT},
                                 model.indices().size(), model.indices().data());

  // Not encapsulated now: set context for writing to renderbuffer
  // Create framebuffer
  GLuint frame_buffer = 0;
  glGenFramebuffers(1, &frame_buffer);
  glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
  // Color -> texture, to output
  gl::Texture write_texture;
  write_texture.Init(GL_RGBA32F, kWidth, kHeight);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                       write_texture.id(), 0);
  // Depth -> render buffer, for depth test
  GLuint render_buffer;
  glGenRenderbuffers(1, &render_buffer);
  glBindRenderbuffer(GL_RENDERBUFFER, render_buffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT,
                        write_texture.width(), write_texture.height());
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                            GL_RENDERBUFFER, render_buffer);
  // Set the list of draw buffers.
  GLenum draw_buffers[1] = {GL_COLOR_ATTACHMENT0};
  glDrawBuffers(1, draw_buffers);
  // Always check that our framebuffer is ok
  if(glCheckFramebufferStatus(GL_FRAMEBUFFER)
     != GL_FRAMEBUFFER_COMPLETE)
    return -1;
  ////////////////////////////////////

  ////////////////////////////////////
  /// Render the model (synthetic or reconstructed)
  gl::Program render_program;
  render_program.Load("../shader/textured_model_multi_light_vertex.glsl",
                      gl::kVertexShader);
  render_program.ReplaceMacro("LIGHT_COUNT", ss.str(), gl::kVertexShader);
  render_program.Load("../shader/textured_model_multi_light_fragment.glsl",
                      gl::kFragmentShader);
  render_program.ReplaceMacro("LIGHT_COUNT", ss.str(), gl::kFragmentShader);
  render_program.Build();

  gl::Uniforms render_uniforms;
  render_uniforms.GetLocation(render_program.id(), "mvp",
                              gl::kMatrix4f);
  render_uniforms.GetLocation(render_program.id(), "c_T_w",
                              gl::kMatrix4f);
  render_uniforms.GetLocation(render_program.id(), "texture_sampler",
                              gl::kTexture2D);
  render_uniforms.GetLocation(render_program.id(), "light",
                              gl::kVector3f);
  render_uniforms.GetLocation(render_program.id(), "light_cnt",
                              gl::kInt);
  render_uniforms.GetLocation(render_program.id(), "light_power",
                              gl::kFloat);
  render_uniforms.GetLocation(render_program.id(), "light_color",
                              gl::kVector3f);

  gl::Args render_args(4);
  render_args.InitBuffer(0, {GL_ARRAY_BUFFER, sizeof(float), 3, GL_FLOAT},
                         model.positions().size());
  render_args.InitBuffer(1, {GL_ARRAY_BUFFER, sizeof(float), 3, GL_FLOAT},
                         model.normals().size());
  render_args.InitBuffer(2, {GL_ARRAY_BUFFER, sizeof(float), 2, GL_FLOAT},
                         model.uvs().size());
  render_args.InitBuffer(3, {GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int),
                             1, GL_UNSIGNED_INT},
                         model.indices().size());
  ////////////////////////////////
  /// Assistance: draw light sources and axises
  gl::Program primitive_program;
  primitive_program.Load("../shader/primitive_vertex.glsl", gl::kVertexShader);
  primitive_program.Load("../shader/primitive_fragment.glsl", gl::kFragmentShader);
  primitive_program.Build();

  gl::Uniforms primitive_uniforms;
  primitive_uniforms.GetLocation(primitive_program.id(), "mvp", gl::kMatrix4f);

  gl::Args light_src_args(2);
  light_src_args.BindBuffer(0, {GL_ARRAY_BUFFER, sizeof(float), 3, GL_FLOAT},
                            light_src_positions.size(), light_src_positions.data());
  light_src_args.BindBuffer(1, {GL_ARRAY_BUFFER, sizeof(float), 3, GL_FLOAT},
                            light_src_colors.size(), light_src_colors.data());
  gl::Args axis_args(2);
  axis_args.BindBuffer(0, {GL_ARRAY_BUFFER, sizeof(float), 3, GL_FLOAT},
                       axises.size(), axises.data());
  axis_args.BindBuffer(1, {GL_ARRAY_BUFFER, sizeof(float), 3, GL_FLOAT},
                       axis_colors.size(), axis_colors.data());

  // Additional settings
  glfwPollEvents();
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  // Buffer to collect written texture
  cv::Mat pixel = cv::Mat(write_texture.height(),
                          write_texture.width(),
                          CV_32FC4);

  int cnt = 0;

  std::chrono::time_point<std::chrono::system_clock> prev_hit, curr_hit;
  prev_hit = std::chrono::system_clock::now();

  std::vector<glm::vec3> viewpoints;
  int light_cnt = 8;
  for (float h = -2; h <= 2; h += 2) {
    for (float t = 0; t < 2*M_PI; t += 2*M_PI/10) {
      viewpoints.push_back(glm::vec3(2*cos(t), 2*sin(t), h));
    }
  }

  int i = 0;
  do {
    if (i >= viewpoints.size()) break;
    // Control
    if (! kIsCameraPathSet) {
      camera.UpdateView(window);
    } else {
      camera.set_view(
          glm::lookAt(viewpoints[i], glm::vec3(0, 0, 0), glm::vec3(0, 0, 1)));
      if (i > 0 && i % 11 == 0) light_cnt += 2;
      ++i;
    }
    glm::mat4 mvp = camera.mvp();
    glm::mat4 view = camera.view();

    // Pass 1:
    // Render to framebuffer, into texture
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /// Choose shader
    glUseProgram(texture_writer_program.id());
    input_texture.Bind(0);
    texture_writer_uniforms.Bind("mvp", &mvp, 1);
    glBindVertexArray(texture_writer_args.vao());
    glDrawElements(GL_TRIANGLES, model.indices().size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // Pass 2:
    // Render the scene for visualization
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(render_program.id());
    input_texture.Bind(0);
    render_uniforms.Bind("mvp", &mvp, 1);
    render_uniforms.Bind("c_T_w", &view, 1);
    render_uniforms.Bind("texture_sampler", GLuint(0));
    render_uniforms.Bind("light", light_src_positions.data(), light_cnt);
    render_uniforms.Bind("light_cnt", &light_cnt, 1);
    render_uniforms.Bind("light_power", &light_power, 1);
    render_uniforms.Bind("light_color", &light_color, 1);

    /// Bind vertex data
    glBindVertexArray(render_args.vao());
    render_args.BindBuffer(0, {GL_ARRAY_BUFFER, sizeof(float), 3, GL_FLOAT},
                           model.positions().size(), model.positions().data());
    render_args.BindBuffer(1, {GL_ARRAY_BUFFER, sizeof(float), 3, GL_FLOAT},
                           model.normals().size(), model.normals().data());
    render_args.BindBuffer(2, {GL_ARRAY_BUFFER, sizeof(float), 2, GL_FLOAT},
                           model.uvs().size(), model.uvs().data());
    render_args.BindBuffer(3, {GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int),
                               1, GL_UNSIGNED_INT},
                           model.indices().size(), model.indices().data());

    /// Render
    glDrawElements(GL_TRIANGLES, model.indices().size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    if (kShowILightSrc) {
      glUseProgram(primitive_program.id());
      primitive_uniforms.Bind("mvp", &mvp, 1);
      glBindVertexArray(axis_args.vao());
      glDrawArrays(GL_LINES, 0, axises.size());
      glBindVertexArray(0);

      glBindVertexArray(light_src_args.vao());
      glEnable(GL_PROGRAM_POINT_SIZE);
      glDrawArrays(GL_POINTS, 0, light_src_positions.size());
      glBindVertexArray(0);
    }

    /// Capture the texture
    if (kIsCameraPathSet || window.get_key(GLFW_KEY_ENTER)) {
      curr_hit = std::chrono::system_clock::now();
      std::chrono::duration<double> elapsed_seconds = curr_hit - prev_hit;
      prev_hit = curr_hit;
      std::cout << elapsed_seconds.count() << "s\n";
      if (kIsCameraPathSet || elapsed_seconds.count() > 3.0f) {

        /// Write texture (i.e. projection map)
        write_texture.Bind(0);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, pixel.data);
        cv::flip(pixel, pixel, 0);

        /// Write image
        std::stringstream ss;
        ss << kBasePath + "/image/image-" + kResPostfix + "-" << cnt << ".png";
        cv::imwrite(ss.str(), window.CaptureRGB());

        ss.str("");
        ss << kBasePath + "/map/map-" + kResPostfix + "-" << cnt << ".txt";
        std::ofstream out(ss.str());
        for (int i = 0; i < pixel.rows; ++i) {
          for (int j = 0; j < pixel.cols; ++j) {
            cv::Vec4f v = pixel.at<cv::Vec4f>(i, j);
            if (v[0] != 0 || v[1] != 0) {
              out << i << " " << j << " "
                  << v[0] << " " << v[1] << std::endl;
            }
          }
        }
        ++cnt;
      }
    }
    window.swap_buffer();
  } while( window.get_key(GLFW_KEY_ESCAPE) != GLFW_PRESS &&
           window.should_close() == 0 );

  ////////////////////////////////////
  /// Finally output shaded texture
  gl::Program shading_program;
  shading_program.Load("../shader/shading_vertex.glsl", gl::kVertexShader);
  shading_program.Load("../shader/shading_fragment.glsl", gl::kFragmentShader);
  shading_program.ReplaceMacro("LIGHT_COUNT", ss.str(), gl::kFragmentShader);
  shading_program.Build();

  gl::Uniforms shading_uniforms;
  shading_uniforms.GetLocation(shading_program.id(), "lights", gl::kVector3f);
  shading_uniforms.GetLocation(shading_program.id(), "light_power", gl::kFloat);
  shading_uniforms.GetLocation(shading_program.id(), "light_color", gl::kVector3f);

  gl::Args shading_args(4);
  shading_args.BindBuffer(0, {GL_ARRAY_BUFFER, sizeof(float), 2, GL_FLOAT},
                          model.uvs().size(), model.uvs().data());
  shading_args.BindBuffer(1, {GL_ARRAY_BUFFER, sizeof(float), 3, GL_FLOAT},
                          model.positions().size(), model.positions().data());
  shading_args.BindBuffer(2, {GL_ARRAY_BUFFER, sizeof(float), 3, GL_FLOAT},
                          model.normals().size(), model.normals().data());
  shading_args.BindBuffer(3, {GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int),
                              1, GL_UNSIGNED_INT},
                          model.indices().size(), model.indices().data());

  /// divided by 2 depends on Resolution: refactor it later
  window.Resize(input_texture.width(), input_texture.height());
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  input_texture.Bind(0);
  glUseProgram(shading_program.id());

  shading_uniforms.Bind("lights",
                        light_src_positions.data(),
                        light_src_positions.size());
  shading_uniforms.Bind("light_power", &light_power, 1);
  shading_uniforms.Bind("light_color", &light_color, 1);
  glBindVertexArray(shading_args.vao());
  glDrawElements(GL_TRIANGLES, model.indices().size(), GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);

  window.swap_buffer();
  pixel = window.CaptureRGB();

  cv::imwrite(kBasePath + "/shaded-atlas-"+ kResPostfix + ".png",
              pixel);

  cv::Mat albedo;
  cv::flip(input_texture.image(), albedo, 0);
  cv::imwrite(kBasePath + "/albedo-atlas-" + kResPostfix + ".png", albedo);

  cv::Mat composed_atlas = cv::Mat(albedo.rows, albedo.cols, CV_8UC3);
  for (int i = 0; i < composed_atlas.rows; ++i) {
    for (int j = 0; j < composed_atlas.cols; ++j) {
      cv::Vec3b r = albedo.at<cv::Vec3b>(i,j);
      float s = pixel.at<cv::Vec3b>(i, j)[0];
      composed_atlas.at<cv::Vec3b>(i, j) = cv::Vec3b(r[0]*s/255.0,
                                                     r[1]*s/255.0,
                                                     r[2]*s/255.0);
    }
  }

  cv::imwrite(kBasePath + "/composed-atlas-" + kResPostfix + ".png",
              composed_atlas);
  cv::Mat composed_atlas2x;
  cv::resize(composed_atlas, composed_atlas2x, cv::Size(2*composed_atlas.rows,
                                                        2*composed_atlas.cols));
  cv::imwrite(kBasePath + "/composed-atlas-" + kResPostfix + "-2x.png",
              composed_atlas2x);
  return 0;
}
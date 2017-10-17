//
// Created by Neo on 21/09/2017.
//

/// In Super Resolution:
/// Input: low-res texture -> low-res images
///        projection matrices of [high-res texture -> high-res image]
/// Output: high-res texture, high-res images
/// (We should generate these ground truth)

#include <fstream>
#include <chrono>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <opencv2/opencv.hpp>
#include "glwrapper.h"

#include "cxxopts.h"
#include "config_loader.h"
#include "../encode_pixel2uv.h"

/// Wrap for light sources
/// Show axis and light sources
std::string kConfigPath = "../example/synthesize_srid_dataset";

int main() {
  /// Config loader
  ConfigLoader config_loader;
  config_loader.LoadParams(kConfigPath + "/params.yaml");
  config_loader.LoadLights(kConfigPath + "/lights.yaml");
  config_loader.LoadPositions("positions.yaml");

  float light_power = config_loader.light_power;
  std::vector<glm::vec3> &light_positions = config_loader.light_positions;
  std::vector<glm::vec3> &light_colors = config_loader.light_colors;
  glm::vec3 light_color = light_colors[0];
  int light_cnt = (int) light_colors.size();

  std::vector<glm::vec3> &camera_positions = config_loader.camera_positions;
  std::vector<glm::ivec3> &camera_polars = config_loader.camera_polars;
  std::stringstream ss;
  ss << light_positions.size();

  //////////////////////////////////
  int image_width = config_loader.output_width_original
                    / config_loader.downsample_factor;
  int image_height = config_loader.output_height_original
                     / config_loader.downsample_factor;
#ifdef __APPLE__
  image_width /= 2;
  image_height /= 2;
#endif
  gl::Window window("Synthesizer", image_width, image_height);
  gl::Camera camera(window.width(), window.height());
  camera.SwitchInteraction(true);

  gl::Model model;
  model.LoadObj(config_loader.input_model_path);
  gl::Texture input_texture;
  cv::Mat origin_texture = cv::imread(config_loader.input_texture_path);
  cv::Mat downsampled_texture;
  cv::resize(origin_texture, downsampled_texture,
             cv::Size(origin_texture.cols / config_loader.downsample_factor,
                      origin_texture.rows / config_loader.downsample_factor));
  input_texture.Init(downsampled_texture);

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
  texture_writer_args.BindBuffer(0,
                                 {GL_ARRAY_BUFFER, sizeof(float), 3, GL_FLOAT},
                                 model.positions().size(),
                                 model.positions().data());
  texture_writer_args.BindBuffer(1,
                                 {GL_ARRAY_BUFFER, sizeof(float), 2, GL_FLOAT},
                                 model.uvs().size(), model.uvs().data());
  texture_writer_args.BindBuffer(2,
                                 {GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int),
                                  1, GL_UNSIGNED_INT},
                                 model.indices().size(),
                                 model.indices().data());

  // Not encapsulated now: set context for writing to renderbuffer
  // Create framebuffer
  GLuint frame_buffer = 0;
  glGenFramebuffers(1, &frame_buffer);
  glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
  // Color -> texture, to output
  gl::Texture write_texture;
  write_texture.Init(GL_RGBA32F, image_width, image_height);
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
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER)
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
  render_uniforms.GetLocation(render_program.id(), "mvp", gl::kMatrix4f);
  render_uniforms.GetLocation(render_program.id(), "c_T_w", gl::kMatrix4f);
  render_uniforms.GetLocation(render_program.id(), "texture_sampler",
                              gl::kTexture2D);
  render_uniforms.GetLocation(render_program.id(), "light", gl::kVector3f);
  render_uniforms.GetLocation(render_program.id(), "light_cnt", gl::kInt);
  render_uniforms.GetLocation(render_program.id(), "light_power", gl::kFloat);
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
  primitive_program.Load("../shader/primitive_fragment.glsl",
                         gl::kFragmentShader);
  primitive_program.Build();

  gl::Uniforms primitive_uniforms;
  primitive_uniforms.GetLocation(primitive_program.id(), "mvp", gl::kMatrix4f);

  gl::Args light_src_args(2);
  light_src_args.BindBuffer(0, {GL_ARRAY_BUFFER, sizeof(float), 3, GL_FLOAT},
                            light_positions.size(), light_positions.data());
  light_src_args.BindBuffer(1, {GL_ARRAY_BUFFER, sizeof(float), 3, GL_FLOAT},
                            light_colors.size(), light_colors.data());
  std::vector<glm::vec3> axises = {
      glm::vec3(-20, 0, 0), glm::vec3(20, 0, 0),
      glm::vec3(0, -20, 0), glm::vec3(0, 20, 0),
      glm::vec3(0, 0, -20), glm::vec3(0, 0, 20)
  };
  std::vector<glm::vec3> axis_colors = {
      glm::vec3(1, 0, 0), glm::vec3(1, 0, 0),
      glm::vec3(0, 1, 0), glm::vec3(0, 1, 0),
      glm::vec3(0, 0, 1), glm::vec3(0, 0, 1)
  };
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
  cv::Mat pixel2uv = cv::Mat(write_texture.height(),
                             write_texture.width(),
                             CV_32FC4);
  std::chrono::time_point<std::chrono::system_clock> prev_hit, curr_hit;
  prev_hit = std::chrono::system_clock::now();

  int i = 0;
  do {
    if (i >= camera_positions.size()) break;
    // Control
    if (!config_loader.use_preset_path) {
      camera.UpdateView(window);
    } else {
      camera.set_view(
          glm::lookAt(camera_positions[i],
                      glm::vec3(0, 0, 0),
                      glm::vec3(0, 0, 1)));
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
    render_uniforms.Bind("light", light_positions.data(), light_cnt);
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

    if (config_loader.show_light_sources) {
      glUseProgram(primitive_program.id());
      primitive_uniforms.Bind("mvp", &mvp, 1);
      glBindVertexArray(axis_args.vao());
      glDrawArrays(GL_LINES, 0, axises.size());
      glBindVertexArray(0);

      glBindVertexArray(light_src_args.vao());
      glEnable(GL_PROGRAM_POINT_SIZE);
      glDrawArrays(GL_POINTS, 0, light_positions.size());
      glBindVertexArray(0);
    }

    /// Capture the texture
    if (config_loader.use_preset_path) {
      /// Write texture (i.e. projection map)
      write_texture.Bind(0);
      glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, pixel2uv.data);
      cv::flip(pixel2uv, pixel2uv, 0);

      /// Write image
      std::stringstream ss;
      ss.str("");
      ss << "_factor_" << config_loader.downsample_factor
         << "_radius_" << camera_polars[i].x
         << "_elevation_" << camera_polars[i].y
         << "_azimuth_" << camera_polars[i].z;
      cv::imwrite(config_loader.output_path + "/image" + ss.str() + ".png",
                  window.CaptureRGB());
      EncodePixelToUV(config_loader.output_path + "/map" + ss.str() + ".txt",
                      pixel2uv);
    } else if (window.get_key(GLFW_KEY_ENTER) == GLFW_PRESS) {
      cv::imwrite(config_loader.output_path + "/lights_config.png",
                  window.CaptureRGB());
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
  int window_width = input_texture.width();
  int window_height = input_texture.height();
#ifdef __APPLE__
  window_width /= 2;
  window_height /= 2;
#endif
  window.Resize(window_width, window_height);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  input_texture.Bind(0);
  glUseProgram(shading_program.id());

  shading_uniforms.Bind("lights",
                        light_positions.data(),
                        light_positions.size());
  shading_uniforms.Bind("light_power", &light_power, 1);
  shading_uniforms.Bind("light_color", &light_color, 1);
  glBindVertexArray(shading_args.vao());
  glDrawElements(GL_TRIANGLES, model.indices().size(), GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);

  window.swap_buffer();

  cv::Mat shading = window.CaptureRGB();
  ss.str("");
  ss << config_loader.output_path + "/atlas_shading"
     << "_factor_" << config_loader.downsample_factor
     << ".png";
  cv::imwrite(ss.str(), shading);

  cv::Mat albedo;
  cv::flip(input_texture.image(), albedo, 0);
  ss.str("");
  ss << config_loader.output_path + "/atlas_albedo"
     << "_factor_" << config_loader.downsample_factor
     << ".png";
  cv::imwrite(ss.str(), albedo);

  // Ugly uchar3 mul float
  cv::Mat composed = cv::Mat(albedo.rows, albedo.cols, CV_8UC3);
  for (int i = 0; i < composed.rows; ++i) {
    for (int j = 0; j < composed.cols; ++j) {
      cv::Vec3b r = albedo.at<cv::Vec3b>(i,j);
      float s = shading.at<cv::Vec3b>(i, j)[0];
      composed.at<cv::Vec3b>(i, j)
          = cv::Vec3b(r[0]*s/255.0f, r[1]*s/255.0f, r[2]*s/255.0f);
    }
  }
  ss.str("");
  ss << config_loader.output_path + "/atlas_composed"
     << "_factor_" << config_loader.downsample_factor
     << ".png";
  cv::imwrite(ss.str(), composed);
  return 0;
}
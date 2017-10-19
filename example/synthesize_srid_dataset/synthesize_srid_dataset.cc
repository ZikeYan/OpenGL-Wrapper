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
static const GLfloat kVertices[] = {
    -1.0f, -1.0f, 0.0f,
    1.0f, -1.0f, 0.0f,
    -1.0f,  1.0f, 0.0f,
    -1.0f,  1.0f, 0.0f,
    1.0f, -1.0f, 0.0f,
    1.0f,  1.0f, 0.0f,
};

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

  /// Load torus
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
  gl::Program program0;
  program0.Load("../shader/pixel_uv_fbo_vertex.glsl", gl::kVertexShader);
  program0.Load("../shader/pixel_uv_fbo_fragment.glsl", gl::kFragmentShader);
  program0.Build();
  gl::Uniforms uniforms0;
  uniforms0.GetLocation(program0.id(), "mvp", gl::kMatrix4f);
  gl::Args args0(3);
  args0.BindBuffer(0, {GL_ARRAY_BUFFER, sizeof(float), 3, GL_FLOAT},
                     model.positions().size(),
                     model.positions().data());
  args0.BindBuffer(1, {GL_ARRAY_BUFFER, sizeof(float), 2, GL_FLOAT},
                     model.uvs().size(),
                     model.uvs().data());
  args0.BindBuffer(2, {GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int),
                         1, GL_UNSIGNED_INT},
                     model.indices().size(),
                     model.indices().data());
  gl::FrameBuffer fbo_uv(GL_RGBA32F, image_width, image_height);

  ////////////////////////////////////
  /// Render the model (synthetic or reconstructed)
  gl::Program program1;
  program1.Load("../shader/textured_model_multi_light_vertex.glsl",
                      gl::kVertexShader);
  program1.ReplaceMacro("LIGHT_COUNT", ss.str(), gl::kVertexShader);
  program1.Load("../shader/textured_model_multi_light_fragment.glsl",
                      gl::kFragmentShader);
  program1.ReplaceMacro("LIGHT_COUNT", ss.str(), gl::kFragmentShader);
  program1.Build();
  gl::Uniforms uniforms1;
  uniforms1.GetLocation(program1.id(), "mvp", gl::kMatrix4f);
  uniforms1.GetLocation(program1.id(), "c_T_w", gl::kMatrix4f);
  uniforms1.GetLocation(program1.id(), "texture_sampler",
                              gl::kTexture2D);
  uniforms1.GetLocation(program1.id(), "light", gl::kVector3f);
  uniforms1.GetLocation(program1.id(), "light_cnt", gl::kInt);
  uniforms1.GetLocation(program1.id(), "light_power", gl::kFloat);
  uniforms1.GetLocation(program1.id(), "light_color",
                              gl::kVector3f);
  gl::Args args1(4);
  args1.BindBuffer(0, {GL_ARRAY_BUFFER, sizeof(float), 3, GL_FLOAT},
                   model.positions().size(),
                   model.positions().data());
  args1.BindBuffer(1, {GL_ARRAY_BUFFER, sizeof(float), 3, GL_FLOAT},
                   model.normals().size(),
                   model.normals().data());
  args1.BindBuffer(2, {GL_ARRAY_BUFFER, sizeof(float), 2, GL_FLOAT},
                   model.uvs().size(),
                   model.uvs().data());
  args1.BindBuffer(3, {GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int),
                       1, GL_UNSIGNED_INT},
                   model.indices().size(),
                   model.indices().data());
  gl::FrameBuffer fbo_image(GL_RGB, image_width, image_height);

  gl::Program program2;
  program2.Load("../shader/simple_texture_vertex.glsl", gl::kVertexShader);
  program2.Load("../shader/simple_texture_fragment.glsl", gl::kFragmentShader);
  program2.Build();
  gl::Uniforms uniforms2;
  uniforms2.GetLocation(program2.id(), "tex", gl::kTexture2D);
  gl::Args args2(2);
  args2.BindBuffer(0, {GL_ARRAY_BUFFER, sizeof(float), 3, GL_FLOAT},
                   6, (void*)kVertices);
  args2.BindBuffer(1, {GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int),
                       1, GL_UNSIGNED_INT},
                   model.indices().size(), model.indices().data());

  ////////////////////////////////
  /// Assistance: draw light sources and axises
  gl::Program programx;
  programx.Load("../shader/primitive_vertex.glsl", gl::kVertexShader);
  programx.Load("../shader/primitive_fragment.glsl", gl::kFragmentShader);
  programx.Build();
  gl::Uniforms uniformsx;
  uniformsx.GetLocation(programx.id(), "mvp", gl::kMatrix4f);
  gl::Args args_lights(2);
  args_lights.BindBuffer(0, {GL_ARRAY_BUFFER, sizeof(float), 3, GL_FLOAT},
                   light_positions.size(),
                   light_positions.data());
  args_lights.BindBuffer(1, {GL_ARRAY_BUFFER, sizeof(float), 3, GL_FLOAT},
                   light_colors.size(),
                   light_colors.data());
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
  gl::Args args_axis(2);
  args_axis.BindBuffer(0, {GL_ARRAY_BUFFER, sizeof(float), 3, GL_FLOAT},
                       axises.size(), axises.data());
  args_axis.BindBuffer(1, {GL_ARRAY_BUFFER, sizeof(float), 3, GL_FLOAT},
                       axis_colors.size(), axis_colors.data());

  // Additional settings
  glfwPollEvents();
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  int i = 0;
  do {
    if (i >= camera_positions.size()) break;

    if (!config_loader.use_preset_path) {
      camera.UpdateView(window);
    } else {
      camera.set_view(glm::lookAt(camera_positions[i],
                      glm::vec3(0, 0, 0),
                      glm::vec3(0, 0, 1)));
      ++i;
    }

    glm::mat4 mvp = camera.mvp();
    glm::mat4 view = camera.view();

    // Pass 1:'
    fbo_uv.Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(program0.id());
    input_texture.Bind(0);
    uniforms0.Bind("mvp", &mvp, 1);
    glBindVertexArray(args0.vao());
    glDrawElements(GL_TRIANGLES, model.indices().size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // Pass 2:
    fbo_image.Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(program1.id());
    input_texture.Bind(0);
    uniforms1.Bind("mvp", &mvp, 1);
    uniforms1.Bind("c_T_w", &view, 1);
    uniforms1.Bind("texture_sampler", GLuint(0));
    uniforms1.Bind("light", light_positions.data(), light_cnt);
    uniforms1.Bind("light_cnt", &light_cnt, 1);
    uniforms1.Bind("light_power", &light_power, 1);
    uniforms1.Bind("light_color", &light_color, 1);
    glBindVertexArray(args1.vao());
    glDrawElements(GL_TRIANGLES, model.indices().size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    if (config_loader.show_light_sources) {
      window.Bind();
      glUseProgram(programx.id());
      uniformsx.Bind("mvp", &mvp, 1);
      glBindVertexArray(args_axis.vao());
      glDrawArrays(GL_LINES, 0, axises.size());
      glBindVertexArray(0);

      glBindVertexArray(args_lights.vao());
      glEnable(GL_PROGRAM_POINT_SIZE);
      glDrawArrays(GL_POINTS, 0, light_positions.size());
      glBindVertexArray(0);
    }

    window.Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(program2.id());
    fbo_image.texture().Bind(1);
    uniforms2.Bind("tex", GLuint(1));
    glBindVertexArray(args2.vao());
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    /// Capture the texture
    if (config_loader.use_preset_path) {
      std::stringstream ss;
      ss.str("");
      ss << "_factor_" << config_loader.downsample_factor
         << "_radius_" << camera_polars[i].x
         << "_elevation_" << camera_polars[i].y
         << "_azimuth_" << camera_polars[i].z;
      EncodePixelToUV(config_loader.output_path + "/map" + ss.str() + ".txt",
                      fbo_uv.Capture());
      cv::imwrite(config_loader.output_path + "/image" + ss.str() + ".png",
                  fbo_image.Capture());
    } else if (window.get_key(GLFW_KEY_ENTER) == GLFW_PRESS) {
      cv::imwrite("lights_config.png", fbo_image.Capture());
    }
    window.swap_buffer();
  } while( window.get_key(GLFW_KEY_ESCAPE) != GLFW_PRESS &&
           window.should_close() == 0 );

  gl::Program program_shading;
  program_shading.Load("../shader/shading_vertex.glsl",
                       gl::kVertexShader);
  program_shading.Load("../shader/shading_fragment.glsl",
                       gl::kFragmentShader);
  program_shading.ReplaceMacro("LIGHT_COUNT", ss.str(), gl::kFragmentShader);
  program_shading.Build();

  gl::Uniforms uniforms_shading;
  uniforms_shading.GetLocation(program_shading.id(), "lights", gl::kVector3f);
  uniforms_shading.GetLocation(program_shading.id(), "light_power", gl::kFloat);
  uniforms_shading.GetLocation(program_shading.id(), "light_color", gl::kVector3f);

  gl::Args args_shading(4);
  args_shading.BindBuffer(0, {GL_ARRAY_BUFFER, sizeof(float), 2, GL_FLOAT},
                          model.uvs().size(), model.uvs().data());
  args_shading.BindBuffer(1, {GL_ARRAY_BUFFER, sizeof(float), 3, GL_FLOAT},
                          model.positions().size(), model.positions().data());
  args_shading.BindBuffer(2, {GL_ARRAY_BUFFER, sizeof(float), 3, GL_FLOAT},
                          model.normals().size(), model.normals().data());
  args_shading.BindBuffer(3, {GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int),
                              1, GL_UNSIGNED_INT},
                          model.indices().size(), model.indices().data());


  /// divided by 2 depends on Resolution: refactor it later
  int window_width = input_texture.width();
  int window_height = input_texture.height();
#ifdef __APPLE__
  window_width /= 2;
  window_height /= 2;
#endif
  gl::FrameBuffer fbo_shading(GL_RGB, window_width, window_height);
  window.Resize(window_width, window_height);

  fbo_shading.Bind();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  input_texture.Bind(0);
  glUseProgram(program_shading.id());

  uniforms_shading.Bind("lights",
                        light_positions.data(),
                        light_positions.size());
  uniforms_shading.Bind("light_power", &light_power, 1);
  uniforms_shading.Bind("light_color", &light_color, 1);
  glBindVertexArray(args_shading.vao());
  glDrawElements(GL_TRIANGLES, model.indices().size(), GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);

  window.swap_buffer();

  ss.str("");
  ss << config_loader.output_path + "/atlas_shading"
     << "_factor_" << config_loader.downsample_factor
     << ".png";
  cv::Mat &shading = fbo_shading.Capture();
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
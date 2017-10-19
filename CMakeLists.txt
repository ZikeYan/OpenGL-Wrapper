CMAKE_MINIMUM_REQUIRED(VERSION 3.0)
PROJECT(opengl_wrapper)

#----------
# C++ version setting
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -Wall -g -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3 -Wall")
#----------

#----------
# Project variable configurations
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

SET(OPENGL_WRAPPER_LIBRARIES "")
SET(OPENGL_WRAPPER_SOURCES
        src/window.cc
        src/framebuffer.cc
        src/program.cc
        src/args.cc
        src/model.cc
        src/texture.cc
        src/uniforms.cc
        src/camera.cc
        src/trajectory.cc)
#----------

#----------
# Package configurations

FIND_PACKAGE(PkgConfig)
## Find packages
LIST(APPEND
        CMAKE_MODULE_PATH
        ${PROJECT_SOURCE_DIR}/cmake_modules)
FIND_PACKAGE(GLFW3 REQUIRED)
FIND_PACKAGE(GLEW REQUIRED)
FIND_PACKAGE(glm REQUIRED)
FIND_PACKAGE(OpenGL REQUIRED)

# Magic for OpenCV with CUDA support
FIND_PACKAGE(OpenCV REQUIRED)

## Include
INCLUDE_DIRECTORIES(
        ${PROJECT_SOURCE_DIR}/include
        ${GLFW3_INCLUDE_DIRS}
        ${GLEW_INCLUDE_DIR}
        ${glm_INCLUDE_DIRS})

## Lib
LIST(APPEND
        OPENGL_WRAPPER_LIBRARIES
        ${OPENGL_LIBRARY}
        ${GLEW_LIBRARY})
if (APPLE)
    LIST(APPEND OPENGL_WRAPPER_LIBRARIES
            ${GLFW3_LIBRARIES})
elseif (UNIX)
    LIST(APPEND OPENGL_WRAPPER_LIBRARIES
            ${GLFW3_STATIC_LIBRARIES})
endif()

ADD_LIBRARY(opengl_wrapper SHARED ${OPENGL_WRAPPER_SOURCES})
TARGET_LINK_LIBRARIES(opengl_wrapper
        ${OPENGL_WRAPPER_LIBRARIES}
        -lopencv_core -lopencv_highgui -lopencv_imgproc)

#---------
# Compile and Link
ADD_EXECUTABLE(render_textured_model_multi_light example/render_textured_model_multi_light.cc)
TARGET_LINK_LIBRARIES(render_textured_model_multi_light opengl_wrapper)

ADD_EXECUTABLE(render_cube example/render_cube.cc)
TARGET_LINK_LIBRARIES(render_cube opengl_wrapper)

ADD_EXECUTABLE(render_shaded_texture example/render_shaded_texture.cc)
TARGET_LINK_LIBRARIES(render_shaded_texture opengl_wrapper)

ADD_EXECUTABLE(render_to_fbo example/render_to_fbo.cc)
TARGET_LINK_LIBRARIES(render_to_fbo opengl_wrapper)

ADD_EXECUTABLE(encode_sequence_pixel2uv_projection example/encode_sequence_pixel2uv_projection.cc)
TARGET_LINK_LIBRARIES(encode_sequence_pixel2uv_projection opengl_wrapper)

ADD_EXECUTABLE(encode_image_pixel2uv_projection example/encode_image_pixel2uv_projection.cc)
TARGET_LINK_LIBRARIES(encode_image_pixel2uv_projection opengl_wrapper)

ADD_EXECUTABLE(generate_spherical_poses
        example/synthesize_srid_dataset/generate_spherical_poses.cc)
TARGET_LINK_LIBRARIES(generate_spherical_poses -lopencv_core)
ADD_EXECUTABLE(synthesize_srid_dataset
        example/synthesize_srid_dataset/synthesize_srid_dataset.cc)
TARGET_LINK_LIBRARIES(synthesize_srid_dataset opengl_wrapper)
#---------
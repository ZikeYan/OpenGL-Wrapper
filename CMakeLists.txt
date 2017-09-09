CMAKE_MINIMUM_REQUIRED(VERSION 3.0)
PROJECT(opengl-snippet)

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

SET(OPENGL_SNIPPET_LIBRARIES "")
SET(OPENGL_SNIPPET_SOURCES
        src/window.cc
        src/program.cc
        src/args.cc
        src/model.cc
        src/texture.cc
        src/uniform.cc
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
        OPENGL_SNIPPET_LIBRARIES
        ${OPENGL_LIBRARY}
        ${GLEW_LIBRARY})
if (APPLE)
    LIST(APPEND OPENGL_SNIPPET_LIBRARIES
            ${GLFW3_LIBRARIES})
elseif (UNIX)
    LIST(APPEND OPENGL_SNIPPET_LIBRARIES
            ${GLFW3_STATIC_LIBRARIES})
endif()

ADD_LIBRARY(opengl-snippet ${OPENGL_SNIPPET_SOURCES})
TARGET_LINK_LIBRARIES(opengl-snippet
        ${OPENGL_SNIPPET_LIBRARIES} ${OpenCV_LIBS})

#---------
# Compile and Link
ADD_EXECUTABLE(model-demo src/model-demo.cc)
TARGET_LINK_LIBRARIES(model-demo opengl-snippet)

ADD_EXECUTABLE(cube-demo src/cube-demo.cc)
TARGET_LINK_LIBRARIES(cube-demo opengl-snippet)

ADD_EXECUTABLE(float-texture src/write-texture-demo.cc)
TARGET_LINK_LIBRARIES(float-texture opengl-snippet)

ADD_EXECUTABLE(atlas-shading src/shading.cc)
TARGET_LINK_LIBRARIES(atlas-shading opengl-snippet)


#---------
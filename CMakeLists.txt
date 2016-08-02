CMAKE_MINIMUM_REQUIRED(VERSION 3.0)
PROJECT(OpenGL_Template)

#----------
# C++ version setting
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -Wall -g -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3 -Wall")
#----------

#----------
# Project variable configurations
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
SET(OpenGL_Template_LIBRARIES "")
SET(OpenGL_Template_SOURCES
        src/main.cc
        src/GLObjects/plane.cc
        src/GLObjects/camera.cc
        src/utils/context.cc
        src/utils/control.cc
        src/utils/model.cc
        src/utils/shader.cc
        src/utils/texture.cc)
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
FIND_PACKAGE(OpenGL REQUIRED)
FIND_PACKAGE(OpenCV REQUIRED)

## Include
INCLUDE_DIRECTORIES(
        ${PROJECT_SOURCE_DIR}/include
        ${GLFW3_INCLUDE_DIRS}
        ${GLEW_INCLUDE_DIRS})
## Lib
LIST(APPEND
        OpenGL_Template_LIBRARIES
        ${OPENGL_LIBRARY}
        ${GLFW3_LIBRARIES}
        ${GLEW_LIBRARY})
#----------

#---------
# Compile and Link
ADD_EXECUTABLE(OpenGL_Template ${OpenGL_Template_SOURCES})
TARGET_LINK_LIBRARIES(OpenGL_Template
        ${OpenGL_Template_LIBRARIES}
        ${OpenCV_LIBS})
#---------
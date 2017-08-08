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
SET(OPENGL_SNIPPET_LIBRARIES "")
SET(OPENGL_SNIPPET_SOURCES
        include/gl_objects/vntf_demo.h
        src/gl_objects/gl_object.cc
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
ADD_EXECUTABLE(vntf src/vntf.cc)
TARGET_LINK_LIBRARIES(vntf opengl-snippet)

ADD_EXECUTABLE(v src/v.cc)
TARGET_LINK_LIBRARIES(v opengl-snippet)

ADD_EXECUTABLE(atlas src/atlas.cc)
TARGET_LINK_LIBRARIES(atlas opengl-snippet)
#---------
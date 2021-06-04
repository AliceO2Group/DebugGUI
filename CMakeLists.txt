cmake_policy(SET CMP0074 NEW)

cmake_minimum_required(VERSION 3.13.0 FATAL_ERROR)
set(BUILD_SHARED_LIBS ON)
set(CMAKE_CXX_STANDARD 17)

# Define project
project(DebugGUI
  VERSION 0.1.0
  DESCRIPTION "O2 DebugGUI Library"
  LANGUAGES C CXX
)

find_package(GLFW NAMES glfw3 CONFIG)

# Set CMAKE_INSTALL_LIBDIR explicitly to lib (to avoid lib64 on CC7)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "RelWithDebInfo"
    CACHE
    STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel Coverage."
    FORCE
  )
endif()
set(CMAKE_INSTALL_LIBDIR lib)
add_subdirectory(DebugGUI)

# Copyright CERN and copyright holders of ALICE O2. This software is distributed
# under the terms of the GNU General Public License v3 (GPL Version 3), copied
# verbatim in the file "COPYING".
#
# See http://alice-o2.web.cern.ch/license for full licensing information.
#
# In applying this license CERN does not waive the privileges and immunities
# granted to it by virtue of its status as an Intergovernmental Organization or
# submit itself to any jurisdiction.

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build targets with install rpath on Mac to dramatically speed up installation
# https://gitlab.kitware.com/cmake/community/wikis/doc/cmake/RPATH-handling
#set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
#list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" isSystemDir)
#if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
#  if("${isSystemDir}" STREQUAL "-1")
#    set(CMAKE_INSTALL_RPATH "@loader_path/../${CMAKE_INSTALL_LIBDIR}")
#  endif()
#  set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
#endif()

if(GLFW_FOUND)
message("FOO ${GLFW3_FOUND} ${GLFW_FOUND}")
  set(GUI_BACKEND
      src/imgui_impl_glfw_gl3.cpp
      src/gl3w.c
      src/Sokol3DUtils.cxx
      src/GL3DUtils.cxx
      src/HandMade3DImpl.cxx
      src/DebugGUI.cxx)
  set(GUI_TARGET glfw)
else()
  set(GUI_BACKEND src/Dummy3DUtils.cxx src/DummyDebugGUI.cxx)
  set(GUI_TARGET "")
endif()

add_library(O2DebugGUI SHARED
            src/imgui.cpp src/imgui_draw.cpp src/imgui_extras.cpp
            ${GUI_BACKEND})

set(HEADERS
  include/DebugGUI/imgui_extras.h
  include/DebugGUI/imgui.h
  include/DebugGUI/imconfig.h
  include/DebugGUI/Sokol3DUtils.h
  include/DebugGUI/GL3DUtils.h
  include/DebugGUI/DebugGUI.h
)

set_target_properties(O2DebugGUI PROPERTIES PUBLIC_HEADER "${HEADERS}")
target_include_directories(O2DebugGUI PUBLIC
                           ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(O2DebugGUI PUBLIC GLFW PRIVATE dl)

if(GLFW_FOUND)
  add_executable(imgui_demo
                 test/test_ImGUI.cpp test/imgui_demo.cpp
                 )
target_link_libraries(imgui_demo O2DebugGUI GLFW)
endif()

add_executable(ImGUIHeadless
               test/test_ImGUIHeadless.cpp test/imgui_demo.cpp
               )
target_link_libraries(ImGUIHeadless O2DebugGUI)

add_executable(test_DebugGUIGL
  test/test_DebugGUIGL.cxx
)
target_link_libraries(test_DebugGUIGL O2DebugGUI)

add_executable(test_DebugGUISokol
  test/test_DebugGUISokol.cxx
)
target_link_libraries(test_DebugGUISokol O2DebugGUI)

install(TARGETS imgui_demo ImGUIHeadless O2DebugGUI test_DebugGUISokol test_DebugGUIGL
  RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
  LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
  ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_PREFIX}/include/DebugGUI
)

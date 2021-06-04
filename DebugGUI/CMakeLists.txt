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

if(APPLE)
  set(GUI_BACKEND
      src/imgui_impl_metal.mm
      src/imgui_impl_glfw.cpp
      src/implot.cpp
      src/implot_items.cpp
#      src/Sokol3DUtils.cxx
      src/HandMade3DImpl.cxx
      src/DebugGUIMacos.mm)
  set(HEADERS
    src/imgui_extras.h
    src/imgui.h
    src/implot.h
    src/implot_internal.h
    src/imconfig.h
#    src/Sokol3DUtils.h
    src/DebugGUI.h
    src/imgui_impl_glfw.h
    src/icons_font_awesome.h
  )
  set(GUI_TARGET glfw)
elseif(GLFW_FOUND)
  set(GUI_BACKEND
      src/imgui_impl_glfw_gl3.cpp
      src/implot.cpp
      src/implot_items.cpp
      src/gl3w.c
      src/Sokol3DUtils.cxx
      src/GL3DUtils.cxx
      src/HandMade3DImpl.cxx
      src/DebugGUI.cxx)
  set(HEADERS
    src/imgui_extras.h
    src/imgui.h
    src/implot.h
    src/implot_internal.h
    src/imconfig.h
    src/Sokol3DUtils.h
    src/GL3DUtils.h
    src/DebugGUI.h
    src/imgui_impl_glfw_gl3.h
    src/icons_font_awesome.h
  )
  set(GUI_TARGET glfw)
else()
  set(GUI_BACKEND src/Dummy3DUtils.cxx src/DummyDebugGUI.cxx)
  set(HEADERS
    src/imgui_extras.h
    src/imgui.h
    src/implot.h
    src/implot_internal.h
    src/imconfig.h
    src/Sokol3DUtils.h
    src/GL3DUtils.h
    src/DebugGUI.h
    src/imgui_impl_glfw_gl3.h
    src/icons_font_awesome.h
  )
  set(GUI_TARGET "")
endif()

add_library(DebugGUI SHARED
            src/imgui.cpp src/imgui_draw.cpp src/imgui_extras.cpp src/imgui_tables.cpp src/imgui_widgets.cpp
            ${GUI_BACKEND})

set_target_properties(DebugGUI PROPERTIES PUBLIC_HEADER "${HEADERS}")
set_target_properties(DebugGUI PROPERTIES OUTPUT_NAME "O2DebugGUI")
if (APPLE)
find_library(METAL Metal)
find_library(CF CoreFoundation)
find_library(FOUNDATION Foundation)
find_library(QUARTZ_CORE QuartzCore)
set_target_properties(DebugGUI PROPERTIES COMPILE_FLAGS "-fobjc-arc")
set(MACOS_FRAMEWORKS ${METAL} ${CF} ${FOUNDATION} ${QUARTZ_CORE})
endif()

target_include_directories(DebugGUI
  PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

if(APPLE)
  target_link_libraries(DebugGUI PUBLIC glfw ${MACOS_FRAMEWORKS} PRIVATE dl)
  add_executable(imgui_demo
                 test/test_DebugGUIMetal.cxx test/imgui_demo.cpp test/implot_demo.cpp
                 )
  target_link_libraries(imgui_demo DebugGUI glfw)
  set(GLFW_TESTS imgui_demo)
elseif(GLFW_FOUND)
  target_link_libraries(DebugGUI PUBLIC glfw PRIVATE dl)
  add_executable(imgui_demo
                 test/test_ImGUI.cpp test/imgui_demo.cpp test/implot_demo.cpp
                 )
  target_link_libraries(imgui_demo DebugGUI glfw)
  add_dependencies(imgui_demo DebugGUI)
  add_executable(test_DebugGUIGL
    test/test_DebugGUIGL.cxx
  )
  target_link_libraries(test_DebugGUIGL DebugGUI)

  add_executable(test_DebugGUISokol
    test/test_DebugGUISokol.cxx
  )
  target_link_libraries(test_DebugGUISokol DebugGUI)
  set(GLFW_TESTS imgui_demo test_DebugGUISokol test_DebugGUIGL)

endif()

add_executable(ImGUIHeadless
               test/test_ImGUIHeadless.cpp test/imgui_demo.cpp
               )
target_link_libraries(ImGUIHeadless DebugGUI)


# Install
include(GNUInstallDirs)

# Create version file
include(CMakePackageConfigHelpers)
write_basic_package_version_file("${CMAKE_CURRENT_BINARY_DIR}/cmake/DebugGUIConfigVersion.cmake"
  VERSION ${PACKAGE_VERSION}
  COMPATIBILITY AnyNewerVersion
)

set(CMAKE_INSTALL_LIBDIR lib)

install(TARGETS DebugGUI
  EXPORT
  DebugGUITargets
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(TARGETS ${GLFW_TESTS} ImGUIHeadless
  EXPORT
  DebugGUITestsTargets
  RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
)

# Export targets
install(EXPORT DebugGUITargets
  FILE
    DebugGUITargets.cmake
  NAMESPACE
    AliceO2::
  DESTINATION
    ${CMAKE_INSTALL_LIBDIR}/cmake/DebugGUI
)

# Configure and install Config files
configure_package_config_file(
  "${PROJECT_SOURCE_DIR}/cmake/DebugGUIConfig.cmake.in" cmake/DebugGUIConfig.cmake
  INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/DebugGUI"
  PATH_VARS CMAKE_INSTALL_PREFIX
)

install(
  FILES
    "${CMAKE_CURRENT_BINARY_DIR}/cmake/DebugGUIConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake/DebugGUIConfigVersion.cmake"
  DESTINATION
  ${CMAKE_INSTALL_LIBDIR}/cmake/DebugGUI
)

install(
  FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/src/HandMadeMath.h"
  DESTINATION
  ${CMAKE_INSTALL_PREFIX}/include
)

install(
  DIRECTORY
  "${CMAKE_CURRENT_SOURCE_DIR}/include/GL"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/KHR"
  DESTINATION
  ${CMAKE_INSTALL_PREFIX}/include
)

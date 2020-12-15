cmake_minimum_required(VERSION 3.5.1 FATAL_ERROR)

project(ign-imgui VERSION 0.0.1)

find_package(ignition-cmake2 REQUIRED)
ign_find_package(ignition-transport9 REQUIRED)
ign_find_package(ignition-msgs6 REQUIRED)
ign_find_package(ignition-common3 REQUIRED)

find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

add_executable(ign_imgui
  Histogram.cc
  main.cc
  ./imgui/imgui.cpp
  ./imgui/imgui_draw.cpp
  ./imgui/imgui_widgets.cpp
  ./imgui/imgui_demo.cpp
  ./imgui/examples/imgui_impl_glfw.cpp
  ./imgui/examples/imgui_impl_opengl3.cpp
)
target_include_directories(ign_imgui
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/imgui
  ${OPENGL_INCLUDE_DIRS}
  ${GLEW_INCLUDE_DIRS}
)

target_compile_definitions(ign_imgui
  PUBLIC
  IMGUI_IMPL_OPENGL_LOADER_GLEW
)

target_link_libraries(ign_imgui
  PRIVATE
  ignition-common3::ignition-common3
  ignition-transport9::ignition-transport9
  ignition-msgs6::ignition-msgs6
  glfw
  ${OPENGL_gl_LIBRARY}
  ${OPENGL_glu_LIBRARY}
  ${GLEW_LIBRARIES}
)


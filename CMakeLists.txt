cmake_minimum_required(VERSION 3.15.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project("video-drugs")
set(VD_TARGET "video-drugs")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -ggdb")

set(BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

find_package(spdlog REQUIRED)

set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)
find_package(SDL3 REQUIRED)

if (CMAKE_SYSTEM_NAME STREQUAL Linux)
  find_package(X11 REQUIRED)

  if (NOT X11_Xi_FOUND)
    message(FATAL_ERROR "X11 Xi library is required")
  endif ()
endif ()

set(IMGUI_SRCS
  # Main imgui files
  ${BASE_DIR}/third-party/imgui/imgui.cpp
  ${BASE_DIR}/third-party/imgui/imgui_demo.cpp
  ${BASE_DIR}/third-party/imgui/imgui_draw.cpp
  ${BASE_DIR}/third-party/imgui/imgui_tables.cpp
  ${BASE_DIR}/third-party/imgui/imgui_widgets.cpp

  # Imgui backend files
  ${BASE_DIR}/third-party/imgui/backends/imgui_impl_sdl3.cpp
  ${BASE_DIR}/third-party/imgui/backends/imgui_impl_opengl3.cpp
)

set(VD_SRCS
  # Core files
  main.cpp
)

add_executable(${VD_TARGET}
  ${IMGUI_SRCS}
  ${VD_SRCS}

  # Main ile
  ${BASE_DIR}/main.cc
)

target_include_directories(${VD_TARGET} PRIVATE
  ${BASE_DIR}/src
  ${BASE_DIR}/third-party/imgui
  ${BASE_DIR}/third-party/imgui/backends
  ${BASE_DIR}/third-party/imgui_club/imgui_memory_editor
  ${OPENGL_INCLUDE_DIRS}
  ${SDL2_INCLUDE_DIRS}
)

target_link_libraries(${VD_TARGET}
  ${SDL2_LIBRARIES}
  ${OPENGL_LIBRARIES}
  spdlog::spdlog_header_only
  -lzip
)

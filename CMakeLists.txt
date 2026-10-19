cmake_minimum_required(VERSION 3.25)

project(sogui LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(SOGUI_BUILD_TESTS "构建测试" ${PROJECT_IS_TOP_LEVEL})

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(SoGuiDependencies)
include(SoGuiWarnings)

add_subdirectory(src)
add_subdirectory(app)
add_subdirectory(examples/minimal)

cmake_minimum_required(VERSION 3.24)
set(PROJECT_NAME scwx-glad)

# Path to glad directory
set(GLAD_SOURCES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/glad/")

# Path to glad CMake files
add_subdirectory("${GLAD_SOURCES_DIR}/cmake" glad_cmake)

# Specify glad settings
#
# Android only exposes OpenGL ES, so generate an ES loader there instead of the
# desktop 3.3 core loader. The target name is kept identical so that every
# consumer (wxdata, scwx-qt, maplibre) links unchanged.
if (ANDROID)
    glad_add_library(glad_gl_core_33 LOADER REPRODUCIBLE API gles2=3.2 egl=1.5)
else()
    glad_add_library(glad_gl_core_33 LOADER REPRODUCIBLE API gl:core=3.3)
endif()

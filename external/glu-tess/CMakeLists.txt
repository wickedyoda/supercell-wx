cmake_minimum_required(VERSION 3.24)
set(PROJECT_NAME scwx-glu-tess)

# libtess2 provides the polygon tessellation that GLU would normally supply.
# It is only needed on platforms without GLU (Android / OpenGL ES).
add_library(tess2 STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/libtess2/Source/bucketalloc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/libtess2/Source/dict.c
    ${CMAKE_CURRENT_SOURCE_DIR}/libtess2/Source/geom.c
    ${CMAKE_CURRENT_SOURCE_DIR}/libtess2/Source/mesh.c
    ${CMAKE_CURRENT_SOURCE_DIR}/libtess2/Source/priorityq.c
    ${CMAKE_CURRENT_SOURCE_DIR}/libtess2/Source/sweep.c
    ${CMAKE_CURRENT_SOURCE_DIR}/libtess2/Source/tess.c)

target_include_directories(tess2 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/libtess2/Include)
target_compile_definitions(tess2 PRIVATE NDEBUG)

add_library(glu_tess STATIC glu_tess.cpp)
target_link_libraries(glu_tess PUBLIC tess2)
target_include_directories(glu_tess PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

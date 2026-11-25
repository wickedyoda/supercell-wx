#pragma once

// Android builds against OpenGL ES, for which glad generates a gles2 header
#if defined(__ANDROID__)
#include <glad/gles2.h>
#else
#include <glad/gl.h>
#endif

// OpenGL ES has no glPolygonMode: wireframe drawing is a desktop-only path
#if defined(__ANDROID__)
#define SCWX_GL_POLYGON_MODE(mode) ((void) 0)
#else
#define SCWX_GL_POLYGON_MODE(mode) glPolygonMode(GL_FRONT_AND_BACK, mode)
#endif

#define SCWX_GL_CHECK_ERROR()                                                  \
   {                                                                           \
      GLenum err;                                                              \
      while ((err = glGetError()) != GL_NO_ERROR)                              \
      {                                                                        \
         logger_->error("GL Error: {}, {}: {}", err, __FILE__, __LINE__);      \
      }                                                                        \
   }

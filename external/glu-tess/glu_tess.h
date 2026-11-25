#pragma once

// Minimal GLU tessellator compatibility layer for OpenGL ES platforms.
//
// Android has no GLU. Supercell Wx only uses GLU for polygon tessellation, so
// this header declares the small subset of the GLU tessellation API that the
// renderer needs. The implementation is backed by libtess2.

#include <GLES3/gl3.h>

#include <cstddef>

#ifndef GLdouble
typedef double GLdouble;
#endif

#ifndef GLU_TESS_BEGIN
#define GLU_TESS_BEGIN 100100
#define GLU_TESS_VERTEX 100101
#define GLU_TESS_END 100102
#define GLU_TESS_ERROR 100103
#define GLU_TESS_EDGE_FLAG 100104
#define GLU_TESS_COMBINE 100105
#define GLU_TESS_BEGIN_DATA 100106
#define GLU_TESS_VERTEX_DATA 100107
#define GLU_TESS_END_DATA 100108
#define GLU_TESS_ERROR_DATA 100109
#define GLU_TESS_EDGE_FLAG_DATA 100110
#define GLU_TESS_COMBINE_DATA 100111
#define GLU_TESS_BOUNDARY_ONLY 100141
#define GLU_TESS_TOLERANCE 100142
#define GLU_TESS_WINDING_RULE 100140
#define GLU_TESS_WINDING_ODD 100130
#define GLU_TESS_WINDING_NONZERO 100131
#define GLU_TESS_WINDING_POSITIVE 100132
#define GLU_TESS_WINDING_NEGATIVE 100133
#define GLU_TESS_WINDING_ABS_GEQ_TWO 100134
#endif

typedef struct GLUtesselator GLUtesselator;

typedef void (*_GLUfuncptr)(void);

GLUtesselator* gluNewTess(void);
void           gluDeleteTess(GLUtesselator* tess);
void           gluTessCallback(GLUtesselator* tess, GLenum which, _GLUfuncptr fn);
void           gluTessProperty(GLUtesselator* tess, GLenum which, GLdouble value);
void           gluTessBeginPolygon(GLUtesselator* tess, void* polygonData);
void           gluTessBeginContour(GLUtesselator* tess);
void           gluTessVertex(GLUtesselator* tess, GLdouble coords[3], void* data);
void           gluTessEndContour(GLUtesselator* tess);
void           gluTessEndPolygon(GLUtesselator* tess);

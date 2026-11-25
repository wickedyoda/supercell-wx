// GLU tessellator compatibility layer for OpenGL ES platforms, backed by
// libtess2. See glu_tess.h for the rationale.

#include "glu_tess.h"

#include <tesselator.h>

#include <cstring>
#include <vector>

namespace
{

struct Contour
{
   std::size_t start_ {0};
   std::size_t count_ {0};
};

} // namespace

struct GLUtesselator
{
   std::vector<GLdouble> coords_;
   std::vector<void*>    pointers_;
   std::vector<Contour>  contours_;

   std::vector<GLdouble> contourCoords_;
   std::vector<void*>    contourPointers_;

   void* polygonData_ {nullptr};
   int   windingRule_ {GLU_TESS_WINDING_ODD};

   void (*beginData_)(GLenum, void*) {nullptr};
   void (*vertexData_)(void*, void*) {nullptr};
   void (*endData_)(void*) {nullptr};
   void (*edgeFlag_)(void) {nullptr};
   void (*combineData_)(GLdouble[3], void*[4], GLfloat[4], void**, void*)
      {nullptr};
   void (*errorData_)(GLenum, void*) {nullptr};
};

GLUtesselator* gluNewTess(void)
{
   return new GLUtesselator();
}

void gluDeleteTess(GLUtesselator* tess)
{
   delete tess;
}

void gluTessCallback(GLUtesselator* tess, GLenum which, _GLUfuncptr fn)
{
   if (tess == nullptr)
   {
      return;
   }

   switch (which)
   {
   case GLU_TESS_BEGIN_DATA:
      tess->beginData_ = reinterpret_cast<void (*)(GLenum, void*)>(fn);
      break;
   case GLU_TESS_VERTEX_DATA:
      tess->vertexData_ = reinterpret_cast<void (*)(void*, void*)>(fn);
      break;
   case GLU_TESS_END_DATA:
      tess->endData_ = reinterpret_cast<void (*)(void*)>(fn);
      break;
   case GLU_TESS_EDGE_FLAG:
      tess->edgeFlag_ = reinterpret_cast<void (*)(void)>(fn);
      break;
   case GLU_TESS_COMBINE_DATA:
      tess->combineData_ =
         reinterpret_cast<void (*)(GLdouble[3], void*[4], GLfloat[4], void**,
                                   void*)>(fn);
      break;
   case GLU_TESS_ERROR_DATA:
      tess->errorData_ = reinterpret_cast<void (*)(GLenum, void*)>(fn);
      break;
   default:
      // Unsupported callbacks are accepted and ignored, matching GLU's
      // behaviour of only invoking callbacks that were registered.
      break;
   }
}

void gluTessProperty(GLUtesselator* tess, GLenum which, GLdouble value)
{
   if (tess == nullptr)
   {
      return;
   }

   if (which == GLU_TESS_WINDING_RULE)
   {
      tess->windingRule_ = static_cast<int>(value);
   }
}

void gluTessBeginPolygon(GLUtesselator* tess, void* polygonData)
{
   if (tess == nullptr)
   {
      return;
   }

   tess->coords_.clear();
   tess->pointers_.clear();
   tess->contours_.clear();
   tess->contourCoords_.clear();
   tess->contourPointers_.clear();
   tess->polygonData_ = polygonData;
}

void gluTessBeginContour(GLUtesselator* tess)
{
   if (tess == nullptr)
   {
      return;
   }

   tess->contourCoords_.clear();
   tess->contourPointers_.clear();
}

void gluTessVertex(GLUtesselator* tess, GLdouble coords[3], void* data)
{
   if (tess == nullptr)
   {
      return;
   }

   tess->contourCoords_.insert(tess->contourCoords_.end(),
                               coords,
                               coords + 3);
   tess->contourPointers_.push_back(data);
}

void gluTessEndContour(GLUtesselator* tess)
{
   if (tess == nullptr)
   {
      return;
   }

   Contour contour {};
   contour.start_ = tess->coords_.size() / 3;
   contour.count_ = tess->contourPointers_.size();

   tess->coords_.insert(tess->coords_.end(),
                        tess->contourCoords_.begin(),
                        tess->contourCoords_.end());
   tess->pointers_.insert(tess->pointers_.end(),
                          tess->contourPointers_.begin(),
                          tess->contourPointers_.end());
   tess->contours_.push_back(contour);
}

void gluTessEndPolygon(GLUtesselator* tess)
{
   if (tess == nullptr || tess->contours_.empty())
   {
      return;
   }

   TESStesselator* tesselator = tessNewTess(nullptr);
   if (tesselator == nullptr)
   {
      return;
   }

   for (const Contour& contour : tess->contours_)
   {
      tessAddContour(tesselator,
                     3,
                     tess->coords_.data() + (contour.start_ * 3),
                     static_cast<int>(sizeof(GLdouble) * 3),
                     static_cast<int>(contour.count_));
   }

   int windingRule = TESS_WINDING_ODD;
   switch (tess->windingRule_)
   {
   case GLU_TESS_WINDING_NONZERO:
      windingRule = TESS_WINDING_NONZERO;
      break;
   case GLU_TESS_WINDING_POSITIVE:
      windingRule = TESS_WINDING_POSITIVE;
      break;
   case GLU_TESS_WINDING_NEGATIVE:
      windingRule = TESS_WINDING_NEGATIVE;
      break;
   case GLU_TESS_WINDING_ABS_GEQ_TWO:
      windingRule = TESS_WINDING_ABS_GEQ_TWO;
      break;
   default:
      windingRule = TESS_WINDING_ODD;
      break;
   }

   if (tessTesselate(tesselator, windingRule, TESS_POLYGONS, 3, 3, nullptr) ==
       0)
   {
      if (tess->errorData_ != nullptr)
      {
         tess->errorData_(GLU_TESS_ERROR, tess->polygonData_);
      }

      tessDeleteTess(tesselator);
      return;
   }

   const TESSindex* elements = tessGetElements(tesselator);
   const int        elementCount = tessGetElementCount(tesselator) * 3;
   const int        vertexCount =
      static_cast<int>(tess->pointers_.size());

   if (tess->beginData_ != nullptr)
   {
      tess->beginData_(GL_TRIANGLES, tess->polygonData_);
   }

   if (tess->vertexData_ != nullptr)
   {
      for (int i = 0; i < elementCount; ++i)
      {
         const int index = elements[i];

         // Vertices generated by the tessellator (from self-intersections)
         // have no application-side payload; dropping those triangles keeps
         // the draw data valid rather than dereferencing a null pointer.
         if (index == TESS_UNDEF || index < 0 || index >= vertexCount)
         {
            continue;
         }

         tess->vertexData_(tess->pointers_[static_cast<std::size_t>(index)],
                           tess->polygonData_);
      }
   }

   if (tess->endData_ != nullptr)
   {
      tess->endData_(tess->polygonData_);
   }

   tessDeleteTess(tesselator);
}

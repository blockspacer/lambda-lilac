#include "frustum.h"
#include "utils/mt_manager.h"
#include <utils/console.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

#if VIOLET_WIN32
#include <Windows.h>
#include <process.h>

#ifdef near
#define __near__ near
#undef near
#endif

#ifdef far
#define __far__ far
#undef far
#endif

#ifdef min
#define __min__ min
#undef min
#endif

#ifdef max
#define __max__ max
#undef max
#endif
#endif

namespace lambda
{
  namespace utilities
  {
    ///////////////////////////////////////////////////////////////////////////
    void Frustum::construct(glm::mat4x4 projection, const glm::mat4x4& view)
    {
      // Create the frustum matrix from the view matrix 
      // and updated projection matrix.
      glm::mat4x4 matrix = projection * view;
      constructPlanes(matrix);
      constructCorners(glm::inverse(matrix));
    }

    ///////////////////////////////////////////////////////////////////////////
    bool Frustum::ContainsAABB(
      const glm::vec3& min, 
      const glm::vec3& max) const
    {
      // Check if any one point of the cube is in the view frustum.
      for (uint8_t i = 0u; i < 6u; ++i)
      {
        if (planes_[i].dot(glm::vec3(min.x, min.y, min.z)) < 0.0f &&
            planes_[i].dot(glm::vec3(min.x, min.y, max.z)) < 0.0f &&
            planes_[i].dot(glm::vec3(min.x, max.y, min.z)) < 0.0f &&
            planes_[i].dot(glm::vec3(min.x, max.y, max.z)) < 0.0f &&
            planes_[i].dot(glm::vec3(max.x, min.y, min.z)) < 0.0f &&
            planes_[i].dot(glm::vec3(max.x, min.y, max.z)) < 0.0f &&
            planes_[i].dot(glm::vec3(max.x, max.y, min.z)) < 0.0f &&
            planes_[i].dot(glm::vec3(max.x, max.y, max.z)) < 0.0f)
        {
          return false;
        }
      }

      return true;
    }

    ///////////////////////////////////////////////////////////////////////////
    bool Frustum::ContainsSphere(const glm::vec3& position, const float& radius) const
    {
      if      (planes_[0].a * position.x + planes_[0].b * 
        position.y + planes_[0].c * 
        position.z + planes_[0].d <= -radius) { return false; }
      
      else if (planes_[1].a * position.x + planes_[1].b * 
        position.y + planes_[1].c * 
        position.z + planes_[1].d <= -radius) { return false; }
      
      else if (planes_[2].a * position.x + planes_[2].b * 
        position.y + planes_[2].c * 
        position.z + planes_[2].d <= -radius) { return false; }
      
      else if (planes_[3].a * position.x + planes_[3].b * 
        position.y + planes_[3].c * 
        position.z + planes_[3].d <= -radius) { return false; }
      
      else if (planes_[4].a * position.x + planes_[4].b * 
        position.y + planes_[4].c * 
        position.z + planes_[4].d <= -radius) { return false; }
      
      else if (planes_[5].a * position.x + planes_[5].b * 
        position.y + planes_[5].c * 
        position.z + planes_[5].d <= -radius) { return false; }
      
      else { return true;  }
    }

    ///////////////////////////////////////////////////////////////////////////
    void Frustum::Contains(
      CullData* data, 
      const uint32_t& count, 
      const CullType& type) const
    {
      CullData* d = data;

      if (type == CullType::kSphere)
      {
        for (uint32_t i = 0u; i < count; ++i)
        {
          d->visible = ContainsSphere(d->sphere.position, d->sphere.radius);
          d++;
        }
      }
      else if (type == CullType::kAABB)
      {
        for (uint32_t i = 0u; i < count; ++i)
        {
          d->visible = ContainsAABB(d->aabb.min, d->aabb.max);
          d++;
        }
      }
    }

    ///////////////////////////////////////////////////////////////////////////
    Vector<Plane> Frustum::getPlanes() const
    {
      return planes_;
    }

    ///////////////////////////////////////////////////////////////////////////
    Vector<glm::vec3> Frustum::getCorners() const
    {
      return corners_;
    }

    ///////////////////////////////////////////////////////////////////////////
    glm::vec3 Frustum::getCenter() const
    {
      return center_;
    }

    ///////////////////////////////////////////////////////////////////////////
    glm::vec3 Frustum::getMin() const
    {
      return min_;
    }

    ///////////////////////////////////////////////////////////////////////////
    glm::vec3 Frustum::getMax() const
    {
      return max_;
    }

    ///////////////////////////////////////////////////////////////////////////
    void Frustum::constructPlanes(const glm::mat4x4& matrix)
    {
      planes_.resize(8u);
      // Calculate near plane of frustum.
      planes_[0].a = matrix[0][3] + matrix[0][2];
      planes_[0].b = matrix[1][3] + matrix[1][2];
      planes_[0].c = matrix[2][3] + matrix[2][2];
      planes_[0].d = matrix[3][3] + matrix[3][2];

      // Calculate far plane of frustum.
      planes_[1].a = matrix[0][3] - matrix[0][2];
      planes_[1].b = matrix[1][3] - matrix[1][2];
      planes_[1].c = matrix[2][3] - matrix[2][2];
      planes_[1].d = matrix[3][3] - matrix[3][2];

      // Calculate left plane of frustum.
      planes_[2].a = matrix[0][3] + matrix[0][0];
      planes_[2].b = matrix[1][3] + matrix[1][0];
      planes_[2].c = matrix[2][3] + matrix[2][0];
      planes_[2].d = matrix[3][3] + matrix[3][0];

      // Calculate right plane of frustum.
      planes_[3].a = matrix[0][3] - matrix[0][0];
      planes_[3].b = matrix[1][3] - matrix[1][0];
      planes_[3].c = matrix[2][3] - matrix[2][0];
      planes_[3].d = matrix[3][3] - matrix[3][0];

      // Calculate top plane of frustum.
      planes_[4].a = matrix[0][3] - matrix[0][1];
      planes_[4].b = matrix[1][3] - matrix[1][1];
      planes_[4].c = matrix[2][3] - matrix[2][1];
      planes_[4].d = matrix[3][3] - matrix[3][1];

      // Calculate bottom plane of frustum.
      planes_[5].a = matrix[0][3] + matrix[0][1];
      planes_[5].b = matrix[1][3] + matrix[1][1];
      planes_[5].c = matrix[2][3] + matrix[2][1];
      planes_[5].d = matrix[3][3] + matrix[3][1];

      // Normalize planes.
      for (uint8_t i = 0u; i < 6u; ++i)
      {
        planes_[i].normalize();
      }
    }

    ///////////////////////////////////////////////////////////////////////////
    void Frustum::constructCorners(const glm::mat4x4& matrix)
    {
      corners_.resize(8u);
      static const glm::vec4 corners[8u] = {
        // Near.
        { -1.0f, -1.0f, -1.0f, 1.0f },
        { +1.0f, -1.0f, -1.0f, 1.0f },
        { +1.0f, +1.0f, -1.0f, 1.0f },
        { -1.0f, +1.0f, -1.0f, 1.0f },
        // Far.
        { -1.0f, -1.0f, +1.0f, 1.0f },
        { +1.0f, -1.0f, +1.0f, 1.0f },
        { +1.0f, +1.0f, +1.0f, 1.0f },
        { -1.0f, +1.0f, +1.0f, 1.0f },
      };

      min_ = glm::vec3(FLT_MAX);
      max_ = glm::vec3(FLT_MIN);

      // Construct corners.
      for (uint8_t i = 0u; i < 8u; ++i)
      {
        glm::vec4 corner = matrix * corners[i];
        glm::vec3 c = glm::vec3(
          corner.x / corner.w,
          corner.y / corner.w,
          corner.z / corner.w
        );

        min_ = glm::min(min_, c);
        max_ = glm::max(max_, c);

        corners_.at(i) = c;
      }

      glm::vec4 c1 = matrix * glm::vec4(0.0f, 0.0f, +1.0f, 1.0f);
      glm::vec4 c2 = matrix * glm::vec4(0.0f, 0.0f, -1.0f, 1.0f);
      center_ = (glm::vec3(c1.x, c1.y, c1.z) / 
        c1.w + glm::vec3(c2.x, c2.y, c2.z) / c2.w) * 0.5f;
    }
  }
}

#if VIOLET_WIN32
#ifdef __near__
#define near __near__
#undef __near__
#endif

#ifdef __far__
#define far __far__
#undef __far__
#endif

#ifdef __min__
#define min __min__
#undef __min__
#endif

#ifdef __max__
#define max __max__
#undef __max__
#endif
#endif

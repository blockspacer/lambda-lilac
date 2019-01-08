#pragma once
#include <containers/containers.h>
#include <glm/glm.hpp>
#include <thread>
#include <mutex>

namespace lambda
{
  namespace utilities
  {
    ///////////////////////////////////////////////////////////////////////////
    struct CullData
    {
      CullData() {};
      union
      {
        struct
        {
          glm::vec3 position;
          float radius;
        } sphere;
        struct
        {
          glm::vec3 min;
          glm::vec3 max;
        } aabb;
      };
      bool visible = true;
    };

    ///////////////////////////////////////////////////////////////////////////
    enum class CullType : uint8_t
    {
      kSphere,
      kAABB
    };
    
    ///////////////////////////////////////////////////////////////////////////
    struct Plane
    {
      Plane normalized() const
      {
        Plane result;
        float distance = std::sqrtf(a * a + b * b + c * c);
        result.a = a / distance;
        result.b = b / distance;
        result.c = c / distance;
        result.d = d / distance;
        return result;
      }
      void normalize()
      {
        float distance = std::sqrtf(a * a + b * b + c * c);
        a /= distance;
        b /= distance;
        c /= distance;
        d /= distance;
      }
      float dot(const glm::vec3& v) const
      {
        return a * v.x + b * v.y + c * v.z + d;
      }

      float a;
      float b;
      float c;
      float d;
    };

    ///////////////////////////////////////////////////////////////////////////
    class Frustum
    {
    public:
      void construct(glm::mat4x4 projection, const glm::mat4x4& view);
      bool ContainsAABB(const glm::vec3& min, const glm::vec3& max) const;
      bool ContainsSphere(
        const glm::vec3& position, 
        const float& radius
      ) const;
      void Contains(
        CullData* data, 
        const uint32_t& count, 
        const CullType& type
      ) const;
      Vector<Plane> getPlanes() const;
      Vector<glm::vec3> getCorners() const;
      glm::vec3 getCenter() const;
      glm::vec3 getMin() const;
      glm::vec3 getMax() const;

    private:
      void constructPlanes(const glm::mat4x4& matrix);
      void constructCorners(const glm::mat4x4& matrix);

    private:
      Vector<Plane> planes_;
      Vector<glm::vec3> corners_;
      glm::vec3 center_;
      glm::vec3 min_;
      glm::vec3 max_;
    };
  }
}
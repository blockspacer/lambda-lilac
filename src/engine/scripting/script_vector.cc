#include "script_vector.h"
#include <math.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>

namespace lambda
{
  namespace scripting
  {
    // Constructors.
    ScriptVec2::ScriptVec2() :
      x(0.0f), y(0.0f) {}
    ScriptVec2::ScriptVec2(const glm::vec2& v) :
      x(v.x), y(v.y)
    {
    }
    ScriptVec2::ScriptVec2(const ScriptVec2& v) :
      x(v.x), y(v.y) {}
    ScriptVec2::ScriptVec2(const float& v) :
      x(v), y(v) {}
    ScriptVec2::ScriptVec2(const float& v1, const float& v2) :
      x(v1), y(v2) {}

    ScriptVec2::operator glm::vec2() const
    {
      return glm::vec2(x, y);
    }

    void ScriptVec2::operator=(const glm::vec2& other)
    {
      x = other.x;
      y = other.y;
    }

    // Operators.
    void ScriptVec2::operator=(const ScriptVec2& other)
    {
      x = other.x;
      y = other.y;
    }
    bool ScriptVec2::operator==(const ScriptVec2& other) const
    {
      return x == other.x&& x == other.y;
    }
    bool ScriptVec2::operator!=(const ScriptVec2& other) const
    {
      return !(*this == other);
    }
    ScriptVec2& ScriptVec2::operator+=(const ScriptVec2& other)
    {
      x += other.x;
      y += other.y;
      return *this;
    }
    ScriptVec2& ScriptVec2::operator-=(const ScriptVec2& other)
    {
      x -= other.x;
      y -= other.y;
      return *this;
    }
    ScriptVec2& ScriptVec2::operator/=(const ScriptVec2& other)
    {
      x /= other.x;
      y /= other.y;
      return *this;
    }
    ScriptVec2& ScriptVec2::operator*=(const ScriptVec2& other)
    {
      x *= other.x;
      y *= other.y;
      return *this;
    }
    ScriptVec2& ScriptVec2::operator+=(const float& v)
    {
      x += v;
      y += v;
      return *this;
    }
    ScriptVec2& ScriptVec2::operator-=(const float& v)
    {
      x -= v;
      y -= v;
      return *this;
    }
    ScriptVec2& ScriptVec2::operator/=(const float& v)
    {
      x /= v;
      y /= v;
      return *this;
    }
    ScriptVec2& ScriptVec2::operator*=(const float& v)
    {
      x *= v;
      y *= v;
      return *this;
    }
    ScriptVec2 ScriptVec2::operator-() const
    {
      return ScriptVec2(-x, -y);
    }
    ScriptVec2 ScriptVec2::operator+(const float& v) const
    {
      return ScriptVec2(
        x + v,
        y + v
      );
    }
    ScriptVec2 ScriptVec2::operator-(const float& v) const
    {
      return ScriptVec2(
        x - v,
        y - v
      );
    }
    ScriptVec2 ScriptVec2::operator/(const float& v) const
    {
      return ScriptVec2(
        x / v,
        y / v
      );
    }
    ScriptVec2 ScriptVec2::operator*(const float& v) const
    {
      return ScriptVec2(
        x * v,
        y * v
      );
    }
    ScriptVec2 ScriptVec2::operator+(const ScriptVec2& other) const
    {
      return ScriptVec2(
        x + other.x,
        y + other.y
      );
    }
    ScriptVec2 ScriptVec2::operator-(const ScriptVec2& other) const
    {
      return ScriptVec2(
        x - other.x,
        y - other.y
      );
    }
    ScriptVec2 ScriptVec2::operator/(const ScriptVec2& other) const
    {
      return ScriptVec2(
        x / other.x,
        y / other.y
      );
    }
    ScriptVec2 ScriptVec2::operator*(const ScriptVec2& other) const
    {
      return ScriptVec2(
        x * other.x,
        y * other.y
      );
    }
    // Functions.
    int ScriptVec2::opCmp(const ScriptVec2& other) const
    {
      const float l1 = LengthSquared();
      const float l2 = other.LengthSquared();

      if (l1 < l2)
      {
        return -1;
      }
      else if (l1 > l2)
      {
        return 1;
      }
      else
      {
        return 0;
      }
    }
    float ScriptVec2::LengthSquared() const
    {
      return x * x + y * y;
    }
    float ScriptVec2::Length() const
    {
      return sqrtf(LengthSquared());
    }
    void ScriptVec2::Normalize()
    {
      const float l = Length();
      x /= l;
      y /= l;
    }
    ScriptVec2 ScriptVec2::Normalized() const
    {
      ScriptVec2 v(*this);
      v.Normalize();
      return v;
    }
    float ScriptVec2::Dot(const ScriptVec2& other) const
    {
      return x * other.x + y * other.y;
    }
    String ScriptVec2::ToString() const
    {
      return "X: " + toString(x) + " Y: " + toString(y);
    }
    std::string ScriptVec2::ToStringStd() const
    {
      return "X: " + std::to_string(x) + " Y: " + std::to_string(y);
    }












    // Constructors.
    ScriptVec3::ScriptVec3() :
      x(0.0f), y(0.0f), z(0.0f) {}
    ScriptVec3::ScriptVec3(const glm::vec3& v) :
      x(v.x), y(v.y), z(v.z)
    {
    }
    ScriptVec3::ScriptVec3(const ScriptVec3& v) :
      x(v.x), y(v.y), z(v.z) {}
    ScriptVec3::ScriptVec3(const float& v) :
      x(v), y(v), z(v) {}
    ScriptVec3::ScriptVec3(const float& v1, const float& v2, const float& v3) :
      x(v1), y(v2), z(v3) {}

    ScriptVec3::operator glm::vec3() const
    {
      return glm::vec3(x, y, z);
    }

    void ScriptVec3::operator=(const glm::vec3& other)
    {
      x = other.x;
      y = other.y;
      z = other.z;
    }

    // Operators.
    void ScriptVec3::operator=(const ScriptVec3& other)
    {
      x = other.x;
      y = other.y;
      z = other.z;
    }
    bool ScriptVec3::operator==(const ScriptVec3& other) const
    {
      return x == other.x&& x == other.y&& z == other.z;
    }
    bool ScriptVec3::operator!=(const ScriptVec3& other) const
    {
      return !(*this == other);
    }
    ScriptVec3& ScriptVec3::operator+=(const ScriptVec3& other)
    {
      x += other.x;
      y += other.y;
      z += other.z;
      return *this;
    }
    ScriptVec3& ScriptVec3::operator-=(const ScriptVec3& other)
    {
      x -= other.x;
      y -= other.y;
      z -= other.z;
      return *this;
    }
    ScriptVec3& ScriptVec3::operator/=(const ScriptVec3& other)
    {
      x /= other.x;
      y /= other.y;
      z /= other.z;
      return *this;
    }
    ScriptVec3& ScriptVec3::operator*=(const ScriptVec3& other)
    {
      x *= other.x;
      y *= other.y;
      z *= other.z;
      return *this;
    }
    ScriptVec3& ScriptVec3::operator+=(const float& v)
    {
      x += v;
      y += v;
      z += v;
      return *this;
    }
    ScriptVec3& ScriptVec3::operator-=(const float& v)
    {
      x -= v;
      y -= v;
      z -= v;
      return *this;
    }
    ScriptVec3& ScriptVec3::operator/=(const float& v)
    {
      x /= v;
      y /= v;
      z /= v;
      return *this;
    }
    ScriptVec3& ScriptVec3::operator*=(const float& v)
    {
      x *= v;
      y *= v;
      z *= v;
      return *this;
    }
    ScriptVec3 ScriptVec3::operator-() const
    {
      return ScriptVec3(-x, -y, -z);
    }
    ScriptVec3 ScriptVec3::operator+(const float& v) const
    {
      return ScriptVec3(
        x + v,
        y + v,
        z + v
      );
    }
    ScriptVec3 ScriptVec3::operator-(const float& v) const
    {
      return ScriptVec3(
        x - v,
        y - v,
        z - v
      );
    }
    ScriptVec3 ScriptVec3::operator/(const float& v) const
    {
      return ScriptVec3(
        x / v,
        y / v,
        z / v
      );
    }
    ScriptVec3 ScriptVec3::operator*(const float& v) const
    {
      return ScriptVec3(
        x * v,
        y * v,
        z * v
      );
    }
    ScriptVec3 ScriptVec3::operator+(const ScriptVec3& other) const
    {
      return ScriptVec3(
        x + other.x,
        y + other.y,
        z + other.z
      );
    }
    ScriptVec3 ScriptVec3::operator-(const ScriptVec3& other) const
    {
      return ScriptVec3(
        x - other.x,
        y - other.y,
        z - other.z
      );
    }
    ScriptVec3 ScriptVec3::operator/(const ScriptVec3& other) const
    {
      return ScriptVec3(
        x / other.x,
        y / other.y,
        z / other.z
      );
    }
    ScriptVec3 ScriptVec3::operator*(const ScriptVec3& other) const
    {
      return ScriptVec3(
        x * other.x,
        y * other.y,
        z * other.z
      );
    }
    // Functions.
    int ScriptVec3::opCmp(const ScriptVec3& other) const
    {
      const float l1 = LengthSquared();
      const float l2 = other.LengthSquared();

      if (l1 < l2)
      {
        return -1;
      }
      else if (l1 > l2)
      {
        return 1;
      }
      else
      {
        return 0;
      }
    }
    float ScriptVec3::LengthSquared() const
    {
      return x * x + y * y + z * z;
    }
    float ScriptVec3::Length() const
    {
      return sqrtf(LengthSquared());
    }
    void ScriptVec3::Normalize()
    {
      const float l = Length();
      x /= l;
      y /= l;
      z /= l;
    }
    ScriptVec3 ScriptVec3::Normalized() const
    {
      ScriptVec3 v(*this);
      v.Normalize();
      return v;
    }
    float ScriptVec3::Dot(const ScriptVec3& other) const
    {
      return x * other.x + y * other.y + z * other.z;
    }
    ScriptVec3 ScriptVec3::Cross(const ScriptVec3& other) const
    {
      return ScriptVec3(
        y * other.z - z * other.y, 
        z * other.x - x * other.z, 
        x * other.y - y * other.x
      );
    }
    String ScriptVec3::ToString() const
    {
      return "X: " + toString(x) + " Y: " + toString(y) + " Z: " + toString(z);
    }
    std::string ScriptVec3::ToStringStd() const
    {
      return "X: " + std::to_string(x) + " Y: " + std::to_string(y) + " Z: " + std::to_string(z);
    }









    // Constructors.
    ScriptVec4::ScriptVec4() :
      x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
    ScriptVec4::ScriptVec4(const glm::vec4& v) :
      x(v.x), y(v.y), z(v.z), w(v.w)
    {
    }
    ScriptVec4::ScriptVec4(const ScriptVec4& v) :
      x(v.x), y(v.y), z(v.z), w(v.w) {}
    ScriptVec4::ScriptVec4(const float& v) :
      x(v), y(v), z(v), w(v) {}
    ScriptVec4::ScriptVec4(const float& v1, const float& v2, const float& v3, const float& v4) :
      x(v1), y(v2), z(v3), w(v4) {}

    ScriptVec4::operator glm::vec4() const
    {
      return glm::vec4(x, y, z, w);
    }

    void ScriptVec4::operator=(const glm::vec4& other)
    {
      x = other.x;
      y = other.y;
      z = other.z;
      w = other.w;
    }

    // Operators.
    void ScriptVec4::operator=(const ScriptVec4& other)
    {
      x = other.x;
      y = other.y;
      z = other.z;
      w = other.w;
    }
    bool ScriptVec4::operator==(const ScriptVec4& other) const
    {
      return x == other.x&& x == other.y&& z == other.z&& w == other.w;
    }
    bool ScriptVec4::operator!=(const ScriptVec4& other) const
    {
      return !(*this == other);
    }
    ScriptVec4& ScriptVec4::operator+=(const ScriptVec4& other)
    {
      x += other.x;
      y += other.y;
      z += other.z;
      w += other.w;
      return *this;
    }
    ScriptVec4& ScriptVec4::operator-=(const ScriptVec4& other)
    {
      x -= other.x;
      y -= other.y;
      z -= other.z;
      w *= other.w;
      return *this;
    }
    ScriptVec4& ScriptVec4::operator/=(const ScriptVec4& other)
    {
      x /= other.x;
      y /= other.y;
      z /= other.z;
      w *= other.w;
      return *this;
    }
    ScriptVec4& ScriptVec4::operator*=(const ScriptVec4& other)
    {
      x *= other.x;
      y *= other.y;
      z *= other.z;
      w *= other.w;
      return *this;
    }
    ScriptVec4& ScriptVec4::operator+=(const float& v)
    {
      x += v;
      y += v;
      z += v;
      w += v;
      return *this;
    }
    ScriptVec4& ScriptVec4::operator-=(const float& v)
    {
      x -= v;
      y -= v;
      z -= v;
      w -= v;
      return *this;
    }
    ScriptVec4& ScriptVec4::operator/=(const float& v)
    {
      x /= v;
      y /= v;
      z /= v;
      w /= v;
      return *this;
    }
    ScriptVec4& ScriptVec4::operator*=(const float& v)
    {
      x *= v;
      y *= v;
      z *= v;
      w *= v;
      return *this;
    }
    ScriptVec4 ScriptVec4::operator-() const
    {
      return ScriptVec4(-x, -y, -z, -w);
    }
    ScriptVec4 ScriptVec4::operator+(const float& v) const
    {
      return ScriptVec4(
        x + v,
        y + v,
        z + v,
        w + v
      );
    }
    ScriptVec4 ScriptVec4::operator-(const float& v) const
    {
      return ScriptVec4(
        x - v,
        y - v,
        z - v,
        w - v
      );
    }
    ScriptVec4 ScriptVec4::operator/(const float& v) const
    {
      return ScriptVec4(
        x / v,
        y / v,
        z / v,
        w / v
      );
    }
    ScriptVec4 ScriptVec4::operator*(const float& v) const
    {
      return ScriptVec4(
        x * v,
        y * v,
        z * v,
        w * v
      );
    }
    ScriptVec4 ScriptVec4::operator+(const ScriptVec4& other) const
    {
      return ScriptVec4(
        x + other.x,
        y + other.y,
        z + other.z,
        w + other.w
      );
    }
    ScriptVec4 ScriptVec4::operator-(const ScriptVec4& other) const
    {
      return ScriptVec4(
        x - other.x,
        y - other.y,
        z - other.z,
        w - other.w
      );
    }
    ScriptVec4 ScriptVec4::operator/(const ScriptVec4& other) const
    {
      return ScriptVec4(
        x / other.x,
        y / other.y,
        z / other.z,
        w / other.w
      );
    }
    ScriptVec4 ScriptVec4::operator*(const ScriptVec4& other) const
    {
      return ScriptVec4(
        x * other.x,
        y * other.y,
        z * other.z,
        w * other.w
      );
    }
    // Functions.
    int ScriptVec4::opCmp(const ScriptVec4& other) const
    {
      const float l1 = LengthSquared();
      const float l2 = other.LengthSquared();

      if (l1 < l2)
      {
        return -1;
      }
      else if (l1 > l2)
      {
        return 1;
      }
      else
      {
        return 0;
      }
    }
    float ScriptVec4::LengthSquared() const
    {
      return x * x + y * y + z * z + w * w;
    }
    float ScriptVec4::Length() const
    {
      return sqrtf(LengthSquared());
    }
    void ScriptVec4::Normalize()
    {
      const float l = Length();
      x /= l;
      y /= l;
      z /= l;
      w /= l;
    }
    ScriptVec4 ScriptVec4::Normalized() const
    {
      ScriptVec4 v(*this);
      v.Normalize();
      return v;
    }
    float ScriptVec4::Dot(const ScriptVec4& other) const
    {
      return x * other.x + y * other.y + z * other.z + w * other.w;
    }
    ScriptVec3 ScriptVec4::Cross(const ScriptVec4& other) const
    {
      return ScriptVec3(x / w, y / w, z / w).Cross(ScriptVec3(other.x / other.w, other.y / other.w, other.z / other.w));
    }
    String ScriptVec4::ToString() const
    {
      return "X: " + toString(x) + " Y: " + toString(y) + " Z: " + toString(z) + " W: " + toString(w);
    }
    std::string ScriptVec4::ToStringStd() const
    {
      return "X: " + std::to_string(x) + " Y: " + std::to_string(y) + " Z: " + std::to_string(z) + " W: " + std::to_string(w);
    }










    glm::quat toGlmQuat(const ScriptQuat& q)
    {
      return glm::quat(q.w, q.x, q.y, q.z);
    }
    ScriptQuat toScriptQuat(const glm::quat& q)
    {
      return ScriptQuat(q.w, q.x, q.y, q.z);
    }
    void toGlmQuat(const ScriptQuat& lhs, glm::quat& rhs)
    {
      rhs.x = lhs.x;
      rhs.y = lhs.y;
      rhs.z = lhs.z;
      rhs.w = lhs.w;
    }
    void toScriptQuat(const glm::quat& lhs, ScriptQuat& rhs)
    {
      rhs.x = lhs.x;
      rhs.y = lhs.y;
      rhs.z = lhs.z;
      rhs.w = lhs.w;
    }

    ScriptQuat::ScriptQuat() :
      x(0.0f), y(0.0f), z(0.0f), w(0.0f)
    {
    }
    ScriptQuat::ScriptQuat(const glm::quat& v) :
      x(v.x), y(v.y), z(v.z), w(v.w)
    {
    }
    ScriptQuat::ScriptQuat(const ScriptQuat& v) :
      x(v.x), y(v.y), z(v.z), w(v.w)
    {
    }
    ScriptQuat::ScriptQuat(const ScriptVec3& euler)
    {
      glm::quat q(glm::vec3(euler.x, euler.y, euler.z));
      x = q.x;
      y = q.y;
      z = q.z;
      w = q.w;
    }
    ScriptQuat::ScriptQuat(const float& x, const float& y, const float& z) :
      x(x), y(y), z(z), w(1.0f)
    {
    }
    ScriptQuat::ScriptQuat(const float& w, const float& x, const float& y, const float& z) :
      x(x), y(y), z(z), w(w)
    {
    }
    ScriptQuat::operator glm::quat() const
    {
      return glm::quat(w, x, y, z);
    }
    void ScriptQuat::operator=(const glm::quat& other)
    {
      x = other.x;
      y = other.y;
      z = other.z;
      w = other.w;
    }
    void ScriptQuat::operator=(const ScriptQuat& other)
    {
      x = other.x;
      y = other.y;
      z = other.z;
      w = other.w;
    }
    bool ScriptQuat::operator==(const ScriptQuat& other) const
    {
      return x == other.x && y == other.y && z == other.z && w == other.w;
    }
    bool ScriptQuat::operator!=(const ScriptQuat& other) const
    {
      return !(*this == other);
    }
    ScriptQuat& ScriptQuat::operator+=(const ScriptQuat& other)
    {
      toScriptQuat(toGlmQuat(*this) + toGlmQuat(other), *this);
      return *this;
    }
    ScriptQuat& ScriptQuat::operator-=(const ScriptQuat& other)
    {
      toScriptQuat(toGlmQuat(*this) - toGlmQuat(other), *this);
      return *this;
    }
    ScriptQuat& ScriptQuat::operator*=(const ScriptQuat& other)
    {
      toScriptQuat(toGlmQuat(*this) * toGlmQuat(other), *this);
      return *this;
    }
    ScriptQuat ScriptQuat::operator-() const
    {
      return toScriptQuat(-toGlmQuat(*this));
    }
    ScriptQuat ScriptQuat::operator+(const ScriptQuat& other) const
    {
      return toScriptQuat(toGlmQuat(*this) + toGlmQuat(other));
    }
    ScriptQuat ScriptQuat::operator-(const ScriptQuat& other) const
    {
      return toScriptQuat(toGlmQuat(*this) - toGlmQuat(other));
    }
    ScriptQuat ScriptQuat::operator*(const ScriptQuat& other) const
    {
      return toScriptQuat(toGlmQuat(*this) * toGlmQuat(other));
    }
    int ScriptQuat::opCmp(const ScriptQuat& other) const
    {
      return 0;
    }
    float ScriptQuat::LengthSquared() const
    {
      return x * x + y * y + z * z + w * w;
    }
    float ScriptQuat::Length() const
    {
      return std::sqrtf(LengthSquared());
    }
    void ScriptQuat::Normalize()
    {
      toScriptQuat(glm::normalize(toGlmQuat(*this)), *this);
    }
    ScriptQuat ScriptQuat::Normalized() const
    {
      return toScriptQuat(glm::normalize(toGlmQuat(*this)));
    }
    ScriptVec3 ScriptQuat::ToEuler() const
    {
      glm::vec3 euler_angles = glm::eulerAngles(toGlmQuat(*this));
      return ScriptVec3(euler_angles.x, euler_angles.y, euler_angles.z);
    }
    String ScriptQuat::ToString() const
    {
      return "X: " + toString(x) + " Y: " + toString(y) + " Z: " + toString(z) + " W: " + toString(w);
    }
    std::string ScriptQuat::ToStringStd() const
    {
      return "X: " + std::to_string(x) + " Y: " + std::to_string(y) + " Z: " + std::to_string(z) + " W: " + std::to_string(w);
    }
  }
}
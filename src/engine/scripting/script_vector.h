#pragma once
#include <containers/containers.h>
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/quaternion.hpp>

namespace lambda
{
  namespace scripting
  {
    ///////////////////////////////////////////////////////////////////////////
    struct ScriptVec2
    {
      float x;
      float y;

      // Constructors.
      ScriptVec2();
      ScriptVec2(const glm::vec2& v);
      ScriptVec2(const ScriptVec2& v);
      ScriptVec2(const float& v);
      ScriptVec2(const float& v1, const float& v2);

      // Operators.
      operator glm::vec2() const;
      void operator=(const glm::vec2& other);
      void operator=(const ScriptVec2& other);
      bool operator==(const ScriptVec2& other) const;
      bool operator!=(const ScriptVec2& other) const;
      ScriptVec2& operator+=(const ScriptVec2& other);
      ScriptVec2& operator-=(const ScriptVec2& other);
      ScriptVec2& operator/=(const ScriptVec2& other);
      ScriptVec2& operator*=(const ScriptVec2& other);
      ScriptVec2& operator+=(const float& v);
      ScriptVec2& operator-=(const float& v);
      ScriptVec2& operator/=(const float& v);
      ScriptVec2& operator*=(const float& v);
      ScriptVec2 operator-() const;
      ScriptVec2 operator+(const float& v) const;
      ScriptVec2 operator-(const float& v) const;
      ScriptVec2 operator/(const float& v) const;
      ScriptVec2 operator*(const float& v) const;
      ScriptVec2 operator+(const ScriptVec2& other) const;
      ScriptVec2 operator-(const ScriptVec2& other) const;
      ScriptVec2 operator/(const ScriptVec2& other) const;
      ScriptVec2 operator*(const ScriptVec2& other) const;
      // Functions.
      int opCmp(const ScriptVec2& other) const;
      float LengthSquared() const;
      float Length() const;
      void Normalize();
      ScriptVec2 Normalized() const;
      float Dot(const ScriptVec2& other) const;
      String ToString() const;
      std::string ToStringStd() const;
    };
   
    ///////////////////////////////////////////////////////////////////////////
    struct ScriptVec3
    {
      float x;
      float y;
      float z;

      // Constructors.
      ScriptVec3();
      ScriptVec3(const glm::vec3& v);
      ScriptVec3(const ScriptVec3& v);
      ScriptVec3(const float& v);
      ScriptVec3(const float& v1, const float& v2, const float& v3);

      // Operators.
      operator glm::vec3() const;
      void operator=(const glm::vec3& other);
      void operator=(const ScriptVec3& other);
      bool operator==(const ScriptVec3& other) const;
      bool operator!=(const ScriptVec3& other) const;
      ScriptVec3& operator+=(const ScriptVec3& other);
      ScriptVec3& operator-=(const ScriptVec3& other);
      ScriptVec3& operator/=(const ScriptVec3& other);
      ScriptVec3& operator*=(const ScriptVec3& other);
      ScriptVec3& operator+=(const float& v);
      ScriptVec3& operator-=(const float& v);
      ScriptVec3& operator/=(const float& v);
      ScriptVec3& operator*=(const float& v);
      ScriptVec3 operator-() const;
      ScriptVec3 operator+(const float& v) const;
      ScriptVec3 operator-(const float& v) const;
      ScriptVec3 operator/(const float& v) const;
      ScriptVec3 operator*(const float& v) const;
      ScriptVec3 operator+(const ScriptVec3& other) const;
      ScriptVec3 operator-(const ScriptVec3& other) const;
      ScriptVec3 operator/(const ScriptVec3& other) const;
      ScriptVec3 operator*(const ScriptVec3& other) const;
      // Functions.
      int opCmp(const ScriptVec3& other) const;
      float LengthSquared() const;
      float Length() const;
      void Normalize();
      ScriptVec3 Normalized() const;
      float Dot(const ScriptVec3& other) const;
      ScriptVec3 Cross(const ScriptVec3& other) const;
      String ToString() const;
      std::string ToStringStd() const;
    };

    ///////////////////////////////////////////////////////////////////////////
    struct ScriptVec4
    {
      float x;
      float y;
      float z;
      float w;

      // Constructors.
      ScriptVec4();
      ScriptVec4(const glm::vec4& v);
      ScriptVec4(const ScriptVec4& v);
      ScriptVec4(const float& v);
      ScriptVec4(
        const float& v1, 
        const float& v2, 
        const float& v3, 
        const float& v4
      );

      // Operators.
      operator glm::vec4() const;
      void operator=(const glm::vec4& other);
      void operator=(const ScriptVec4& other);
      bool operator==(const ScriptVec4& other) const;
      bool operator!=(const ScriptVec4& other) const;
      ScriptVec4& operator+=(const ScriptVec4& other);
      ScriptVec4& operator-=(const ScriptVec4& other);
      ScriptVec4& operator/=(const ScriptVec4& other);
      ScriptVec4& operator*=(const ScriptVec4& other);
      ScriptVec4& operator+=(const float& v);
      ScriptVec4& operator-=(const float& v);
      ScriptVec4& operator/=(const float& v);
      ScriptVec4& operator*=(const float& v);
      ScriptVec4 operator-() const;
      ScriptVec4 operator+(const float& v) const;
      ScriptVec4 operator-(const float& v) const;
      ScriptVec4 operator/(const float& v) const;
      ScriptVec4 operator*(const float& v) const;
      ScriptVec4 operator+(const ScriptVec4& other) const;
      ScriptVec4 operator-(const ScriptVec4& other) const;
      ScriptVec4 operator/(const ScriptVec4& other) const;
      ScriptVec4 operator*(const ScriptVec4& other) const;
      // Functions.
      int opCmp(const ScriptVec4& other) const;
      float LengthSquared() const;
      float Length() const;
      void Normalize();
      ScriptVec4 Normalized() const;
      float Dot(const ScriptVec4& other) const;
      ScriptVec3 Cross(const ScriptVec4& other) const;
      String ToString() const;
      std::string ToStringStd() const;
    };

    ///////////////////////////////////////////////////////////////////////////
    struct ScriptQuat
    {
      float x;
      float y;
      float z;
      float w;

      // Constructors.
      ScriptQuat();
      ScriptQuat(const glm::quat& v);
      ScriptQuat(const ScriptQuat& v);
      ScriptQuat(const ScriptVec3& euler);
      ScriptQuat(const float& x, const float& y, const float& z);
      ScriptQuat(
        const float& w, 
        const float& x, 
        const float& y, 
        const float& z
      );

      // Operators.
      operator glm::quat() const;
      void operator=(const glm::quat& other);
      void operator=(const ScriptQuat& other);
      bool operator==(const ScriptQuat& other) const;
      bool operator!=(const ScriptQuat& other) const;
      ScriptQuat& operator+=(const ScriptQuat& other);
      ScriptQuat& operator-=(const ScriptQuat& other);
      ScriptQuat& operator*=(const ScriptQuat& other);
      ScriptQuat operator-() const;
      ScriptQuat operator+(const ScriptQuat& other) const;
      ScriptQuat operator-(const ScriptQuat& other) const;
      ScriptQuat operator*(const ScriptQuat& other) const;
      // Functions.
      int opCmp(const ScriptQuat& other) const;
      float LengthSquared() const;
      float Length() const;
      void Normalize();
      ScriptQuat Normalized() const;
      ScriptVec3 ToEuler() const;
      String ToString() const;
      std::string ToStringStd() const;
    };
  }
}
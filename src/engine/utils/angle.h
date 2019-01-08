#pragma once

namespace lambda
{
  namespace utilities
  {
    class Angle
    {
    public:
      Angle(const float& rad = 0.0f) : rad_(rad) {}

      float asDeg() const { return asRad() * radToDeg; }
      float asRad() const { return rad_; }

      void setDeg(const float& deg) { setRad(deg * degToRad); }
      void setRad(const float& rad) { rad_ = rad; }

      static Angle fromDeg(const float& deg) { return Angle(deg * degToRad); }
      static Angle fromRad(const float& rad) { return Angle(rad); }

      static constexpr float pi = 3.14159265f;
      static constexpr float tau = 6.28318531f;
      static constexpr float degToRad = tau / 360.0f;
      static constexpr float radToDeg = 360.0f / tau;

    private:
      float rad_;
    };
  }
}
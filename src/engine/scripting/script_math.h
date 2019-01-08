#include <limits>

namespace lambda
{
  namespace scripting
  {
    ///////////////////////////////////////////////////////////////////////////
    class ScriptMath
    {
    public:
      float Cos(const float& v) const;
      float Sin(const float& v) const;
      float Tan(const float& v) const;
      float Acos(const float& v) const;
      float Asin(const float& v) const;
      float Atan(const float& v) const;
      float Atan2(const float& y, const float& x) const;
      float Cosh(const float& v) const;
      float Sinh(const float& v) const;
      float Tanh(const float& v) const;
      float Log(const float& v) const;
      float Log10(const float& v) const;
      float Pow(const float& v, const float& p) const;
      float Sqrt(const float& v) const;
      float Min(const float& lhs, const float& rhs) const;
      float Max(const float& lhs, const float& rhs) const;
      float Ceil(const float& v) const;
      float Floor(const float& v) const;
      float Abs(const float& v) const;
      float Trunc(const float& v) const;
      float Fraction(const float& v) const;

    public:
      const float pi_         = 3.14159265f;
      const float tau_        = 6.28318531f;
      const float deg_to_rad_ = tau_ / 360.0f;
      const float rad_to_deg_ = 360.0f / tau_;
      const float epsilon_    = std::numeric_limits<float>::epsilon();
      const float infinity_   = std::numeric_limits<float>::infinity();
      const float negative_infinity_ = 
        -1.0f * std::numeric_limits<float>::infinity();
    };
  }
}
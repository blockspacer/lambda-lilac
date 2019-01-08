#include "script_math.h"
#include <cmath>

namespace lambda
{
  namespace scripting
  {
    ///////////////////////////////////////////////////////////////////////////
    float ScriptMath::Cos(const float& v) const
    {
      return std::cosf(v);
    }

    ///////////////////////////////////////////////////////////////////////////
    float ScriptMath::Sin(const float& v) const
    {
      return std::sinf(v);
    }

    ///////////////////////////////////////////////////////////////////////////
    float ScriptMath::Tan(const float& v) const
    {
      return std::tanf(v);
    }

    ///////////////////////////////////////////////////////////////////////////
    float ScriptMath::Acos(const float& v) const
    {
      return std::acosf(v);
    }

    ///////////////////////////////////////////////////////////////////////////
    float ScriptMath::Asin(const float& v) const
    {
      return std::asinf(v);
    }

    ///////////////////////////////////////////////////////////////////////////
    float ScriptMath::Atan(const float& v) const
    {
      return std::atanf(v);
    }

    ///////////////////////////////////////////////////////////////////////////
    float ScriptMath::Atan2(const float& y, const float& x) const
    {
      return std::atan2f(y, x);
    }

    ///////////////////////////////////////////////////////////////////////////
    float ScriptMath::Cosh(const float& v) const
    {
      return std::coshf(v);
    }

    ///////////////////////////////////////////////////////////////////////////
    float ScriptMath::Sinh(const float& v) const
    {
      return std::sinhf(v);
    }

    ///////////////////////////////////////////////////////////////////////////
    float ScriptMath::Tanh(const float& v) const
    {
      return std::tanhf(v);
    }

    ///////////////////////////////////////////////////////////////////////////
    float ScriptMath::Log(const float& v) const
    {
      return std::logf(v);
    }

    ///////////////////////////////////////////////////////////////////////////
    float ScriptMath::Log10(const float& v) const
    {
      return std::log10f(v);
    }

    ///////////////////////////////////////////////////////////////////////////
    float ScriptMath::Pow(const float& v, const float& p) const
    {
      return std::powf(v, p);
    }

    ///////////////////////////////////////////////////////////////////////////
    float ScriptMath::Sqrt(const float& v) const
    {
      return std::sqrtf(v);
    }

    ///////////////////////////////////////////////////////////////////////////
    float ScriptMath::Min(const float& lhs, const float& rhs) const
    {
      return std::fminf(lhs, rhs);
    }

    ///////////////////////////////////////////////////////////////////////////
    float ScriptMath::Max(const float& lhs, const float& rhs) const
    {
      return std::fmaxf(lhs, rhs);
    }

    ///////////////////////////////////////////////////////////////////////////
    float ScriptMath::Ceil(const float& v) const
    {
      return std::ceilf(v);
    }

    ///////////////////////////////////////////////////////////////////////////
    float ScriptMath::Floor(const float& v) const
    {
      return std::floorf(v);
    }

    ///////////////////////////////////////////////////////////////////////////
    float ScriptMath::Abs(const float& v) const
    {
      return std::fabsf(v);
    }

    ///////////////////////////////////////////////////////////////////////////
    float ScriptMath::Trunc(const float& v) const
    {
      return std::truncf(v);
    }

    ///////////////////////////////////////////////////////////////////////////
    float ScriptMath::Fraction(const float& v) const
    {
      float int_part;
      return std::modff(v, &int_part);
    }
  }
}

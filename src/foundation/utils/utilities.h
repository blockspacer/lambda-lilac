#pragma once
#include <random>
#include <containers/containers.h>

namespace lambda
{
  namespace utilities
  {
    // todo (Hilze): Make cc file.
    inline float random()
    {
      static std::random_device rd;  // Will be used to obtain a seed for the random number engine
      static std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
      static std::uniform_real_distribution<> dis(0.0f, 1.0f);
      return (float)dis(gen);
    }
    inline float randomRange(const float& min, const float& max)
    {
      return random() * (max - min) + min;
    }
    template<typename T, typename C>
    Vector<C> convertVec(const Vector<T>& vt)
    {
      uint32_t size = (uint32_t)((vt.size() * sizeof(T) + (sizeof(C) - 1u)) / sizeof(C));
      Vector<C> vc(size, '\0');
      memcpy(vc.data(), vt.data(), vt.size() * sizeof(T));
      return eastl::move(vc);
    }
  }
}
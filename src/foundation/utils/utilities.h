#pragma once
#include <random>
#include <containers/containers.h>

namespace lambda
{
  namespace utilities
  {
	static std::random_device k_rd;  // Will be used to obtain a seed for the random number engine
	static std::mt19937 k_gen(k_rd()); // Standard mersenne_twister_engine seeded with rd()
	static std::uniform_real_distribution<> k_dis(0.0f, 1.0f);

    inline float random()
    {
      return (float)k_dis(k_gen);
    }
    inline float randomRange(const float& min, const float& max)
    {
      return random() * (max - min) + min;
    }
	inline void setRandomSeed(const uint32_t& seed)
	{
		k_gen.seed(seed);
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
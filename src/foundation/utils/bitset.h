#pragma once
#include <cstdio>

namespace lambda
{
  namespace utilities
  {
    class BitSet
    {
    public:
      bool get(const uint32_t& flag) const
      {
        return (flags_ & (1u << flag)) != 0u;
      }
      void set(const uint32_t& flag)
      {
        flags_ |= (1u << flag);
      }
      void clear(const uint32_t& flag)
      {
        flags_ &= ~(1u << flag);
      }
      void toggle(const uint32_t& flag)
      {
        flags_ ^= (1u << flag);
      }

    private:
      uint32_t flags_ = 0u;
    };

  }
}
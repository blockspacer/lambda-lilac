#pragma once
#include <stddef.h>

namespace lambda
{
  namespace foundation
  {
    inline void* offsetBytes(const void* adr, intptr_t offset)
    {
      return (void*)(((size_t)adr) + offset);
    }
    inline size_t alignUp(const void* adr, size_t align)
    {
      return (((size_t)(adr)+(size_t)(align - 1)) & (~(align - 1)));
    }
    inline size_t alignUpDelta(const void* adr, size_t align)
    {
      size_t delta = align - (((size_t)(adr)) & ((size_t)(align - 1)));
      if (delta == align)
      {
        return 0;
      }
      return delta;
    }
    inline size_t alignUpDeltaWithOffset(const void* adr, size_t allign, size_t offset)
    {
      size_t adjustment = alignUpDelta(adr, allign);
      size_t extra_space = offset;

      if (adjustment < extra_space)
      {
        extra_space -= adjustment;

        adjustment += allign * (extra_space / allign);

        if (extra_space % allign > 0)
        {
          adjustment += allign;
        }
      }

      return adjustment;
    }
  }
}

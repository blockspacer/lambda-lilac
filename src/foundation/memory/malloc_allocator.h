#pragma once
#include "iallocator.h"

namespace lambda
{
  namespace foundation
  {
    class MallocAllocator : public IAllocator
    {
    public:
      MallocAllocator(size_t max_size);

    protected:
      struct AllocationHeader
      {
        size_t size; //!< The size of the allocation
      };
      
      void* AllocateImpl(size_t size, size_t align) override;
      size_t DeallocateImpl(void* ptr) override;
      size_t GetSize(void* ptr);
    };
  }
}
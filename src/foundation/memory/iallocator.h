#pragma once
#include <cinttypes>
#include <cstddef>
#include <atomic>

#define VIOLET_DEBUG_MEMORY 1
#define VIOLET_OPEN_ALLOCATIONS 0
#define VIOLET_BUFFER_OVERFLOW 0

namespace lambda
{
  namespace foundation
  {

    class Memory;

    class IAllocator
    {
      friend class Memory;

    public:
      IAllocator(size_t max_size);
      IAllocator(const IAllocator& other) = delete;
      IAllocator(const IAllocator&& other) = delete;
      size_t open_allocations() const;
      virtual size_t allocated() const;
      virtual ~IAllocator();

    protected:
			size_t Deallocate(void* ptr);
			virtual size_t DeallocateImpl(void* ptr) = 0;
			void* Allocate(size_t size, size_t align);
			virtual void* AllocateImpl(size_t size, size_t align) = 0;

    private:
      const size_t max_size_;
    protected:
      std::atomic<size_t> open_allocations_;
      std::atomic<size_t> allocated_;
    };
  }
}
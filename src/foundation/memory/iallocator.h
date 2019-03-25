#pragma once
#include <cinttypes>
#include <cstddef>
#include <atomic>

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
      void* Allocate(size_t size, size_t align);
      size_t Deallocate(void* ptr);
      virtual void* AllocateImpl(size_t size, size_t align) = 0;
      virtual size_t DeallocateImpl(void* ptr) = 0;

    private:
      const size_t max_size_;
    protected:
      std::atomic<size_t> open_allocations_;
      std::atomic<size_t> allocated_;
    };
  }
}
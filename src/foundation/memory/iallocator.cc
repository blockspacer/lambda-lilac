#include "iallocator.h"
#include "utils/console.h"

#define LMB_OPEN_ALLOCATIONS
#define LMB_BUFFER_OVERFLOW

namespace lambda
{
  namespace foundation
  {
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    IAllocator::IAllocator(size_t max_size) :
      max_size_(max_size),
      open_allocations_(0),
      allocated_(0)
    {
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    size_t IAllocator::open_allocations() const
    {
      return open_allocations_;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    size_t IAllocator::allocated() const
    {
      return allocated_;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    IAllocator::~IAllocator()
    {
#ifdef LMB_OPEN_ALLOCATIONS
      assert(open_allocations_ == 0 && allocated_ == 0);
#endif
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma optimize("", off)
    void* IAllocator::Allocate(size_t size, size_t align)
    {
#ifdef LMB_BUFFER_OVERFLOW
      // TODO (Hilze): Fix this. Very serious issue!
      if (allocated_ + size > max_size_)
      {
        LMB_ASSERT(false, "Buffer overflow in allocator");
        return nullptr;
      }
#endif

	  if (allocated_ + size < allocated_)
		  int xxxx = 0;

      allocated_ += size;
      ++open_allocations_;

      return AllocateImpl(size, align);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    size_t IAllocator::Deallocate(void* ptr)
    {
      size_t deallocated = DeallocateImpl(ptr);

      if (max_size_ < deallocated)
      {
        LMB_ASSERT(false, "Attempted to deallocate more than was ever allocated");
        return 0;
      }

      allocated_ -= deallocated;
      --open_allocations_;

      return deallocated;
    }
  }
}
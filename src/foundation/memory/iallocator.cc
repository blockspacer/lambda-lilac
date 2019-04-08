#include "iallocator.h"
#include "utils/console.h"

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
#ifdef VIOLET_OPEN_ALLOCATIONS
      assert(open_allocations_ == 0 && allocated_ == 0);
#endif
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void* IAllocator::Allocate(size_t size, size_t align)
	{
#ifdef VIOLET_BUFFER_OVERFLOW
		// TODO (Hilze): Fix this. Very serious issue!
		if (allocated_ + size > max_size_)
		{
			LMB_ASSERT(false, "Buffer overflow in allocator");
			return nullptr;
		}
#endif

		void* ptr = AllocateImpl(size, align);

		allocated_ += size;
		++open_allocations_;
		
		return ptr;
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
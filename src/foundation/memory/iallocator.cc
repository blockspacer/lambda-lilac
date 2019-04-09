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

#if VIOLET_DEBUG_MEMORY
		Header* new_headers_ = (Header*)malloc(sizeof(Header) * (header_count_ + 1));
		memcpy(new_headers_, headers_, sizeof(Header) * header_count_);
		free(headers_);
		headers_ = new_headers_;
		headers_[header_count_] = Header{ captureCallStack(2), ptr, size, align };
		header_count_++;
#endif

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

	  if (deallocated <= allocated_)
		  allocated_ -= deallocated;
	  else
		  allocated_ = 0u;

      --open_allocations_;

#if VIOLET_DEBUG_MEMORY
	  for (uint32_t i = 0; i < header_count_; ++i)
	  {
		  if (headers_[i].ptr == ptr)
		  {
			  if (strcmp(headers_[i].src, "") != 0)
				  free((void*)headers_[i].src);
			  Header* new_headers_ = (Header*)malloc(sizeof(Header) * (header_count_ - 1));
			  memcpy(new_headers_, headers_, sizeof(Header) * i);
			  memcpy(new_headers_ + i, headers_ + i + 1, sizeof(Header) * (header_count_ - i - 1));
			  free(headers_);
			  headers_ = new_headers_;
			  header_count_--;
			  break;
		  }
	  }
#endif

      return deallocated;
    }
  }
}
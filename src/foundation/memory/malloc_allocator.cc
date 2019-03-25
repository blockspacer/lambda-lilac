#include "malloc_allocator.h"
#include "pointer_arithmetic.h"

#include <memory>
#include <cstring>

namespace lambda
{
  namespace foundation
  {
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    MallocAllocator::MallocAllocator(size_t max_size) :
      IAllocator(max_size)
    {
    }

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	MallocAllocator::~MallocAllocator()
	{
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	size_t MallocAllocator::allocated() const
	{
		return allocated_;
	}

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void* MallocAllocator::AllocateImpl(size_t size, size_t align)
    {
      static constexpr size_t kHeaderSize = sizeof(AllocationHeader);

      size_t total_size = size + kHeaderSize;
      void* base_addr = _aligned_malloc(total_size, align);

	  allocated_ += total_size;

      AllocationHeader header;
      header.size = size;

      memcpy(base_addr, &header, kHeaderSize);

      return offsetBytes(base_addr, kHeaderSize);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    size_t MallocAllocator::DeallocateImpl(void* ptr)
    {
      static constexpr intptr_t kHeaderSize = static_cast<intptr_t>(sizeof(AllocationHeader));

      AllocationHeader* header = reinterpret_cast<AllocationHeader*>(offsetBytes(ptr, -kHeaderSize));

      size_t size = header->size;

	  allocated_ -= size + kHeaderSize;

      _aligned_free(header);

      return size;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    size_t MallocAllocator::GetSize(void* ptr)
    {
      if (ptr == nullptr)
        return 0;

      intptr_t header_size = static_cast<intptr_t>(sizeof(AllocationHeader));

      AllocationHeader* header = reinterpret_cast<AllocationHeader*>(offsetBytes(ptr, -header_size));

      size_t size = header->size;

      return size;
    }
  }
}
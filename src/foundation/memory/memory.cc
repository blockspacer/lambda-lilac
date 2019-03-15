#include "memory.h"
#include "pointer_arithmetic.h"
#include "utils/console.h"

namespace lambda
{
  namespace foundation
  {
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    const size_t Memory::kDefaultHeapSize_ = 1024ul * 1024ul * 1024ul * 2ul;
    const size_t Memory::kDefaultAlignment_ = 16ul;

    Memory::DefaultAllocator Memory::default_allocator_(kDefaultHeapSize_);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void* Memory::allocate(size_t size, size_t align, IAllocator* allocator)
    {
      LMB_ASSERT(allocator != nullptr, "Attempted to allocate memory with a null allocator");

      size_t header_size = sizeof(AllocationHeader);
      void* base = reinterpret_cast<AllocationHeader*>(allocator->Allocate(size + header_size + align - 1, align));

      void* ptr = offsetBytes(base, header_size);

      size_t a = alignUpDelta(ptr, align);
      ptr = offsetBytes(ptr, a);

      AllocationHeader* header = reinterpret_cast<AllocationHeader*>(offsetBytes(base, a));

      header->allocator = allocator;
      header->size = size;
      header->align = a;

      return ptr;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void* Memory::allocate(size_t size, IAllocator* allocator)
    {
      return allocate(size, kDefaultAlignment_, allocator);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void* Memory::reallocate(void* data, size_t size, IAllocator* allocator)
    {
      // Early out.
      if (size == 0u)
	  {
        if (data != nullptr)
          deallocate(data);
        return nullptr;
	  }

      void* new_data = allocate(size, allocator);

      if (data != nullptr)
      {
        AllocationHeader* header = reinterpret_cast<AllocationHeader*>(offsetBytes(data, -static_cast<int>(sizeof(AllocationHeader))));
        memcpy(new_data, data, (size < header->size) ? size : header->size);
        deallocate(data);
      }
      
      return new_data;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void Memory::deallocate(void* ptr)
    {
      if (ptr == nullptr)
        return;

      AllocationHeader* header = reinterpret_cast<AllocationHeader*>(offsetBytes(ptr, -static_cast<int>(sizeof(AllocationHeader))));
      IAllocator* alloc = header->allocator;

      alloc->Deallocate(offsetBytes(header, -static_cast<intptr_t>(header->align)
      ));
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    Memory::DefaultAllocator& Memory::default_allocator()
    {
      return default_allocator_;
    }
  }
}
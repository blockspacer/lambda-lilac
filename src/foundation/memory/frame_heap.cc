#include "frame_heap.h"
#include "memory.h"
#include <algorithm>

namespace lambda
{
  namespace foundation
  {
    FrameHeap* FrameHeap::s_frame_heap_ = nullptr;
    
    
    FrameHeap::FrameHeap() :
      current_frame_heap_(0u)
      , current_history_(0u)
      , allocated_(0u)
    {
			mutex_.lock();
			memset(frame_heaps_, 0, sizeof(frame_heaps_));
			memset(heap_sizes_, 0, sizeof(heap_sizes_));
			memset(size_history_, 0, sizeof(size_history_));
			mutex_.unlock();
		}
		FrameHeap::~FrameHeap()
		{
			mutex_.lock();
			for (const auto& frame_heap : frame_heaps_)
				foundation::Memory::deallocate(frame_heap);

			for (const auto& temp_allocs : temp_allocs_)
				for (const auto& alloc : temp_allocs)
					foundation::Memory::deallocate(alloc);
			mutex_.unlock();
		}
    void FrameHeap::update()
    {
			mutex_.lock();
			// Calculate the size of this alloc.
      size_history_[current_history_] = allocated_;
      
      // Calculate the largest size in history.
      uint32_t size = 0u;
      for (uint32_t i = 0u; i < kHistoryCount; ++i)
      {
        if (size_history_[i] > size)
        {
          size = size_history_[i];
        }
      }
      // Reset the state.
      allocated_ = 0u;
      current_frame_heap_ = (current_frame_heap_ + 1u) % kHeapCount;
      current_history_    = (current_history_    + 1u) % kHistoryCount;

      // Create the new alloc for the next frame.
      if (heap_sizes_[current_frame_heap_] != size)
      {
        if (frame_heaps_[current_frame_heap_] != nullptr)
					foundation::Memory::deallocate(frame_heaps_[current_frame_heap_]);
        heap_sizes_[current_frame_heap_] = size;
        frame_heaps_[current_frame_heap_] = foundation::Memory::allocate(size);
      }
      
      // Remove the previous temporary allocs.
      for (const auto& temp_alloc : temp_allocs_[current_frame_heap_])
				foundation::Memory::deallocate(temp_alloc);
      temp_allocs_[current_frame_heap_].resize(0u);
			mutex_.unlock();
    }
    void* FrameHeap::alloc(uint32_t size)
    {
			mutex_.lock();
			if (allocated_ + size <= heap_sizes_[current_frame_heap_])
      {
        void* data = (char*)frame_heaps_[current_frame_heap_] + allocated_;
        allocated_ += size;
				mutex_.unlock();
				memset(data, 0, size);
				return data;
      }
      else
      {
				allocated_ += size;
        temp_allocs_[current_frame_heap_].push_back(foundation::Memory::allocate(size));
        void* data = temp_allocs_[current_frame_heap_].back();
				mutex_.unlock();
				memset(data, 0, size);
				return data;
			}
    }
		void* FrameHeap::realloc(void* prev, uint32_t prev_size, uint32_t new_size)
		{
			if (!new_size)
				return nullptr;

			void* mem = alloc(new_size);
			memcpy(mem, prev, std::min(prev_size, new_size));
			return mem;
		}
    uint32_t FrameHeap::currentHeapSize()
    {
			mutex_.lock();
			uint32_t heap_size = heap_sizes_[current_frame_heap_];
			mutex_.unlock();

			return heap_size;
    }
    FrameHeap* GetFrameHeap()
    {
      if (FrameHeap::s_frame_heap_ == nullptr)
        FrameHeap::s_frame_heap_ = foundation::Memory::construct<FrameHeap>();
      return FrameHeap::s_frame_heap_;
    }
  }
}
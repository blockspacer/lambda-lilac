#include "frame_heap.h"
#include "memory.h"

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
      for (uint32_t i = 0u; i < kHeapCount; ++i)
      {
        frame_heaps_[i] = nullptr;
        heap_sizes_[i]  = 0u;
      }
      for (uint32_t i = 0u; i < kHistoryCount; ++i)
        size_history_[i] = 0u;
    }
		FrameHeap::~FrameHeap()
		{
			for (const auto& frame_heap : frame_heaps_)
				foundation::Memory::deallocate(frame_heap);

			for (const auto& temp_allocs : temp_allocs_)
				for (const auto& alloc : temp_allocs)
					foundation::Memory::deallocate(alloc);
		}
    void FrameHeap::update()
    {
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

    }
    void* FrameHeap::alloc(uint32_t size)
    {
      if (allocated_ + size <= heap_sizes_[current_frame_heap_])
      {
        void* data = (char*)frame_heaps_[current_frame_heap_] + allocated_;
        allocated_ += size;
        return data;
      }
      else
      {
        allocated_ += size;
        temp_allocs_[current_frame_heap_].push_back(foundation::Memory::allocate(size));
        return temp_allocs_[current_frame_heap_].back();
      }
    }
    uint32_t FrameHeap::currentHeapSize() const
    {
      return heap_sizes_[current_frame_heap_];
    }
    FrameHeap* GetFrameHeap()
    {
      if (FrameHeap::s_frame_heap_ == nullptr)
        FrameHeap::s_frame_heap_ = foundation::Memory::construct<FrameHeap>();
      return FrameHeap::s_frame_heap_;
    }
  }
}
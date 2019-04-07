#include <containers/containers.h>

namespace lambda
{
  namespace foundation
  {
    class FrameHeap
    {
    public:
      friend FrameHeap* GetFrameHeap();
      static constexpr uint32_t kHeapCount    = 3u;
      static constexpr uint32_t kHistoryCount = 10u;

			FrameHeap();
			~FrameHeap();
      void update();
      void* alloc(uint32_t size);
      template<typename T>
      inline T* construct()
      {
        T* allocated = reinterpret_cast<T*>(alloc(sizeof(T)));
        new (allocated) T();
        return allocated;
      }
      template<typename T, typename ...Args>
      inline T* construct(Args&& ...args)
      {
        T* allocated = reinterpret_cast<T*>(alloc(sizeof(T)));
        new (allocated) T(eastl::forward<Args>(args)...);
        return allocated;
      }

      uint32_t currentHeapSize() const;

    private:
      uint32_t current_frame_heap_;
      uint32_t current_history_;

      uint32_t allocated_;
      void*    frame_heaps_[kHeapCount];
      uint32_t heap_sizes_[kHeapCount];
      uint32_t size_history_[kHistoryCount];
      Vector<void*> temp_allocs_[kHeapCount];
      
    protected:
      static FrameHeap* s_frame_heap_;
    };

    extern FrameHeap* GetFrameHeap();
  }
}
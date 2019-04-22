#include <containers/containers.h>
#include <mutex>

namespace lambda
{
  namespace foundation
  {
    class FrameHeap
    {
    public:
      friend FrameHeap* GetFrameHeap();
      static constexpr uint32_t kHeapCount    = 5u;
      static constexpr uint32_t kHistoryCount = 10u;

			FrameHeap();
			~FrameHeap();
      void update();
      void* alloc(uint32_t size);
			void* realloc(void* prev, uint32_t prev_size, uint32_t new_size);
			template<typename T>
			inline void deconstruct(T* t)
			{
				t->~T();
			}
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

      uint32_t currentHeapSize();

    private:
      uint32_t current_frame_heap_;
      uint32_t current_history_;

      uint32_t allocated_;
      void*    frame_heaps_[kHeapCount];
      uint32_t heap_sizes_[kHeapCount];
      uint32_t size_history_[kHistoryCount];
      Vector<void*> temp_allocs_[kHeapCount];
			std::mutex mutex_;

    protected:
      static FrameHeap* s_frame_heap_;
    };

    extern FrameHeap* GetFrameHeap();
  }
}
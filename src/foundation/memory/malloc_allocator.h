#pragma once
#include "iallocator.h"
#include <unordered_map>

namespace lambda
{
	namespace foundation
	{
		class MallocAllocator : public IAllocator
		{
		public:
			MallocAllocator(size_t max_size);
			virtual ~MallocAllocator();
			virtual size_t allocated() const override;

		protected:
			struct AllocationHeader
			{
				size_t size; //!< The size of the allocation
			};

			virtual void* AllocateImpl(size_t size, size_t align) override;
			size_t DeallocateImpl(void* ptr) override;
			size_t GetSize(void* ptr);

		private:
#if VIOLET_DEBUG_MEMORY
			struct Header
			{
				const char* src;
				void* ptr;
				size_t size;
				size_t align;
			};
			Header* headers_;
			size_t header_count_;
#endif
			std::atomic<size_t> allocated_;
		};
	}
}
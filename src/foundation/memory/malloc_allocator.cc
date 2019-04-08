#include "malloc_allocator.h"
#include "pointer_arithmetic.h"
#include "utils/console.h"

#include <memory>
#include <cstring>

namespace lambda
{
	namespace foundation
	{
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		MallocAllocator::MallocAllocator(size_t max_size) 
			: IAllocator(max_size)
#if VIOLET_DEBUG_MEMORY
			, header_count_(0ul)
			, headers_(nullptr)
#endif
		{
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		MallocAllocator::~MallocAllocator()
		{
#if VIOLET_DEBUG_MEMORY
			for (uint32_t i = 0; i < header_count_; ++i)
				std::printf("==========CALLSTACK==========\n%s", headers_[i].src);
#endif

 			LMB_ASSERT(allocated_ == 0ull, "Not all allocations were released");
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		size_t MallocAllocator::allocated() const
		{
			return allocated_;
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		void* MallocAllocator::AllocateImpl(size_t& size, size_t align)
		{
			static constexpr size_t kHeaderSize = sizeof(AllocationHeader);

			size += kHeaderSize;
			void* base_addr = _aligned_malloc(size, align);

			allocated_ += size;

			AllocationHeader header;
			header.size = size;

			memcpy(base_addr, &header, kHeaderSize);

#if VIOLET_DEBUG_MEMORY
			Header* new_headers_ = (Header*)malloc(sizeof(Header) * (header_count_ + 1));
			memcpy(new_headers_, headers_, sizeof(Header) * header_count_);
			free(headers_);
			headers_ = new_headers_;
			headers_[header_count_] = Header{ captureCallStack(2), base_addr, size, align };
			header_count_++;
#endif

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

#if VIOLET_DEBUG_MEMORY
			for (uint32_t i = 0; i < header_count_; ++i)
			{
				if (headers_[i].ptr == header)
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

			return size + kHeaderSize;
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
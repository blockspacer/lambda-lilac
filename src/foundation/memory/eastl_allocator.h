#pragma once
#include <EASTL/allocator.h>

namespace lambda
{
  namespace foundation
  {
    class EASTLAllocator
    {
    public:
      EASTLAllocator(const char* pName = EASTL_NAME_VAL("EASTLAllocator"));
      EASTLAllocator(const eastl::allocator& x, const char* pName = EASTL_NAME_VAL("EASTLAllocator"));
      ~EASTLAllocator();
      EASTLAllocator& operator=(const EASTLAllocator& x);
      bool operator==(const EASTLAllocator& x);
      bool operator!=(const EASTLAllocator& x);
      void* allocate(size_t n, int flags = 0);
      void* allocate(size_t n, size_t alignment, size_t offset, int flags = 0);
      void deallocate(void* p, size_t n);
      const char* get_name() const;
      void set_name(const char* pName);

    protected:

#if EASTL_NAME_ENABLED
      const char* mpName;
#endif
    };
  }
}
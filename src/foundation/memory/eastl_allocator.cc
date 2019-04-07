#include "eastl_allocator.h"
#include "memory.h"

namespace lambda
{
  namespace foundation
  {
    ////////////////////////////////////////////////////////////////////////////
#if EASTL_NAME_ENABLED
    EASTLAllocator::EASTLAllocator(const char* pName)
      : mpName(pName ? pName : EASTL_ALLOCATOR_DEFAULT_NAME)
#else
    EASTLAllocator::EASTLAllocator(const char*)
#endif
    {
    }

    ////////////////////////////////////////////////////////////////////////////
#if EASTL_NAME_ENABLED
    EASTLAllocator::EASTLAllocator(const eastl::allocator&, const char* pName)
      : mpName(pName ? pName : EASTL_ALLOCATOR_DEFAULT_NAME)
#else
    EASTLAllocator::EASTLAllocator(const eastl::allocator&, const char*)
#endif
    {
    }

    ////////////////////////////////////////////////////////////////////////////
    EASTLAllocator::~EASTLAllocator()
    {
    }

    ////////////////////////////////////////////////////////////////////////////
    EASTLAllocator& EASTLAllocator::operator=(const EASTLAllocator& /*x*/)
    {
      return *this;
    }

    ////////////////////////////////////////////////////////////////////////////
    bool EASTLAllocator::operator==(const EASTLAllocator& /*x*/)
    {
      return true;
    }

    ////////////////////////////////////////////////////////////////////////////
    bool EASTLAllocator::operator!=(const EASTLAllocator& /*x*/)
    {
      return false;
    }

    ////////////////////////////////////////////////////////////////////////////
    void* EASTLAllocator::allocate(size_t n, int /*flags*/)
    {
      return foundation::Memory::allocate(n);
    }

    ////////////////////////////////////////////////////////////////////////////
    void* EASTLAllocator::allocate(size_t n,
                                   size_t alignment,
                                   size_t /*offset*/,
                                   int /*flags*/)
    {
      return foundation::Memory::allocate(n , alignment);
    }

    ////////////////////////////////////////////////////////////////////////////
    void EASTLAllocator::deallocate(void* p, size_t /*n*/)
    {
			foundation::Memory::deallocate(p);
    }

    ////////////////////////////////////////////////////////////////////////////
#if EASTL_NAME_ENABLED
    void EASTLAllocator::set_name(const char* pName)
    {
      mpName = pName;
    }
#else
    void EASTLAllocator::set_name(const char*)
    {
    }
#endif

    ////////////////////////////////////////////////////////////////////////////
    const char* EASTLAllocator::get_name() const
    {
#if EASTL_NAME_ENABLED
      return mpName;
#else
      return "Custom EASTL allocator";
#endif
    }
  }
}

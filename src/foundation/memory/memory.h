#pragma once
#include "malloc_allocator.h"
#include "eastl_allocator.h"

#include <EASTL/memory.h>

#include <EASTL/shared_ptr.h>
#include <EASTL/unique_ptr.h>

namespace lambda
{
  namespace foundation
  {
    template <typename T>
    struct MemoryDeleter
    {
      MemoryDeleter();
      
      template <typename U>
      using is_convertible = typename eastl::enable_if<eastl::is_convertible<U*, T*>::value>::type;

      template <typename U>
      MemoryDeleter(const MemoryDeleter<U>& other, is_convertible<U>* test = nullptr);

      void operator()(T* ptr);
    };

    template <typename T>
    using UniquePointer = eastl::unique_ptr<T, MemoryDeleter<T>>;
    template <typename T>
    using UniquePtr = UniquePointer<T>;

    template <typename T>
    using SharedPointer = eastl::shared_ptr<T>;
    template <typename T>
    using SharedPtr = SharedPointer<T>;

    class Memory
    {
    public:
      using DefaultAllocator = MallocAllocator;

			static void* allocate(size_t size, size_t align, IAllocator* allocator = default_allocator());
			static void* allocate(size_t size, IAllocator* allocator = default_allocator());
      static void* reallocate(void* data, size_t size, IAllocator* allocator = default_allocator());
      static void deallocate(void* ptr);
      template <typename T, typename ... Args>
      static T* construct(Args&&... args);
      template <typename T, typename ... Args>
      static T* construct(IAllocator* allocator, Args&&... args);
      template <typename T>
      static void destruct(T* ptr);
      template <typename T>
      static SharedPointer<T> makeShared(T* ptr);
      template <typename T>
      static UniquePointer<T> makeUnique(T* ptr);
      template <typename T, typename ... Args>
      static SharedPointer<T> constructShared(Args&&... args);
      template <typename T, typename ... Args>
      static SharedPointer<T> constructShared(IAllocator* alloc, Args&&... args);
      template <typename T, typename ... Args>
      static UniquePointer<T> constructUnique(Args&&... args);
      template <typename T, typename ... Args>
      static UniquePointer<T> constructUnique(IAllocator* alloc, Args&&... args);
	  static DefaultAllocator* default_allocator();
	  static DefaultAllocator* new_allocator();

    protected:
      struct AllocationHeader
      {
        IAllocator* allocator; 
        size_t align; 
        size_t size; 
      };

    private:
      static const size_t kDefaultHeapSize_; 
      static const size_t kDefaultAlignment_; 
    };

    //--------------------------------------------------------------------------
    template <typename T>
    MemoryDeleter<T>::MemoryDeleter()
    {

    }

    //--------------------------------------------------------------------------
    template <typename T> template <typename U>
    inline MemoryDeleter<T>::MemoryDeleter(
      const MemoryDeleter<U>&,
      is_convertible<U>*)
    {

    }

    //--------------------------------------------------------------------------
    template <typename T>
    inline void MemoryDeleter<T>::operator()(T* ptr)
    {
      Memory::destruct(ptr);
    }

    template<typename T, typename ...Args>
    inline T* Memory::construct(Args&& ...args)
    {
			T* allocated = reinterpret_cast<T*>(allocate(sizeof(T)));
      new (allocated) T(eastl::forward<Args>(args)...);

      return allocated;
    }

    //--------------------------------------------------------------------------
    template <typename T, typename ... Args>
    inline T* Memory::construct(IAllocator* allocator, Args&&... args)
    {
			T* allocated = reinterpret_cast<T*>(allocate(sizeof(T), allocator));
      new (allocated) T(eastl::forward<Args>(args)...);

      return allocated;
    }

    //--------------------------------------------------------------------------
    template <typename T>
    inline void Memory::destruct(T* ptr)
    {
      if (ptr == nullptr)
      {
        return;
      }

      ptr->~T();
      deallocate(ptr);
    }

    //--------------------------------------------------------------------------
    template <typename T>
    inline SharedPointer<T> Memory::makeShared(T* ptr)
    {
      return SharedPointer<T>(ptr, MemoryDeleter<T>(), EASTLAllocator());
    }

    //--------------------------------------------------------------------------
    template <typename T>
    inline UniquePointer<T> Memory::makeUnique(T* ptr)
    {
      return UniquePointer<T>(ptr, MemoryDeleter<T>());
    }

    template<typename T, typename ...Args>
    inline SharedPointer<T> Memory::constructShared(Args&& ...args)
    {
      return makeShared<T>(construct<T, Args...>(eastl::forward<Args>(args)...));
    }

    //--------------------------------------------------------------------------
    template <typename T, typename ... Args>
    inline SharedPointer<T> Memory::constructShared(IAllocator* alloc, Args&&... args)
    {
      return makeShared<T>(construct<T, Args...>(alloc, eastl::forward<Args>(args)...));
    }

    template<typename T, typename ...Args>
    inline UniquePointer<T> Memory::constructUnique(Args&& ...args)
    {
      return makeUnique<T>(construct<T, Args...>(eastl::forward<Args>(args)...));
    }

    //--------------------------------------------------------------------------
    template <typename T, typename ... Args>
    inline UniquePointer<T> Memory::constructUnique(IAllocator* alloc, Args&&... args)
    {
      return makeUnique<T>(construct<T, Args...>(alloc, eastl::forward<Args>(args)...));
    }
  }
}
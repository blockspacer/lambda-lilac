#pragma once
#include <type_traits>
#include <unordered_map>
#include <memory/memory.h>
#include "utils/name.h"

namespace lambda
{
  namespace asset
  {
    class AssetManager;
  }
}

namespace lambda
{
  namespace asset
  {
    class IGPUAsset
    {
    public:
      virtual ~IGPUAsset() {};
    };

    class IAsset
    {
    public:
      virtual ~IAsset() {};
      foundation::SharedPointer<IGPUAsset>& gpuData()
      {
        return gpu_data_;
      }
      foundation::SharedPointer<IGPUAsset>& operator->()
      {
        return gpu_data_;
      }
      operator bool() const
      {
        return gpu_data_ != nullptr;
      }

    private:
      foundation::SharedPointer<IGPUAsset> gpu_data_;
    };
    
    void AssetHandleDestroyAsset(AssetManager* asset_manager, size_t asset);

    template <typename T>
    class AssetHandle
    {
    protected:
      friend class AssetManager;

      template<typename V>
      friend class AssetHandle;

    public:
      T* ptr = nullptr;
      AssetManager* manager = nullptr;
      size_t id;
      int* count = nullptr;	//shared. shallow copied

      inline void dec_count() {
        if (count != nullptr) (*count)--;
      }

      inline void inc_count() {
        if (count != nullptr) (*count)++;
      }

    protected:
      AssetHandle(T* _ptr, AssetManager* _manager, size_t _id)
      {
        ptr = _ptr;
        manager = _manager;
        id = _id;
        count = foundation::Memory::construct<int>(1);
      }

    public:
      ~AssetHandle()
      {
        //cout << "In ~SharedPointer" << endl;
        if (count != nullptr) {
          dec_count();
          if (*count <= 0) {
            AssetHandleDestroyAsset(manager, id);
            //foundation::Memory::destruct(ptr);
            foundation::Memory::destruct(count);
            count = nullptr;
          }
        }
      }

      AssetHandle() {
        ptr = nullptr;
        manager = nullptr;
        count = nullptr;
      }

      AssetHandle(const AssetHandle<T>& other) {
        ptr = other.ptr;
        manager = other.manager;
        id = other.id;
        count = other.count;
        inc_count();
      }
      template<typename V>
      AssetHandle(const AssetHandle<V>& other) {
        reset();
        ptr = (T*)other.ptr;
        manager = other.manager;
        id = other.id;
        count = other.count;
        inc_count();
      }
      AssetHandle<T>& operator=(const AssetHandle<T>& other) {
        //TODO: only if count != nullptr?
        reset();
        ptr = other.ptr;
        manager = other.manager;
        id = other.id;
        count = other.count;
        inc_count();
        return *this;
      }

      template<typename V>
      AssetHandle<T>& operator=(const AssetHandle<V>& other) {
        //TODO: only if count != nullptr?
        reset();
        ptr = (T*)other.ptr;
        manager = other.manager;
        count = other.count;
        id = other.id;
        inc_count();
        return *this;
      }

      int use_count() const {
        if (count != nullptr)
          return *count;
        else
          return 0;
      }

      T* get() {
        return ptr;
      }
      const T* get() const {
        return ptr;
      }
      T& data() {
        return *get();
      }
      const T& data() const {
        return *get();
      }

      void reset()
      {
        if (count != nullptr) {
          dec_count();
          if (*count <= 0) {
            AssetHandleDestroyAsset(manager, id);
            //foundation::Memory::destruct(ptr);
            foundation::Memory::destruct(count);
          }
          count = nullptr;
          ptr = nullptr;
          manager = nullptr;
          id = 0;
        }
      }

      T* operator*() {
        //cout << "In operator *" << endl;
        return ptr;
      }

      T* operator ->() {
        return ptr;
      }
      const T* operator ->() const {
        return ptr;
      }
      
      bool operator!() const
      {
        return ptr == nullptr;
      }
      operator bool() const
      {
        return ptr != nullptr;
      }
      template<typename V>
      bool operator==(const AssetHandle<V>& other) const
      {
        return ptr == other.ptr;
      }
      template<typename V>
      bool operator!=(const AssetHandle<V>& other) const
      {
        return ptr != other.ptr;
      }
      bool operator==(const T* other) const
      {
        return ptr == other;
      }
      bool operator!=(const T* other) const
      {
        return ptr != other;
      }
      bool operator==(const void* other) const
      {
        return ptr == other;
      }
      bool operator!=(const void* other) const
      {
        return ptr != other;
      }
      bool operator==(const std::nullptr_t& other) const
      {
        return ptr == other;
      }
      bool operator!=(const std::nullptr_t& other) const
      {
        return ptr != other;
      }
    };
  }
}
#pragma once
#include "utils/name.h"

namespace lambda
{
  namespace asset
  {
    enum class VioletFlags : uint32_t
    {
      kGPUOnly = 1u,
    };
    
    template<typename T>
    class VioletHandle
    {
    public:
      VioletHandle()
        : hash_(0u)
        , data_(nullptr)
      {
      }
      VioletHandle(T* data, Name name)
        : hash_(name.getHash())
        , data_(data)
      {
      }
      VioletHandle(const VioletHandle& other)
        : hash_(other.hash_)
        , data_(other.data_)
      {
      }
      void operator=(const VioletHandle<T>& other)
      {
        data_ = other.data_;
        hash_ = other.hash_;
      }
      void operator=(const std::nullptr_t& /*null*/)
      {
        data_ = nullptr;
      }
      bool operator==(const VioletHandle<T>& other) const
      {
        return hash_ == other.hash_;
      }
      bool operator==(const std::nullptr_t& /*null*/) const
      {
        return data_ == nullptr;
      }
      bool operator!=(const VioletHandle<T>& other) const
      {
        return hash_ != other.hash_;
      }
      bool operator!=(const std::nullptr_t& /*null*/) const
      {
        return data_ != nullptr;
      }
      bool operator!() const
      {
        return data_ == nullptr;
      }
      operator bool() const
      {
        return data_ != nullptr;
      }
      T* operator*()
      {
        return data_;
      }

      T* operator->()
      {
        return data_;
      }
      const T* operator->() const
      {
        return data_;
      }
      const T* get() const
      {
        return data_;
      }
      T* get()
      {
        return data_;
      }
      size_t getHash() const
      {
        return hash_;
      }

    private:
      size_t hash_;
      T* data_;
    };
  }
}

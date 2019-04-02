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

	class VioletRefHandler
	{
	public:
		static void incRef(const size_t& hash)
		{
			if (hash == 0u)
				return;

			g_refs[hash]++;
		}
		static bool decRef(const size_t& hash)
		{
			if (hash == 0u)
				return true;

			return --g_refs[hash] > 0;
		}

	private:
		static UnorderedMap<size_t, int> g_refs;
	};
    
    template<typename T>
    class VioletHandle
    {
    public:
      VioletHandle()
        : name_()
        , data_(nullptr)
      {
      }
      VioletHandle(T* data, Name name)
        : name_(name)
        , data_(data)
      {
		  VioletRefHandler::incRef(name_.getHash());
	  }
      VioletHandle(const VioletHandle& other)
        : name_(other.name_)
        , data_(other.data_)
      {
		  VioletRefHandler::incRef(name_.getHash());
	  }
	  ~VioletHandle()
	  {
		release();
	  }
      void operator=(const VioletHandle<T>& other)
      {
		if (name_ == other.name_)
		  return;
		
		release();

        data_ = other.data_;
		name_ = other.name_;

		VioletRefHandler::incRef(name_.getHash());
      }
      void operator=(const std::nullptr_t& /*null*/)
      {
        release();
        data_ = nullptr;
		name_ = 0u;
      }
      bool operator==(const VioletHandle<T>& other) const
      {
        return name_ == other.name_;
      }
      bool operator==(const std::nullptr_t& /*null*/) const
      {
        return data_ == nullptr;
      }
      bool operator!=(const VioletHandle<T>& other) const
      {
        return name_ != other.name_;
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
        return name_.getHash();
      }
	  Name getName() const
	  {
		  return name_;
	  }

	  void release()
	  {
        if (!VioletRefHandler::decRef(name_.getHash()))
		{
		  T::release(data_, name_.getHash());
		  data_ = nullptr;
		  name_ = 0u;

		}
	  }

    private:
      Name name_;
      T* data_;
    };
  }
}

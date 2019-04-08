#pragma once
#include "utils/name.h"
#include <utils/console.h>

namespace lambda
{
	namespace asset
	{
		enum class VioletFlags : uint32_t
		{
			kGPUOnly = 1u,
		};

		template<typename T>
		class VioletRefHandler
		{
		public:
			static void incRef(const size_t& hash)
			{
				LMB_ASSERT(g_valid, "AssetHandle not valid anymore");

				if (hash == 0u)
					return;

				g_refs[hash]++;
			}
			static bool decRef(const size_t& hash)
			{
				LMB_ASSERT(g_valid, "AssetHandle not valid anymore");

				if (hash == 0u)
					return true;

				const int ref = --g_refs[hash];

				if (ref <= 0)
				{
					g_refs.erase(hash);
					g_names.erase(hash);
				}

				return ref > 0;
			}
			static Name& getName(const size_t& hash)
			{
				LMB_ASSERT(g_valid, "AssetHandle not valid anymore");
				return g_names[hash];
			}
			static void releaseAll()
			{
				g_refs  = UnorderedMap<size_t, int>();
				g_names = UnorderedMap<size_t, Name>();
				g_valid = false;
			}

		private:
			static UnorderedMap<size_t, int> g_refs;
			static UnorderedMap<size_t, Name> g_names;
			static bool g_valid;
		};


		template<typename T>
		UnorderedMap<size_t, int> VioletRefHandler<T>::g_refs;
		template<typename T>
		UnorderedMap<size_t, Name> VioletRefHandler<T>::g_names;
		template<typename T>
		bool VioletRefHandler<T>::g_valid = true;


		template<typename T>
		class VioletHandle
		{
		public:
			VioletHandle()
				: data_(nullptr)
				, hash_(0ull)
			{
			}
			VioletHandle(T* data, Name name)
				: data_(data)
				, hash_(name.getHash())
			{
				if (hash_)
				{
					VioletRefHandler<T>::getName(hash_) = name;
					VioletRefHandler<T>::incRef(hash_);
				}
			}
			VioletHandle(const VioletHandle& other)
				: data_(other.data_)
				, hash_(other.hash_)
			{
				VioletRefHandler<T>::incRef(hash_);
			}
			~VioletHandle()
			{
				release();
			}
			void operator=(const VioletHandle<T>& other)
			{
				if (hash_ == other.hash_)
					return;

				release();

				data_ = other.data_;
				hash_ = other.hash_;

				VioletRefHandler<T>::incRef(hash_);
			}
			void operator=(const std::nullptr_t& /*null*/)
			{
				release();
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
			Name getName() const
			{
				return hash_ ? VioletRefHandler<T>::getName(hash_) : Name();
			}

			void release()
			{
				if (!VioletRefHandler<T>::decRef(hash_))
				{
					T::release(data_, hash_);
					data_ = nullptr;
					hash_ = 0ull;
				}
			}

		private:
			size_t hash_;
			T* data_;
		};
	}
}

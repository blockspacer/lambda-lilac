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
				return g_names[hash];
			}

		private:
			static UnorderedMap<size_t, int> g_refs;
			static UnorderedMap<size_t, Name> g_names;
		};

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
					VioletRefHandler::getName(hash_) = name;
					VioletRefHandler::incRef(hash_);
				}
			}
			VioletHandle(const VioletHandle& other)
				: data_(other.data_)
				, hash_(other.hash_)
			{
				VioletRefHandler::incRef(hash_);
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

				VioletRefHandler::incRef(hash_);
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
				return hash_ ? VioletRefHandler::getName(hash_) : Name();
			}

			void release()
			{
				if (!VioletRefHandler::decRef(hash_))
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

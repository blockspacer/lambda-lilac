#pragma once
#include <rapidjson/document.h>
#include <Meta.h>

namespace lambda
{
	namespace utilities
	{
		template <typename T>
		inline rapidjson::Value serialize(rapidjson::Document& doc, const T& t)
		{
			rapidjson::Value self(rapidjson::kObjectType);

			meta::doForAllMembers<T>([&](const auto& member)
			{
				self.AddMember(rapidjson::StringRef(member.getName()), serialize(doc, member.get(t)), doc.GetAllocator());
			});

			return self;
		}

		template <typename T>
		inline void deserialize(const rapidjson::Value& self, T& t)
		{
			meta::doForAllMembers<T>([&](const auto& member)
			{
				auto copy = member.get(t);
				deserialize(self[member.getName()], copy);
				member.set(t, copy);
			});
		}
	}
}
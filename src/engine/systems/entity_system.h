#pragma once
#include "entity.h"
#include "interfaces/isystem.h"
#include <containers/containers.h>

namespace lambda
{
	namespace scene
	{
		struct Scene;
	}

	namespace components
	{
		namespace EntitySystem
		{
			struct SystemData
			{
				static constexpr entity::Entity kFreeIdIncrement = 64ull;
				entity::Entity free_id_count = 1;
				Queue<entity::Entity> free_ids;

				entity::Entity create();
				void destroy(entity::Entity entity);
				bool valid(entity::Entity entity);
			};
		}
	}
}

#include "utils/serializer.h"

namespace meta
{
	template <>
	inline auto registerMembers<lambda::components::EntitySystem::SystemData>()
	{
		return members(
			member("free_id_count", &lambda::components::EntitySystem::SystemData::free_id_count),
			member("free_ids", &lambda::components::EntitySystem::SystemData::free_ids)
		);
	}
}

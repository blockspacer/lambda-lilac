#pragma once
#include "entity.h"
#include "interfaces/isystem.h"
#include <containers/containers.h>

namespace lambda
{
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
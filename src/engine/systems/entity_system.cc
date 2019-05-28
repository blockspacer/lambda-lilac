#include "entity_system.h"
#include <platform/scene.h>

namespace lambda
{
	namespace components
	{
		namespace EntitySystem
		{
			entity::Entity SystemData::create()
			{
				if (free_ids.empty() == true)
				{
					for (entity::Entity i = 0ull; i < kFreeIdIncrement; ++i)
						free_ids.push(free_id_count + i);

					free_id_count += kFreeIdIncrement;
				}

				entity::Entity id = free_ids.front();
				free_ids.pop();

				return id;
			}

			void SystemData::destroy(entity::Entity entity)
			{
				free_ids.push(entity);
			}

			bool SystemData::valid(entity::Entity entity)
			{
				return entity > 0ull;
			}
		}
	}
}
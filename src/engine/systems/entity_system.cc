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
			void serialize(scene::Scene& scene, scene::Serializer& serializer)
			{
				serializer.serialize("entity/free_id_count", toString(scene.entity.free_id_count));

				for (uint32_t i = 0; i < scene.entity.free_ids.size(); ++i)
					serializer.serialize("entity/free_ids/", toString(scene.entity.free_ids.get_container().at(i)));
			}
			void deserialize(scene::Scene& scene, scene::Serializer& serializer)
			{
				scene.entity.free_id_count = (entity::Entity)std::stoul(stlString(serializer.deserialize("entity/free_id_count")));
				while (!scene.entity.free_ids.empty())
					scene.entity.free_ids.pop();

				for (auto str : serializer.deserializeNamespace("entity/free_ids/"))
					scene.entity.free_ids.push((entity::Entity)std::stoul(stlString(str)));
			}
		}
	}
}
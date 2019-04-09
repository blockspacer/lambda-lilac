#pragma once
#include "interfaces/isystem.h"
#include "interfaces/icomponent.h"
#include "interfaces/iphysics.h"
#include <containers/containers.h>
#include <memory/memory.h>
#include "assets/mesh.h"
namespace lambda
{
	namespace world
	{
		struct SceneData;
	}
	namespace components
	{
		enum class ColliderType : uint8_t
		{
			kBox = 0,
			kSphere = 1,
			kCapsule = 2,
			kMesh = 3
		};

		class ColliderComponent : public IComponent
		{
		public:
			ColliderComponent(const entity::Entity& entity, world::SceneData& scene);
			ColliderComponent(const ColliderComponent& other);
			ColliderComponent();

			void makeBoxCollider();
			void makeSphereCollider();
			void makeCapsuleCollider();
			void makeMeshCollider(asset::VioletMeshHandle mesh, const uint32_t& sub_mesh_id);
			uint16_t getLayers() const;
			void setLayers(const uint16_t& layers);

		private:
			world::SceneData* scene_;
		};

		namespace ColliderSystem
		{
			struct Data
			{
				Data(const entity::Entity& entity) : entity(entity) {};
				Data(const Data& other);
				Data& operator=(const Data& other);

				ColliderType             type = ColliderType::kCapsule; // TODO (Hilze): Remove this
				physics::ICollisionBody* collision_body = nullptr;
				bool                     is_trigger = false;
				bool                     valid = true;

				entity::Entity entity;
			};

			struct SystemData
			{
				Vector<Data>                  data;
				Map<entity::Entity, uint32_t> entity_to_data;
				Map<uint32_t, entity::Entity> data_to_entity;
				Set<entity::Entity>           marked_for_delete;
				Queue<uint32_t>               unused_data_entries;

				Data& add(const entity::Entity& entity);
				Data& get(const entity::Entity& entity);
				void  remove(const entity::Entity& entity);
				bool  has(const entity::Entity& entity);
			};

			ColliderComponent addComponent(const entity::Entity& entity, world::SceneData& data);
			ColliderComponent getComponent(const entity::Entity& entity, world::SceneData& data);
			bool hasComponent(const entity::Entity& entity, world::SceneData& data);
			void removeComponent(const entity::Entity& entity, world::SceneData& data);

			void deinitialize(world::SceneData& data);
			void collectGarbage(world::SceneData& data);

			void makeBox(const entity::Entity& entity, world::SceneData& data);
			void makeSphere(const entity::Entity& entity, world::SceneData& data);
			void makeCapsule(const entity::Entity& entity, world::SceneData& data);
			void makeMeshCollider(const entity::Entity& entity, asset::VioletMeshHandle mesh, const uint32_t& sub_mesh_id, world::SceneData& data);
			uint16_t getLayers(const entity::Entity& entity, world::SceneData& data);
			void setLayers(const entity::Entity& entity, const uint16_t& layers, world::SceneData& data);
		}
	}
}
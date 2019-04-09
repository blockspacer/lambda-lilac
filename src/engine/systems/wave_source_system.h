#pragma once
#include "interfaces/isystem.h"
#include <containers/containers.h>
#include <memory/memory.h>
#include <glm/glm.hpp>
#include "assets/wave.h"

namespace SoLoud
{
  class Soloud;
}

namespace lambda
{
	namespace world
	{
		struct SceneData;
	}

	namespace components
	{
		enum class WaveSourceState : uint8_t
		{
			kInitial = 0,
			kPlaying = 1,
			kPaused = 2,
			kStopped = 3
		};

		class WaveSourceComponent : public IComponent
		{
		public:
			WaveSourceComponent(const entity::Entity& entity, world::SceneData& scene);
			WaveSourceComponent(const WaveSourceComponent& other);
			WaveSourceComponent();

			void setBuffer(const asset::VioletWaveHandle& buffer);
			asset::VioletWaveHandle getBuffer() const;
			void play();
			void pause();
			void stop();
			WaveSourceState getState() const;
			void setRelativeToListener(bool relative);
			bool getRelativeToListener() const;
			void setLoop(bool loop);
			bool getLoop() const;
			void setOffset(float seconds);
			void setVolume(float volume); // Range 0 to 100;
			float getVolume() const;
			void setGain(float gain); // Range 0 to 1
			float getGain() const;
			void setPitch(float pitch);
			float getPitch() const;
			void setRadius(float radius);
			float getRadius() const;

		private:
			world::SceneData* scene_;
		};

		namespace WaveSourceSystem
		{
			struct Data
			{
				Data(const entity::Entity& entity) : entity(entity) {}
				Data(const Data& other);
				Data& operator=(const Data& other);

				entity::Entity    entity;
				WaveSourceState   state;
				asset::VioletWaveHandle buffer;
				unsigned int handle = 0u;
				bool  in_world = false;
				bool  loop = false;
				float gain = 1.0f;
				float pitch = 1.0f;
				float radius = 100.0f;
				bool  valid = true;
				glm::vec3 last_position;
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

				entity::Entity listener;
				glm::vec3 last_listener_position;
				SoLoud::Soloud* engine;
			};


			WaveSourceComponent addComponent(const entity::Entity& entity, world::SceneData& scene);
			WaveSourceComponent getComponent(const entity::Entity& entity, world::SceneData& scene);
			bool hasComponent(const entity::Entity& entity, world::SceneData& scene);
			void removeComponent(const entity::Entity& entity, world::SceneData& scene);

			void initialize(world::SceneData& scene);
			void deinitialize(world::SceneData& scene);
			void update(const float& delta_time, world::SceneData& scene);
			void collectGarbage(world::SceneData& scene);

			void setBuffer(const entity::Entity& entity, const asset::VioletWaveHandle& buffer, world::SceneData& scene);
			asset::VioletWaveHandle getBuffer(const entity::Entity& entity, world::SceneData& scene);
			void play(const entity::Entity& entity, world::SceneData& scene);
			void pause(const entity::Entity& entity, world::SceneData& scene);
			void stop(const entity::Entity& entity, world::SceneData& scene);
			WaveSourceState getState(const entity::Entity& entity, world::SceneData& scene);
			void setRelativeToListener(const entity::Entity& entity, bool relative, world::SceneData& scene);
			bool getRelativeToListener(const entity::Entity& entity, world::SceneData& scene);
			void setLoop(const entity::Entity& entity, bool loop, world::SceneData& scene);
			bool getLoop(const entity::Entity& entity, world::SceneData& scene);
			void setOffset(const entity::Entity& entity, float seconds, world::SceneData& scene);
			void setVolume(const entity::Entity& entity, float volume, world::SceneData& scene); // Range 0 to 100;
			float getVolume(const entity::Entity& entity, world::SceneData& scene);
			void setGain(const entity::Entity& entity, float gain, world::SceneData& scene); // Range 0 to 1
			float getGain(const entity::Entity& entity, world::SceneData& scene);
			void setPitch(const entity::Entity& entity, float pitch, world::SceneData& scene);
			float getPitch(const entity::Entity& entity, world::SceneData& scene);
			void setRadius(const entity::Entity& entity, float radius, world::SceneData& scene);
			float getRadius(const entity::Entity& entity, world::SceneData& scene);

			void setListener(entity::Entity listener, world::SceneData& scene);
		}
	}
}
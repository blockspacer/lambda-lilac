#pragma once
#include "interfaces/icomponent.h"
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
	namespace scene
	{
		struct Scene;
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
			WaveSourceComponent(const entity::Entity& entity, scene::Scene& scene);
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
			scene::Scene* scene_;
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


			WaveSourceComponent addComponent(const entity::Entity& entity, scene::Scene& scene);
			WaveSourceComponent getComponent(const entity::Entity& entity, scene::Scene& scene);
			bool hasComponent(const entity::Entity& entity, scene::Scene& scene);
			void removeComponent(const entity::Entity& entity, scene::Scene& scene);

			void initialize(scene::Scene& scene);
			void deinitialize(scene::Scene& scene);
			void update(const float& delta_time, scene::Scene& scene);
			void collectGarbage(scene::Scene& scene);

			void setBuffer(const entity::Entity& entity, const asset::VioletWaveHandle& buffer, scene::Scene& scene);
			asset::VioletWaveHandle getBuffer(const entity::Entity& entity, scene::Scene& scene);
			void play(const entity::Entity& entity, scene::Scene& scene);
			void pause(const entity::Entity& entity, scene::Scene& scene);
			void stop(const entity::Entity& entity, scene::Scene& scene);
			WaveSourceState getState(const entity::Entity& entity, scene::Scene& scene);
			void setRelativeToListener(const entity::Entity& entity, bool relative, scene::Scene& scene);
			bool getRelativeToListener(const entity::Entity& entity, scene::Scene& scene);
			void setLoop(const entity::Entity& entity, bool loop, scene::Scene& scene);
			bool getLoop(const entity::Entity& entity, scene::Scene& scene);
			void setOffset(const entity::Entity& entity, float seconds, scene::Scene& scene);
			void setVolume(const entity::Entity& entity, float volume, scene::Scene& scene); // Range 0 to 100;
			float getVolume(const entity::Entity& entity, scene::Scene& scene);
			void setGain(const entity::Entity& entity, float gain, scene::Scene& scene); // Range 0 to 1
			float getGain(const entity::Entity& entity, scene::Scene& scene);
			void setPitch(const entity::Entity& entity, float pitch, scene::Scene& scene);
			float getPitch(const entity::Entity& entity, scene::Scene& scene);
			void setRadius(const entity::Entity& entity, float radius, scene::Scene& scene);
			float getRadius(const entity::Entity& entity, scene::Scene& scene);

			void setListener(entity::Entity listener, scene::Scene& scene);
		}
	}
}
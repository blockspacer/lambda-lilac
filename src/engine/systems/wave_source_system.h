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
	namespace components
	{
		class TransformSystem;
		class WaveSourceSystem;

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
			WaveSourceComponent(const entity::Entity& entity, WaveSourceSystem* system);
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
			WaveSourceSystem* system_;
		};

		struct WaveSourceData
		{
			WaveSourceData(const entity::Entity& entity) : entity(entity) {}
			WaveSourceData(const WaveSourceData& other);
			WaveSourceData& operator=(const WaveSourceData& other);

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

		class WaveSourceSystem : public ISystem
		{
		public:
			virtual void initialize(world::IWorld& world) override;
			virtual void deinitialize() override;
			virtual void update(const double& delta_time) override;
			virtual void collectGarbage() override;
			~WaveSourceSystem();

			static size_t systemId() { return (size_t)SystemIds::kWaveSourceSystem; };
			WaveSourceComponent addComponent(const entity::Entity& entity);
			WaveSourceComponent getComponent(const entity::Entity& entity);
			bool hasComponent(const entity::Entity& entity);
			void removeComponent(const entity::Entity& entity);

			void setBuffer(const entity::Entity& entity, const asset::VioletWaveHandle& buffer);
			asset::VioletWaveHandle getBuffer(const entity::Entity& entity) const;
			void play(const entity::Entity& entity);
			void pause(const entity::Entity& entity);
			void stop(const entity::Entity& entity);
			WaveSourceState getState(const entity::Entity& entity) const;
			void setRelativeToListener(const entity::Entity& entity, bool relative);
			bool getRelativeToListener(const entity::Entity& entity) const;
			void setLoop(const entity::Entity& entity, bool loop);
			bool getLoop(const entity::Entity& entity) const;
			void setOffset(const entity::Entity& entity, float seconds);
			void setVolume(const entity::Entity& entity, float volume); // Range 0 to 100;
			float getVolume(const entity::Entity& entity) const;
			void setGain(const entity::Entity& entity, float gain); // Range 0 to 1
			float getGain(const entity::Entity& entity) const;
			void setPitch(const entity::Entity& entity, float pitch);
			float getPitch(const entity::Entity& entity) const;
			void setRadius(const entity::Entity& entity, float radius);
			float getRadius(const entity::Entity& entity) const;

			void setListener(entity::Entity listener);

		private:
			WaveSourceData& lookUpData(const entity::Entity& entity);
			const WaveSourceData& lookUpData(const entity::Entity& entity) const;
			void updateState(WaveSourceData& data, float delta_time);

		private:
			Vector<WaveSourceData> data_;
			Map<entity::Entity, uint32_t> entity_to_data_;
			Map<uint32_t, entity::Entity> data_to_entity_;
			Set<entity::Entity> marked_for_delete_;
			Queue<uint32_t> unused_data_entries_;

			entity::Entity listener_;
			glm::vec3 last_listener_position_;
			SoLoud::Soloud* engine_;

			foundation::SharedPointer<TransformSystem> transform_system_;
		};
	}
}
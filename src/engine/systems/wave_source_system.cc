#include "wave_source_system.h"
#include "transform_system.h"
#include <soloud.h>
#include <soloud_thread.h>
#include <soloud_audiosource.h>
#include <soloud_wav.h>
#include <platform/scene.h>

namespace lambda
{
	namespace components
	{
		namespace WaveSourceSystem
		{
			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			void updateState(Data& data, float delta_time, scene::Scene& scene)
			{
				if (data.handle == 0u)
				{
					data.state = WaveSourceState::kStopped;
					return;
				}

				glm::vec3 position = data.in_world ? TransformSystem::getWorldTranslation(data.entity, scene) : scene.wave_source.last_listener_position;
				glm::vec3 velocity = data.in_world ? ((data.last_position - position) / delta_time) : glm::vec3(0.0f);
				data.last_position = position;

				scene.wave_source.engine->setLooping(data.handle, data.loop);
				scene.wave_source.engine->setVolume(data.handle, data.gain);
				scene.wave_source.engine->setRelativePlaySpeed(data.handle, data.pitch);
				scene.wave_source.engine->set3dSourceParameters(
					data.handle,
					position.x,
					position.y,
					position.z,
					velocity.x,
					velocity.y,
					velocity.z
				);
				scene.wave_source.engine->set3dSourceMinMaxDistance(data.handle, 1.0f, data.radius);
				scene.wave_source.engine->set3dSourceAttenuation(data.handle, SoLoud::AudioSource::LINEAR_DISTANCE, 1.0f);

				data.state = WaveSourceState::kInitial;
				if (scene.wave_source.engine->isValidVoiceHandle(data.handle))
				{
					if (scene.wave_source.engine->getPause(data.handle))
						data.state = WaveSourceState::kPaused;
					else
						data.state = WaveSourceState::kPlaying;
				}
				else
				{
					data.handle = 0u;
					data.state = WaveSourceState::kStopped;
				}
			}

			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			WaveSourceComponent addComponent(const entity::Entity& entity, scene::Scene& scene)
			{
				if (!TransformSystem::hasComponent(entity, scene))
					TransformSystem::addComponent(entity, scene);
				
				scene.wave_source.add(entity);

				return WaveSourceComponent(entity, scene);
			}

			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			WaveSourceComponent getComponent(const entity::Entity& entity, scene::Scene& scene)
			{
				return WaveSourceComponent(entity, scene);
			}

			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			bool hasComponent(const entity::Entity& entity, scene::Scene& scene)
			{
				return scene.wave_source.has(entity);
			}

			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			void removeComponent(const entity::Entity& entity, scene::Scene& scene)
			{
				stop(entity, scene);
				scene.wave_source.remove(entity);
			}
			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			void initialize(scene::Scene& scene)
			{
				scene.wave_source.engine = foundation::Memory::construct<SoLoud::Soloud>();
				scene.wave_source.engine->init();
			}

			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			void deinitialize(scene::Scene& scene)
			{
				Vector<entity::Entity> entities;
				for (const auto& it : scene.wave_source.entity_to_data)
					entities.push_back(it.first);

				for (const auto& entity : entities)
					removeComponent(entity, scene);
				collectGarbage(scene);

				for (const auto& data : scene.wave_source.data)
					if (scene.wave_source.engine->isValidVoiceHandle(data.handle))
						scene.wave_source.engine->stop(data.handle);
				scene.wave_source.data.clear();

				scene.wave_source.engine->stopAll();

				while (scene.wave_source.engine->getActiveVoiceCount() > 0)
					SoLoud::Thread::sleep(100);

				scene.wave_source.engine->deinit();
				foundation::Memory::destruct(scene.wave_source.engine);
			}

			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			void update(const float& delta_time, scene::Scene& scene)
			{
				// Listener.
				glm::vec3 listener_position(0.0f);
				glm::vec3 listener_up(0.0f, 1.0f, 0.0f);
				glm::vec3 listener_forward(1.0f, 0.0f, 0.0f);
				glm::vec3 listener_velocity(0.0f);
				if (scene.wave_source.listener != 0u)
				{
					listener_position = TransformSystem::getWorldTranslation(scene.wave_source.listener, scene);
					listener_forward = TransformSystem::getWorldForward(scene.wave_source.listener, scene);
					listener_up = TransformSystem::getWorldUp(scene.wave_source.listener, scene);
					listener_velocity = (scene.wave_source.last_listener_position - listener_position) / (float)delta_time;
					scene.wave_source.last_listener_position = listener_position;
				}
				scene.wave_source.engine->set3dListenerPosition(listener_position.x, listener_position.y, listener_position.z);
				scene.wave_source.engine->set3dListenerAt(listener_forward.x, listener_forward.y, listener_forward.z);
				scene.wave_source.engine->set3dListenerUp(listener_up.x, listener_up.y, listener_up.z);
				scene.wave_source.engine->set3dListenerVelocity(listener_velocity.x, listener_velocity.y, listener_velocity.z);

				// Audio sources
				for (Data& data : scene.wave_source.data)
				{
					if (data.entity == scene.wave_source.listener)
						continue;

					updateState(data, (float)delta_time, scene);
				}

				scene.wave_source.engine->update3dAudio();
			}

			void collectGarbage(scene::Scene& scene)
			{
				if (!scene.wave_source.marked_for_delete.empty())
				{
					for (entity::Entity entity : scene.wave_source.marked_for_delete)
					{
						const auto& it = scene.wave_source.entity_to_data.find(entity);
						if (it != scene.wave_source.entity_to_data.end())
						{
							uint32_t idx = it->second;
							scene.wave_source.unused_data_entries.push(idx);
							scene.wave_source.data_to_entity.erase(idx);
							scene.wave_source.entity_to_data.erase(entity);
							scene.wave_source.data[idx].valid = false;
						}
					}
					scene.wave_source.marked_for_delete.clear();
				}
			}
			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			void setBuffer(const entity::Entity& entity, const asset::VioletWaveHandle& buffer, scene::Scene& scene)
			{
				scene.wave_source.get(entity).buffer = buffer;
				updateState(scene.wave_source.get(entity), 0.0f, scene);
			}

			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			asset::VioletWaveHandle getBuffer(const entity::Entity& entity, scene::Scene& scene)
			{
				return scene.wave_source.get(entity).buffer;
			}

			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			void play(const entity::Entity& entity, scene::Scene& scene)
			{
				Data& data = scene.wave_source.get(entity);

				if (data.handle != 0u)
					scene.wave_source.engine->setPause(scene.wave_source.get(entity).handle, false);
				else
				{
					if (data.in_world)
						data.handle = scene.wave_source.engine->play3d(*data.buffer->getBuffer(), 0.0f, 0.0f, 0.0f);
					else
						data.handle = scene.wave_source.engine->play(*data.buffer->getBuffer());
				}

				updateState(data, 0.0f, scene);
			}

			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			void pause(const entity::Entity& entity, scene::Scene& scene)
			{
				scene.wave_source.engine->setPause(scene.wave_source.get(entity).handle, true);
			}

			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			void stop(const entity::Entity& entity, scene::Scene& scene)
			{
				Data& data = scene.wave_source.get(entity);
				scene.wave_source.engine->stop(data.handle);
				data.handle = 0u;
			}

			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			WaveSourceState getState(const entity::Entity& entity, scene::Scene& scene)
			{
				return scene.wave_source.get(entity).state;
			}

			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			void setRelativeToListener(const entity::Entity& entity, bool relative, scene::Scene& scene)
			{
				scene.wave_source.get(entity).in_world = relative;
				updateState(scene.wave_source.get(entity), 0.0f, scene);
			}

			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			bool getRelativeToListener(const entity::Entity& entity, scene::Scene& scene)
			{
				return scene.wave_source.get(entity).in_world;
			}

			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			void setLoop(const entity::Entity& entity, bool loop, scene::Scene& scene)
			{
				scene.wave_source.get(entity).loop = loop;
				updateState(scene.wave_source.get(entity), 0.0f, scene);
			}

			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			bool getLoop(const entity::Entity& entity, scene::Scene& scene)
			{
				return scene.wave_source.get(entity).loop;
			}

			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			void setOffset(const entity::Entity& entity, float seconds, scene::Scene& scene)
			{
				LMB_ASSERT(false, "");
			}

			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			void setVolume(const entity::Entity& entity, float volume, scene::Scene& scene)
			{
				setGain(entity, volume / 100.0f, scene);
				updateState(scene.wave_source.get(entity), 0.0f, scene);
			}

			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			float getVolume(const entity::Entity& entity, scene::Scene& scene)
			{
				return getGain(entity, scene) * 100.0f;
			}

			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			void setGain(const entity::Entity& entity, float gain, scene::Scene& scene)
			{
				scene.wave_source.get(entity).gain = gain;
				updateState(scene.wave_source.get(entity), 0.0f, scene);
			}

			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			float getGain(const entity::Entity& entity, scene::Scene& scene)
			{
				return scene.wave_source.get(entity).gain;
			}

			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			void setPitch(const entity::Entity& entity, float pitch, scene::Scene& scene)
			{
				scene.wave_source.get(entity).pitch = pitch;
				updateState(scene.wave_source.get(entity), 0.0f, scene);
			}

			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			float getPitch(const entity::Entity& entity, scene::Scene& scene)
			{
				return scene.wave_source.get(entity).pitch;
			}

			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			void setRadius(const entity::Entity& entity, float radius, scene::Scene& scene)
			{
				scene.wave_source.get(entity).radius = radius;
				updateState(scene.wave_source.get(entity), 0.0f, scene);
			}

			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			float getRadius(const entity::Entity& entity, scene::Scene& scene)
			{
				return scene.wave_source.get(entity).radius;
			}

			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			void setListener(entity::Entity listener, scene::Scene& scene)
			{
				scene.wave_source.listener = listener;
			}
		}




		// The system data.
		namespace WaveSourceSystem
		{
			Data& SystemData::add(const entity::Entity& entity)
			{
				uint32_t idx = 0ul;
				if (!unused_data_entries.empty())
				{
					idx = unused_data_entries.front();
					unused_data_entries.pop();
					data[idx] = Data(entity);
				}
				else
				{
					idx = (uint32_t)data.size();
					data.push_back(Data(entity));
					data_to_entity[idx] = entity;
				}

				data_to_entity[idx] = entity;
				entity_to_data[entity] = idx;

				return data[idx];
			}

			Data& SystemData::get(const entity::Entity& entity)
			{
				auto it = entity_to_data.find(entity);
				LMB_ASSERT(it != entity_to_data.end(), "NAME: %llu does not have a component", entity);
				LMB_ASSERT(data[it->second].valid, "NAME: %llu's data was not valid", entity);
				return data[it->second];
			}

			void SystemData::remove(const entity::Entity& entity)
			{
				marked_for_delete.insert(entity);
			}

			bool SystemData::has(const entity::Entity& entity)
			{
				return entity_to_data.find(entity) != entity_to_data.end();
			}
		}




		namespace WaveSourceSystem
		{
			Data::Data(const Data & other)
			{
				entity = other.entity;
				state = other.state;
				buffer = other.buffer;
				handle = other.handle;
				in_world = other.in_world;
				loop = other.loop;
				gain = other.gain;
				pitch = other.pitch;
				radius = other.radius;
				last_position = other.last_position;
				valid = other.valid;
			}
			Data & Data::operator=(const Data & other)
			{
				entity = other.entity;
				state = other.state;
				buffer = other.buffer;
				handle = other.handle;
				in_world = other.in_world;
				loop = other.loop;
				gain = other.gain;
				pitch = other.pitch;
				radius = other.radius;
				last_position = other.last_position;
				valid = other.valid;

				return *this;
			}
		}







		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		WaveSourceComponent::WaveSourceComponent(const entity::Entity& entity, scene::Scene& scene) :
			IComponent(entity), scene_(&scene)
		{
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		WaveSourceComponent::WaveSourceComponent(const WaveSourceComponent& other) :
			IComponent(other.entity_), scene_(other.scene_)
		{
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		WaveSourceComponent::WaveSourceComponent() :
			IComponent(entity::Entity()), scene_(nullptr)
		{
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		void WaveSourceComponent::setBuffer(const asset::VioletWaveHandle& buffer)
		{
			WaveSourceSystem::setBuffer(entity_, buffer, *scene_);
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		asset::VioletWaveHandle WaveSourceComponent::getBuffer() const
		{
			return WaveSourceSystem::getBuffer(entity_, *scene_);
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		void WaveSourceComponent::play()
		{
			WaveSourceSystem::play(entity_, *scene_);
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		void WaveSourceComponent::pause()
		{
			WaveSourceSystem::pause(entity_, *scene_);
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		void WaveSourceComponent::stop()
		{
			WaveSourceSystem::stop(entity_, *scene_);
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		WaveSourceState WaveSourceComponent::getState() const
		{
			return WaveSourceSystem::getState(entity_, *scene_);
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		void WaveSourceComponent::setRelativeToListener(bool relative)
		{
			WaveSourceSystem::setRelativeToListener(entity_, relative, *scene_);
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		bool WaveSourceComponent::getRelativeToListener() const
		{
			return WaveSourceSystem::getRelativeToListener(entity_, *scene_);
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		void WaveSourceComponent::setLoop(bool loop)
		{
			WaveSourceSystem::setLoop(entity_, loop, *scene_);
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		bool WaveSourceComponent::getLoop() const
		{
			return WaveSourceSystem::getLoop(entity_, *scene_);
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		void WaveSourceComponent::setOffset(float seconds)
		{
			WaveSourceSystem::setOffset(entity_, seconds, *scene_);
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		void WaveSourceComponent::setVolume(float volume)
		{
			WaveSourceSystem::setVolume(entity_, volume, *scene_);
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		float WaveSourceComponent::getVolume() const
		{
			return WaveSourceSystem::getVolume(entity_, *scene_);
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		void WaveSourceComponent::setGain(float gain)
		{
			WaveSourceSystem::setGain(entity_, gain, *scene_);
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		float WaveSourceComponent::getGain() const
		{
			return WaveSourceSystem::getGain(entity_, *scene_);
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		void WaveSourceComponent::setPitch(float pitch)
		{
			WaveSourceSystem::setPitch(entity_, pitch, *scene_);
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		float WaveSourceComponent::getPitch() const
		{
			return WaveSourceSystem::getPitch(entity_, *scene_);
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		void WaveSourceComponent::setRadius(float radius)
		{
			WaveSourceSystem::setRadius(entity_, radius, *scene_);
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		float WaveSourceComponent::getRadius() const
		{
			return WaveSourceSystem::getRadius(entity_, *scene_);
		}

	}
}
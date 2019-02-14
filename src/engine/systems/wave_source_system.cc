#include "wave_source_system.h"
#include "transform_system.h"
#include "interfaces/iworld.h"
#include <soloud.h>
#include <soloud_thread.h>
#include <soloud_audiosource.h>
#include <soloud_wav.h>

namespace lambda
{
  namespace components
  {
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void WaveSourceSystem::initialize(world::IWorld& world)
    {
      transform_system_ = world.getScene().getSystem<TransformSystem>();
      
      engine_ = foundation::Memory::construct<SoLoud::Soloud>();
      engine_->init();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void WaveSourceSystem::deinitialize()
    {
      for (const auto& data : data_)
        if (engine_->isValidVoiceHandle(data.handle))
          engine_->stop(data.handle);
      data_.clear();

      engine_->stopAll();

      while (engine_->getActiveVoiceCount() > 0)
        SoLoud::Thread::sleep(100);

      engine_->deinit();
      foundation::Memory::destruct(engine_);

      transform_system_.reset();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void WaveSourceSystem::update(const double& delta_time)
    {
      // Listener.
      glm::vec3 listener_position(0.0f);
      glm::vec3 listener_up(0.0f, 1.0f, 0.0f);
      glm::vec3 listener_forward(1.0f, 0.0f, 0.0f);
      glm::vec3 listener_velocity(0.0f);
      if (listener_ != 0u)
      {
        listener_position = transform_system_->getComponent(listener_).getWorldTranslation();
        listener_forward  = transform_system_->getComponent(listener_).getWorldForward();
        listener_up       = transform_system_->getComponent(listener_).getWorldUp();
        listener_velocity = (last_listener_position_ - listener_position) / (float)delta_time;
        last_listener_position_ = listener_position;
      }
      engine_->set3dListenerPosition(listener_position.x, listener_position.y, listener_position.z);
      engine_->set3dListenerAt(listener_forward.x, listener_forward.y, listener_forward.z);
      engine_->set3dListenerUp(listener_up.x, listener_up.y, listener_up.z);
      engine_->set3dListenerVelocity(listener_velocity.x, listener_velocity.y, listener_velocity.z);

      // Audio sources
      for (WaveSourceData& data : data_)
      {
        if (data.entity == listener_)
          continue;

        updateState(data, (float)delta_time);
      }

      engine_->update3dAudio();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    WaveSourceSystem::~WaveSourceSystem()
    {
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    WaveSourceComponent WaveSourceSystem::addComponent(const entity::Entity& entity)
    {
      if (false == transform_system_->hasComponent(entity))
        transform_system_->addComponent(entity);

			if (!unused_data_entries_.empty())
			{
				uint32_t idx = unused_data_entries_.front();
				unused_data_entries_.pop();

				data_[idx] = WaveSourceData(entity);
				data_to_entity_[idx] = entity;
				entity_to_data_[entity] = idx;
			}
			else
			{
				data_.push_back(WaveSourceData(entity));
				uint32_t idx = (uint32_t)data_.size() - 1u;
				data_to_entity_[idx] = entity;
				entity_to_data_[entity] = idx;
			}

      return WaveSourceComponent(entity, this);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    WaveSourceComponent WaveSourceSystem::getComponent(const entity::Entity& entity)
    {
      return WaveSourceComponent(entity, this);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    bool WaveSourceSystem::hasComponent(const entity::Entity& entity)
    {
      return entity_to_data_.find(entity) != entity_to_data_.end();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void WaveSourceSystem::removeComponent(const entity::Entity& entity)
    {
			getComponent(entity).stop();
			marked_for_delete_.insert(entity);
    }

		void WaveSourceSystem::collectGarbage()
		{
			if (!marked_for_delete_.empty())
			{
				for (entity::Entity entity : marked_for_delete_)
				{
					const auto& it = entity_to_data_.find(entity);
					if (it != entity_to_data_.end())
					{
						uint32_t idx = it->second;
						unused_data_entries_.push(idx);
						data_to_entity_.erase(idx);
						entity_to_data_.erase(entity);
						data_[idx].valid = false;
					}
				}
				marked_for_delete_.clear();
			}
		}
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void WaveSourceSystem::setBuffer(const entity::Entity& entity, const asset::VioletWaveHandle& buffer)
    {
      lookUpData(entity).buffer = buffer;
      updateState(lookUpData(entity), 0.0f);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    asset::VioletWaveHandle WaveSourceSystem::getBuffer(const entity::Entity& entity) const
    {
      return lookUpData(entity).buffer;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void WaveSourceSystem::play(const entity::Entity& entity)
    {
      WaveSourceData& data = lookUpData(entity);

      if (data.handle != 0u)
        engine_->setPause(lookUpData(entity).handle, false);
      else
      {
        if (data.in_world)
          data.handle = engine_->play3d(*data.buffer->getBuffer(), 0.0f, 0.0f, 0.0f);
        else
          data.handle = engine_->play(*data.buffer->getBuffer());
      }

      updateState(data, 0.0f);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void WaveSourceSystem::pause(const entity::Entity& entity)
    {
      engine_->setPause(lookUpData(entity).handle, true);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void WaveSourceSystem::stop(const entity::Entity& entity)
    {
      WaveSourceData& data = lookUpData(entity);
      engine_->stop(data.handle);
      data.handle = 0u;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    WaveSourceState WaveSourceSystem::getState(const entity::Entity& entity) const
    {
      return lookUpData(entity).state;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void WaveSourceSystem::setRelativeToListener(const entity::Entity& entity, bool relative)
    {
      lookUpData(entity).in_world = relative;
      updateState(lookUpData(entity), 0.0f);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    bool WaveSourceSystem::getRelativeToListener(const entity::Entity& entity) const
    {
      return lookUpData(entity).in_world;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void WaveSourceSystem::setLoop(const entity::Entity& entity, bool loop)
    {
      lookUpData(entity).loop = loop;
      updateState(lookUpData(entity), 0.0f);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    bool WaveSourceSystem::getLoop(const entity::Entity& entity) const
    {
      return lookUpData(entity).loop;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void WaveSourceSystem::setOffset(const entity::Entity& entity, float seconds)
    {
      LMB_ASSERT(false, "");
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void WaveSourceSystem::setVolume(const entity::Entity& entity, float volume)
    {
      setGain(entity, volume / 100.0f);
      updateState(lookUpData(entity), 0.0f);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    float WaveSourceSystem::getVolume(const entity::Entity& entity) const
    {
      return getGain(entity) * 100.0f;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void WaveSourceSystem::setGain(const entity::Entity& entity, float gain)
    {
      lookUpData(entity).gain = gain;
      updateState(lookUpData(entity), 0.0f);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    float WaveSourceSystem::getGain(const entity::Entity& entity) const
    {
      return lookUpData(entity).gain;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void WaveSourceSystem::setPitch(const entity::Entity& entity, float pitch)
    {
      lookUpData(entity).pitch = pitch;
      updateState(lookUpData(entity), 0.0f);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    float WaveSourceSystem::getPitch(const entity::Entity& entity) const
    {
      return lookUpData(entity).pitch;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void WaveSourceSystem::setRadius(const entity::Entity& entity, float radius)
    {
      lookUpData(entity).radius = radius;
      updateState(lookUpData(entity), 0.0f);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    float WaveSourceSystem::getRadius(const entity::Entity& entity) const
    {
      return lookUpData(entity).radius;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void WaveSourceSystem::setListener(entity::Entity listener)
    {
      listener_ = listener;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    WaveSourceData& WaveSourceSystem::lookUpData(const entity::Entity& entity)
    {
      assert(entity_to_data_.find(entity) != entity_to_data_.end());
			assert(data_.at(entity_to_data_.at(entity)).valid);
			return data_.at(entity_to_data_.at(entity));
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    const WaveSourceData& WaveSourceSystem::lookUpData(const entity::Entity& entity) const
    {
      assert(entity_to_data_.find(entity) != entity_to_data_.end());
			assert(data_.at(entity_to_data_.at(entity)).valid);
			return data_.at(entity_to_data_.at(entity));
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void WaveSourceSystem::updateState(WaveSourceData& data, float delta_time)
    {
      if (data.handle == 0u)
      {
        data.state = WaveSourceState::kStopped;
        return;
      }

      glm::vec3 position = data.in_world ? transform_system_->getComponent(data.entity).getWorldTranslation() : last_listener_position_;
      glm::vec3 velocity = data.in_world ? ((data.last_position - position) / delta_time) : glm::vec3(0.0f);
      data.last_position = position;

      engine_->setLooping(data.handle, data.loop);
      engine_->setVolume(data.handle, data.gain);
      engine_->setRelativePlaySpeed(data.handle, data.pitch);
      engine_->set3dSourceParameters(
        data.handle,
        position.x,
        position.y,
        position.z,
        velocity.x,
        velocity.y,
        velocity.z
      );
      engine_->set3dSourceMinMaxDistance(data.handle, 1.0f, data.radius);
      engine_->set3dSourceAttenuation(data.handle, SoLoud::AudioSource::LINEAR_DISTANCE, 1.0f);

      data.state = WaveSourceState::kInitial;
      if (engine_->isValidVoiceHandle(data.handle))
      {
        if (engine_->getPause(data.handle))
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
    WaveSourceComponent::WaveSourceComponent(const entity::Entity& entity, WaveSourceSystem* system) :
      IComponent(entity), system_(system)
    {
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    WaveSourceComponent::WaveSourceComponent(const WaveSourceComponent& other) :
      IComponent(other.entity_), system_(other.system_)
    {
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    WaveSourceComponent::WaveSourceComponent() :
      IComponent(entity::Entity()), system_(nullptr)
    {
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void WaveSourceComponent::setBuffer(const asset::VioletWaveHandle& buffer)
    {
      system_->setBuffer(entity_, buffer);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    asset::VioletWaveHandle WaveSourceComponent::getBuffer() const
    {
      return system_->getBuffer(entity_);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void WaveSourceComponent::play()
    {
      system_->play(entity_);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void WaveSourceComponent::pause()
    {
      system_->pause(entity_);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void WaveSourceComponent::stop()
    {
      system_->stop(entity_);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    WaveSourceState WaveSourceComponent::getState() const
    {
      return system_->getState(entity_);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void WaveSourceComponent::setRelativeToListener(bool relative)
    {
      system_->setRelativeToListener(entity_, relative);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    bool WaveSourceComponent::getRelativeToListener() const
    {
      return system_->getRelativeToListener(entity_);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void WaveSourceComponent::setLoop(bool loop)
    {
      system_->setLoop(entity_, loop);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    bool WaveSourceComponent::getLoop() const
    {
      return system_->getLoop(entity_);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void WaveSourceComponent::setOffset(float seconds)
    {
      system_->setOffset(entity_, seconds);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void WaveSourceComponent::setVolume(float volume)
    {
      system_->setVolume(entity_, volume);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    float WaveSourceComponent::getVolume() const
    {
      return system_->getVolume(entity_);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void WaveSourceComponent::setGain(float gain)
    {
      system_->setGain(entity_, gain);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    float WaveSourceComponent::getGain() const
    {
      return system_->getGain(entity_);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void WaveSourceComponent::setPitch(float pitch)
    {
      system_->setPitch(entity_, pitch);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    float WaveSourceComponent::getPitch() const
    {
      return system_->getPitch(entity_);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void WaveSourceComponent::setRadius(float radius)
    {
      system_->setRadius(entity_, radius);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    float WaveSourceComponent::getRadius() const
    {
      return system_->getRadius(entity_);
    }
    WaveSourceData::WaveSourceData(const WaveSourceData & other)
    {
      entity        = other.entity;
      state         = other.state;
      buffer        = other.buffer;
      handle        = other.handle;
      in_world      = other.in_world;
      loop          = other.loop;
      gain          = other.gain;
      pitch         = other.pitch;
      radius        = other.radius;
      last_position = other.last_position;
			valid         = other.valid;
    }
    WaveSourceData & WaveSourceData::operator=(const WaveSourceData & other)
    {
      entity        = other.entity;
      state         = other.state;
      buffer        = other.buffer;
      handle        = other.handle;
      in_world      = other.in_world;
      loop          = other.loop;
      gain          = other.gain;
      pitch         = other.pitch;
      radius        = other.radius;
      last_position = other.last_position;
			valid         = other.valid;
      
      return *this;
    }
}
}
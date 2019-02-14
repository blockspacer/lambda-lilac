#include "systems/mono_behaviour_system.h"
#include "systems/transform_system.h"
#include "interfaces/iworld.h"
#include "interfaces/iscript_context.h"

namespace lambda
{
  namespace components
  {
    MonoBehaviourComponent MonoBehaviourSystem::addComponent(const entity::Entity& entity)
    {
			if (!unused_data_entries_.empty())
			{
				uint32_t idx = unused_data_entries_.front();
				unused_data_entries_.pop();

				data_[idx] = MonoBehaviourData(entity);
				data_to_entity_[idx] = entity;
				entity_to_data_[entity] = idx;
			}
			else
			{
				data_.push_back(MonoBehaviourData(entity));
				uint32_t idx = (uint32_t)data_.size() - 1u;
				data_to_entity_[idx] = entity;
				entity_to_data_[entity] = idx;
			}

      return MonoBehaviourComponent(entity, this);
    }
    MonoBehaviourComponent MonoBehaviourSystem::getComponent(const entity::Entity& entity)
    {
      return MonoBehaviourComponent(entity, this);
    }
    bool MonoBehaviourSystem::hasComponent(const entity::Entity& entity) const
    {
      return entity_to_data_.find(entity) != entity_to_data_.end();
    }
    void MonoBehaviourSystem::removeComponent(const entity::Entity& entity)
    {
			marked_for_delete_.insert(entity);
    }
		void MonoBehaviourSystem::collectGarbage()
		{

			if (!marked_for_delete_.empty())
			{
				for (entity::Entity entity : marked_for_delete_)
				{
					{
						MonoBehaviourData& data = lookUpData(entity);
#define FREE(x) if (x) world_->getScripting()->freeHandle(x), x = nullptr
						FREE(data.object);
						FREE(data.initialize);
						FREE(data.deinitialize);
						FREE(data.update);
						FREE(data.fixed_update);
						FREE(data.on_collision_enter);
						FREE(data.on_collision_exit);
						FREE(data.on_trigger_enter);
						FREE(data.on_trigger_exit);
#undef FREE
					}

					const auto& it = entity_to_data_.find(entity);
					if (it != entity_to_data_.end())
					{
						uint32_t idx = it->second;
						unused_data_entries_.push(idx);
						data_to_entity_.erase(idx);
						entity_to_data_.erase(entity);
						data_[idx].valid = false;
					}
					else
						Warning("Could not find id: " + toString(entity));
				}
				marked_for_delete_.clear();
			}
		}
    void MonoBehaviourSystem::initialize(world::IWorld& world)
    {
      world_ = &world;
    }
    void MonoBehaviourSystem::deinitialize()
    {
      world_ = nullptr;
    }
		void MonoBehaviourSystem::update(const double& delta_time)
		{
			for (uint32_t i = 0u; i < data_.size(); ++i)
			{
				const auto& data = data_[i];
				if (data.valid && data.object && data.update)
					world_->getScripting()->executeFunction(data.object, data.update, {});
			}
		}
    void MonoBehaviourSystem::fixedUpdate(const double& delta_time)
    {
			for(uint32_t i = 0u; i < data_.size(); ++i)
			{
				const auto& data = data_[i];
				if (data.valid && data.object && data.fixed_update)
					world_->getScripting()->executeFunction(data.object, data.fixed_update, {});
			}
    }
    void MonoBehaviourSystem::setObject(const entity::Entity& entity, void* ptr)
    {
      lookUpData(entity).object = ptr;
    }
    void MonoBehaviourSystem::setInitialize(const entity::Entity& entity, void* ptr)
    {
      lookUpData(entity).initialize = ptr;
    }
    void MonoBehaviourSystem::setDeinitialize(const entity::Entity& entity, void* ptr)
    {
      lookUpData(entity).deinitialize = ptr;
    }
    void MonoBehaviourSystem::setUpdate(const entity::Entity& entity, void* ptr)
    {
      lookUpData(entity).update = ptr;
    }
    void MonoBehaviourSystem::setFixedUpdate(const entity::Entity& entity, void* ptr)
    {
      lookUpData(entity).fixed_update = ptr;
    }
    void MonoBehaviourSystem::setOnCollisionEnter(const entity::Entity& entity, void* ptr)
    {
      lookUpData(entity).on_collision_enter = ptr;
    }
    void MonoBehaviourSystem::setOnCollisionExit(const entity::Entity& entity, void* ptr)
    {
      lookUpData(entity).on_collision_exit = ptr;
    }
    void MonoBehaviourSystem::setOnTriggerEnter(const entity::Entity& entity, void* ptr)
    {
      lookUpData(entity).on_trigger_enter = ptr;
    }
    void MonoBehaviourSystem::setOnTriggerExit(const entity::Entity& entity, void* ptr)
    {
      lookUpData(entity).on_trigger_exit = ptr;
    }
    void* MonoBehaviourSystem::getObject(const entity::Entity& entity) const
    {
      return lookUpData(entity).object;
    }
    void* MonoBehaviourSystem::getInitialize(const entity::Entity& entity) const
    {
      return lookUpData(entity).initialize;
    }
    void* MonoBehaviourSystem::getDeinitialize(const entity::Entity& entity) const
    {
      return lookUpData(entity).deinitialize;
    }
    void* MonoBehaviourSystem::getUpdate(const entity::Entity& entity) const
    {
      return lookUpData(entity).update;
    }
    void* MonoBehaviourSystem::getFixedUpdate(const entity::Entity& entity) const
    {
      return lookUpData(entity).fixed_update;
    }
    void* MonoBehaviourSystem::getOnCollisionEnter(const entity::Entity& entity) const
    {
      return lookUpData(entity).on_collision_enter;
    }
    void* MonoBehaviourSystem::getOnCollisionExit(const entity::Entity& entity) const
    {
      return lookUpData(entity).on_collision_exit;
    }
    void* MonoBehaviourSystem::getOnTriggerEnter(const entity::Entity& entity) const
    {
      return lookUpData(entity).on_trigger_enter;
    }
    void* MonoBehaviourSystem::getOnTriggerExit(const entity::Entity& entity) const
    {
      return lookUpData(entity).on_trigger_exit;
    }
#define SCR_COL(other, normal) { scripting::ScriptValue(other, true), scripting::ScriptValue(normal) }
		void MonoBehaviourSystem::onCollisionEnter(const entity::Entity& lhs, const entity::Entity& rhs, glm::vec3 normal) const
		{
			auto data_lhs = getClosest(lhs);
			auto data_rhs = getClosest(rhs);
			if (data_lhs) world_->getScripting()->executeFunction(data_lhs->object, data_lhs->on_collision_enter, SCR_COL(rhs, normal));
			if (data_rhs) world_->getScripting()->executeFunction(data_rhs->object, data_rhs->on_collision_enter, SCR_COL(lhs, normal));
		}
		void MonoBehaviourSystem::onCollisionExit(const entity::Entity& lhs, const entity::Entity& rhs, glm::vec3 normal) const
		{
			auto data_lhs = getClosest(lhs);
			auto data_rhs = getClosest(rhs);
			if (data_lhs) world_->getScripting()->executeFunction(data_lhs->object, data_lhs->on_collision_exit, SCR_COL(rhs, normal));
			if (data_rhs) world_->getScripting()->executeFunction(data_rhs->object, data_rhs->on_collision_exit, SCR_COL(lhs, normal));
		}
		void MonoBehaviourSystem::onTriggerEnter(const entity::Entity& lhs, const entity::Entity& rhs, glm::vec3 normal) const
		{
			auto data_lhs = getClosest(lhs);
			auto data_rhs = getClosest(rhs);
			if (data_lhs) world_->getScripting()->executeFunction(data_lhs->object, data_lhs->on_trigger_enter, SCR_COL(rhs, normal));
			if (data_rhs) world_->getScripting()->executeFunction(data_rhs->object, data_rhs->on_trigger_enter, SCR_COL(lhs, normal));
		}
		void MonoBehaviourSystem::onTriggerExit(const entity::Entity& lhs, const entity::Entity& rhs, glm::vec3 normal) const
		{
			auto data_lhs = getClosest(lhs);
			auto data_rhs = getClosest(rhs);
			if (data_lhs) world_->getScripting()->executeFunction(data_lhs->object, data_lhs->on_trigger_exit, SCR_COL(rhs, normal));
			if (data_rhs) world_->getScripting()->executeFunction(data_rhs->object, data_rhs->on_trigger_exit, SCR_COL(lhs, normal));
		}
    MonoBehaviourData& MonoBehaviourSystem::lookUpData(const entity::Entity& entity)
    {
      assert(entity_to_data_.find(entity) != entity_to_data_.end());
			assert(data_.at(entity_to_data_.at(entity)).valid);
			return data_.at(entity_to_data_.at(entity));
    }
    const MonoBehaviourData& MonoBehaviourSystem::lookUpData(const entity::Entity& entity) const
    {
      assert(entity_to_data_.find(entity) != entity_to_data_.end());
			assert(data_.at(entity_to_data_.at(entity)).valid);
			return data_.at(entity_to_data_.at(entity));
    }
		const MonoBehaviourData* MonoBehaviourSystem::getClosest(entity::Entity entity) const
		{
			if (hasComponent(entity))
				return &lookUpData(entity);

			auto parent = world_->getScene().getSystem<TransformSystem>()->getParent(entity);
			if (parent)
				return getClosest(parent);

			return nullptr;
		}
    MonoBehaviourComponent::MonoBehaviourComponent(const entity::Entity& entity, MonoBehaviourSystem* system) :
      IComponent(entity), system_(system)
    {
    }
    MonoBehaviourComponent::MonoBehaviourComponent(const MonoBehaviourComponent& other) :
      IComponent(other.entity_), system_(other.system_)
    {
    }
    MonoBehaviourComponent::MonoBehaviourComponent() :
      IComponent(entity::Entity()), system_(nullptr)
    {
    }
    MonoBehaviourData::MonoBehaviourData(const MonoBehaviourData & other)
    {
      object             = other.object;
      initialize         = other.initialize;
      deinitialize       = other.deinitialize;
      update             = other.update;
      fixed_update       = other.fixed_update;
      on_collision_enter = other.on_collision_enter;
      on_collision_exit  = other.on_collision_exit;
      on_trigger_enter   = other.on_trigger_enter;
      on_trigger_exit    = other.on_trigger_exit;
      entity             = other.entity;
			valid              = other.valid;
    }
    MonoBehaviourData & MonoBehaviourData::operator=(const MonoBehaviourData & other)
    {
      object             = other.object;
      initialize         = other.initialize;
      deinitialize       = other.deinitialize;
      update             = other.update;
      fixed_update       = other.fixed_update;
      on_collision_enter = other.on_collision_enter;
      on_collision_exit  = other.on_collision_exit;
      on_trigger_enter   = other.on_trigger_enter;
      on_trigger_exit    = other.on_trigger_exit;
      entity             = other.entity;
			valid              = other.valid;

      return *this;
    }
}
}

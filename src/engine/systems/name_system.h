#pragma once
#include "interfaces/icomponent.h"
#include "interfaces/isystem.h"

namespace lambda
{
  namespace components
  {
		class NameSystem;

    class NameComponent : public IComponent
    {
    public:
			NameComponent(const entity::Entity& entity, NameSystem* system);
			NameComponent(const NameComponent& other);
			NameComponent();

      void setName(const String& name);
      String getName() const;
			void setTags(const Vector<String>& tags);
			Vector<String> getTags() const;

    private:
      NameSystem* system_;
    };

    struct NameData
    {
			NameData(const entity::Entity& entity) : entity(entity) {};
			NameData(const NameData& other);
			NameData& operator=(const NameData& other);

			String name;
			Vector<String> tags;
      entity::Entity entity;
			bool valid = true;
    };

    class NameSystem : public ISystem
    {
    public:
      static size_t systemId() { return (size_t)SystemIds::kNameSystem; };
      NameComponent addComponent(const entity::Entity& entity);
      NameComponent getComponent(const entity::Entity& entity);
      bool hasComponent(const entity::Entity& entity);
      void removeComponent(const entity::Entity& entity);
      virtual void initialize(world::IWorld& world) override;
      virtual void deinitialize() override;
      virtual void collectGarbage() override;
			virtual ~NameSystem() override {};

			void setName(const entity::Entity& entity, const String& name);
			String getName(const entity::Entity& entity) const;
			void setTags(const entity::Entity& entity, const Vector<String>& tags);
			Vector<String> getTags(const entity::Entity& entity) const;

    protected:
      NameData& lookUpData(const entity::Entity& entity);
      const NameData& lookUpData(const entity::Entity& entity) const;

    private:
      Vector<NameData> data_;
			Map<entity::Entity, uint32_t> entity_to_data_;
			Map<uint32_t, entity::Entity> data_to_entity_;
			Set<entity::Entity> marked_for_delete_;
			Queue<uint32_t> unused_data_entries_;
    };
  }
}
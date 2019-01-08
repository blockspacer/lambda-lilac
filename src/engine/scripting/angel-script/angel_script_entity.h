#pragma once
#include <containers/containers.h>
#include <angelscript.h>
#include "systems/entity.h"
#include <memory/memory.h>
#include <utils/console.h>

namespace lambda
{
  namespace scripting
  {
    typedef int ASID;
    
    class AngelScriptComponent
    {
    public:
      void construct(ASID asid, uint64_t entity_id);
      void destruct();
      asIScriptObject* get() const;
      bool empty() const;
      static bool k_terminating;

    private:
      ASID             id_        = 0;
      asIScriptObject* object_    = nullptr;
    };

    class AngelScriptComponents
    {
    public:
      AngelScriptComponents(uint64_t id);
      void addComponent(void* ptr, ASID asid);
      void getComponent(void* ptr, ASID asid);
      void removeComponent(void* ptr, ASID asid);

      void clear();
      void collectGarbage();
      bool empty() const;

    private:
      UnorderedMap<ASID, AngelScriptComponent*> components_;
      uint64_t id_ = 0u;
    };

    class AngelScriptComponentManager
    {
    public:
      AngelScriptComponents* getComponents(uint64_t id);
      void clear(uint64_t id);
      void collectGarbage();
      void setEntitySystem(entity::EntitySystem* entity_system)
      {
        entity_system_ = entity_system;
      }
      entity::EntitySystem* getEntitySystem() const
      {
        return entity_system_;
      }

    private:
      UnorderedMap<uint64_t, AngelScriptComponents*> entity_components_;
      Set<uint64_t> called_entities_;
      entity::EntitySystem* entity_system_;
    };

    class AngelScriptEntity
    {
    public:
      AngelScriptEntity() :
        id_(0u)
      {
        addRef();
      }
      AngelScriptEntity(AngelScriptComponentManager* component_manager, uint64_t id) :
        id_(id)
      {
        addRef();
      }
      AngelScriptEntity(const AngelScriptEntity& other) :
        id_(other.id_)
      {
        addRef();
      }
      ~AngelScriptEntity()
      {
        release();
      }
      void setId(uint64_t id)
      {
        release();
        id_ = id;
        addRef();
      }
      uint64_t getId() const
      {
        return id_;
      }
      void construct();
      void destruct();

      int setRef(const int& ref)
      {
        if (id_ == 0u) return 1u;
        return k_ref_counts_[id_] = ref;
      }
      int getRef() const
      {
        return k_ref_counts_[id_];
      }
      int addRef()
      {
        if (id_ == 0u) return 1u;
        return setRef(getRef() + 1);
      }
      int release()
      {
        if (id_ == 0u) return 1u;
        if (setRef(getRef() - 1) == 0)
        {
          destruct();
        }
        return getRef();
      }

      void operator=(const AngelScriptEntity& other)
      {
        setId(other.id_);
      }

      void addComponent(void* ptr, ASID asid)
      {
        k_component_manager_->getComponents(id_)->addComponent(ptr, asid);
      }
      void getComponent(void* ptr, ASID asid)
      {
        k_component_manager_->getComponents(id_)->getComponent(ptr, asid);
      }
      void removeComponent(void* ptr, ASID asid)
      {
        k_component_manager_->getComponents(id_)->removeComponent(ptr, asid);
      }

      static void setComponentManager(AngelScriptComponentManager* component_manager)
      {
        k_component_manager_ = component_manager;
      }

    private:
      static Map<uint64_t, int> k_ref_counts_;
      static AngelScriptComponentManager* k_component_manager_;
      uint64_t id_;
    };
  }
}
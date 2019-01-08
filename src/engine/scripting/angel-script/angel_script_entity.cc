#include "angel_script_entity.h"
#include "systems/entity_system.h"

namespace lambda
{
  namespace scripting
  {
    bool AngelScriptComponent::k_terminating = false;

    void AngelScriptComponent::construct(ASID asid, uint64_t entity_id)
    {
      id_ = asid;
      asIScriptEngine*  engine    = asGetActiveContext()->GetEngine();
      asITypeInfo*      type_info = engine->GetTypeInfoById(id_);
      asIScriptContext* context   = engine->RequestContext();

      asIScriptFunction* constructor = type_info->GetFactoryByIndex(0u);
      
      int r;
      r = context->Prepare(constructor); assert(r >= 0);
      r = context->Execute(); assert(r >= 0);
      object_ = *(asIScriptObject**)context->GetAddressOfReturnValue();
      object_->AddRef();
      

      // Set the id.
      void* ptr_id = nullptr;
      for (unsigned i = 0; i < object_->GetPropertyCount(); ++i)
      {
        String name = object_->GetPropertyName(i);
        if (name == "id")
        {
          ptr_id = object_->GetAddressOfProperty(i);
          break;
        }
      }
      if (ptr_id != nullptr)
      {
        uint64_t& id = *((uint64_t*)ptr_id);
        id = entity_id;
      }

      // Call initialize.
      asIScriptFunction* initialize = type_info->GetMethodByName("Initialize");
      r = context->Prepare(initialize); assert(r >= 0);
      r = context->SetObject(object_); assert(r >= 0);
      r = context->Execute(); assert(r >= 0);
    }
    void AngelScriptComponent::destruct()
    {
      if (false == k_terminating)
      {
        asIScriptEngine*  engine    = asGetActiveContext()->GetEngine();
        asITypeInfo*      type_info = engine->GetTypeInfoById(id_);
        asIScriptContext* context   = engine->RequestContext();

        // Call Destroy.
        int r;
        asIScriptFunction* destroy = type_info->GetMethodByName("Destroy");
        r = context->Prepare(destroy); assert(r >= 0);
        r = context->SetObject(object_); assert(r >= 0);
        r = context->Execute(); assert(r >= 0);
      }

      object_->Release();
      object_ = nullptr;
    }
    asIScriptObject* AngelScriptComponent::get() const
    {
      return object_;
    }
    bool AngelScriptComponent::empty() const
    {
      return object_ == nullptr;
    }


    AngelScriptComponents::AngelScriptComponents(uint64_t id) :
      id_(id)
    {
    }

    void AngelScriptComponents::addComponent(void* ptr, ASID asid)
    {
      ASID  id = asid & asTYPEID_MASK_SEQNBR;
      components_.insert(eastl::make_pair(id, foundation::Memory::construct<AngelScriptComponent>()));
      components_.at(id)->construct(asid, id_);

      getComponent(ptr, asid | asTYPEID_OBJHANDLE);
    }
    void AngelScriptComponents::removeComponent(void* ptr, ASID asid)
    {
      ASID  id = asid & asTYPEID_MASK_SEQNBR;
      auto& it = components_.find(id);

      if (it != components_.end() && it->second->get() != nullptr)
      {
        it->second->destruct();
        if (it->second->get() == nullptr)
        {
          foundation::Memory::destruct(it->second);
          components_.erase(it);
        }
      }
    }
    void AngelScriptComponents::getComponent(void* ptr, ASID asid)
    {
      ASID  id = asid & asTYPEID_MASK_SEQNBR;
      auto& it = components_.find(id);

      if (it != components_.end() && it->second->get() != nullptr)
      {
        *static_cast<asIScriptObject**>(ptr) = it->second->get();
        it->second->get()->AddRef();
      }
      else
      {
        ptr = nullptr;
      }
    }

    void AngelScriptComponents::clear()
    {
      for (auto& component : components_)
      {
        component.second->destruct();
      }
      components_.clear();
    }

    void AngelScriptComponents::collectGarbage()
    {
      for (const auto& component : components_)
      {
        if (component.second->empty())
        {
          foundation::Memory::destruct(component.second);
          components_.erase(component.first);
        }
      }
    }
    bool AngelScriptComponents::empty() const
    {
      return components_.empty();
    }







    AngelScriptComponents* AngelScriptComponentManager::getComponents(uint64_t id)
    {
      auto it = entity_components_.find(id);
      
      // Insert entity.
      if (it == entity_components_.end())
      {
        entity_components_.insert(eastl::make_pair(id, foundation::Memory::construct<AngelScriptComponents>(id)));
        it = entity_components_.find(id);
      }

      // Mark as used.
      called_entities_.insert(id);
      
      return it->second;
    }
    void AngelScriptComponentManager::clear(uint64_t id)
    {
      getComponents(id)->clear();
      collectGarbage();
    }
    void AngelScriptComponentManager::collectGarbage()
    {
      for (const uint64_t& id : called_entities_)
      {
        AngelScriptComponents* components = entity_components_.at(id);
        components->collectGarbage();
        if (true == components->empty())
        {
          foundation::Memory::destruct(components);
          entity_components_.erase(id);
        }
      }

      called_entities_.clear();
    }




    void AngelScriptEntity::construct()
    {
      setId(k_component_manager_->getEntitySystem()->createEntity().id());
    }

    void AngelScriptEntity::destruct()
    {
      k_component_manager_->getEntitySystem()->destroyEntity(entity::Entity(id_, k_component_manager_->getEntitySystem()));
      k_component_manager_->clear(id_);
      k_ref_counts_.erase(id_);
      id_ = 0u;
    }

    Map<uint64_t, int> AngelScriptEntity::k_ref_counts_;
    AngelScriptComponentManager* AngelScriptEntity::k_component_manager_ = nullptr;
  }
}
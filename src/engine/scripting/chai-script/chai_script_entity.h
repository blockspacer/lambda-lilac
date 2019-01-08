#pragma once
#include <containers/containers.h>
#include <memory/memory.h>

namespace lambda
{
  namespace world
  {
    class IWorld;
  }
  namespace scripting
  {
    extern world::IWorld* k_chai_world;

    class ChaiGameObject;
    class ChaiComponent
    {
    public:
      ChaiComponent(ChaiGameObject* game_object);
      virtual ~ChaiComponent();

      virtual void Begin() = 0;
      virtual void End() = 0;

      ChaiGameObject* GetGameObject() const;

    private:
      ChaiGameObject* game_object_;
    };

    class ChaiGameObject
    {
    private:
      struct Data
      {
        Map<size_t, ChaiComponent*> components;
        size_t id;
      };

    public:
      void destroy();
      ChaiGameObject(size_t id);
        ChaiGameObject(const ChaiGameObject& other);
        void operator=(const ChaiGameObject& other);
        ~ChaiGameObject();

    public:
      size_t GetId() const;
      template<typename T>
      T* AddComponent()
      {
        T* t = foundation::Memory::construct<T>(k_chai_world, this);
        data_->components.insert(eastl::make_pair(typeid(T).hash_code(), (ChaiComponent*)t));
        t->Begin();
        return t;
      }
      template<typename T>
      T* GetComponent()
      {
        return (T*)data_->components[typeid(T).hash_code()];
      }
      template<typename T>
      void RemoveComponent()
      {
        auto it = data_->components.find(typeid(T).hash_code());
        foundation::Memory::destruct((T*)it->second);
        data_->components.erase(it);
      }

    private:
      Data* data_ = nullptr;
    };
  }
}
#pragma once
#include "interfaces/isystem.h"
#include "interfaces/icomponent.h"
#include "interfaces/iwindow.h"
#include "interfaces/irenderer.h"
#include "utils/angle.h"
#include "utils/distance.h"
#include "platform/shader_pass.h"
#include "platform/frustum.h"
#include "platform/culling.h"

namespace lambda
{
  namespace components
  {
    class CameraSystem;
    class TransformSystem;
    class MeshRenderSystem;

    class CameraComponent : public IComponent
    {
    public:
      CameraComponent(const entity::Entity& entity, CameraSystem* system);
      CameraComponent(const CameraComponent& other);
      CameraComponent();

      void setFov(const utilities::Angle& fov);
      utilities::Angle getFov() const;
      void setNearPlane(const utilities::Distance& near_plane);
      utilities::Distance getNearPlane() const;
      void setFarPlane(const utilities::Distance& far_plane);
      utilities::Distance getFarPlane() const;
      void addShaderPass(const platform::ShaderPass& shader_pass);
      void setShaderPasses(const Vector<platform::ShaderPass>& shader_pass);
      platform::ShaderPass getShaderPass(uint32_t id) const;
      Vector<platform::ShaderPass> getShaderPasses() const;

      CameraSystem* getSystem() const { return system_; }
    private:
      CameraSystem* system_;
    };

    struct CameraData
    {
      CameraData(const entity::Entity& entity) : entity(entity) {};
      CameraData(const CameraData& other);
      CameraData& operator=(const CameraData& other);

      utilities::Angle    fov        = utilities::Angle::fromDeg(90.0f);
      utilities::Distance near_plane = utilities::Distance::fromMeter(0.1f);
      utilities::Distance far_plane  = utilities::Distance::fromMeter(1000.0f);
      Vector<platform::ShaderPass> shader_passes;

      entity::Entity entity;
    };

    class CameraSystem : public ISystem
    {
    public:
      ~CameraSystem();
 
      virtual void initialize(world::IWorld& world) override;
      virtual void deinitialize() override;
      virtual void onRender() override;

      static size_t systemId() { return (size_t)SystemIds::kCameraSystem; };
      CameraComponent addComponent(const entity::Entity& entity);
      CameraComponent getComponent(const entity::Entity& entity);
      bool hasComponent(const entity::Entity& entity);
      void removeComponent(const entity::Entity& entity);

      void setFov(const entity::Entity& entity, const utilities::Angle& fov);
      utilities::Angle getFov(const entity::Entity& entity) const;
      void setNearPlane(const entity::Entity& entity, const utilities::Distance& near_plane);
      utilities::Distance getNearPlane(const entity::Entity& entity) const;
      void setFarPlane(const entity::Entity& entity, const utilities::Distance& far_plane);
      utilities::Distance getFarPlane(const entity::Entity& entity) const;
      void addShaderPass(const entity::Entity& entity, const platform::ShaderPass& shader_pass);
      void setShaderPasses(const entity::Entity& entity, const Vector<platform::ShaderPass>& shader_pass);
      platform::ShaderPass getShaderPass(const entity::Entity& entity, uint32_t id) const;
      Vector<platform::ShaderPass> getShaderPasses(const entity::Entity& entity) const;

      void bindCamera(const entity::Entity& entity);
      entity::Entity getMainCamera();
      void setMainCamera(const entity::Entity& main_camera);

    private:
      Vector<CameraData> data_;
      Map<uint64_t, uint32_t> entity_to_data_;
      Map<uint32_t, uint64_t> data_to_entity_;

      CameraData& lookUpData(const entity::Entity& entity);
      const CameraData& lookUpData(const entity::Entity& entity) const;

    private:
      foundation::SharedPointer<TransformSystem> transform_system_;
      foundation::SharedPointer<MeshRenderSystem> mesh_render_system_;
      world::IWorld* world_;

      entity::Entity main_camera_ = entity::InvalidEntity;
      utilities::Frustum main_camera_frustum_;
      utilities::Culler main_camera_culler_;
    };
  }
}
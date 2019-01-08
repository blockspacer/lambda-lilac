#pragma once
#include "../chai_script_entity.h"
#include "../../script_vector.h"
#include "systems/camera_system.h"
#include "assets/shader_pass.h"

namespace lambda
{
  namespace scripting
  {
    class ChaiCamera : public ChaiComponent
    {
    public:
      ChaiCamera(world::IWorld* world, ChaiGameObject* game_object);
      virtual ~ChaiCamera();
      
      virtual void Begin() final override {};
      virtual void End() final override {};

      void setFov(const utilities::Angle& fov);
      utilities::Angle getFov() const;
      void setNearPlane(const utilities::Distance& near_plane);
      utilities::Distance getNearPlane() const;
      void setFarPlane(const utilities::Distance& far_plane);
      utilities::Distance getFarPlane() const;

      void setShaderPass(asset::VioletShaderPassHandle shader_pass);
      asset::VioletShaderPassHandle getShaderPass() const;

    private:
      components::CameraComponent camera_;
    };
  }
}
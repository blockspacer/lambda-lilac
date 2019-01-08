#include "chai_script_camera.h"
#include "interfaces/iworld.h"
#include "platform/scene.h"
#include "systems/entity_system.h"

namespace lambda
{
  namespace scripting
  {
    ChaiCamera::ChaiCamera(world::IWorld* world, ChaiGameObject* game_object) : 
      ChaiComponent(game_object)
    {
      world::Scene& scene = world->getScene();
      camera_ = scene.getSystem<components::CameraSystem>()->addComponent(
        entity::Entity(
          game_object->GetId(),
          scene.getSystem<entity::EntitySystem>().get()
        )
      );
    }
    ChaiCamera::~ChaiCamera()
    {
      camera_.getSystem()->removeComponent(camera_.entity());
    }
    void ChaiCamera::setFov(const utilities::Angle& fov)
    {
      camera_.setFov(fov);
    }
    utilities::Angle ChaiCamera::getFov() const
    {
      return camera_.getFov();
    }
    void ChaiCamera::setNearPlane(const utilities::Distance& near_plane)
    {
      camera_.setNearPlane(near_plane);
    }
    utilities::Distance ChaiCamera::getNearPlane() const
    {
      return camera_.getNearPlane();
    }
    void ChaiCamera::setFarPlane(const utilities::Distance& far_plane)
    {
      camera_.setFarPlane(far_plane);
    }
    utilities::Distance ChaiCamera::getFarPlane() const
    {
      return camera_.getFarPlane();
    }
    void ChaiCamera::setShaderPass(asset::VioletShaderPassHandle shader_pass)
    {
      // TODO (Hilze): Implement.
    }
    asset::VioletShaderPassHandle ChaiCamera::getShaderPass() const
    {
      return asset::VioletShaderPassHandle(); // TODO (Hilze): Implement.
    }
  }
}
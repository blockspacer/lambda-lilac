//#include "irenderer_flush.h"
//#include <memory/memory.h>
//#include <platform/post_process_manager.h>
//#include <platform/rasterizer_state.h>
//
//namespace lambda
//{
//  namespace platform
//  {
//    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//    void IRenderFlush::flush(IRenderFlushData flush)
//    {
//      flushCameras(flush);
//      flushLights(flush);
//      flushPostProcessing(flush);
//      flushUi(flush);
//    }
//
//    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//    void IRenderFlush::flushLights(IRenderFlushData flush)
//    {
//      beginTimer("Lighting");
//
//      // Prepare the light buffer.
//      pushMarker("Clear Light Buffer");
//      clearRenderTarget(
//        getPostProcessManager().getTarget(Name("light_map")).getTexture(),
//        glm::vec4(0.0f)
//      );
//      popMarker();
//
//      for (components::LightData& data : flush.lights.data)
//      {
//        // Skip disabled lights.
//        if (data.enabled == false)
//        {
//          switch (data.type)
//          {
//          case components::LightData::LightType::kDirectional: setMarker("Directional Light - Disabled"); break;
//          case components::LightData::LightType::kSpot:        setMarker("Spot Light - Disabled"); break;
//          case components::LightData::LightType::kPoint:       setMarker("Point Light - Disabled"); break;
//          case components::LightData::LightType::kCascade:     setMarker("Cascade Light - Disabled"); break;
//          }
//          continue;
//        }
//
//        switch (data.type)
//        {
//        case components::LightData::LightType::kDirectional:
//          if (flush.shaders_directional.shader_generate != nullptr && flush.shaders_directional.shader_publish != nullptr)
//          {
//            pushMarker("Directional Light");
//            renderDirectional(data.entity);
//            popMarker();
//          }
//          break;
//        case components::LightData::LightType::kSpot:
//          if (flush.shaders_spot.shader_generate != nullptr && flush.shaders_spot.shader_publish != nullptr)
//          {
//            pushMarker("Spot Light");
//            renderSpot(data.entity);
//            popMarker();
//          }
//          break;
//        case components::LightData::LightType::kPoint:
//          if (flush.shaders_point.shader_generate != nullptr && flush.shaders_point.shader_publish != nullptr)
//          {
//            pushMarker("Point Light");
//            renderPoint(data.entity);
//            popMarker();
//          }
//          break;
//        case components::LightData::LightType::kCascade:
//          if (flush.shaders_cascade.shader_generate != nullptr && flush.shaders_cascade.shader_publish != nullptr)
//          {
//            pushMarker("Cascade Light");
//            renderCascade(data.entity);
//            popMarker();
//          }
//          break;
//        }
//      }
//
//      // Reset states.
//      setRasterizerState(platform::RasterizerState::SolidFront());
//      camera_system_->bindCamera(camera_system_->getMainCamera());
//
//      endTimer("Lighting");
//    }
//
//    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//    void IRenderFlush::flushCameras(IRenderFlushData flush)
//    {
//    }
//
//    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//    void IRenderFlush::flushPostProcessing(IRenderFlushData flush)
//    {
//    }
//
//    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//    void IRenderFlush::flushUi(IRenderFlushData flush)
//    {
//    }
//
//    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//    void IRenderFlush::renderAll(IRenderFlushData flush)
//    {
//    }
//    
//    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//    void IRenderFlush::bindCamera(IRenderFlushData flush, entity::Entity entity)
//    {
//      components::CameraData&    data      = flush.cameras.data[flush.cameras.entity_to_data[entity.id()]];
//      components::TransformData& transform = flush.transforms.data[flush.transforms.entity_to_data[entity.id()]];
//      
//      const glm::mat4x4 view = glm::inverse(transform.getWorld());
//      const glm::mat4x4 projection = glm::perspective(
//        data.fov.asRad(),
//        (float)world_->getWindow()->getSize().x / (float)world_->getWindow()->getSize().y,
//        data.near_plane.asMeter(),
//        data.far_plane.asMeter()
//      );
//
//      // Update the frustum.
//      main_camera_frustum_.construct(
//        projection,
//        view
//      );
//
//      world_->getRenderer()->setShaderVariable(platform::ShaderVariable(Name("view_matrix"), view));
//      world_->getRenderer()->setShaderVariable(platform::ShaderVariable(Name("inverse_view_matrix"), glm::inverse(view)));
//
//      world_->getRenderer()->setShaderVariable(platform::ShaderVariable(Name("projection_matrix"), projection));
//      world_->getRenderer()->setShaderVariable(platform::ShaderVariable(Name("inverse_projection_matrix"), glm::inverse(projection)));
//
//      world_->getRenderer()->setShaderVariable(platform::ShaderVariable(Name("view_projection_matrix"), projection * view));
//      world_->getRenderer()->setShaderVariable(platform::ShaderVariable(Name("camera_position"), transform.getWorldTranslation()));
//
//      world_->getRenderer()->setShaderVariable(platform::ShaderVariable(Name("camera_near"), data.near_plane.asMeter()));
//      world_->getRenderer()->setShaderVariable(platform::ShaderVariable(Name("camera_far"), data.far_plane.asMeter()));
//    }
//  }
//}
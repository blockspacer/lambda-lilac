//#pragma once
//#include <interfaces/irenderer.h>
//#include <systems/transform_system.h>
//#include <systems/mesh_render_system.h>
//#include <systems/camera_system.h>
//#include <systems/light_system.h>
//
//namespace lambda
//{
//  namespace platform
//  {
//    template<class T>
//    struct SystemData
//    {
//      Vector<T> data;
//      Map<uint64_t, uint32_t> entity_to_data;
//    };
//
//    struct IRenderFlushData
//    {
//      SystemData<components::TransformData> transforms;
//      SystemData<components::MeshRenderData> mesh_renders;
//      SystemData<components::CameraData> cameras;
//      SystemData<components::LightData> lights;
//      entity::Entity main_camera;
//
//      struct ShaderPass
//      {
//        asset::VioletShaderHandle         shader_generate;
//        Vector<asset::VioletShaderHandle> shader_modify;
//        asset::VioletShaderHandle         shader_publish;
//      };
//      ShaderPass shaders_directional;
//      ShaderPass shaders_directional_rsm;
//      ShaderPass shaders_spot;
//      ShaderPass shaders_spot_rsm;
//      ShaderPass shaders_point;
//      ShaderPass shaders_cascade;
//    };
//
//    class IRenderFlush : public IRenderer
//    {
//    public:
//      void flush(IRenderFlushData flush);
//
//    private:
//      void flushLights(IRenderFlushData flush);
//      void flushCameras(IRenderFlushData flush);
//      void flushPostProcessing(IRenderFlushData flush);
//      void flushUi(IRenderFlushData flush);
//      void renderAll(IRenderFlushData flush);
//      void bindCamera(IRenderFlushData flush, entity::Entity entity);
//    };
//  }
//}
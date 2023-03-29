#include "engine/post_process.as"
#include "engine/ini_reader.as"
#include "engine/graphics.as"

class Settings
{
  bool  lighting_enabled;
  bool  ssao_enabled;
  Vec2  ssao_blur_scale;
  Vec2  ssao_blur_passes;
  float ssao_rt_scale;
  bool  skydome_enabled;
  bool  bloom_enabled;
  Vec2  bloom_blur_scale;
  Vec2  bloom_blur_passes;
  float bloom_rt_scale;
  bool  tone_mapping_enabled;
  bool  fxaa_enabled;
  bool  dof_enabled;
  Vec2  dof_blur_scale;
  Vec2  dof_blur_passes;
  float dof_rt_scale;
  bool  water_enabled;
}

class PostProcesser
{
  void InitializePostProcessing()
  {
    Utility::IniReader ini_reader("resources/settings.ini");

    Settings settings;
    settings.lighting_enabled     = ini_reader.GetBool ("Lighting",    "Enabled");
    settings.ssao_enabled         = ini_reader.GetBool ("SSAO",        "Enabled");
    settings.ssao_blur_scale.x    = ini_reader.GetFloat("SSAO",        "BlurScaleX");
    settings.ssao_blur_scale.y    = ini_reader.GetFloat("SSAO",        "BlurScaleY");
    settings.ssao_blur_passes.x   = ini_reader.GetFloat("SSAO",        "BlurPassesX");
    settings.ssao_blur_passes.y   = ini_reader.GetFloat("SSAO",        "BlurPassesY");
    settings.ssao_rt_scale        = ini_reader.GetFloat("SSAO",        "TargetScale");
    settings.skydome_enabled      = ini_reader.GetBool ("Skydome",     "Enabled");
    settings.bloom_enabled        = ini_reader.GetBool ("Bloom",       "Enabled");
    settings.bloom_blur_scale.x   = ini_reader.GetFloat("Bloom",       "BlurScaleX");
    settings.bloom_blur_scale.y   = ini_reader.GetFloat("Bloom",       "BlurScaleY");
    settings.bloom_blur_passes.x  = ini_reader.GetFloat("Bloom",       "BlurPassesX");
    settings.bloom_blur_passes.y  = ini_reader.GetFloat("Bloom",       "BlurPassesY");
    settings.bloom_rt_scale       = ini_reader.GetFloat("Bloom",       "TargetScale");
    settings.tone_mapping_enabled = ini_reader.GetBool ("ToneMapping", "Enabled");
    settings.fxaa_enabled         = ini_reader.GetBool ("FXAA",        "Enabled");
    settings.dof_enabled          = ini_reader.GetBool ("DOF",         "Enabled");
    settings.dof_blur_scale.x     = ini_reader.GetFloat("DOF",         "BlurScaleX");
    settings.dof_blur_scale.y     = ini_reader.GetFloat("DOF",         "BlurScaleY");
    settings.dof_blur_passes.x    = ini_reader.GetFloat("DOF",         "BlurPassesX");
    settings.dof_blur_passes.y    = ini_reader.GetFloat("DOF",         "BlurPassesY");
    settings.dof_rt_scale         = ini_reader.GetFloat("DOF",         "TargetScale");
    settings.water_enabled        = ini_reader.GetBool ("Water",       "Enabled");

    AddRenderTarget("albedo",              1.0f, Asset::TextureFormat::kR8G8B8A8);
    AddRenderTarget("position",            1.0f, Asset::TextureFormat::kR32G32B32A32);
    AddRenderTarget("normal",              1.0f, Asset::TextureFormat::kR8G8B8A8);
    AddRenderTarget("metallic_roughness",  1.0f, Asset::TextureFormat::kR8G8B8A8);
    AddRenderTarget("depth_buffer",        1.0f, Asset::TextureFormat::kR24G8);
    AddRenderTarget("post_process_buffer", 1.0f, Asset::TextureFormat::kR32G32B32A32);
    
    AddRenderTarget("brdf_lut", asset_manager.LoadTexture("resources/textures/ibl_brdf_lut.png"));
    AddRenderTarget("environment_map", asset_manager.LoadTexture("resources/textures/bell_park_dawn.hdr"));
    //AddRenderTarget("irradiance_map", asset_manager.LoadTexture("resources/textures/bell_park_dawn.hdr"));
    //AddRenderTarget("prefiltered", asset_manager.LoadTexture("resources/textures/bell_park_dawn.hdr"));
    IrradianceConvolution("environment_map", "irradiance_map");
    Hammerhead("environment_map", "prefiltered");

    // Create all passes.
    CopyAlbedoToPostProcessBuffer();
    ShadowMapping(settings.lighting_enabled);
    ApplyLighting(settings.lighting_enabled);
    SkyDome(settings.skydome_enabled);
    Bloom(settings.bloom_enabled, settings.bloom_blur_scale, settings.bloom_blur_passes, settings.bloom_rt_scale);
    ToneMapping(settings.tone_mapping_enabled);
    FXAA(settings.fxaa_enabled);   
    DOF(settings.dof_enabled, settings.dof_blur_scale, settings.dof_blur_passes, settings.dof_rt_scale);
    Water(settings.water_enabled);
    SSAO(settings.ssao_enabled, settings.ssao_blur_scale, settings.ssao_blur_passes, settings.ssao_rt_scale);

    // Final pass.
    SetFinalRenderTarget("post_process_buffer");
  }

  private void CopyAlbedoToPostProcessBuffer()
  {
    Asset::Shader shader = asset_manager.LoadShader("resources/shaders/copy.fx");
    Array<String> input  = { "albedo" };
    Array<String> output = { "post_process_buffer" };
    AddShaderPass("copy_albedo_to_post_process_buffer", shader, input, output);
  }

  private void ShadowMapping(const bool&in enabled)
  {
    if(!enabled) return;
    
    // Get all shared shaders.
    Asset::Shader generate = asset_manager.LoadShader("resources/shaders/vsm_generate.fx");
    Asset::Shader blur_x = asset_manager.LoadShader("resources/shaders/vsm_blur_7x1.fx");
    Asset::Shader blur_y = asset_manager.LoadShader("resources/shaders/vsm_blur_7x1.fx");
    blur_x.SetVariable("blur_scale", Vec2(1.0f, 0.0f));
    blur_y.SetVariable("blur_scale", Vec2(0.0f, 1.0f));
    Array<Asset::Shader> modify = { blur_x, blur_y };


    // Set all lighting shaders.
    graphics.SetDirectionalLightShaders(generate, modify, asset_manager.LoadShader("resources/shaders/vsm_publish_directional.fx"));
    graphics.SetPointLightShaders(      generate, modify, asset_manager.LoadShader("resources/shaders/vsm_publish_point.fx"));
    graphics.SetSpotLightShaders(       generate, modify, asset_manager.LoadShader("resources/shaders/vsm_publish_spot.fx"));
    graphics.SetCascadeLightShaders(    generate, modify, asset_manager.LoadShader("resources/shaders/vsm_publish_cascade.fx"));

    // Set RSM shaders.
    // I'm actually using Reflective Variance Shadow Maps.
    Asset::Shader rsm_generate = asset_manager.LoadShader("resources/shaders/rvsm_generate.fx");
    graphics.SetDirectionalLightRSMShaders(rsm_generate, modify, asset_manager.LoadShader("resources/shaders/rvsm_publish_directional.fx"));
    graphics.SetSpotLightRSMShaders(rsm_generate, modify, asset_manager.LoadShader("resources/shaders/rvsm_publish_spot.fx"));    
  }
  private void ApplyLighting(const bool&in enabled)
  {
    if(!enabled) return;
    
    Asset::Shader shader = asset_manager.LoadShader("resources/shaders/apply_lighting.fx");
    Array<String> input  = { "post_process_buffer", "position", "normal", "metallic_roughness", "light_map", "irradiance_map", "prefiltered", "brdf_lut" };
    Array<String> output = { "post_process_buffer" };
    AddShaderPass("apply_lighting", shader, input, output);
  }
  private void SSAO(const bool&in enabled, const Vec2&in blur_scale, const Vec2&in blur_passes, const float&in render_target_scale)
  {
    if(!enabled) return;
  	
    // Add the required render targets.
    AddRenderTarget("ssao_target", render_target_scale, Asset::TextureFormat::kR32G32B32A32);
    AddRenderTarget("random_texture", asset_manager.LoadTexture("resources/textures/noise.png"));
    

    // Add the main SSAO pass.
    Asset::Shader shader_ssao = asset_manager.LoadShader("resources/shaders/ssao.fx");
    Array<String> input_ssao  = { "position", "normal", "random_texture", "depth_buffer" };
    Array<String> output_ssao = { "ssao_target" };
    AddShaderPass("ssao", shader_ssao, input_ssao, output_ssao);

    Asset::Shader shader_blur_x = asset_manager.LoadShader("resources/shaders/blur_9x1.fx");
    Asset::Shader shader_blur_y = asset_manager.LoadShader("resources/shaders/blur_9x1.fx");
    shader_blur_x.SetVariable("blur_scale", Vec2(blur_scale.x, 0.0f));
    shader_blur_y.SetVariable("blur_scale", Vec2(0.0f, blur_scale.y));
    
    // Horizontal blur.
    for(uint8 i = 0; i < uint8(blur_passes.x) && blur_scale.x > 0.0f; ++i)
    {
      Array<String> input_blur  = { "ssao_target" };
      Array<String> output_blur = { "ssao_target" };
      AddShaderPass("ssao_blur_x" + i, shader_blur_x, input_blur, output_blur);
    }


    // Vertical blur.
    for(uint8 i = 0; i < uint8(blur_passes.y) && blur_scale.y > 0.0f; ++i)
    {
      Array<String> input_blur  = { "ssao_target" };
      Array<String> output_blur = { "ssao_target" };
      AddShaderPass("ssao_blur_y" + i, shader_blur_y, input_blur, output_blur);
    }


    // Apply SSAO.
    Asset::Shader shader_apply = asset_manager.LoadShader("resources/shaders/ssao_apply.fx");
    Array<String> input_apply  = { "post_process_buffer", "ssao_target" };
    Array<String> output_apply = { "post_process_buffer" };
    AddShaderPass("ssao_apply", shader_apply, input_apply, output_apply);
  }

  private void SkyDome(const bool&in enabled)
  {
    if(!enabled) return;

    Asset::Shader shader = asset_manager.LoadShader("resources/shaders/skydome.fx");
    Array<String> input  = { "post_process_buffer", "position", "environment_map" };
    Array<String> output = { "post_process_buffer", "position" };
    AddShaderPass("skydome", shader, input, output);
  }
  private void Bloom(const bool&in enabled, const Vec2&in blur_scale, const Vec2&in blur_passes, const float&in render_target_scale)
  {
    if(!enabled) return;

    // Add the render target.
    AddRenderTarget("bloom_target", render_target_scale, Asset::TextureFormat::kR32G32B32A32);


    // Bloom extract.
    Asset::Shader shader_extract = asset_manager.LoadShader("resources/shaders/bloom_extract.fx");
    Array<String> input_extract  = { "post_process_buffer" };
    Array<String> output_extract = { "bloom_target" };
    AddShaderPass("bloom_extract", shader_extract, input_extract, output_extract);


    // Bloom blur horizontal.
    Asset::Shader shader_blur_x  = asset_manager.LoadShader("resources/shaders/blur_9x1.fx");

    for (uint8 i = 0; i < uint8(blur_passes.x) && blur_scale.x > 0.0f; ++i)
    {
      Array<String> input  = { "bloom_target" };
      Array<String> output = { "bloom_target" };
      AddShaderPass("bloom_blur_x" + i, shader_blur_x, input, output);
    }
    shader_blur_x.SetVariable("blur_scale", Vec2(blur_scale.x, 0.0f));


    // Bloom blur vertical.
    Asset::Shader shader_blur_y  = asset_manager.LoadShader("resources/shaders/blur_9x1.fx");
    
    for (uint8 i = 0; i < uint8(blur_passes.y) && blur_scale.y > 0.0f; ++i)
    {
      Array<String> input  = { "bloom_target" };
      Array<String> output = { "bloom_target" };
      AddShaderPass("bloom_blur_y" + i, shader_blur_y, input, output);
    }
    shader_blur_y.SetVariable("blur_scale", Vec2(0.0f, blur_scale.y));


    // Bloom apply.
    Asset::Shader shader_apply = asset_manager.LoadShader("resources/shaders/bloom_apply.fx");
    Array<String> input_apply  = { "post_process_buffer", "bloom_target" };
    Array<String> output_apply = { "post_process_buffer" };
    AddShaderPass("bloom_apply", shader_apply, input_apply, output_apply);
  }
  private void ToneMapping(const bool&in enabled)
  {
    if(!enabled) return;

    Asset::Shader shader = asset_manager.LoadShader("resources/shaders/tone_mapping.fx");
    Array<String> input  = { "post_process_buffer" };
    Array<String> output = { "post_process_buffer" };
    AddShaderPass("tone_mapping", shader, input, output);
  }
  private void FXAA(const bool&in enabled)
  {
    if(!enabled) return;

    Asset::Shader shader = asset_manager.LoadShader("resources/shaders/fxaa.fx");
    Array<String> input  = { "post_process_buffer" };
    Array<String> output = { "post_process_buffer" };
    AddShaderPass("fxaa", shader, input, output);
  }
  private void DOF(const bool&in enabled, const Vec2&in blur_scale, const Vec2&in blur_passes, const float&in render_target_scale)
  {
    if(!enabled) return;

    // Add required render targets.
    AddRenderTarget("dof_target", render_target_scale, Asset::TextureFormat::kR32G32B32A32);
    AddRenderTarget("dof_pos", asset_manager.CreateTexture(Vec2(1.0f), Array<uint8>(16, 0), Asset::TextureFormat::kR32G32B32A32));


    // Blur horizontal.
    Asset::Shader shader_blur_x = asset_manager.LoadShader("resources/shaders/blur_7x1.fx");
    for(uint8 i = 0; i < uint8(blur_passes.x) && blur_scale.x > 0.0f; ++i)
    {
      Array<String> input  = { (i == 0 ? "post_process_buffer" : "dof_target") };
      Array<String> output = { "dof_target" };
      AddShaderPass("dof_blur_x" + i, shader_blur_x, input, output);
    }
    shader_blur_x.SetVariable("blur_scale", Vec2(blur_scale.x, 0.0f));


    // Blur vertical.
    Asset::Shader shader_blur_y = asset_manager.LoadShader("resources/shaders/blur_7x1.fx");
    for(uint8 i = 0; i < uint8(blur_passes.y) && blur_scale.y > 0.0f; ++i)
    {
      Array<String> input  = { "dof_target" };
      Array<String> output = { "dof_target" };
      AddShaderPass("dof_blur_y" + i, shader_blur_y, input, output);
    }
    shader_blur_y.SetVariable("blur_scale", Vec2(blur_scale.y, 0.0f));


    // Position update.
    Asset::Shader shader_pos = asset_manager.LoadShader("resources/shaders/dof_pos.fx");
    Array<String> input_pos  = { "dof_pos", "position" };
    Array<String> output_pos = { "dof_pos" };
    AddShaderPass("dof_pos", shader_pos, input_pos, output_pos);


    // Apply.
    Asset::Shader shader_apply = asset_manager.LoadShader("resources/shaders/dof.fx");
    Array<String> input_apply  = { "post_process_buffer", "dof_target", "dof_pos", "position" };
    Array<String> output_apply = { "post_process_buffer" };
    AddShaderPass("dof_apply", shader_apply, input_apply, output_apply);
  }
  private void Water(const bool&in enabled)
  {
    if(!enabled) return;

    Asset::Shader shader = asset_manager.LoadShader("resources/shaders/water.fx");
    Array<String> input  = { "post_process_buffer" };
    Array<String> output = { "post_process_buffer" };
    AddShaderPass("water", shader, input, output);
  }
}
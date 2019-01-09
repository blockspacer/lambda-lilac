#include "engine/asset_manager.as"

class Lighting
{
  void InitializeLighting()
  {
    //Sun();
    ReflectiveShadowMap();
    //FlashLight();
  }

  Entity& GetSunFront()
  {
    return sun_front;
  }

  Entity& GetSunBack()
  {
    return sun_back;
  }

  Entity& GetFlashLight()
  {
    return flash_light;
  }

  Entity& GetRSM()
  {
    return rsm;
  }

  private void Sun()
  {
    //--------------------SUN-FRONT--------------------

    // Create all required things.
    AddRenderTarget("sun_front_rt1", asset_manager.CreateTexture(Vec2(512.0f/4),  Asset::TextureFormat::kR32G32B32A32));
    AddRenderTarget("sun_front_rt2", asset_manager.CreateTexture(Vec2(1024.0f/4), Asset::TextureFormat::kR32G32B32A32));
    AddRenderTarget("sun_front_rt3", asset_manager.CreateTexture(Vec2(2048.0f/4), Asset::TextureFormat::kR32G32B32A32));
    sun_front.Create();

    // Add a transform.
    Transform@ trans_sun_front;
    sun_front.AddComponent(@trans_sun_front);

    // Add a cascaded light.
    CascadeLight@ light_sun_front;
    sun_front.AddComponent(@light_sun_front);
    //light_sun_front.MakeRSM();
    light_sun_front.SetLightColour(Colour(RGB(1.0f, 1.0f, 1.0f)));
    //light_sun_front.SetAmbientColour(Colour(RGB(0.2f, 0.2f, 0.3f)));
    light_sun_front.SetLightIntensity(7.0f);
    light_sun_front.SetShadowType(ShadowType::kDynamic);
    light_sun_front.SetDynamicFrequency(25);
    Array<String> render_targets = { "sun_front_rt1", "sun_front_rt2", "sun_front_rt3" };
    light_sun_front.SetRenderTargets(render_targets);

    //--------------------SUN-BACK--------------------

    return;

    // Create all required things.
    sun_back.Create();

    // Add a transform.
    Transform@ trans_sun_back;
    sun_back.AddComponent(@trans_sun_back);

    // Add a directional light.
    DirectionalLight@ light_sun_back;
    sun_back.AddComponent(@light_sun_back);
    light_sun_back.SetLightColour(Colour(RGB(0.2f, 0.2f, 0.25f)));

    light_sun_front.SetEnabled(false);
    light_sun_back.SetEnabled(false);
  }

  private void FlashLight()
  {
    //--------------------FLASH-LIGHT--------------------

    // Create all required things.
    Asset::Texture flash_light_texture = asset_manager.LoadTexture("resources/textures/flashlight.png");
    AddRenderTarget("flash_light_rt", asset_manager.CreateTexture(Vec2(512.0f), Asset::TextureFormat::kR32G32B32A32));
    flash_light.Create();

    // Add a transform.
    Transform@ trans_flash_light;
    flash_light.AddComponent(@trans_flash_light);
    trans_flash_light.SetWorldRotationEuler(Vec3(0.0f, DegToRad * 90.0f, 0.0f));
    trans_flash_light.SetWorldPosition(Vec3(7.0f, 2.0f, 0.0f));

    // Add a spot light.
    SpotLight@ light_flash_light;
    flash_light.AddComponent(@light_flash_light);
    //light_flash_light.MakeRSM();
    Array<String> render_targets = { "rsm_shad", "rsm_posi", "rsm_norm", "rsm_flux" };
    light_flash_light.SetRenderTargets(render_targets);
    light_flash_light.SetLightColour(Colour(RGB(1.0f, 1.0f, 1.0f)));
    light_flash_light.SetLightIntensity(20.0f);
    light_flash_light.SetDepth(100.0f);
    light_flash_light.SetInnerCutOff(Utility::AngleFromDeg(30.0f));
    light_flash_light.SetOuterCutOff(Utility::AngleFromDeg(50.0f));
    light_flash_light.SetShadowType(ShadowType::kDynamic);
    //light_flash_light.SetRenderTarget("flash_light_rt");
    light_flash_light.SetTexture(flash_light_texture);
    light_flash_light.SetEnabled(false);


    // Add a sound source.
    WaveSource@ wave_source;
    flash_light.AddComponent(@wave_source);
    wave_source.SetBuffer(asset_manager.LoadWave("resources/waves/light_switch.wav"));
    wave_source.SetRelativeToListener(false);
  }

  private void ReflectiveShadowMap()
  {
    rsm.Create();

    // Add a transform.
    Transform@ trans_rsm;
    rsm.AddComponent(@trans_rsm);
    trans_rsm.SetWorldRotation(LookRotation(Vec3(0.0f, 1.0f, 1.0f).Normalized(), Vec3(0.0f, 1.0f, 0.0f)));

    AddRenderTarget("rsm_shad", asset_manager.CreateTexture(Vec2(2048.0f), Asset::TextureFormat::kR16G16B16A16));
    AddRenderTarget("rsm_posi", asset_manager.CreateTexture(Vec2(2048.0f), Asset::TextureFormat::kR16G16B16A16));
    AddRenderTarget("rsm_norm", asset_manager.CreateTexture(Vec2(2048.0f), Asset::TextureFormat::kR8G8B8A8));
    AddRenderTarget("rsm_flux", asset_manager.CreateTexture(Vec2(2048.0f), Asset::TextureFormat::kR16G16B16A16));

    // Add a RSM light.
    DirectionalLight@ light_rsm;
    rsm.AddComponent(@light_rsm);
    //light_rsm.MakeRSM();
    light_rsm.SetLightColour(Colour(RGB(1.0f, 1.0f, 1.0f)));
    light_rsm.SetLightIntensity(7.0f);
    light_rsm.SetShadowType(ShadowType::kDynamic);
    Array<String> render_targets = { "rsm_shad" };
    light_rsm.SetRenderTargets(render_targets);
    light_rsm.SetDepth(200.0f);
    light_rsm.SetSize(100.0f);
    //light_rsm.SetEnabled(false);
  }

  private Entity sun_front;
  private Entity sun_back;
  private Entity flash_light;
  private Entity rsm;
}

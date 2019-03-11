SET(AssetsSources
  "assets/asset.h"
  "assets/asset.cc"
  "assets/asset_handle.h"
  "assets/asset_manager.h"
  "assets/asset_manager.cc"
  "assets/mesh.h"
  "assets/mesh.cc"
  "assets/mesh_io.h"
  "assets/mesh_io.cc"
  "assets/shader.h"
  "assets/shader.cc"
  "assets/shader_program.h"
  "assets/shader_program.cc"
  "assets/shader_pass.h"
  "assets/shader_pass.cc"
  "assets/shader_io.h"
  "assets/shader_io.cc"
  "assets/texture.h"
  "assets/texture.cc"
  "assets/wave.h"
	"assets/wave.cc"
)
SET(NuklearImGuiSources
  "imgui/nuklear_imgui.h"
  "imgui/nuklear_imgui.cc"
)
SET(DearImGuiSources
  "imgui/dear_imgui.h"
  "imgui/dear_imgui.cc"
)
SET(NoImGuiSources
  "imgui/no_imgui.h"
)
SET(InputSources
  "input/controller.h"
  "input/controller.cc"
  "input/input.h"
  "input/input_manager.h"
  "input/keyboard.h"
  "input/mouse.h"
)
SET(UltralightGuiSources
  "gui/gui.h"
  "gui/gui.cc"
  "gui/file_system.h"
  "gui/file_system.cc"
  "gui/font_handler.h"
  "gui/font_handler.cc"
  "gui/gpu_driver.h"
  "gui/gpu_driver.cc"
  "gui/gui_value.h"
  "gui/gui_value.cc"
)
SET(InterfacesSources
  "interfaces/icomponent.h"
  "interfaces/icomponent.cc"
  "interfaces/iimgui.h"
  "interfaces/iphysics.h"
  "interfaces/irenderer.h"
  "interfaces/irenderer_flush.h"
  "interfaces/irenderer_flush.cc"
  "interfaces/iscript_context.h"
  "interfaces/isystem.h"
  "interfaces/iwindow.h"
  "interfaces/iworld.h"
  "interfaces/iworld.cc"
)
SET(PhysicsBulletSources
  "physics/bullet/physics_visualizer.h"
  "physics/bullet/physics_visualizer.cc"
  "physics/bullet/physics_world.h"
  "physics/bullet/physics_world.cc"
)
SET(PhysicsReactSources
  "physics/react/physics_visualizer.h"
  "physics/react/physics_visualizer.cc"
  "physics/react/physics_world.h"
  "physics/react/physics_world.cc"
)
SET(PlatformSources
  "platform/blend_state.h"
  "platform/depth_stencil_state.h"
  "platform/culling.h"
  "platform/culling.cc"
  "platform/debug_renderer.h"
  "platform/debug_renderer.cc"
  "platform/frustum.h"
  "platform/frustum.cc"
  "platform/post_process_manager.h"
  "platform/post_process_manager.cc"
  "platform/rasterizer_state.h"
  "platform/render_target.h"
  "platform/sampler_state.h"
  "platform/scene.h"
  "platform/scene.cc"
  "platform/shader_pass.h"
  "platform/shader_variable.h"
  "platform/shader_variable_manager.h"
  "platform/shader_variable_manager.cc"
)
SET(D3D11RendererSources
  "renderers/d3d11/d3d11_context.h"
  "renderers/d3d11/d3d11_context.cc"
  "renderers/d3d11/d3d11_default_shader.h"
  "renderers/d3d11/d3d11_mesh.h"
  "renderers/d3d11/d3d11_mesh.cc"
  "renderers/d3d11/d3d11_mip_map_generator.h"
  "renderers/d3d11/d3d11_renderer.h"
  "renderers/d3d11/d3d11_renderer.cc"
  "renderers/d3d11/d3d11_shader.h"
  "renderers/d3d11/d3d11_shader.cc"
  "renderers/d3d11/d3d11_state_manager.h"
  "renderers/d3d11/d3d11_state_manager.cc"
  "renderers/d3d11/d3d11_texture.h"
  "renderers/d3d11/d3d11_texture.cc"
)
SET(NoRendererSources
  "renderers/no/no_renderer.h"
  "renderers/no/no_renderer.cc"
)
SET(MetalRendererSources
  "renderers/metal/metal_renderer.h"
  "renderers/metal/metal_renderer.cc"
  "renderers/metal/metal_cpp.h"
  "renderers/metal/metal_cpp.mm"
  "renderers/metal/metal_window.h"
	"renderers/metal/metal_window.mm"
	"renderers/metal/shader.metal"
)
SET(ScriptingSources
  "scripting/script_class.h"
  "scripting/script_function.h"
  "scripting/script_math.h"
  "scripting/script_math.cc"
  "scripting/script_noise.h"
  "scripting/script_noise.cc"
  "scripting/script_value.h"
  "scripting/script_vector.h"
  "scripting/script_vector.cc"
)
SET(ScriptingBindingBindingSources
  "scripting/binding/script_binding.h"
  "scripting/binding/script_binding.cc"
)
SET(ScriptingBindingAssetsSources
  "scripting/binding/assets/mesh.h"
  "scripting/binding/assets/mesh.cc"
  "scripting/binding/assets/shader.h"
  "scripting/binding/assets/shader.cc"
  "scripting/binding/assets/texture.h"
  "scripting/binding/assets/texture.cc"
  "scripting/binding/assets/wave.h"
  "scripting/binding/assets/wave.cc"
)
SET(ScriptingBindingComponentsSources
  "scripting/binding/components/camera.h"
  "scripting/binding/components/camera.cc"
  "scripting/binding/components/collider.h"
  "scripting/binding/components/collider.cc"
  "scripting/binding/components/entity.h"
  "scripting/binding/components/entity.cc"
  "scripting/binding/components/light.h"
  "scripting/binding/components/light.cc"
  "scripting/binding/components/lod.h"
  "scripting/binding/components/lod.cc"
  "scripting/binding/components/mesh.h"
  "scripting/binding/components/mesh.cc"
  "scripting/binding/components/rigid_body.h"
  "scripting/binding/components/rigid_body.cc"
  "scripting/binding/components/transform.h"
  "scripting/binding/components/transform.cc"
  "scripting/binding/components/wave_source.h"
  "scripting/binding/components/wave_source.cc"
)
SET(ScriptingBindingGraphicsSources
  "scripting/binding/graphics/globals.h"
  "scripting/binding/graphics/globals.cc"
  "scripting/binding/graphics/renderer.h"
  "scripting/binding/graphics/renderer.cc"
)
SET(ScriptingBindingInputSources
  "scripting/binding/input/controller.h"
  "scripting/binding/input/controller.cc"
  "scripting/binding/input/keyboard.h"
  "scripting/binding/input/keyboard.cc"
  "scripting/binding/input/mouse.h"
  "scripting/binding/input/mouse.cc"
)
SET(ScriptingBindingMathSources
  "scripting/binding/math/quat.h"
  "scripting/binding/math/quat.cc"
  "scripting/binding/math/vec2.h"
  "scripting/binding/math/vec2.cc"
  "scripting/binding/math/vec3.h"
  "scripting/binding/math/vec3.cc"
  "scripting/binding/math/vec4.h"
  "scripting/binding/math/vec4.cc"
)
SET(ScriptingBindingUtilitiesSources
  "scripting/binding/utilities/ini.h"
  "scripting/binding/utilities/ini.cc"
  "scripting/binding/utilities/render_target.h"
  "scripting/binding/utilities/render_target.cc"
  "scripting/binding/utilities/shader_pass.h"
  "scripting/binding/utilities/shader_pass.cc"
  "scripting/binding/utilities/utilities.h"
  "scripting/binding/utilities/utilities.cc"
)
SET(ScriptingBindingSources
  ${ScriptingBindingBindingSources}
  ${ScriptingBindingAssetsSources}
  ${ScriptingBindingComponentsSources}
  ${ScriptingBindingGraphicsSources}
  ${ScriptingBindingInputSources}
  ${ScriptingBindingMathSources}
  ${ScriptingBindingUtilitiesSources}
)
SET(AngelScriptSources
  "scripting/angel-script/angel_script_context.h"
  "scripting/angel-script/angel_script_context.cc"
  "scripting/angel-script/angel_script_entity.h"
  "scripting/angel-script/angel_script_entity.cc"
)
SET(AngelScriptAddonsSources
  "scripting/angel-script/addons/datetime.h"
  "scripting/angel-script/addons/datetime.cpp"
  "scripting/angel-script/addons/debugger.h"
  "scripting/angel-script/addons/debugger.cpp"
  "scripting/angel-script/addons/scriptany.h"
  "scripting/angel-script/addons/scriptany.cpp"
  "scripting/angel-script/addons/scriptarray.h"
  "scripting/angel-script/addons/scriptarray.cpp"
  "scripting/angel-script/addons/scriptbuilder.h"
  "scripting/angel-script/addons/scriptbuilder.cpp"
  "scripting/angel-script/addons/scriptdictionary.h"
  "scripting/angel-script/addons/scriptdictionary.cpp"
  "scripting/angel-script/addons/scriptfile.h"
  "scripting/angel-script/addons/scriptfile.cpp"
  "scripting/angel-script/addons/scriptfilesystem.h"
  "scripting/angel-script/addons/scriptfilesystem.cpp"
  "scripting/angel-script/addons/scriptgrid.h"
  "scripting/angel-script/addons/scriptgrid.cpp"
  "scripting/angel-script/addons/scripthandle.h"
  "scripting/angel-script/addons/scripthandle.cpp"
  "scripting/angel-script/addons/scripthelper.h"
  "scripting/angel-script/addons/scripthelper.cpp"
  "scripting/angel-script/addons/scriptlmbstring.h"
  "scripting/angel-script/addons/scriptlmbstring.cpp"
  "scripting/angel-script/addons/scriptlmbstring_utils.cpp"
  "scripting/angel-script/addons/scriptmath.h"
  "scripting/angel-script/addons/scriptmath.cpp"
  "scripting/angel-script/addons/scriptmathcomplex.h"
  "scripting/angel-script/addons/scriptmathcomplex.cpp"
  "scripting/angel-script/addons/serializer.h"
  "scripting/angel-script/addons/serializer.cpp"
  "scripting/angel-script/addons/weakref.h"
  "scripting/angel-script/addons/weakref.cpp"
)
SET(WrenSources
  "scripting/wren/wren_binding.h"
  "scripting/wren/wren_binding.cc"
  "scripting/wren/wren_context.h"
  "scripting/wren/wren_context.cc"
  "scripting/wren/wren_entity.h"
  "scripting/wren/wren_entity.cc"
)
SET(SystemsSources
  "systems/camera_system.h"
  "systems/camera_system.cc"
  "systems/collider_system.h"
  "systems/collider_system.cc"
  "systems/entity.h"
  "systems/entity.cc"
  "systems/entity_system.h"
  "systems/entity_system.cc"
  "systems/light_system.h"
  "systems/light_system.cc"
  "systems/lod_system.h"
  "systems/lod_system.cc"
  "systems/mono_behaviour_system.h"
  "systems/mono_behaviour_system.cc"
  "systems/mesh_render_system.h"
  "systems/mesh_render_system.cc"
  "systems/name_system.h"
  "systems/name_system.cc"
  "systems/rigid_body_system.h"
  "systems/rigid_body_system.cc"
  "systems/skeleton_system.h"
  "systems/skeleton_system.cc"
  "systems/transform_system.h"
  "systems/transform_system.cc"
  "systems/wave_source_system.h"
  "systems/wave_source_system.cc"
)
SET(UtilsSources
  "utils/angle.h"
  "utils/decompose_matrix.h"
  "utils/distance.h"
  "utils/mesh_decimator.h"
  "utils/mesh_decimator.cc"
  "utils/mt_manager.h"
  "utils/mt_manager.cc"
  "utils/name.h"
  "utils/renderable.h"
  "utils/zone_manager.h"
  "utils/zone_manager.cc"
)
SET(WindowGLFWSources
  "windows/glfw/glfw_window.h"
  "windows/glfw/glfw_window.cc"
)
SET(WindowSDL2Sources
  "windows/sdl2/sdl2_window.h"
  "windows/sdl2/sdl2_window.cc"
)
SET(WindowWin32Sources
  "windows/win32/win32_window.h"
  "windows/win32/win32_window.cc"
)
SET(MainSources
  "main.cc"
)

SOURCE_GROUP("assets" FILES ${AssetsSources})
SOURCE_GROUP("audio" FILES ${AudioSources})
SOURCE_GROUP("imgui\\imgui" FILES ${ImGuiSources})
SOURCE_GROUP("imgui\\nuklear" FILES ${NuklearImGuiSources})
SOURCE_GROUP("imgui\\no" FILES ${NoImGuiSources})
SOURCE_GROUP("input" FILES ${InputSources})
SOURCE_GROUP("gui\\gui" FILES ${UltralightGuiSources})
SOURCE_GROUP("interfaces" FILES ${InterfacesSources})
SOURCE_GROUP("physics\\bullet" FILES ${PhysicsBulletSources})
SOURCE_GROUP("physics\\react" FILES ${PhysicsReactSources})
SOURCE_GROUP("platform" FILES ${PlatformSources})
SOURCE_GROUP("renderers\\d3d11" FILES ${D3D11RendererSources})
SOURCE_GROUP("renderers\\metal" FILES ${MetalRendererSources})
SOURCE_GROUP("renderers\\no" FILES ${NoRendererSources})
SOURCE_GROUP("scripting" FILES ${ScriptingSources})
SOURCE_GROUP("scripting\\binding" FILES ${ScriptingBindingBindingSources})
SOURCE_GROUP("scripting\\binding\\Assets" FILES ${ScriptingBindingAssetsSources})
SOURCE_GROUP("scripting\\binding\\Components" FILES ${ScriptingBindingComponentsSources})
SOURCE_GROUP("scripting\\binding\\Graphics" FILES ${ScriptingBindingGraphicsSources})
SOURCE_GROUP("scripting\\binding\\Input" FILES ${ScriptingBindingInputSources})
SOURCE_GROUP("scripting\\binding\\Math" FILES ${ScriptingBindingMathSources})
SOURCE_GROUP("scripting\\binding\\Utilities" FILES ${ScriptingBindingUtilitiesSources})
SOURCE_GROUP("scripting\\angel-script" FILES ${AngelScriptSources})
SOURCE_GROUP("scripting\\angel-script\\addons" FILES ${AngelScriptAddonsSources})
SOURCE_GROUP("scripting\\wren" FILES ${WrenSources})
SOURCE_GROUP("systems" FILES ${SystemsSources})
SOURCE_GROUP("utils" FILES ${UtilsSources})
SOURCE_GROUP("windows\\glfw" FILES ${WindowGLFWSources})
SOURCE_GROUP("windows\\sdl2" FILES ${WindowSDL2Sources})
SOURCE_GROUP("windows\\win32" FILES ${WindowWin32Sources})

SET(Sources
  ${AssetsSources}
  ${AudioSources}
  ${InputSources}
  ${InterfacesSources}
  ${PlatformSources}
  ${ScriptingSources}
  ${ScriptingBindingSources}
  ${SystemsSources}
  ${UtilsSources}
  ${MainSources}
)

# ///////////////////////////////////////////////////////////////
# /// SOURCES ///////////////////////////////////////////////////
# ///////////////////////////////////////////////////////////////
# /// RENDERERS /////////////////////////////////////////////////
IF(${VIOLET_RENDERER_D3D11})
	SET(Sources ${Sources} ${D3D11RendererSources})
ENDIF()
IF(${VIOLET_RENDERER_METAL})
	SET(Sources ${Sources} ${MetalRendererSources})
ENDIF()
IF(${VIOLET_RENDERER_NO})
	SET(Sources ${Sources} ${NoRendererSources})
ENDIF()

# ///////////////////////////////////////////////////////////////
# /// WINDOWS ///////////////////////////////////////////////////
IF(${VIOLET_WINDOW_GLFW})
	SET(Sources ${Sources} ${WindowGLFWSources})
ENDIF()
IF(${VIOLET_WINDOW_SDL2})
	SET(Sources ${Sources} ${WindowSDL2Sources})
ENDIF()
IF(${VIOLET_WINDOW_WIN32})
	SET(Sources ${Sources} ${WindowWin32Sources})
ENDIF()

# ///////////////////////////////////////////////////////////////
# /// SCRIPTING /////////////////////////////////////////////////
IF(${VIOLET_SCRIPTING_ANGEL})
	SET(Sources ${Sources} ${AngelScriptSources} ${AngelScriptAddonsSources})
ENDIF()
IF(${VIOLET_SCRIPTING_WREN})
	SET(Sources ${Sources} ${WrenSources})
ENDIF()

# ///////////////////////////////////////////////////////////////
# /// PHYSICS ///////////////////////////////////////////////////
IF(${VIOLET_PHYSICS_BULLET})
	SET(Sources ${Sources} ${PhysicsBulletSources})
ENDIF()
IF(${VIOLET_PHYSICS_REACT})
	SET(Sources ${Sources} ${PhysicsReactSources})
ENDIF()

# ///////////////////////////////////////////////////////////////
# /// IMGUI /////////////////////////////////////////////////////
IF(${VIOLET_IMGUI_NUKLEAR})
	SET(Sources ${Sources} ${NuklearImGuiSources})
ENDIF()
IF(${VIOLET_IMGUI_DEAR})
	SET(Sources ${Sources} ${DearImGuiSources})
ENDIF()
IF(${VIOLET_IMGUI_NO})
	SET(Sources ${Sources} ${NoImGuiSources})
ENDIF()

# ///////////////////////////////////////////////////////////////
# /// GUI ///////////////////////////////////////////////////////
IF(${VIOLET_GUI_ULTRALIGHT})
	SET(Sources ${Sources} ${UltralightGuiSources})
ENDIF()

# ///////////////////////////////////////////////////////////////
# /// FOUNDATION ////////////////////////////////////////////////
IF(NOT ${VIOLET_CONFIG_FOUNDATION})
	FATAL_ERROR("Engine requires Foundation")
ENDIF()

# ///////////////////////////////////////////////////////////////
# /// CREATE EXE ////////////////////////////////////////////////
# ///////////////////////////////////////////////////////////////
ADD_EXECUTABLE(lambda-engine ${Sources})
TARGET_LINK_LIBRARIES(lambda-engine PUBLIC lambda-foundation fast-noise inih rapidjson mesh-decimation stb tiny-gltf soloud)

# ///////////////////////////////////////////////////////////////
# /// MISC //////////////////////////////////////////////////////
IF(${VIOLET_CONFIG_NETWORKING})
	TARGET_LINK_LIBRARIES(lambda-engine PUBLIC lambda-networking)
ENDIF()

IF(${VIOLET_WIN32})
  TARGET_LINK_LIBRARIES(lambda-engine PUBLIC Shcore.lib Xinput.lib winmm.lib imm32.lib version.lib)
ENDIF()


# ///////////////////////////////////////////////////////////////
# /// GUI ///////////////////////////////////////////////////////
IF(${VIOLET_GUI_ULTRALIGHT})
  TARGET_LINK_LIBRARIES(lambda-engine PUBLIC ultralight)

  IF(VIOLET_WIN32)
    ADD_CUSTOM_COMMAND(TARGET lambda-engine POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/deps/ultralight/bin/win/x64/Ultralight.dll ${CMAKE_BINARY_DIR}/bin/Debug/Ultralight.dll
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/deps/ultralight/bin/win/x64/UltralightCore.dll ${CMAKE_BINARY_DIR}/bin/Debug/UltralightCore.dll
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/deps/ultralight/bin/win/x64/WebCore.dll ${CMAKE_BINARY_DIR}/bin/Debug/WebCore.dll
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/deps/ultralight/bin/win/x64/Ultralight.dll ${CMAKE_BINARY_DIR}/bin/Release/Ultralight.dll
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/deps/ultralight/bin/win/x64/UltralightCore.dll ${CMAKE_BINARY_DIR}/bin/Release/UltralightCore.dll
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/deps/ultralight/bin/win/x64/WebCore.dll ${CMAKE_BINARY_DIR}/bin/Release/WebCore.dll
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/deps/ultralight/bin/win/x64/Ultralight.dll ${CMAKE_BINARY_DIR}/bin/MinSizeRel/Ultralight.dll
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/deps/ultralight/bin/win/x64/UltralightCore.dll ${CMAKE_BINARY_DIR}/bin/MinSizeRel/UltralightCore.dll
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/deps/ultralight/bin/win/x64/WebCore.dll ${CMAKE_BINARY_DIR}/bin/MinSizeRel/WebCore.dll
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/deps/ultralight/bin/win/x64/Ultralight.dll ${CMAKE_BINARY_DIR}/bin/RelWithDebInfo/Ultralight.dll
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/deps/ultralight/bin/win/x64/UltralightCore.dll ${CMAKE_BINARY_DIR}/bin/RelWithDebInfo/UltralightCore.dll
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/deps/ultralight/bin/win/x64/WebCore.dll ${CMAKE_BINARY_DIR}/bin/RelWithDebInfo/WebCore.dll)
  ENDIF()
ENDIF()

TARGET_INCLUDE_DIRECTORIES(lambda-engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# ///////////////////////////////////////////////////////////////
# /// LINK AND DEFINE ///////////////////////////////////////////
# ///////////////////////////////////////////////////////////////
# /// RENDERERS /////////////////////////////////////////////////
IF(${VIOLET_RENDERER_D3D11})
	TARGET_LINK_LIBRARIES(lambda-engine PUBLIC dxgi.lib d3dcompiler.lib d3d11.lib)
	TARGET_COMPILE_DEFINITIONS(lambda-engine PRIVATE VIOLET_RENDERER_D3D11)
ENDIF()
IF(${VIOLET_RENDERER_METAL})
  TARGET_LINK_LIBRARIES(lambda-engine PUBLIC "-framework Metal -framework MetalKit -framework Cocoa -framework AudioToolbox -framework CoreFoundation -framework AppKit -framework QuartzCore")
	TARGET_COMPILE_DEFINITIONS(lambda-engine PRIVATE VIOLET_RENDERER_METAL)
ENDIF()
IF(${VIOLET_RENDERER_NO})
	TARGET_COMPILE_DEFINITIONS(lambda-engine PRIVATE VIOLET_RENDERER_NO)
ENDIF()


# ///////////////////////////////////////////////////////////////
# /// WINDOWS ///////////////////////////////////////////////////
IF(${VIOLET_WINDOW_GLFW})
	TARGET_LINK_LIBRARIES(lambda-engine PUBLIC glfw)
	TARGET_COMPILE_DEFINITIONS(lambda-engine PRIVATE VIOLET_WINDOW_GLFW)
ENDIF()
IF(${VIOLET_WINDOW_SDL2})
	TARGET_LINK_LIBRARIES(lambda-engine PUBLIC sdl)
	TARGET_COMPILE_DEFINITIONS(lambda-engine PRIVATE VIOLET_WINDOW_SDL2)
ENDIF()
IF(${VIOLET_WINDOW_WIN32})
	TARGET_COMPILE_DEFINITIONS(lambda-engine PRIVATE VIOLET_WINDOW_WIN32)
ENDIF()

# ///////////////////////////////////////////////////////////////
# /// SCRIPTING /////////////////////////////////////////////////
IF(${VIOLET_SCRIPTING_ANGEL})
	TARGET_LINK_LIBRARIES(lambda-engine PUBLIC angelscript)
	TARGET_COMPILE_DEFINITIONS(lambda-engine PRIVATE VIOLET_SCRIPTING_ANGEL)
  TARGET_INCLUDE_DIRECTORIES(lambda-engine PUBLIC "../../deps/AngelScript/sdk/angelscript/include")
ENDIF()

IF(${VIOLET_SCRIPTING_WREN})
	TARGET_LINK_LIBRARIES(lambda-engine PUBLIC wren)
	TARGET_COMPILE_DEFINITIONS(lambda-engine PRIVATE VIOLET_SCRIPTING_WREN)
ENDIF()

# ///////////////////////////////////////////////////////////////
# /// IMGUI /////////////////////////////////////////////////////
IF(${VIOLET_IMGUI_NUKLEAR})
	TARGET_LINK_LIBRARIES(lambda-engine PUBLIC nuklear-imgui)
	TARGET_COMPILE_DEFINITIONS(lambda-engine PRIVATE VIOLET_IMGUI_NUKLEAR)
ENDIF()
IF(${VIOLET_IMGUI_DEAR})
  TARGET_LINK_LIBRARIES(lambda-engine PUBLIC dear-imgui)
	TARGET_COMPILE_DEFINITIONS(lambda-engine PRIVATE VIOLET_IMGUI_DEAR)
ENDIF()
IF(${VIOLET_IMGUI_NO})
	TARGET_COMPILE_DEFINITIONS(lambda-engine PRIVATE VIOLET_IMGUI_NO)
ENDIF()

# ///////////////////////////////////////////////////////////////
# /// GUI ///////////////////////////////////////////////////////
IF(${VIOLET_GUI_ULTRALIGHT})
  TARGET_COMPILE_DEFINITIONS(lambda-engine PRIVATE VIOLET_GUI_ULTRALIGHT)
ENDIF()

# ///////////////////////////////////////////////////////////////
# /// PHYSICS ///////////////////////////////////////////////////
IF(${VIOLET_PHYSICS_BULLET})
  TARGET_LINK_LIBRARIES(lambda-engine PUBLIC bullet)
  TARGET_COMPILE_DEFINITIONS(lambda-engine PRIVATE VIOLET_PHYSICS_BULLET)
ENDIF()
IF(${VIOLET_PHYSICS_REACT})
  TARGET_LINK_LIBRARIES(lambda-engine PUBLIC reactphysics3d)
  TARGET_COMPILE_DEFINITIONS(lambda-engine PRIVATE VIOLET_PHYSICS_REACT)
ENDIF()

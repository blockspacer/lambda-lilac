function(LinkDependencies)
  # add all required cmake functions.
  include(SetSolutionFolder)





  SET(BUILD_TESTING OFF CACHE BOOL "" FORCE)





  IF (VIOLET_WIN32)
    IF (CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
      STRING(REGEX REPLACE "/W[0-4]" "/W0" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    ELSE ()
      SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W0")
    ENDIF ()
  ELSEIF (VIOLET_LINUX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")
  ELSEIF (VIOLET_OSX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")
  ELSE ()
    MESSAGE(FATAL_ERROR "Unknown compiler platform")
  ENDIF (VIOLET_WIN32)





  # /// FOUNDATION ////////////////////////////////////////////////
  # ///////////////////////////////////////////////////////////////
  IF(${VIOLET_CONFIG_FOUNDATION})
  # /// EASTL /////////////////////////////////////////////////////
    SET(EASTL_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    ADD_SUBDIRECTORY("deps/eastl")
    ADD_SUBDIRECTORY("deps/eastl/test/packages/EAAssert")
    ADD_SUBDIRECTORY("deps/eastl/test/packages/EAStdC")
    ADD_SUBDIRECTORY("deps/eastl/test/packages/EAMain")
    ADD_SUBDIRECTORY("deps/eastl/test/packages/EATest")
    ADD_SUBDIRECTORY("deps/eastl/test/packages/EAThread")

    ADD_LIBRARY(eastl INTERFACE)
    TARGET_LINK_LIBRARIES(eastl INTERFACE EASTL EAAssert EAStdC EAMain EATest EAThread)
    TARGET_INCLUDE_DIRECTORIES(eastl INTERFACE "deps/eastl/include")
    SetSolutionFolder("deps/memory"     EABase_ide EASTL EAAssert_ide EAMain_ide EAStdC EATest EAThread_ide)
  ENDIF()










  # /// NETWORKING ////////////////////////////////////////////////
  # ///////////////////////////////////////////////////////////////
  IF(${VIOLET_CONFIG_NETWORKING})
  # /// ENET //////////////////////////////////////////////////////
    ADD_SUBDIRECTORY("deps/enet")
    TARGET_INCLUDE_DIRECTORIES(enet INTERFACE "deps/enet/include")
    TARGET_COMPILE_DEFINITIONS(enet PRIVATE _WINSOC_DEPRECATED_NO_WARNINGS)
    SetSolutionFolder("deps/networking" enet)
  ENDIF()










  # /// ENGINE ////////////////////////////////////////////////////
  # ///////////////////////////////////////////////////////////////
  IF(${VIOLET_CONFIG_ENGINE})
  # /// ASSIMP ////////////////////////////////////////////////////
    IF(${VIOLET_ASSET_ASSIMP})
      SET(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
      SET(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
      SET(ASSIMP_INSTALL_PDB OFF CACHE BOOL "" FORCE)
      ADD_SUBDIRECTORY("deps/assimp")

      ADD_LIBRARY(_assimp INTERFACE)
      TARGET_LINK_LIBRARIES(_assimp INTERFACE assimp IrrXML zlib zlibstatic)
      TARGET_INCLUDE_DIRECTORIES(_assimp INTERFACE "deps/assimp/include")
      SetSolutionFolder("deps/asset"      assimp IrrXML UpdateAssimpLibsDebugSymbolsAndDLLs zlib zlibstatic)
    ENDIF()
    
  # /// CHROMIUM EMBEDDED FRAMEWORK ///////////////////////////////
    # Add this project's cmake/ directory to the module path.
      IF(${VIOLET_GUI_CEF})
      SET(CEF_RUNTIME_LIBRARY_FLAG "/MD" CACHE STRING "" FORCE)

      include(IncludeCEF)
      IncludeCEF()
      
      SetSolutionFolder("deps/gui" libcef_dll_wrapper)
    ENDIF()

  # /// BULLET 3 //////////////////////////////////////////////////
    SET(USE_GLUT OFF CACHE BOOL "" FORCE)
    SET(USE_GRAPHICAL_BENCHMARK OFF CACHE BOOL "" FORCE)
    SET(INSTALL_CMAKE_FILES OFF CACHE BOOL "" FORCE)
    SET(BUILD_BULLET2_DEMOS OFF CACHE BOOL "" FORCE)
    SET(BUILD_CPU_DEMOS OFF CACHE BOOL "" FORCE)
    SET(BUILD_EXTRAS OFF CACHE BOOL "" FORCE)
    SET(BUILD_OPENGL3_DEMOS OFF CACHE BOOL "" FORCE)
    SET(BUILD_PYBULLET OFF CACHE BOOL "" FORCE)
    SET(BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
    SET(USE_MSVC_RUNTIME_LIBRARY_DLL ON CACHE BOOL "" FORCE)
    ADD_SUBDIRECTORY("deps/bullet3")

    ADD_LIBRARY(bullet INTERFACE)
    TARGET_LINK_LIBRARIES(bullet INTERFACE Bullet2FileLoader Bullet3Collision Bullet3Common Bullet3Dynamics Bullet3Geometry Bullet3OpenCL_clew BulletCollision BulletDynamics BulletInverseDynamics BulletSoftBody LinearMath)
    TARGET_INCLUDE_DIRECTORIES(bullet INTERFACE "deps/bullet3/src")
    SetSolutionFolder("deps/physics"    Bullet2FileLoader Bullet3Collision Bullet3Common Bullet3Dynamics Bullet3Geometry Bullet3OpenCL_clew BulletCollision BulletDynamics BulletInverseDynamics BulletSoftBody LinearMath)

  # /// FAST NOISE ////////////////////////////////////////////////
    SET(FastNoiseSources
      "deps/FastNoise/FastNoise.h"
      "deps/FastNoise/FastNoise.cpp"
    )
    ADD_LIBRARY(fast-noise ${FastNoiseSources})
    TARGET_INCLUDE_DIRECTORIES(fast-noise INTERFACE "deps/FastNoise")
    SetSolutionFolder("deps/math"       fast-noise)

  # /// GLM ///////////////////////////////////////////////////////
    ADD_LIBRARY(glm INTERFACE)
    TARGET_INCLUDE_DIRECTORIES(glm INTERFACE "deps/glm")

  # /// INIH //////////////////////////////////////////////////////
    SET(InihSources
      "deps/inih/ini.h"           "deps/inih/ini.c"
      "deps/inih/cpp/INIReader.h" "deps/inih/cpp/INIReader.cpp")
    ADD_LIBRARY(inih ${InihSources})
    TARGET_INCLUDE_DIRECTORIES(inih INTERFACE "deps/inih/cpp")
    SetSolutionFolder("deps/asset"      inih)

  # /// RAPID JSON ////////////////////////////////////////////////
  ADD_LIBRARY(rapidjson INTERFACE)
  TARGET_INCLUDE_DIRECTORIES(rapidjson INTERFACE "deps/rapidjson/include")

  # /// MESH DECIMATION ///////////////////////////////////////////
    SET(MeshDecimationSources
      "deps/MeshDecimation/src/mdMeshDecimator.h" "deps/MeshDecimation/src/mdMeshDecimator.cpp"
      "deps/MeshDecimation/src/mdVector.h"        "deps/MeshDecimation/src/mdVector.inl")
    ADD_LIBRARY(mesh-decimation ${MeshDecimationSources})
    TARGET_INCLUDE_DIRECTORIES(mesh-decimation INTERFACE "deps/MeshDecimation/src")
    SetSolutionFolder("deps/math"       mesh-decimation)

  # /// SOLOUD ////////////////////////////////////////////////////
    include(LinkSoLoud)
    LinkSoLoud()
    SetSolutionFolder("deps/audio"      soloud)

  # /// STB ///////////////////////////////////////////////////////
    SET(StbSources                          "deps/stb/stb.h"                       "deps/stb/stb_c_lexer.h"
      "deps/stb/stb_connected_components.h" "deps/stb/stb_divide.h"                "deps/stb/stb_dxt.h"
      "deps/stb/stb_easy_font.h"            "deps/stb/stb_herringbone_wang_tile.h" "deps/stb/stb_image.h"
      "deps/stb/stb_image_resize.h"         "deps/stb/stb_image_write.h"           "deps/stb/stb_leakcheck.h"
      "deps/stb/stb_perlin.h"               "deps/stb/stb_rect_pack.h"             "deps/stb/stb_sprintf.h"
      "deps/stb/stb_textedit.h"             "deps/stb/stb_tilemap_editor.h"        "deps/stb/stb_truetype.h"
      "deps/stb/stb_vorbis.c"               "deps/stb/stb_voxel_render.h"          "deps/stb/stretchy_buffer.h")
    ADD_LIBRARY(stb ${StbSources})
    TARGET_INCLUDE_DIRECTORIES(stb INTERFACE "deps/stb")
    SetSolutionFolder("deps/asset"      stb)

  # /// TINY GLTF /////////////////////////////////////////////////
    ADD_LIBRARY(tiny-gltf INTERFACE)
    TARGET_INCLUDE_DIRECTORIES(tiny-gltf INTERFACE "deps/tinygltf")

  # /// TINY XML2 /////////////////////////////////////////////////
    SET(BUILD_TESTS OFF CACHE BOOL "" FORCE)
    ADD_SUBDIRECTORY("deps/tinyxml2")
    SetSolutionFolder("deps/asset"      tinyxml2)

  # /// ANGEL SCRIPT //////////////////////////////////////////////
    IF(${VIOLET_SCRIPTING_ANGEL})
      ADD_SUBDIRECTORY("deps/AngelScript/sdk/angelscript/projects/cmake")
      # TARGET_INCLUDE_DIRECTORIES(angelscript PUBLIC "deps/AngelScript/sdk/angelscript/include")
      SetSolutionFolder("deps/scripting"  angelscript)
    ENDIF()

  # /// WREN SCRIPT ///////////////////////////////////////////////
    IF(${VIOLET_SCRIPTING_WREN})
      include(LinkWren)
      LinkWren()
      SetSolutionFolder("deps/scripting"  wren)
        TARGET_COMPILE_DEFINITIONS(wren PRIVATE WREN_OPT_META=0 WREN_OPT_RANDOM=0)
    ENDIF()

  # /// NUKLEAR IMGUI /////////////////////////////////////////////
    IF(${VIOLET_IMGUI_NUKLEAR})
      FILE(GLOB FILES "deps/nuklear/src/*.*")
      ADD_LIBRARY("nuklear-imgui" ${FILES})
      TARGET_INCLUDE_DIRECTORIES(nuklear-imgui INTERFACE "deps/nuklear")

      target_compile_definitions(nuklear-imgui PRIVATE NK_INCLUDE_FIXED_TYPES NK_INCLUDE_STANDARD_IO NK_INCLUDE_STANDARD_VARARGS NK_INCLUDE_VERTEX_BUFFER_OUTPUT NK_INCLUDE_FONT_BAKING NK_INCLUDE_DEFAULT_FONT NK_IMPLEMENTATION _CRT_SECURE_NO_WARNINGS)
      SetSolutionFolder("deps/imgui"      nuklear-imgui)
    ENDIF()

  # /// DEAR IMGUI ////////////////////////////////////////////////
    IF(${VIOLET_IMGUI_DEAR})
      SET(DearImGuiSources             "deps/imgui/imconfig.h"
        "deps/imgui/imgui.h"           "deps/imgui/imgui.cpp"
        "deps/imgui/imgui_draw.cpp"    "deps/imgui/imgui_demo.cpp"
        "deps/imgui/imgui_internal.h"  "deps/imgui/stb_rect_pack.h"
        "deps/imgui/stb_textedit.h"    "deps/imgui/stb_truetype.h")
      ADD_LIBRARY(dear-imgui ${DearImGuiSources})
      TARGET_INCLUDE_DIRECTORIES(dear-imgui INTERFACE "deps/imgui")
      SetSolutionFolder("deps/imgui"      dear-imgui)
    ENDIF()

  # /// GLFW //////////////////////////////////////////////////////
    IF(${VIOLET_WINDOW_GLFW})
      SET(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
      SET(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
      SET(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
      SET(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
      ADD_SUBDIRECTORY("deps/glfw")
      target_include_directories(glfw INTERFACE "deps/glfw/include")
      SetSolutionFolder("deps/window"     glfw)
    ENDIF()

  # /// SDL2 //////////////////////////////////////////////////////
    IF(${VIOLET_WINDOW_SDL2})
      #[[SET(3DNOW OFF CACHE BOOL "" FORCE)
      SET(ALTIVEC OFF CACHE BOOL "" FORCE)
      SET(ASSEMBLY OFF CACHE BOOL "" FORCE)
      SET(DIRECTX OFF CACHE BOOL "" FORCE)
      SET(DISKAUDIO OFF CACHE BOOL "" FORCE)
      SET(DUMMYAUDIO OFF CACHE BOOL "" FORCE)
      SET(MMX OFF CACHE BOOL "" FORCE)
      SET(SSE OFF CACHE BOOL "" FORCE)
      SET(SSE2 OFF CACHE BOOL "" FORCE)
      SET(SSE3 OFF CACHE BOOL "" FORCE)
      SET(SSEMATH OFF CACHE BOOL "" FORCE)
      SET(WASAPI OFF CACHE BOOL "" FORCE)
      SET(RENDER_D3D OFF CACHE BOOL "" FORCE)
      SET(VIDEO_DUMMY OFF CACHE BOOL "" FORCE)
      SET(VIDEO_OPENGL OFF CACHE BOOL "" FORCE)
      SET(VIDEO_OPENES OFF CACHE BOOL "" FORCE)
      SET(VIDEO_VULKAN OFF CACHE BOOL "" FORCE)]]
      ADD_SUBDIRECTORY("deps/SDL-mirror")

      ADD_LIBRARY(sdl INTERFACE)
      TARGET_LINK_LIBRARIES(sdl INTERFACE SDL2 SDL2main SDL2-static)
      TARGET_INCLUDE_DIRECTORIES(sdl INTERFACE "deps/SDL-mirror/include")
      SetSolutionFolder("deps/window"     SDL2 SDL2main SDL2-static)
    ENDIF()
  ENDIF()










  # /// TOOLS /////////////////////////////////////////////////////
  # ///////////////////////////////////////////////////////////////
  IF(${VIOLET_CONFIG_TOOLS})
  # /// GLSLANG ///////////////////////////////////////////////////
    ADD_SUBDIRECTORY("deps/glslang")
    TARGET_INCLUDE_DIRECTORIES(glslang INTERFACE "deps/glslang/spirv")
    TARGET_INCLUDE_DIRECTORIES(glslang INTERFACE "deps/glslang/glslang")
    SetSolutionFolder("deps/tools"      glslang glslang-default-resource-limits OGLCompiler OSDependent SPIRV SPVRemapper glslangValidator spirv-remap HLSL)

  # /// SPIRV CROSS ///////////////////////////////////////////////
    ADD_SUBDIRECTORY("deps/SPIRV-Cross")
    TARGET_INCLUDE_DIRECTORIES(SPIRV INTERFACE "deps/SPIRV-Cross/include")
    SetSolutionFolder("deps/tools"      spirv-cross spirv-cross-core spirv-cross-cpp spirv-cross-glsl spirv-cross-hlsl spirv-cross-msl spirv-cross-reflect spirv-cross-util)


  # /// DIRECTX TEX ///////////////////////////////////////////////
    SET(DirectXTexSources
    "deps/DirectXTex/DirectXTex/BC.h"
    "deps/DirectXTex/DirectXTex/BCDirectCompute.h"
    "deps/DirectXTex/DirectXTex/d3dx12.h"
    "deps/DirectXTex/DirectXTex/DDS.h"
    "deps/DirectXTex/DirectXTex/DirectXTex.h"
    "deps/DirectXTex/DirectXTex/DirectXTexP.h"
    "deps/DirectXTex/DirectXTex/Filters.h"
    "deps/DirectXTex/DirectXTex/scoped.h"
    "deps/DirectXTex/DirectXTex/BC.cpp"
    "deps/DirectXTex/DirectXTex/BC4BC5.cpp"
    "deps/DirectXTex/DirectXTex/BC6HBC7.cpp"
    "deps/DirectXTex/DirectXTex/BCDirectCompute.cpp"
    "deps/DirectXTex/DirectXTex/DirectXTexCompress.cpp"
    "deps/DirectXTex/DirectXTex/DirectXTexCompressGPU.cpp"
    "deps/DirectXTex/DirectXTex/DirectXTexConvert.cpp"
    "deps/DirectXTex/DirectXTex/DirectXTexD3D11.cpp"
    "deps/DirectXTex/DirectXTex/DirectXTexD3D12.cpp"
    "deps/DirectXTex/DirectXTex/DirectXTexDDS.cpp"
    "deps/DirectXTex/DirectXTex/DirectXTexFlipRotate.cpp"
    "deps/DirectXTex/DirectXTex/DirectXTexHDR.cpp"
    "deps/DirectXTex/DirectXTex/DirectXTexImage.cpp"
    "deps/DirectXTex/DirectXTex/DirectXTexMipmaps.cpp"
    "deps/DirectXTex/DirectXTex/DirectXTexMisc.cpp"
    "deps/DirectXTex/DirectXTex/DirectXTexNormalMaps.cpp"
    "deps/DirectXTex/DirectXTex/DirectXTexPMAlpha.cpp"
    "deps/DirectXTex/DirectXTex/DirectXTexResize.cpp"
    "deps/DirectXTex/DirectXTex/DirectXTexTGA.cpp"
    "deps/DirectXTex/DirectXTex/DirectXTexUtil.cpp"
    "deps/DirectXTex/DirectXTex/DirectXTexWIC.cpp"
    )
    ADD_LIBRARY(directxtex ${DirectXTexSources})
    TARGET_INCLUDE_DIRECTORIES(directxtex INTERFACE "deps/DirectXTex/DirectXTex")
    SetSolutionFolder("deps/assets" directxtex)
  ENDIF()

  # /// LZ4 ///////////////////////////////////////////////////////
  SET(LZ4Sources 
    "deps/lz4/lib/lz4.h" 
    "deps/lz4/lib/lz4.c"
  )
  ADD_LIBRARY(lz4 ${LZ4Sources})
  TARGET_INCLUDE_DIRECTORIES(lz4 INTERFACE "deps/lz4/lib")
  SetSolutionFolder("deps/tools"      lz4)
  # TODO (Hilze): Move back ASAP!







  # TODO (Hilze): Find out where this is from?
  SetSolutionFolder("cmake-targets"   uninstall)
endfunction()
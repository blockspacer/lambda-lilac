function(LinkDependencies)
  # add all required cmake functions.
  include(SetSolutionFolder)
  




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

  

  SET(USE_MSVC_RUNTIME_LIBRARY_DLL OFF CACHE BOOL "" FORCE)

  
  SET(USE_EASTL           FALSE)
  SET(USE_STACKWALKER     FALSE)
  SET(USE_ENET            FALSE)
  SET(USE_VMA             FALSE)
  SET(USE_ASSIMP          FALSE)
  SET(USE_ULTRALIGHT      FALSE)
  SET(USE_BULLET3         FALSE)
  SET(USE_REACT           FALSE)
  SET(USE_FASTNOISE       FALSE)
  SET(USE_GLM             FALSE)
  SET(USE_INIH            FALSE)
  SET(USE_MESHDECIMATION  FALSE)
  SET(USE_SOLOUD          FALSE)
  SET(USE_STB             FALSE)
  SET(USE_RAPIDJSON       FALSE)
  SET(USE_TINYGLTF        FALSE)
  SET(USE_ANGELSCRIPT     FALSE)
  SET(USE_WREN            FALSE)
  SET(USE_NUKLEAR         FALSE)
  SET(USE_DEAR            FALSE)
  SET(USE_GLFW            FALSE)
  SET(USE_SDL2            FALSE)
  SET(USE_SHADERCONDUCTOR FALSE)
  SET(USE_DIRECTXTEX      FALSE)
  SET(USE_LZ4             FALSE)

  # /// FOUNDATION ////////////////////////////////////////////////
  # ///////////////////////////////////////////////////////////////
  IF(${VIOLET_CONFIG_FOUNDATION})
	  SET(USE_EASTL TRUE)
	  SET(USE_STACKWALKER TRUE)
  ENDIF()





  # /// NETWORKING ////////////////////////////////////////////////
  # ///////////////////////////////////////////////////////////////
  IF(${VIOLET_CONFIG_NETWORKING})
  	SET(USE_ENET TRUE)
  ENDIF()





  # /// ENGINE ////////////////////////////////////////////////////
  # ///////////////////////////////////////////////////////////////
  IF(${VIOLET_CONFIG_ENGINE})
    IF(${VIOLET_RENDERER} STREQUAL "Vulkan")
	    SET(USE_VMA TRUE)
    ENDIF()

    IF(${VIOLET_ASSET_ASSIMP})
	    SET(USE_ASSIMP TRUE)
    ENDIF()
    
    IF (${VIOLET_GUI} STREQUAL "Ultralight")
      SET(USE_ULTRALIGHT TRUE)
    ENDIF()
	
    IF(${VIOLET_PHYSICS} STREQUAL "Bullet3")
	    SET(USE_BULLET3 TRUE)
    ENDIF()
	
    IF(${VIOLET_PHYSICS} STREQUAL "React")
	    SET(USE_REACT TRUE)
    ENDIF()
	
    SET(USE_FASTNOISE TRUE)
    SET(USE_GLM TRUE)
    SET(USE_INIH TRUE)
    SET(USE_MESHDECIMATION TRUE)
    SET(USE_SOLOUD TRUE)
    SET(USE_STB TRUE)
    SET(USE_RAPIDJSON TRUE)
	  SET(USE_TINYGLTF TRUE)

    IF(${VIOLET_SCRIPTING} STREQUAL "AngelScript")
	    SET(USE_ANGELSCRIPT TRUE)
    ENDIF()

    IF(${VIOLET_SCRIPTING} STREQUAL "Wren")
	    SET(USE_WREN TRUE)
    ENDIF()

    IF(${VIOLET_IMGUI} STREQUAL "Nuklear")
	    SET(USE_NUKLEAR TRUE)
    ENDIF()

    IF(${VIOLET_IMGUI} STREQUAL "Dear ImGUI")
	    SET(USE_DEAR TRUE)
    ENDIF()

    IF(${VIOLET_WINDOW} STREQUAL "GLFW")
	    SET(USE_GLFW TRUE)
    ENDIF()

    IF(${VIOLET_WINDOW} STREQUAL "SDL2")
	    SET(USE_SDL2 TRUE)
    ENDIF()
  ENDIF()





  # /// TOOLS /////////////////////////////////////////////////////
  # ///////////////////////////////////////////////////////////////
  IF(${VIOLET_CONFIG_TOOLS})
    IF (${VIOLET_SHADER_CONDUCTOR})
	    SET(USE_SHADERCONDUCTOR TRUE)
    ENDIF()
	
    SET(USE_STB TRUE)
    SET(USE_RAPIDJSON TRUE)
    SET(USE_SOLOUD TRUE)
    SET(USE_GLM TRUE)
    
    IF(${VIOLET_DIRECTX_TEX})
	    SET(USE_DIRECTXTEX TRUE)
    ENDIF()
  ENDIF()

  SET(USE_LZ4 TRUE)


  


  # /// ADD ALL TARGETS ///////////////////////////////////////////
  # ///////////////////////////////////////////////////////////////
  IF(${USE_EASTL})
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
  
  IF(${USE_STACKWALKER})
    ADD_SUBDIRECTORY("deps/StackWalker")
    SET(StackWalker_DISABLE_TESTS ON CACHE BOOL "" FORCE)
    TARGET_INCLUDE_DIRECTORIES(StackWalker INTERFACE "deps/StackWalker/main")
    SetSolutionFolder("deps/stackwalker" StackWalker)
  ENDIF()
  
  IF(${USE_ENET})
    ADD_SUBDIRECTORY("deps/enet")
    TARGET_INCLUDE_DIRECTORIES(enet INTERFACE "deps/enet/include")
    TARGET_COMPILE_DEFINITIONS(enet PRIVATE _WINSOC_DEPRECATED_NO_WARNINGS)
    SetSolutionFolder("deps/networking" enet)
  ENDIF()

  
  IF(${USE_VMA})
    ADD_LIBRARY(vma INTERFACE)
    TARGET_INCLUDE_DIRECTORIES(vma INTERFACE "deps/VulkanMemoryAllocator/src")
  ENDIF()
  
  IF(${USE_ASSIMP})
      SET(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
      SET(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
      SET(ASSIMP_INSTALL_PDB OFF CACHE BOOL "" FORCE)
      ADD_SUBDIRECTORY("deps/assimp")

      ADD_LIBRARY(_assimp INTERFACE)
      TARGET_LINK_LIBRARIES(_assimp INTERFACE assimp IrrXML zlib zlibstatic)
      TARGET_INCLUDE_DIRECTORIES(_assimp INTERFACE "deps/assimp/include")
      SetSolutionFolder("deps/asset"      assimp IrrXML UpdateAssimpLibsDebugSymbolsAndDLLs zlib zlibstatic)
  ENDIF()
  
  IF(${USE_ULTRALIGHT})
      include(LinkUltralight)
      LinkUltralight()
  ENDIF()
  
  IF(${USE_BULLET3})
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
  ENDIF()
  
  IF(${USE_REACT})
      ADD_SUBDIRECTORY("deps/reactphysics3d")
      TARGET_INCLUDE_DIRECTORIES(reactphysics3d INTERFACE "deps/reactphysics3d/src")
      SetSolutionFolder("deps/physics" reactphysics3d)
  ENDIF()
  
  IF(${USE_FASTNOISE})
    SET(FastNoiseSources
      "deps/FastNoise/FastNoise.h"
      "deps/FastNoise/FastNoise.cpp"
    )
    ADD_LIBRARY(fast-noise ${FastNoiseSources})
    TARGET_INCLUDE_DIRECTORIES(fast-noise INTERFACE "deps/FastNoise")
    SetSolutionFolder("deps/math"       fast-noise)
  ENDIF()
  
  IF(${USE_GLM})
    ADD_LIBRARY(glm INTERFACE)
    TARGET_INCLUDE_DIRECTORIES(glm INTERFACE "deps/glm")
  ENDIF()
  
  IF(${USE_INIH})
    SET(InihSources
      "deps/inih/ini.h"           "deps/inih/ini.c"
      "deps/inih/cpp/INIReader.h" "deps/inih/cpp/INIReader.cpp")
    ADD_LIBRARY(inih ${InihSources})
    TARGET_INCLUDE_DIRECTORIES(inih INTERFACE "deps/inih/cpp")
    SetSolutionFolder("deps/asset"      inih)
  ENDIF()
  
  IF(${USE_MESHDECIMATION})
    SET(MeshDecimationSources
      "deps/MeshDecimation/src/mdMeshDecimator.h" "deps/MeshDecimation/src/mdMeshDecimator.cpp"
      "deps/MeshDecimation/src/mdVector.h"        "deps/MeshDecimation/src/mdVector.inl")
    ADD_LIBRARY(mesh-decimation ${MeshDecimationSources})
    TARGET_INCLUDE_DIRECTORIES(mesh-decimation INTERFACE "deps/MeshDecimation/src")
    SetSolutionFolder("deps/math"       mesh-decimation)
  ENDIF()
  
  IF(${USE_SOLOUD})
    include(LinkSoLoud)
    LinkSoLoud()
    SetSolutionFolder("deps/audio"      soloud)
  ENDIF()
  
  IF(${USE_STB})
    SET(StbSources                          "deps/stb/stb.h"                       "deps/stb/stb_c_lexer.h"
      "deps/stb/stb_connected_components.h" "deps/stb/stb_divide.h"                "deps/stb/stb_dxt.h"
      "deps/stb/stb_easy_font.h"            "deps/stb/stb_herringbone_wang_tile.h" "deps/stb/stb_image.h"
      "deps/stb/stb_image_resize.h"         "deps/stb/stb_image_write.h"           "deps/stb/stb_leakcheck.h"
      "deps/stb/stb_perlin.h"               "deps/stb/stb_rect_pack.h"             "deps/stb/stb_sprintf.h"
      "deps/stb/stb_textedit.h"             "deps/stb/stb_tilemap_editor.h"        "deps/stb/stb_truetype.h"
      "deps/stb/stb_vorbis.c"               "deps/stb/stb_voxel_render.h"          "deps/stb/stretchy_buffer.h")
    ADD_LIBRARY(stb ${StbSources})
    TARGET_INCLUDE_DIRECTORIES(stb INTERFACE "deps/stb")
    SetSolutionFolder("deps/asset" stb)
  ENDIF()
  
  IF(${USE_RAPIDJSON})
    ADD_LIBRARY(rapidjson INTERFACE)
    TARGET_INCLUDE_DIRECTORIES(rapidjson INTERFACE "deps/rapidjson/include")
  ENDIF()
  
  IF(${USE_TINYGLTF})
    ADD_LIBRARY(tiny-gltf INTERFACE)
    TARGET_INCLUDE_DIRECTORIES(tiny-gltf INTERFACE "deps/tinygltf")
  ENDIF()
  
  IF(${USE_ANGELSCRIPT})
      ADD_SUBDIRECTORY("deps/AngelScript/sdk/angelscript/projects/cmake")
      # TARGET_INCLUDE_DIRECTORIES(angelscript PUBLIC "deps/AngelScript/sdk/angelscript/include")
      SetSolutionFolder("deps/scripting"  angelscript)
  ENDIF()
  
  IF(${USE_WREN})
      include(LinkWren)
      LinkWren()
      SetSolutionFolder("deps/scripting"  wren)
      TARGET_COMPILE_DEFINITIONS(wren PRIVATE WREN_OPT_META=0 WREN_OPT_RANDOM=0)
  ENDIF()
  
  IF(${USE_NUKLEAR})
      FILE(GLOB FILES "deps/nuklear/src/*.*")
      ADD_LIBRARY("nuklear-imgui" ${FILES})
      TARGET_INCLUDE_DIRECTORIES(nuklear-imgui INTERFACE "deps/nuklear")

      target_compile_definitions(nuklear-imgui PRIVATE NK_INCLUDE_FIXED_TYPES NK_INCLUDE_STANDARD_IO NK_INCLUDE_STANDARD_VARARGS NK_INCLUDE_VERTEX_BUFFER_OUTPUT NK_INCLUDE_FONT_BAKING NK_INCLUDE_DEFAULT_FONT NK_IMPLEMENTATION _CRT_SECURE_NO_WARNINGS)
      SetSolutionFolder("deps/imgui"      nuklear-imgui)
  ENDIF()
  
  IF(${USE_DEAR})
      SET(DearImGuiSources             "deps/imgui/imconfig.h"
        "deps/imgui/imgui.h"           "deps/imgui/imgui.cpp"
        "deps/imgui/imgui_draw.cpp"    "deps/imgui/imgui_demo.cpp"
        "deps/imgui/imgui_internal.h"  "deps/imgui/stb_rect_pack.h"
        "deps/imgui/stb_textedit.h"    "deps/imgui/stb_truetype.h")
      ADD_LIBRARY(dear-imgui ${DearImGuiSources})
      TARGET_INCLUDE_DIRECTORIES(dear-imgui INTERFACE "deps/imgui")
      SetSolutionFolder("deps/imgui"      dear-imgui)
  ENDIF()
  
  IF(${USE_GLFW})
      SET(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
      SET(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
      SET(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
      SET(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
      ADD_SUBDIRECTORY("deps/glfw")
      target_include_directories(glfw INTERFACE "deps/glfw/include")
      SetSolutionFolder("deps/window"     glfw)
  ENDIF()

  IF(${USE_SDL2})
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
  
  IF(${USE_SHADERCONDUCTOR})
      SET(SC_WITH_CSHARP OFF CACHE BOOL "" FORCE)
      ADD_SUBDIRECTORY("deps/ShaderConductor")
      TARGET_INCLUDE_DIRECTORIES(ShaderConductor INTERFACE "deps/ShaderConductor/Include")
  ENDIF()
  
  IF(${USE_DIRECTXTEX})
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
	    SetSolutionFolder("deps/asset" directxtex)
  ENDIF()
  
  IF(${USE_LZ4})
	  SET(LZ4Sources 
		"deps/lz4/lib/lz4.h" 
		"deps/lz4/lib/lz4.c"
	  )
	  ADD_LIBRARY(lz4 ${LZ4Sources})
	  TARGET_INCLUDE_DIRECTORIES(lz4 INTERFACE "deps/lz4/lib")
	  SetSolutionFolder("deps/tools"      lz4)
  ENDIF()
endfunction()
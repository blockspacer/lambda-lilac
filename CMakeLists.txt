CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

SET(CMAKE_CXX_STANDARD 14)
SET(CMAKE_MODULE_PATH "${CMAKE_ROOT}/Modules ${CMAKE_CURRENT_SOURCE_DIR}/cmake")
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DVIOLET_DEBUG")

SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

SET(CMAKE_CXX_FLAGS_RELEASE "/MT")
SET(CMAKE_CXX_FLAGS_DEBUG "/MTd")
SET(CompilerFlags
        CMAKE_CXX_FLAGS
        CMAKE_CXX_FLAGS_DEBUG
        CMAKE_CXX_FLAGS_RELEASE
        CMAKE_C_FLAGS
        CMAKE_C_FLAGS_DEBUG
        CMAKE_C_FLAGS_RELEASE
        )
FOREACH(CompilerFlag ${CompilerFlags})
  STRING(REPLACE "/MT" "/MD" ${CompilerFlag} "${${CompilerFlag}}")
ENDFOREACH()
FOREACH(CompilerFlag ${CompilerFlags})
  STRING(REPLACE "/MDd" "/MTd" ${CompilerFlag} "${${CompilerFlag}}")
ENDFOREACH()

# Add all platform defines.
IF (WIN32)
  SET(VIOLET_WIN32 ON)
	ADD_DEFINITIONS (-DVIOLET_WIN32)
ELSEIF (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  SET(VIOLET_LINUX ON)
	ADD_DEFINITIONS (-DVIOLET_LINUX)
ELSEIF (APPLE)
  SET(VIOLET_OSX ON)
	ADD_DEFINITIONS (-DVIOLET_OSX)
ENDIF (WIN32)

# Disable RTTI
if (VIOLET_WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GR-")
elseif (VIOLET_LINUX || VIOLET_OSX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
endif()

# Do required things.
IF (VIOLET_WIN32)
#  SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX")
  SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")

  SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)
  SET_PROPERTY(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "cmake-targets")
ELSEIF (VIOLET_LINUX)
#  SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
ELSEIF (VIOLET_OSX)
#  SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
ELSE ()
  MESSAGE(FATAL_ERROR "Unknown copmiler platform")
ENDIF (VIOLET_WIN32)

set(CompilerFlags
  CMAKE_C_FLAGS
  CMAKE_C_FLAGS_DEBUG
  CMAKE_C_FLAGS_MINSIZEREL
  CMAKE_C_FLAGS_RELEASE
  CMAKE_C_FLAGS_RELWITHDEBINFO
  CMAKE_CXX_FLAGS
  CMAKE_CXX_FLAGS_DEBUG
  CMAKE_CXX_FLAGS_MINSIZEREL
  CMAKE_CXX_FLAGS_RELEASE
  CMAKE_CXX_FLAGS_RELWITHDEBINFO
)
#foreach(CompilerFlag ${CompilerFlags})
#  string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
#endforeach()

# ///////////////////////////////////////////////////////////////
# /// BOOLEANS //////////////////////////////////////////////////
# ///////////////////////////////////////////////////////////////
# /// CONFIG .../////////////////////////////////////////////////
SET(VIOLET_CONFIG_FOUNDATION TRUE CACHE BOOL "[CORE] Should Foundation be build?")
SET(VIOLET_CONFIG_ENGINE TRUE CACHE BOOL "[CORE] Should Engine be build?")

IF(${VIOLET_WIN32})
  SET(VIOLET_CONFIG_TOOLS TRUE CACHE BOOL "[CORE] Should Tools be build?")
ELSE()
  SET(VIOLET_CONFIG_TOOLS FALSE CACHE INTERNAL "[CORE] Tools not available on this platform.")
ENDIF()

IF(${VIOLET_WIN32})
  SET(VIOLET_CONFIG_NETWORKING TRUE CACHE BOOL "[CORE] Should Networking be build?")
ELSE()
  SET(VIOLET_CONFIG_NETWORKING FALSE CACHE INTERNAL "[CORE] Networking not available on this platform.")
ENDIF()

# /// FOUNDATION ////////////////////////////////////////////////
# ///////////////////////////////////////////////////////////////
IF(${VIOLET_CONFIG_FOUNDATION})
ENDIF()

# /// NETWORKING ////////////////////////////////////////////////
# ///////////////////////////////////////////////////////////////
IF(${VIOLET_CONFIG_NETWORKING})
ENDIF()

# /// ENGINE ////////////////////////////////////////////////////
# ///////////////////////////////////////////////////////////////
IF(${VIOLET_CONFIG_ENGINE})
# /// RENDERERS /////////////////////////////////////////////////
  IF(${VIOLET_WIN32})
	  SET(VIOLET_RENDERER_D3D11 TRUE CACHE BOOL "[RENDERER] Should D3D11 Renderer be used?")
  ELSE()
	  SET(VIOLET_RENDERER_D3D11 FALSE CACHE INTERNAL "[RENDERER] D3D11 Renderer not available on this platform.")
  ENDIF()

  IF(${VIOLET_OSX})
	  SET(VIOLET_RENDERER_METAL TRUE CACHE BOOL "[RENDERER] Should Metal Renderer be used?")
  ELSE()
	  SET(VIOLET_RENDERER_METAL FALSE CACHE INTERNAL "[RENDERER] Metal Renderer not available on this platform.")
  ENDIF()

  IF(NOT ${VIOLET_RENDERER_D3D11} AND NOT ${VIOLET_RENDERER_METAL})
    SET(VIOLET_RENDERER_NO TRUE)
  ENDIF()

# /// WINDOWS ///////////////////////////////////////////////////
	SET(VIOLET_WINDOW_GLFW FALSE CACHE BOOL "[WINDOW] Should GLFW Window be used?")
	SET(VIOLET_WINDOW_SDL2 TRUE CACHE BOOL "[WINDOW] Should SDL2 Window be used?")

  IF(${VIOLET_WIN32})
    SET(VIOLET_WINDOW_WIN32 TRUE CACHE BOOL "[WINDOW] Should Win32 Window be used?")
  ELSE()
    SET(VIOLET_WINDOW_WIN32 FALSE CACHE INTERNAL "[WINDOW] Win32 Window not available on this platform.")
  ENDIF()

# /// SCRIPTING /////////////////////////////////////////////////
  SET(VIOLET_SCRIPTING_ANGEL TRUE CACHE BOOL "[SCRIPTING] Should AngelScript be used?")
  SET(VIOLET_SCRIPTING_WREN TRUE CACHE BOOL "[SCRIPTING] Should Wren be used?")

# /// IMGUI /////////////////////////////////////////////////////
	SET(VIOLET_IMGUI_NUKLEAR TRUE CACHE BOOL "[IMGUI] Should Nuklear ImGUI be used?")
	SET(VIOLET_IMGUI_DEAR FALSE CACHE BOOL "[IMGUI] Should Dear ImGUI be used?")
	SET(VIOLET_IMGUI_NO FALSE CACHE BOOL "[IMGUI] Should No ImGUI be used?")
	IF(NOT ${VIOLET_IMGUI_NUKLEAR} AND NOT ${VIOLET_IMGUI_DEAR})
		SET(VIOLET_IMGUI_NO TRUE)
  ENDIF()
  
# /// GUI ///////////////////////////////////////////////////////
  SET(VIOLET_GUI_ULTRALIGHT TRUE CACHE BOOL "[GUI] Should Ultralight be used?")
ENDIF()


# /// TOOLS /////////////////////////////////////////////////////
# ///////////////////////////////////////////////////////////////
IF(${VIOLET_CONFIG_TOOLS})
ENDIF()
# ///////////////////////////////////////////////////////////////
# /// BOOLEANS //////////////////////////////////////////////////
# ///////////////////////////////////////////////////////////////


# Create the project.
PROJECT(lambda-violet)

# Add all dependencies folders.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
include(LinkDependencies)
LinkDependencies()

ADD_SUBDIRECTORY("src")

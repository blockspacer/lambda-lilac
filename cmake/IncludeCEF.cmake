function(IncludeCEF)
  # Specify the CEF distribution version.
  set(CEF_VERSION "3.3538.1852.gcb937fc")

  # Determine the platform.
  if("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
    set(CEF_PLATFORM "macosx64")
  elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    if(CMAKE_SIZEOF_VOID_P MATCHES 8)
      set(CEF_PLATFORM "linux64")
    else()
      set(CEF_PLATFORM "linux32")
    endif()
  elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
    if(CMAKE_SIZEOF_VOID_P MATCHES 8)
      set(CEF_PLATFORM "windows64")
    else()
      set(CEF_PLATFORM "windows32")
    endif()
  endif()

  # Add this project's cmake/ directory to the module path.
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

  # Download and extract the CEF binary distribution (executes DownloadCEF.cmake).
  include(DownloadCEF)
  DownloadCEF("${CEF_PLATFORM}" "${CEF_VERSION}" "${CMAKE_CURRENT_SOURCE_DIR}/cef")

  # Add the CEF binary distribution's cmake/ directory to the module path.
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CEF_ROOT}/cmake")

  # Load the CEF configuration (executes FindCEF.cmake).
  find_package(CEF REQUIRED)

  add_subdirectory(${CEF_ROOT}/libcef_dll)
	TARGET_INCLUDE_DIRECTORIES(libcef_dll_wrapper INTERFACE "${CEF_ROOT}/include")
	TARGET_INCLUDE_DIRECTORIES(libcef_dll_wrapper INTERFACE "${CEF_ROOT}")

endfunction()
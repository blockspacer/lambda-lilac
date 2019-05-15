function(LinkUltralight)
	add_subdirectory("deps/ultralight")
	set(SDK_ROOT "${CMAKE_BINARY_DIR}/SDK/")
	set(ULTRALIGHT_LIBRARY_DIR "${SDK_ROOT}/lib")
	link_directories(${ExeName} PUBLIC "${ULTRALIGHT_LIBRARY_DIR}")
endfunction()

function(BindUltralight ExeName)
	set(SDK_ROOT "${CMAKE_BINARY_DIR}/SDK/")
	set(ULTRALIGHT_INCLUDE_DIR "${SDK_ROOT}/include")
	set(ULTRALIGHT_BINARY_DIR "${SDK_ROOT}/bin")

	target_include_directories(${ExeName} PUBLIC "${ULTRALIGHT_INCLUDE_DIR}")
	target_link_libraries(${ExeName} PUBLIC UltralightCore AppCore Ultralight WebCore)

	# Copy all binaries to target directory
	add_custom_command(TARGET ${ExeName} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory "${ULTRALIGHT_BINARY_DIR}" $<TARGET_FILE_DIR:${ExeName}>) 
    add_dependencies(${ExeName} CopySDK)
endfunction()
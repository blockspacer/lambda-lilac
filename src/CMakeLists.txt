Function(SetWarningAsErrors proj)
  if(MSVC)
		target_compile_options(${proj} PRIVATE /WX)
	else()
		target_compile_options(${proj} PRIVATE -Wall -Wextra -Werror -Wno-unused-variable -Wno-unused-parameter -Wno-ignored-qualifiers)
	endif()
EndFunction()

IF(${VIOLET_CONFIG_FOUNDATION})
  ADD_SUBDIRECTORY("foundation")
  SetWarningAsErrors(lambda-foundation)
ENDIF()

IF(${VIOLET_CONFIG_NETWORKING})
  ADD_SUBDIRECTORY("networking")
  SetWarningAsErrors(lambda-networking)
ENDIF()

IF(${VIOLET_CONFIG_TOOLS})
  ADD_SUBDIRECTORY("packager")
  ADD_SUBDIRECTORY("builder")
  SetWarningAsErrors(lambda-packager)
  SetWarningAsErrors(lambda-builder)
ENDIF()

IF(${VIOLET_CONFIG_ENGINE})
	ADD_SUBDIRECTORY("engine")
  SetWarningAsErrors(lambda-engine)
ENDIF()



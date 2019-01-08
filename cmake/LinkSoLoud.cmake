function(LinkSoLoud)
  SET(AudioSource
    "deps/soloud/src/audiosource/monotone/soloud_monotone.cpp"
    "deps/soloud/src/audiosource/openmpt/soloud_openmpt.cpp"
    "deps/soloud/src/audiosource/sfxr/soloud_sfxr.cpp"
    "deps/soloud/src/audiosource/speech/darray.cpp"
    "deps/soloud/src/audiosource/speech/darray.h"
    "deps/soloud/src/audiosource/speech/klatt.cpp"
    "deps/soloud/src/audiosource/speech/klatt.h"
    "deps/soloud/src/audiosource/speech/resonator.cpp"
    "deps/soloud/src/audiosource/speech/resonator.h"
    "deps/soloud/src/audiosource/speech/soloud_speech.cpp"
    "deps/soloud/src/audiosource/speech/tts.cpp"
    "deps/soloud/src/audiosource/speech/tts.h"
    "deps/soloud/src/audiosource/tedsid/sid.cpp"
    "deps/soloud/src/audiosource/tedsid/sid.h"
    "deps/soloud/src/audiosource/tedsid/soloud_tedsid.cpp"
    "deps/soloud/src/audiosource/tedsid/ted.cpp"
    "deps/soloud/src/audiosource/tedsid/ted.h"
    "deps/soloud/src/audiosource/vic/soloud_vic.cpp"
    "deps/soloud/src/audiosource/vizsn/soloud_vizsn.cpp"
    "deps/soloud/src/audiosource/wav/soloud_wav.cpp"
    "deps/soloud/src/audiosource/wav/soloud_wavstream.cpp"
    "deps/soloud/src/audiosource/wav/stb_vorbis.h"
    "deps/soloud/src/audiosource/wav/stb_vorbis.c"
  )
  SET(Core
    "deps/soloud/src/core/soloud.cpp"
    "deps/soloud/src/core/soloud_audiosource.cpp"
    "deps/soloud/src/core/soloud_bus.cpp"
    "deps/soloud/src/core/soloud_core_3d.cpp"
    "deps/soloud/src/core/soloud_core_basicops.cpp"
    "deps/soloud/src/core/soloud_core_faderops.cpp"
    "deps/soloud/src/core/soloud_core_filterops.cpp"
    "deps/soloud/src/core/soloud_core_getters.cpp"
    "deps/soloud/src/core/soloud_core_setters.cpp"
    "deps/soloud/src/core/soloud_core_voicegroup.cpp"
    "deps/soloud/src/core/soloud_core_voiceops.cpp"
    "deps/soloud/src/core/soloud_fader.cpp"
    "deps/soloud/src/core/soloud_fft.cpp"
    "deps/soloud/src/core/soloud_fft_lut.cpp"
    "deps/soloud/src/core/soloud_file.cpp"
    "deps/soloud/src/core/soloud_filter.cpp"
    "deps/soloud/src/core/soloud_queue.cpp"
    "deps/soloud/src/core/soloud_thread.cpp"

    "deps/soloud/include/soloud.h"
    "deps/soloud/include/soloud_audiosource.h"
    "deps/soloud/include/soloud_bassboostfilter.h"
    "deps/soloud/include/soloud_biquadresonantfilter.h"
    "deps/soloud/include/soloud_bus.h"
    "deps/soloud/include/soloud_c.h"
    "deps/soloud/include/soloud_dcremovalfilter.h"
    "deps/soloud/include/soloud_echofilter.h"
    "deps/soloud/include/soloud_error.h"
    "deps/soloud/include/soloud_fader.h"
    "deps/soloud/include/soloud_fft.h"
    "deps/soloud/include/soloud_fftfilter.h"
    "deps/soloud/include/soloud_file.h"
    "deps/soloud/include/soloud_file_hack_off.h"
    "deps/soloud/include/soloud_file_hack_on.h"
    "deps/soloud/include/soloud_filter.h"
    "deps/soloud/include/soloud_flangerfilter.h"
    "deps/soloud/include/soloud_internal.h"
    "deps/soloud/include/soloud_lofifilter.h"
    "deps/soloud/include/soloud_monotone.h"
    "deps/soloud/include/soloud_openmpt.h"
    "deps/soloud/include/soloud_queue.h"
    "deps/soloud/include/soloud_robotizefilter.h"
    "deps/soloud/include/soloud_sfxr.h"
    "deps/soloud/include/soloud_speech.h"
    "deps/soloud/include/soloud_tedsid.h"
    "deps/soloud/include/soloud_thread.h"
    "deps/soloud/include/soloud_vic.h"
    "deps/soloud/include/soloud_vizsn.h"
    "deps/soloud/include/soloud_wav.h"
    "deps/soloud/include/soloud_waveshaperfilter.h"
    "deps/soloud/include/soloud_wavstream.h"
  )

  SET(Sources
    ${Core}
    ${AudioSource}
  )

  SET(VIOLET_SOLOUD_BACKEND_ALSA FALSE CACHE BOOL "Should soloud use the asla backend?")
  SET(VIOLET_SOLOUD_BACKEND_OSX FALSE CACHE BOOL "Should soloud use the OSX backend?")
  SET(VIOLET_SOLOUD_BACKEND_OSS FALSE CACHE BOOL "Should soloud use the oss backend?")
  SET(VIOLET_SOLOUD_BACKEND_PORTAUDIO FALSE CACHE BOOL "Should soloud use the portaudio backend?")
  SET(VIOLET_SOLOUD_BACKEND_SDL_STATIC FALSE CACHE BOOL "Should soloud use the sdl static backend?")
  SET(VIOLET_SOLOUD_BACKEND_SDL2_STATIC FALSE CACHE BOOL "Should soloud use the sdl2 static backend?")
  SET(VIOLET_SOLOUD_BACKEND_WASAPI FALSE CACHE BOOL "Should soloud use the wasapi backend?")
  SET(VIOLET_SOLOUD_BACKEND_WINMM FALSE CACHE BOOL "Should soloud use the winmm backend?")
  SET(VIOLET_SOLOUD_BACKEND_XAUDIO2 FALSE CACHE BOOL "Should soloud use the xaudio2 backend?")

  IF(NOT ${VIOLET_SOLOUD_BACKEND_ALSA} 
    AND NOT ${VIOLET_SOLOUD_BACKEND_OSX} 
    AND NOT ${VIOLET_SOLOUD_BACKEND_OSS} 
    AND NOT ${VIOLET_SOLOUD_BACKEND_PORTAUDIO} 
    AND NOT ${VIOLET_SOLOUD_BACKEND_SDL_STATIC} 
    AND NOT ${VIOLET_SOLOUD_BACKEND_SDL2_STATIC} 
    AND NOT ${VIOLET_SOLOUD_BACKEND_WASAPI} 
    AND NOT ${VIOLET_SOLOUD_BACKEND_WINMM} 
    AND NOT ${VIOLET_SOLOUD_BACKEND_XAUDIO2})

    IF(${VIOLET_WIN32})
      SET(VIOLET_SOLOUD_BACKEND_WINMM TRUE CACHE BOOL "Should soloud use the winmm backend?" FORCE)
    ELSEIF(${VIOLET_OSX})
      SET(VIOLET_SOLOUD_BACKEND_OSX TRUE CACHE BOOL "Should soloud use the OSX backend?" FORCE)
    ENDIF()
  ENDIF()

  IF(${VIOLET_SOLOUD_BACKEND_ALSA})
    SET(Sources ${Sources} "deps/soloud/src/backend/alsa/soloud_alsa.cpp")
  ELSEIF(${VIOLET_SOLOUD_BACKEND_OSX})
    SET(Sources ${Sources} "deps/soloud/src/backend/coreaudio/soloud_coreaudio.cpp")
  ELSEIF(${VIOLET_SOLOUD_BACKEND_OSS})
    SET(Sources ${Sources} "deps/soloud/src/backend/oss/soloud_oss.cpp")
  ELSEIF(${VIOLET_SOLOUD_BACKEND_PORTAUDIO})
    SET(Sources ${Sources} "deps/soloud/src/backend/portaudio/soloud_portaudio.cpp")
  ELSEIF(${VIOLET_SOLOUD_BACKEND_SDL_STATIC})
    SET(Sources ${Sources} "deps/soloud/src/backend/sdl/soloud_sdl_static.cpp")
  ELSEIF(${VIOLET_SOLOUD_BACKEND_SDL2_STATIC})
    SET(Sources ${Sources} "deps/soloud/src/backend/sdl2/soloud_sdl2_static.cpp")
  ELSEIF(${VIOLET_SOLOUD_BACKEND_WASAPI})
    SET(Sources ${Sources} "deps/soloud/src/backend/wasapi/soloud_wasapi.cpp")
  ELSEIF(${VIOLET_SOLOUD_BACKEND_WINMM})
    SET(Sources ${Sources} "deps/soloud/src/backend/winmm/soloud_winmm.cpp")
  ELSEIF(${VIOLET_SOLOUD_BACKEND_XAUDIO2})
    SET(Sources ${Sources} "deps/soloud/src/backend/xaudio2/soloud_xaudio2.cpp")
  ELSE()
    SET(Sources ${Sources} "deps/soloud/src/backend/null/soloud_null.cpp")
  ENDIF()

  ADD_LIBRARY(soloud ${Sources})

  TARGET_COMPILE_DEFINITIONS(soloud PRIVATE DR_WAV_IMPLEMENTATION DR_MP3_IMPLEMENTATION DR_FLAC_IMPLEMENTATION)
  
  TARGET_INCLUDE_DIRECTORIES(soloud PUBLIC "deps/soloud/include")

  IF(${VIOLET_SOLOUD_BACKEND_ALSA})
    TARGET_COMPILE_DEFINITIONS(soloud PRIVATE WITH_ALSA)
  ELSEIF(${VIOLET_SOLOUD_BACKEND_OSX})
    TARGET_COMPILE_DEFINITIONS(soloud PRIVATE WITH_COREAUDIO)
  ELSEIF(${VIOLET_SOLOUD_BACKEND_OSS})
    TARGET_COMPILE_DEFINITIONS(soloud PRIVATE WITH_OSS)
  ELSEIF(${VIOLET_SOLOUD_BACKEND_PORTAUDIO})
    TARGET_COMPILE_DEFINITIONS(soloud PRIVATE WITH_PORTAUDIO)
  ELSEIF(${VIOLET_SOLOUD_BACKEND_SDL_STATIC})
    TARGET_COMPILE_DEFINITIONS(soloud PRIVATE WITH_SDL_STATIC)
  ELSEIF(${VIOLET_SOLOUD_BACKEND_SDL2_STATIC})
    TARGET_COMPILE_DEFINITIONS(soloud PRIVATE WITH_SDL2_STATIC)
  ELSEIF(${VIOLET_SOLOUD_BACKEND_WASAPI})
    TARGET_COMPILE_DEFINITIONS(soloud PRIVATE WITH_WASAPI)
  ELSEIF(${VIOLET_SOLOUD_BACKEND_WINMM})
    TARGET_COMPILE_DEFINITIONS(soloud PRIVATE WITH_WINMM)
  ELSEIF(${VIOLET_SOLOUD_BACKEND_XAUDIO2})
    TARGET_COMPILE_DEFINITIONS(soloud PRIVATE WITH_XAUDIO2)
  ELSE()
    TARGET_COMPILE_DEFINITIONS(soloud PRIVATE WITH_NULLDRIVER)
  ENDIF()
endfunction()
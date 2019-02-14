function(DownloadDXC DOWNLOAD_PATH)
  set(DOWNLOAD_URL "https://ci.appveyor.com/api/buildjobs/mje8eyn2uei2h7n9/artifacts/build%2FRelease%2Fdxc-artifacts.zip")

  if(NOT IS_DIRECTORY "${DOWNLOAD_PATH}")
    message(STATUS "Downloading ${DOWNLOAD_PATH}/temp.zip...")
    file(
      DOWNLOAD "${DOWNLOAD_URL}" "${DOWNLOAD_PATH}/temp.zip"
      SHOW_PROGRESS
    )
      
    # Extract the binary distribution.
    message(STATUS "Extracting ${DOWNLOAD_PATH}...")
    execute_process(
      COMMAND ${CMAKE_COMMAND} -E tar xzf "${DOWNLOAD_PATH}/temp.zip"
      WORKING_DIRECTORY ${DOWNLOAD_PATH}
    )
    file(REMOVE "${DOWNLOAD_PATH}/temp.zip" SHOW_PROGRESS)
  endif()
endfunction()

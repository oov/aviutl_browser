cmake_minimum_required(VERSION 3.0)

project(aviutl_browser_package NONE)

if("${BUILD_PACKAGE}" STREQUAL "1")

  set(EMOJI_VERSION "v0.0.2")
  set(EMOJI_URL "https://github.com/oov/aviutl_browser_emoji/releases/download/${EMOJI_VERSION}/aviutl_browser_emoji_${EMOJI_VERSION}.zip")
  set(EMOJI_PATH "${CMAKE_CURRENT_BINARY_DIR}/emoji.zip")
  set(EMOJI_DIR "${CMAKE_CURRENT_BINARY_DIR}/emoji")
  if(NOT EXISTS "${EMOJI_PATH}")
    file(DOWNLOAD "${EMOJI_URL}" "${EMOJI_PATH}")
  endif()
  if(NOT EXISTS "${EMOJI_DIR}")
    file(MAKE_DIRECTORY "${EMOJI_DIR}")
    execute_process(COMMAND ${CMAKE_COMMAND} -E tar xvzf "${EMOJI_PATH}" WORKING_DIRECTORY "${EMOJI_DIR}")
  endif()

  set(MARKDOWN_VERSION "v0.0.3")
  set(MARKDOWN_URL "https://github.com/oov/aviutl_browser_markdown/releases/download/${MARKDOWN_VERSION}/aviutl_browser_markdown_${MARKDOWN_VERSION}.zip")
  set(MARKDOWN_PATH "${CMAKE_CURRENT_BINARY_DIR}/markdown.zip")
  set(MARKDOWN_DIR "${CMAKE_CURRENT_BINARY_DIR}/markdown")
  if(NOT EXISTS "${MARKDOWN_PATH}")
    file(DOWNLOAD "${MARKDOWN_URL}" "${MARKDOWN_PATH}")
  endif()
  if(NOT EXISTS "${MARKDOWN_DIR}")
    file(MAKE_DIRECTORY "${MARKDOWN_DIR}")
    execute_process(COMMAND ${CMAKE_COMMAND} -E tar xvzf "${MARKDOWN_PATH}" WORKING_DIRECTORY "${MARKDOWN_DIR}")
  endif()

  add_custom_target(copy_script
    COMMAND ${CMAKE_COMMAND} -E copy "${PROJECT_SOURCE_DIR}/README.md" "${PROJECT_BINARY_DIR}/bin/aviutl_browser.txt"
    COMMAND ${CMAKE_COMMAND} -E copy "${PROJECT_SOURCE_DIR}/LICENSES.chromium.html" "${PROJECT_BINARY_DIR}/bin/LICENSES.chromium.html"
    COMMAND ${CMAKE_COMMAND} -E copy "${PROJECT_SOURCE_DIR}/src/lua/browser.lua" "${PROJECT_BINARY_DIR}/bin/script/browser/browser.lua"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${EMOJI_DIR}" "${PROJECT_BINARY_DIR}/bin"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${MARKDOWN_DIR}" "${PROJECT_BINARY_DIR}/bin"
  )

endif()

if("${BUILD_EXE}" STREQUAL "1")

  add_subdirectory(src)

endif()

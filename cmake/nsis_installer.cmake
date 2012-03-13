cmake_minimum_required(VERSION 2.8)

IF(WIN32 AND NOT UNIX)
  SET(CPACK_NSIS_INSTALLED_ICON_NAME "${CMAKE_CURRENT_SOURCE_DIR}/clients/roomeditor/images\\\\app.ico")
  SET(CPACK_NSIS_MUI_ICON "${CPACK_NSIS_INSTALLED_ICON_NAME}")
  SET(CPACK_NSIS_MUI_UNIICON "${CPACK_NSIS_INSTALLED_ICON_NAME}")
  SET(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_NAME} ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}")
  SET(CPACK_NSIS_HELP_LINK "${CPACK_PROJECT_WEBURL}")
  SET(CPACK_NSIS_URL_INFO_ABOUT "${CPACK_PROJECT_WEBURL}")
  SET(CPACK_NSIS_MODIFY_PATH OFF)
ENDIF(WIN32 AND NOT UNIX)
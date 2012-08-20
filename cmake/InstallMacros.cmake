cmake_minimum_required(VERSION 2.8.8)

# set default build type
IF (NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE release)
  SET(CMAKE_CONFIGURATION_TYPES RelWithDebInfo release Debug)
ENDIF (NOT CMAKE_BUILD_TYPE)

# Distribution via cpack. Set variables
INCLUDE(InstallRequiredSystemLibraries)
IF(WIN32 AND NOT UNIX)
SET(CPACK_GENERATOR "NSIS")
ELSE()
SET(CPACK_GENERATOR "DEB")
  INCLUDE(cmake/Build_tools.cmake)
  execute_process(COMMAND ${TOOL_prepareReadme} "${CMAKE_CURRENT_SOURCE_DIR}/README.markdown" OUTPUT_VARIABLE CPACK_PACKAGE_DESCRIPTION_SUMMARY)
ENDIF()
SET(CPACK_PACKAGE_NAME "RoomcontrolSuite")
SET(CPACK_PACKAGE_VERSION_MAJOR "2")
SET(CPACK_PACKAGE_VERSION_MINOR "4")
SET(CPACK_PACKAGE_VERSION_PATCH "0")
SET(CPACK_PACKAGE_CONTACT "David Gr�ff <david.graeff@web.d_e>")
SET(CPACK_PACKAGE_VENDOR "David Gr�ff")
#SET(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/clients/roomeditor/images\\\\serviceprovider.png")
SET(CPACK_PACKAGE_DESCRIPTION "Roomcontrol server application, clients and plugins")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.markdown")
SET(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.markdown")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
SET(CPACK_DEBIAN_PACKAGE_SECTION "net")
SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt4-network (>= 4:4.7.0), libpam0g (>= 1.1.1), libudev0 (>= 160), libssl1.0.0(>= 1.0.0)")
SET(CPACK_STRIP_FILES 1)
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}")
SET(CPACK_PROJECT_WEBURL "http://davidgraeff.github.com/roomcontrol/")

INCLUDE(nsis_installer)

SET(CPACK_PACKAGE_EXECUTABLES "" CACHE INTERNAL "")

# modify registry path after installation
IF(WIN32)
	add_subdirectory(tools/modify_registry_path)
	SET(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
		ExecWait \\\"$INSTDIR\\\\bin\\\\modify_registry_path.exe install\\\"
		")
	SET(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "
		ExecWait \\\"$INSTDIR\\\\bin\\\\modify_registry_path.exe uninstall\\\"
		")
ENDIF(WIN32)

# Distribution via cpack
INCLUDE(InstallRequiredSystemLibraries)
INCLUDE(CPack)

# Installation types
cpack_add_install_type(Full DISPLAY_NAME "Alles")
#cpack_add_install_type(ServerOnly DISPLAY_NAME "Nur Server und Serverplugins")
#cpack_add_install_type(ClientsOnly DISPLAY_NAME "Nur administrative Werkzeuge und Clientplugins")

# Component groups
cpack_add_component_group(Plugins
  DISPLAY_NAME "Plugins"
  EXPANDED
  DESCRIPTION "Server Plugins")

# Components
cpack_add_component(ServerPlugins
  DISPLAY_NAME "Server Plugins"
  DESCRIPTION "Die Serverversionen der Plugins"
  GROUP Plugins
  INSTALL_TYPES Full)

cpack_add_component(Server
  DISPLAY_NAME "Raumkontrollserver"
  DESCRIPTION "Serverprogramm und Dienstprogramme"
  INSTALL_TYPES Full)
# cpack_add_component(Tools
#   DISPLAY_NAME "Werkzeuge"
#   DESCRIPTION "Administrative Werkzeuge um Einstellungen am Server vorzunehmen oder Profile zu �ndern"
#   INSTALL_TYPES Full ClientsOnly)

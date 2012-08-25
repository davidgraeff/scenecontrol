cmake_minimum_required(VERSION 2.8.8)

# set default build type
IF (NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE release)
  SET(CMAKE_CONFIGURATION_TYPES RelWithDebInfo release Debug)
ENDIF (NOT CMAKE_BUILD_TYPE)

# Distribution via cpack: make package
IF(WIN32 AND NOT UNIX)
	# Use NSIS Installer
	INCLUDE(nsis_installer)
	SET(CPACK_GENERATOR "NSIS")
ELSE()
	SET(CPACK_GENERATOR "DEB")
	INCLUDE(cmake/Build_tools.cmake)
	execute_process(COMMAND ${TOOL_prepareReadme} "${CMAKE_CURRENT_SOURCE_DIR}/README.markdown" OUTPUT_VARIABLE CPACK_PACKAGE_DESCRIPTION_SUMMARY)
	SET(CPACK_DEBIAN_PACKAGE_SECTION "net")
	# Try to guess the architecture
	SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "i586")
	IF (${CMAKE_LIBRARY_ARCHITECTURE} MATCHES "^x86_64")
		SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
	ENDIF()
	IF (${CMAKE_LIBRARY_ARCHITECTURE} MATCHES "^arm")
		SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "armel")
	ENDIF()
	IF (${CMAKE_LIBRARY_ARCHITECTURE} MATCHES "[^ ]gnueabihf")
		SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "armhf")
	ENDIF()
	
	SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt4-network (>= 4:4.7.0), libpam0g (>= 1.1.1), libudev0 (>= 160), libssl1.0.0(>= 1.0.0)")
ENDIF()

# Package descriptions
SET(CPACK_PACKAGE_NAME "SceneControlSuite")
SET(CPACK_PACKAGE_VERSION_MAJOR "2")
SET(CPACK_PACKAGE_VERSION_MINOR "4")
SET(CPACK_PACKAGE_VERSION_PATCH "1")
SET(CPACK_PACKAGE_CONTACT "David Gräff <david.graeff@web.d_e>")
SET(CPACK_PACKAGE_VENDOR "David Gräff")
SET(CPACK_PROJECT_WEBURL "http://davidgraeff.github.com/scenecontrol/")
SET(CPACK_PACKAGE_DESCRIPTION "SceneControl server application, clients and plugins")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.markdown")
SET(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.markdown")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}")

# Package executables
SET(CPACK_STRIP_FILES 1)
SET(CPACK_PACKAGING_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")
SET(CPACK_PACKAGE_EXECUTABLES "" CACHE INTERNAL "")
INCLUDE(InstallRequiredSystemLibraries)

# Distribution via cpack
INCLUDE(CPack)

# Installation types
cpack_add_install_type(Full DISPLAY_NAME "Alles")

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

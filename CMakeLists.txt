cmake_minimum_required(VERSION 2.6)
PROJECT(roomcontrol_suite)
SET(PRODUCTID ${PROJECT_NAME})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

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
ENDIF()
SET(CPACK_PACKAGE_NAME "Roomcontrol Suite")
SET(CPACK_PACKAGE_VERSION_MAJOR "1")
SET(CPACK_PACKAGE_VERSION_MINOR "1")
SET(CPACK_PACKAGE_VERSION_PATCH "2")
SET(CPACK_PACKAGE_CONTACT "David Gräff <david.graeff@_web.de>")
SET(CPACK_PACKAGE_VENDOR "David Gräff")
#SET(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/clients/roomeditor/images\\\\serviceprovider.png")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "The roomcontrol server plus server plugins and cli, editor, data viewer clients and client plugins")
SET(CPACK_PACKAGE_DESCRIPTION "Plugin orientierte Steuersoftware für Raumautomatisierung. Plugins für Steckdosen, Rollo, Led Beleuchtung, PulseAudio, Videoabspielprogramm, Beamersteuerung und Zeitbasierte Ereignisse vorhanden und in Benutzung.")
SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt4-network (>= 4:4.7.1), libpam0g (>= 1.1.1)")
SET(CPACK_STRIP_FILES 1)
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}")

IF(WIN32 AND NOT UNIX)
  SET(CPACK_NSIS_INSTALLED_ICON_NAME "${CMAKE_CURRENT_SOURCE_DIR}/clients/roomeditor/images\\\\app.ico")
  SET(CPACK_NSIS_MUI_ICON "${CPACK_NSIS_INSTALLED_ICON_NAME}")
  SET(CPACK_NSIS_MUI_UNIICON "${CPACK_NSIS_INSTALLED_ICON_NAME}")
  SET(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_NAME} ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}")
  SET(CPACK_NSIS_HELP_LINK "http://github.com/davidgraeff/roomcontrol")
  SET(CPACK_NSIS_URL_INFO_ABOUT "http://github.com/davidgraeff/roomcontrol")
  SET(CPACK_NSIS_MODIFY_PATH OFF)
ENDIF(WIN32 AND NOT UNIX)
SET(CPACK_PACKAGE_EXECUTABLES "" CACHE INTERNAL "")

set (ROOTDIR "${CMAKE_CURRENT_SOURCE_DIR}")
set (SHAREDDIR "${ROOTDIR}/shared")
set (SHAREDCLIENTDIR "${SHAREDDIR}/client")

# install path of libs
IF(WIN32)
	IF (CMAKE_BUILD_TYPE EQUAL "DEBUG" OR CMAKE_BUILD_TYPE EQUAL "Debug")
		SET(LIBPATH "lib/debug")
	ELSE()
		SET(LIBPATH "lib")
	ENDIF()
	SET(CERTPATH ".")
ELSE()
	IF (CMAKE_BUILD_TYPE EQUAL "DEBUG" OR CMAKE_BUILD_TYPE EQUAL "Debug")
		SET(LIBPATH "lib/debug/${PRODUCTID}")
	ELSE()
		SET(LIBPATH "lib/${PRODUCTID}")
	ENDIF()
	SET(CERTPATH "shares/${PRODUCTID}")
ENDIF()

SET(CONFIG_SERVER_LISTENPORT "3101" CACHE INT "Server Listenport")
configure_file(config.h.in config.h @ONLY)

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

# prepare configuration file
SET(ROOM_VERSION "${CPACK_PACKAGE_VERSION}")
SET(ROOM_PATHNAME "${PRODUCTID}")
SET(CONFIG_SERVER_LISTENPORT "3101" CACHE INT "Server Listenport")
configure_file(config.h.in config.h @ONLY)

# add directories
add_subdirectory(server)
add_subdirectory(clients)
add_subdirectory(plugins)

# Distribution via cpack
INCLUDE(InstallRequiredSystemLibraries)
INCLUDE(CPack)

# Installation types
cpack_add_install_type(Full DISPLAY_NAME "Alles")
cpack_add_install_type(ServerOnly DISPLAY_NAME "Nur Server und Serverplugins")
cpack_add_install_type(ClientsOnly DISPLAY_NAME "Nur administrative Werkzeuge und Clientplugins")

# Component groups
cpack_add_component_group(Plugins
  DISPLAY_NAME "Plugins"
  EXPANDED
  DESCRIPTION "Server und Client Plugins")

# Components
cpack_add_component(ClientPlugins
  DISPLAY_NAME "Client Plugins"
  DESCRIPTION "Die Clientversionen der Plugins"
  GROUP Plugins
  INSTALL_TYPES Full ClientsOnly)
cpack_add_component(ServerPlugins
  DISPLAY_NAME "Server Plugins"
  DESCRIPTION "Die Serverversionen der Plugins"
  GROUP Plugins
  INSTALL_TYPES Full ServerOnly)

cpack_add_component(Server
  DISPLAY_NAME "Raumkontrollserver"
  DESCRIPTION "Serverprogramm und Dienstprogramme"
  INSTALL_TYPES Full ServerOnly)
cpack_add_component(Tools
  DISPLAY_NAME "Werkzeuge"
  DESCRIPTION "Administrative Werkzeuge um Einstellungen am Server vorzunehmen oder Profile zu ändern"
  INSTALL_TYPES Full ClientsOnly)

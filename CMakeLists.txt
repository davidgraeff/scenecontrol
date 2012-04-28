cmake_minimum_required(VERSION 2.8)
PROJECT(roomcontrol_suite)

SET(PRODUCTID ${PROJECT_NAME})
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
INCLUDE(InstallMacros)

# Distribution via cpack. Set variables
INCLUDE(InstallRequiredSystemLibraries)
IF(WIN32 AND NOT UNIX)
SET(CPACK_GENERATOR "NSIS")
ELSE()
SET(CPACK_GENERATOR "DEB")
ENDIF()
SET(CPACK_PACKAGE_NAME "RoomcontrolSuite")
SET(CPACK_PACKAGE_VERSION_MAJOR "2")
SET(CPACK_PACKAGE_VERSION_MINOR "3")
SET(CPACK_PACKAGE_VERSION_PATCH "1")
SET(CPACK_PACKAGE_CONTACT "David Gräff <david.graeff@web.d_e>")
SET(CPACK_PACKAGE_VENDOR "David Gräff")
#SET(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/clients/roomeditor/images\\\\serviceprovider.png")
SET(CPACK_PACKAGE_DESCRIPTION "Roomcontrol server application, clients and plugins")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.markdown")
file(STRINGS ${CPACK_PACKAGE_DESCRIPTION_FILE} description)
 STRING(REGEX REPLACE ";" "\n " description "${description}")
 SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${description})
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

set (ROOTDIR "${CMAKE_CURRENT_SOURCE_DIR}")
set (SHAREDPLUGINDIR "${ROOTDIR}/plugins/_sharedsrc")

# install path of libs
IF(WIN32)
	IF (CMAKE_BUILD_TYPE EQUAL "DEBUG" OR CMAKE_BUILD_TYPE EQUAL "Debug")
		SET(LIBPATH "lib/debug")
	ELSE()
		SET(LIBPATH "lib")
	ENDIF()
	SET(DATAPATH ".")
	set(LASTCOMMITDATE "")
	set(COMMITHASH "")
ELSE()
	IF (CMAKE_BUILD_TYPE EQUAL "DEBUG" OR CMAKE_BUILD_TYPE EQUAL "Debug")
		SET(LIBPATH "lib/debug/${PRODUCTID}")
	ELSE()
		SET(LIBPATH "lib/${PRODUCTID}")
	ENDIF()
	SET(DATAPATH "share/${PRODUCTID}")
	execute_process(COMMAND /usr/bin/git log --pretty=format:%ad -n 1 --date=iso
					WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
					OUTPUT_VARIABLE LASTCOMMITDATE)
	execute_process(COMMAND /usr/bin/git log --pretty=format:%H -n 1
					WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
					OUTPUT_VARIABLE COMMITHASH)
	MESSAGE(STATUS "Exact version: ${LASTCOMMITDATE} ${COMMITHASH}")
ENDIF()

INCLUDE(configfile)

# Find Qt and boost for the mongodb c++ client api
find_package(Qt4 4.7.2 COMPONENTS QtCore QtNetwork QUIET)
IF (NOT ${QT_FOUND})
message(FATAL_ERROR "Qt libraries not intalled!")
	return()
ENDIF()

# add directories
add_subdirectory(libdatabase)
add_subdirectory(server)
add_subdirectory(plugins)
add_subdirectory(clients)
add_subdirectory(tools/startonboot)

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
#   DESCRIPTION "Administrative Werkzeuge um Einstellungen am Server vorzunehmen oder Profile zu ändern"
#   INSTALL_TYPES Full ClientsOnly)

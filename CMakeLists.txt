cmake_minimum_required(VERSION 2.6)

# set default build type
IF (NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release)
  SET(CMAKE_CONFIGURATION_TYPES RelWithDebInfo Release Debug)
ENDIF (NOT CMAKE_BUILD_TYPE)

INCLUDE(InstallRequiredSystemLibraries)
SET(CPACK_GENERATOR "DEB")
SET(CPACK_PACKAGE_NAME "roomcontrol")
SET(CPACK_PACKAGE_VERSION "1.0.22")
SET(CPACK_PACKAGE_CONTACT "David Gräff <david.graeff>")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "The roomcontrol server, cli client, editor, data viewer and all plugins")
SET(CPACK_PACKAGE_DESCRIPTION "Plugin orientierte Steuersoftware für Raumautomatisierung. Plugins für
Steckdosen, Rollo, Led Beleuchtung, PulseAudio, Videoabspielprogramm,
Beamersteuerung und Zeitbasierte Ereignisse vorhanden und in Benutzung.")
SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt4-network (>= 4:4.7.1), libpam0g (>= 1.1.1)")

add_subdirectory(clients)
add_subdirectory(server)
add_subdirectory(plugins)

INCLUDE(CPack)


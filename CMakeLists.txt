cmake_minimum_required(VERSION 2.8)
PROJECT(roomcontrol_suite)

SET(PRODUCTID ${PROJECT_NAME}) # define PRODUCTID otherwise all other CMakeFiles will fail
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
INCLUDE(cmake/InstallMacros.cmake)
INCLUDE(cmake/basevariables.cmake)
INCLUDE(cmake/configfile.cmake)

# Find Qt
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

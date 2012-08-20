cmake_minimum_required(VERSION 2.8.8)
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

# Shared source
include_directories(${QT_INCLUDES} ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})
file(GLOB SRCS_SHARED_PLUGINS "${CMAKE_CURRENT_SOURCE_DIR}/shared/plugins/*.cpp")
QT4_WRAP_CPP(SRCS_SHARED_PLUGINS_MOCS "${CMAKE_CURRENT_SOURCE_DIR}/shared/plugins/abstractplugin.h")
add_library(basePluginLib OBJECT ${SRCS_SHARED_PLUGINS} ${SRCS_SHARED_PLUGINS_MOCS})
file(GLOB SRCS_SHARED_JSON "${CMAKE_CURRENT_SOURCE_DIR}/shared/jsondocuments/*.cpp")
add_library(jsonDocumentsLib OBJECT ${SRCS_SHARED_JSON})

# add directories
add_subdirectory(libdatastorage)
add_subdirectory(server)
add_subdirectory(plugins)
add_subdirectory(editor)
add_subdirectory(tools/startonboot)

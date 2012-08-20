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
file(GLOB SRCS_SHARED_JSON "${CMAKE_CURRENT_SOURCE_DIR}/shared/jsondocuments/*.cpp") # JSon Document shared source
add_library(jsonDocumentsLib OBJECT ${SRCS_SHARED_JSON})
file(GLOB SRCS_SHARED_UTILS "${CMAKE_CURRENT_SOURCE_DIR}/shared/utils/*.cpp") # Utils shared source
add_library(utilsLib OBJECT ${SRCS_SHARED_UTILS})
file(GLOB SRCS_SHARED_PLUGINS "${CMAKE_CURRENT_SOURCE_DIR}/shared/plugins/*.cpp") # Plugins shared source: Uses sources from utils and json document, too.
QT4_WRAP_CPP(SRCS_SHARED_PLUGINS_MOCS "${CMAKE_CURRENT_SOURCE_DIR}/shared/plugins/abstractplugin.h")
add_library(PluginLib OBJECT ${SRCS_SHARED_PLUGINS} ${SRCS_SHARED_PLUGINS_MOCS} ${SRCS_SHARED_UTILS} ${SRCS_SHARED_JSON})

# add directories
add_subdirectory(libdatastorage)
add_subdirectory(server)
add_subdirectory(plugins)
add_subdirectory(editor)
add_subdirectory(tools/startonboot)

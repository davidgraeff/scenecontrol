cmake_minimum_required(VERSION 2.8.8)
#Only allow this file to be included by the root cmake file
IF (NOT DEFINED PRODUCTID)
	RETURN()
ENDIF()

OPTION(BUILD_PLUGINS "Build Plugins" ON)
IF (NOT BUILD_PLUGINS)
	RETURN()
ENDIF()

# Find Qt
find_package(Qt4 4.7.2 COMPONENTS QtCore QtNetwork QUIET)
IF (NOT ${QT_FOUND})
	message(FATAL_ERROR "Qt libraries not intalled!")
	return()
ENDIF()

# Shared source
ADD_DEFINITIONS(-D__STRICT_ANSI__)
include_directories(${QT_INCLUDES} ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})
file(GLOB SRCS_SHARED_JSON "${CMAKE_CURRENT_SOURCE_DIR}/_shared/jsondocuments/*.cpp") # JSon Document shared source
add_library(jsonDocumentsLib OBJECT ${SRCS_SHARED_JSON})
file(GLOB SRCS_SHARED_UTILS "${CMAKE_CURRENT_SOURCE_DIR}/_shared/utils/*.cpp") # Utils shared source
add_library(utilsLib OBJECT ${SRCS_SHARED_UTILS})
file(GLOB SRCS_SHARED_PLUGINS "${CMAKE_CURRENT_SOURCE_DIR}/_shared/plugins/*.cpp") # Plugins shared source: Uses sources from utils and json document, too.
QT4_WRAP_CPP(SRCS_SHARED_PLUGINS_MOCS "${CMAKE_CURRENT_SOURCE_DIR}/_shared/plugins/abstractplugin.h")
add_library(PluginLib OBJECT ${SRCS_SHARED_PLUGINS} ${SRCS_SHARED_PLUGINS_MOCS} ${SRCS_SHARED_UTILS} ${SRCS_SHARED_JSON})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/_shared ${CMAKE_CURRENT_SOURCE_DIR}/_shared/plugins/)

SET(PLUGINSDIR ${CMAKE_CURRENT_SOURCE_DIR})

FILE(GLOB children RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*)
FOREACH(child ${children})
IF(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${child} AND NOT ${child} STREQUAL "_prototyp" AND NOT ${child} STREQUAL "_shared")
		ADD_SUBDIRECTORY(${child})
	ENDIF()
ENDFOREACH()

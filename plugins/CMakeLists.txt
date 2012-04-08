cmake_minimum_required(VERSION 2.8)
#Only allow this file to be included by the root cmake file
IF (NOT DEFINED PRODUCTID)
	RETURN()
ENDIF()

OPTION(BUILD_PLUGINS "Build Plugins" ON)
IF (BUILD_PLUGINS)
	file(GLOB SRCS_SHARED_PLUGINS_H _sharedsrc/*.h)
	file(GLOB SRCS_SHARED_PLUGINS _sharedsrc/*.cpp)
	SET(PLUGINSDIR ${CMAKE_CURRENT_SOURCE_DIR})
	
	FILE(GLOB children RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*)
	FOREACH(child ${children})
		IF(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${child} AND NOT ${child} STREQUAL "_sharedsrc")
			ADD_SUBDIRECTORY(${child})
		ENDIF()
	ENDFOREACH()
ENDIF (BUILD_PLUGINS)

cmake_minimum_required(VERSION 2.8)

# project name
get_filename_component(targetname ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${targetname})
message(STATUS "Configure Plugin: ${targetname}")

#Only allow this file to be included by the root cmake file
IF (NOT DEFINED PRODUCTID)
	RETURN()
ENDIF()

find_package(Qt4 4.7.0 COMPONENTS QtCore REQUIRED)

get_filename_component(ROOTDIR "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
#set(ROOTDIR "${CMAKE_CURRENT_SOURCE_DIR}/../..")
set(SHAREDDIR "${ROOTDIR}/shared")
set(PLUGINDIR "${ROOTDIR}/plugins")

LIST(APPEND Shared_SRCS_H "")
LIST(APPEND Shared_SRCS "${SHAREDDIR}/pluginhelper.cpp")

LIST(APPEND SRCS_SERVER "plugin.cpp")
LIST(APPEND SRCS_SERVER_H "plugin.h")

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${ROOTDIR} ${PLUGINDIR})

# rs232
if (DEFINED USE_SERIALPORT)
	LIST(APPEND SRCS_SERVER_H ${SHAREDDIR}/qextserialport/qextserialport.h)
	LIST(APPEND SRCS_SERVER ${SHAREDDIR}/qextserialport/qextserialport.cpp)
	IF (WIN32)
		LIST(APPEND SRCS_SERVER ${SHAREDDIR}/qextserialport/win_qextserialport.cpp)
	ELSE()
		LIST(APPEND SRCS_SERVER ${SHAREDDIR}/qextserialport/posix_qextserialport.cpp)
	ENDIF()
endif()

#-DQT_NO_CAST_FROM_ASCII
ADD_DEFINITIONS(-D_GNU_SOURCE -Wall -W -DQT_NO_CAST_TO_ASCII -DQT_USE_FAST_OPERATOR_PLUS -DQT_USE_FAST_CONCATENATION ${QT_DEFINITIONS} ${QDBUS_DEFINITIONS})


macro(build_server_lib)
	QT4_WRAP_CPP(SRCS_MOCS_SERVER ${SRCS_SERVER_H} ${Shared_SRCS_H} ${ADD_H})
	add_library(${PROJECT_NAME}_server SHARED ${SRCS_SERVER} ${Shared_SRCS} ${SRCS_MOCS_SERVER} ${ADD_CPP})
endmacro(build_server_lib)

macro(install_server_lib)
	INSTALL(TARGETS ${PROJECT_NAME}_server
		RUNTIME DESTINATION ${LIBPATH}/server/
		COMPONENT ServerPlugins
		LIBRARY DESTINATION ${LIBPATH}/server/
		COMPONENT ServerPlugins
		)
	FILE(GLOB files "www/*")
	install( FILES ${files} DESTINATION ${WWWPATH} COMPONENT ServerPlugins)
endmacro()


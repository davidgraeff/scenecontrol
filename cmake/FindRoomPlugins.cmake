cmake_minimum_required(VERSION 2.8)

#Only allow this file to be included by the root cmake file
IF (NOT DEFINED PRODUCTID)
	RETURN()
ENDIF()

# project name
get_filename_component(targetname ${CMAKE_CURRENT_SOURCE_DIR} NAME)
message(STATUS "Configure Plugin: ${targetname}")
project(${targetname}_plugin)

find_package(Qt4 4.7.0 COMPONENTS QtCore REQUIRED)

# get_filename_component(ROOTDIR "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
# #set(ROOTDIR "${CMAKE_CURRENT_SOURCE_DIR}/../..")
# set(SHAREDDIR "${ROOTDIR}/shared")
# set(PLUGINDIR "${ROOTDIR}/plugins")

LIST(APPEND Shared_SRCS_H "${SHAREDDIR}/abstractplugin.h")
LIST(APPEND Shared_SRCS "${SHAREDDIR}/plugineventmap.cpp" "${SHAREDDIR}/abstractplugin.cpp" "${SHAREDDIR}/pluginservicehelper.cpp" "${SHAREDDIR}/json.cpp")

file(GLOB SRCS_SERVER "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB SRCS_SERVER_H "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${ROOTDIR} ${PLUGINDIR})

# rs232
if (DEFINED USE_SERIALPORT)
	LIST(APPEND SRCS_SERVER_H ${SHAREDDIR}/qxtserialdevice/qxtserialdevice.h ${SHAREDDIR}/qxtserialdevice/qxtserialdevice_p.h)
	LIST(APPEND SRCS_SERVER ${SHAREDDIR}/qxtserialdevice/qxtserialdevice.cpp)
	IF (WIN32)
		#LIST(APPEND SRCS_SERVER ${SHAREDDIR}/qxtserialdevice/win_qxtserialdevice.cpp)
	ELSE()
		LIST(APPEND SRCS_SERVER ${SHAREDDIR}/qxtserialdevice/qxtserialdevice_unix.cpp)
	ENDIF()
endif()

#-DQT_NO_CAST_FROM_ASCII
ADD_DEFINITIONS(-DPLUGIN_ID="${targetname}" -D_GNU_SOURCE -Wall -W -DQT_NO_CAST_TO_ASCII -DQT_USE_FAST_OPERATOR_PLUS -DQT_USE_FAST_CONCATENATION ${QT_DEFINITIONS})


macro(build_server_lib)
	QT4_WRAP_CPP(SRCS_MOCS_SERVER ${SRCS_SERVER_H} ${Shared_SRCS_H} ${ADD_H})
	add_executable(${PROJECT_NAME} ${SRCS_SERVER} ${Shared_SRCS} ${SRCS_MOCS_SERVER} ${ADD_CPP})
	GET_TARGET_PROPERTY(BINARY_NAME ${PROJECT_NAME} OUTPUT_NAME)
endmacro(build_server_lib)

macro(install_server_lib)
	INSTALL(DIRECTORY "couchdb/" DESTINATION ${ROOM_COUCHDBPATH}/${targetname} COMPONENT ServerPlugins)
	INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${LIBPATH} COMPONENT ServerPlugins)
	SET(CPACK_PACKAGE_EXECUTABLES ${CPACK_PACKAGE_EXECUTABLES} ${PROJECT_NAME} CACHE INTERNAL "")
endmacro()


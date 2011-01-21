cmake_minimum_required(VERSION 2.8)
get_filename_component(targetname ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${targetname}Plugin)
find_package(Qt4 REQUIRED)

set(ROOTDIR "${CMAKE_CURRENT_SOURCE_DIR}/../..")
set(SHAREDDIR "${ROOTDIR}/shared")
set(PLUGINDIR "${ROOTDIR}/plugins")
#file(GLOB_RECURSE Shared_SRCS_H "${SHAREDDIR}/services/*.h" )
#file(GLOB_RECURSE Shared_SRCS "${SHAREDDIR}/services/*.cpp")
set(Shared_SRCS_H ${Shared_SRCS_H} "${SHAREDDIR}/abstractserviceprovider.h" "${SHAREDDIR}/abstractstatetracker.h" "${SHAREDDIR}/abstractplugin.h") 
set(Shared_SRCS ${Shared_SRCS} "${SHAREDDIR}/abstractserviceprovider.cpp" "${SHAREDDIR}/abstractstatetracker.cpp")

set(SharedServer_SRCS_H ${Shared_SRCS_H} "${SHAREDDIR}/server/executeservice.h" "${SHAREDDIR}/server/executeWithBase.h" "${SHAREDDIR}/server/executeplugin.h")
set(SharedServer_SRCS ${Shared_SRCS} "${SHAREDDIR}/server/executeservice.cpp" "${SHAREDDIR}/server/executeWithBase.cpp")
set(SharedClient_SRCS_H ${Shared_SRCS_H} "${SHAREDDIR}/client/clientplugin.h")
set(SharedClient_SRCS ${Shared_SRCS})

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${ROOTDIR} ${PLUGINDIR})

#client
file(GLOB_RECURSE SRCS_CLIENT services/*.cpp statetracker/*.cpp client/*.cpp plugin.cpp)
file(GLOB_RECURSE SRCS_CLIENT_H services/*.h statetracker/*.h client/*.h plugin.h)

#server
file(GLOB_RECURSE SRCS_SERVER services/*.cpp statetracker/*.cpp services_server/*.cpp server/*.cpp plugin.cpp)
file(GLOB_RECURSE SRCS_SERVER_H services/*.h statetracker/*.h services_server/*.h server/*.h plugin.h)
if (DEFINED USE_SERIALPORT)
  set(SRCS_SERVER_H ${SRCS_SERVER_H} ${PLUGINDIR}/shared/qextserialport.h)
  set(SRCS_SERVER ${SRCS_SERVER} ${PLUGINDIR}/shared/qextserialport.cpp)
endif()

ADD_DEFINITIONS(-D_GNU_SOURCE -Wall -W -DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII -DQT_USE_FAST_OPERATOR_PLUS -DQT_USE_FAST_CONCATENATION ${QT_DEFINITIONS} ${QDBUS_DEFINITIONS})

macro(build_client_lib)
	QT4_WRAP_CPP(SRCS_MOCS_CLIENT ${SRCS_CLIENT_H} ${SharedClient_SRCS_H})
	add_library(${PROJECT_NAME}_client SHARED ${SRCS_CLIENT} ${SharedClient_SRCS} ${SRCS_MOCS_CLIENT})
endmacro(build_client_lib)

macro(build_server_lib)
	QT4_WRAP_CPP(SRCS_MOCS_SERVER ${SRCS_SERVER_H} ${SharedServer_SRCS_H})
	add_library(${PROJECT_NAME}_server SHARED ${SRCS_SERVER} ${SharedServer_SRCS} ${SRCS_MOCS_SERVER})
endmacro(build_server_lib)

set(SERVERNAME "RoomControlServer")
SET(ROOM_SYSTEM_CLIENTPLUGINS "lib/${SERVERNAME}/plugins/client" CACHE STRING "Set the directory where client plugins are located")
SET(ROOM_SYSTEM_SERVERPLUGINS "lib/${SERVERNAME}/plugins/server" CACHE STRING "Set the directory where server plugins are located")

macro(install_client_lib)
	INSTALL(TARGETS ${PROJECT_NAME}_client DESTINATION ${ROOM_SYSTEM_CLIENTPLUGINS} CONFIGURATIONS Debug Release RelWithDebInfo)
endmacro()

macro(install_server_lib)
	INSTALL(TARGETS ${PROJECT_NAME}_server DESTINATION ${ROOM_SYSTEM_SERVERPLUGINS} CONFIGURATIONS Debug Release RelWithDebInfo)
endmacro()


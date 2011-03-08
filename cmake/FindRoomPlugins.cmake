cmake_minimum_required(VERSION 2.8)

# project name
get_filename_component(targetname ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${targetname}Plugin)
message(STATUS "Configure Plugin: ${targetname}")

IF (NOT DEFINED PRODUCTID)
	RETURN()
ENDIF()

find_package(Qt4 4.7.0 COMPONENTS QtCore QtGui REQUIRED)

get_filename_component(ROOTDIR "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
#set(ROOTDIR "${CMAKE_CURRENT_SOURCE_DIR}/../..")
set(SHAREDDIR "${ROOTDIR}/shared")
set(PLUGINDIR "${ROOTDIR}/plugins")
#file(GLOB_RECURSE Shared_SRCS_H "${SHAREDDIR}/services/*.h" )
#file(GLOB_RECURSE Shared_SRCS "${SHAREDDIR}/services/*.cpp")
set(Shared_SRCS_H ${Shared_SRCS_H} "${SHAREDDIR}/abstractserviceprovider.h" "${SHAREDDIR}/abstractstatetracker.h" "${SHAREDDIR}/abstractplugin.h") 
set(Shared_SRCS ${Shared_SRCS} "${SHAREDDIR}/abstractserviceprovider.cpp" "${SHAREDDIR}/abstractstatetracker.cpp")

set(SharedServer_SRCS_H ${Shared_SRCS_H} "${SHAREDDIR}/server/executeservice.h" "${SHAREDDIR}/server/executeWithBase.h" "${SHAREDDIR}/server/executeplugin.h")
set(SharedServer_SRCS ${Shared_SRCS} "${SHAREDDIR}/server/executeservice.cpp" "${SHAREDDIR}/server/executeWithBase.cpp" "${SHAREDDIR}/server/executeplugin.cpp")
set(SharedClient_SRCS_H ${Shared_SRCS_H} "${SHAREDDIR}/client/clientplugin.h" "${SHAREDDIR}/client/servicestorage.h" "${SHAREDDIR}/client/modelstorage.h")
set(SharedClient_SRCS ${Shared_SRCS} "${SHAREDDIR}/client/clientplugin.cpp" "${SHAREDDIR}/client/servicestorage.cpp" "${SHAREDDIR}/client/modelstorage.cpp")

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${ROOTDIR} ${PLUGINDIR})

#server
file(GLOB_RECURSE SRCS_SERVER services/*.cpp statetracker/*.cpp services_server/*.cpp server/*.cpp plugin.cpp)
file(GLOB_RECURSE SRCS_SERVER_H services/*.h statetracker/*.h services_server/*.h server/*.h plugin.h)
if (DEFINED USE_SERIALPORT)
	LIST(APPEND SRCS_SERVER_H ${SHAREDDIR}/server/qextserialport/qextserialport.h)
	LIST(APPEND SRCS_SERVER ${SHAREDDIR}/server/qextserialport/qextserialport.cpp)
	IF (WIN32)
		LIST(APPEND SRCS_SERVER ${SHAREDDIR}/server/qextserialport/win_qextserialport.cpp)
	ELSE()
		LIST(APPEND SRCS_SERVER ${SHAREDDIR}/server/qextserialport/posix_qextserialport.cpp)
	ENDIF()
endif()

ADD_DEFINITIONS(-D_GNU_SOURCE -Wall -W -DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII -DQT_USE_FAST_OPERATOR_PLUS -DQT_USE_FAST_CONCATENATION ${QT_DEFINITIONS} ${QDBUS_DEFINITIONS})


macro(build_server_lib)
	QT4_WRAP_CPP(SRCS_MOCS_SERVER ${SRCS_SERVER_H} ${SharedServer_SRCS_H} ${ADD_H})
	add_library(${PROJECT_NAME}_server SHARED ${SRCS_SERVER} ${SharedServer_SRCS} ${SRCS_MOCS_SERVER} ${ADD_CPP})
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

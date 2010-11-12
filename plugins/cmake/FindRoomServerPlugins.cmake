cmake_minimum_required(VERSION 2.8)
get_filename_component(targetname ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${targetname}Plugin)
find_package(Qt4 REQUIRED)

set(ROOTDIR ${CMAKE_CURRENT_SOURCE_DIR}/../..)
set(PLUGINDIR "${ROOTDIR}/plugins")
set(Shared_SRCS_H "${ROOTDIR}/shared/abstractserviceprovider.h" "${ROOTDIR}/shared/abstractstatetracker.h" "${ROOTDIR}/shared/abstractplugin.h") 
set(Shared_SRCS "${ROOTDIR}/shared/abstractserviceprovider.cpp" "${ROOTDIR}/shared/abstractstatetracker.cpp")

set(SERVERDIR "${ROOTDIR}/server")
set(SharedServer_SRCS_H ${Shared_SRCS_H} "${ROOTDIR}/shared/server/executeservice.h" "${ROOTDIR}/shared/server/executeWithBase.h" "${ROOTDIR}/shared/server/executeplugin.h")
set(SharedServer_SRCS ${Shared_SRCS} "${ROOTDIR}/shared/server/executeservice.cpp" "${ROOTDIR}/shared/server/executeWithBase.cpp")

set(CLIENTDIR "${ROOTDIR}/clients")
set(SharedClient_SRCS_H ${Shared_SRCS_H} "${CLIENTDIR}/clientplugin.h")
set(SharedClient_SRCS ${Shared_SRCS})

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${ROOTDIR} ${SERVERDIR} ${CLIENTDIR} ${PLUGINDIR})

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

macro(build_lib)
ADD_DEFINITIONS(-D_GNU_SOURCE -Wall -W -DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII -DQT_USE_FAST_OPERATOR_PLUS -DQT_USE_FAST_CONCATENATION ${QT_DEFINITIONS} ${QDBUS_DEFINITIONS})
QT_WRAP_CPP(${PROJECT_NAME}_client SRCS_MOCS ${SRCS_CLIENT_H} ${SharedClient_SRCS_H})
QT_WRAP_CPP(${PROJECT_NAME}_server SRCS_MOCS ${SRCS_SERVER_H} ${SharedServer_SRCS_H})
add_library(${PROJECT_NAME}_client SHARED ${SRCS_CLIENT} ${SharedClient_SRCS})
add_library(${PROJECT_NAME}_server SHARED ${SRCS_SERVER} ${SharedServer_SRCS})
endmacro(build_lib)

macro(install_lib)
INSTALL(TARGETS ${PROJECT_NAME}_server DESTINATION lib/roomcontrol/plugins/server CONFIGURATIONS Debug Release RelWithDebInfo)

INSTALL(TARGETS ${PROJECT_NAME}_client DESTINATION lib/roomcontrol/plugins/client CONFIGURATIONS Debug Release RelWithDebInfo)
endmacro(install_lib)
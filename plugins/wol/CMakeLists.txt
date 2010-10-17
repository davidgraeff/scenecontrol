get_filename_component(targetname ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${targetname}Plugin)
cmake_minimum_required(VERSION 2.6)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
find_package(Qt4 REQUIRED)

set(PLUGINDIR ${CMAKE_CURRENT_SOURCE_DIR}/..)
set(Shared_SRCS_H "${PLUGINDIR}/shared/abstractserviceprovider.h" "${PLUGINDIR}/shared/abstractstatetracker.h" "${PLUGINDIR}/shared/abstractplugin.h") 
set(Shared_SRCS "${PLUGINDIR}/shared/abstractserviceprovider.cpp" "${PLUGINDIR}/shared/abstractstatetracker.cpp")

set(SERVERDIR "${CMAKE_CURRENT_SOURCE_DIR}/../../server")
set(SharedServer_SRCS_H ${Shared_SRCS_H} "${SERVERDIR}/executeservice.h" "${SERVERDIR}/executeWithBase.h" "${SERVERDIR}/executeplugin.h")
set(SharedServer_SRCS ${Shared_SRCS} "${SERVERDIR}/executeservice.cpp" "${SERVERDIR}/executeWithBase.cpp")

#base
file(GLOB_RECURSE SRCS_BASE services/*.cpp statetracker/*.cpp)
file(GLOB_RECURSE SRCS_BASE_H services/*.h statetracker/*.h)
set(SRCS_BASE_H ${SRCS_BASE_H} plugin.h)
QT_WRAP_CPP(${PROJECT_NAME} SRCS_MOCS ${SRCS_BASE_H} ${Shared_SRCS_H})
set(SRCS_BASE ${SRCS_BASE} ${SRCS_MOCS} plugin.cpp)

#server
file(GLOB_RECURSE SRCS_SERVER services_server/*.cpp)
file(GLOB_RECURSE SRCS_SERVER_H services_server/*.h)
set(SRCS_SERVER_H ${SRCS_SERVER_H} plugin.h plugin_server.h)
QT_WRAP_CPP(${PROJECT_NAME}_server SRCS_MOCS ${SRCS_BASE_H} ${SRCS_SERVER_H} ${SharedServer_SRCS_H})
set(SRCS_SERVER ${SRCS_SERVER} ${SRCS_BASE} ${SRCS_MOCS} plugin_server.cpp)

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ../ ../../server)

ADD_DEFINITIONS(-D_GNU_SOURCE -Wall -W -DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII -DQT_USE_FAST_OPERATOR_PLUS -DQT_USE_FAST_CONCATENATION ${QT_DEFINITIONS} ${QDBUS_DEFINITIONS})

add_library(${PROJECT_NAME} SHARED ${SRCS_BASE} ${Shared_SRCS} plugin_base.cpp)
target_link_libraries(${PROJECT_NAME} ${QT_QTCORE_LIBRARY})
INSTALL(TARGETS ${PROJECT_NAME} DESTINATION lib/roomcontrol/plugins CONFIGURATIONS Debug Release RelWithDebInfo)

add_library(${PROJECT_NAME}_server SHARED ${SRCS_SERVER} ${SharedServer_SRCS})
target_link_libraries(${PROJECT_NAME}_server ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARIES})
INSTALL(TARGETS ${PROJECT_NAME}_server DESTINATION lib/roomcontrol/plugins/server CONFIGURATIONS Debug Release RelWithDebInfo)

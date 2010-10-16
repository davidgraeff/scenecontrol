get_filename_component(targetname ${CMAKE_SOURCE_DIR} NAME)
project(${targetname}Plugin)
cmake_minimum_required(VERSION 2.6)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
find_package(Qt4 REQUIRED)

set(Shared_SRCS_H "../shared/abstractserviceprovider.h" "../shared/abstractstatetracker.h" "../shared/abstractplugin.h") 
set(Shared_SRCS "../shared/abstractserviceprovider.cpp" "../shared/abstractstatetracker.cpp")

#base
file(GLOB_RECURSE SRCS_BASE services/*.cpp statetracker/*.cpp)
file(GLOB_RECURSE SRCS_BASE_H services/*.h statetracker/*.h)
set(SRCS_BASE_H ${SRCS_BASE_H} plugin.h)
QT_WRAP_CPP(${PROJECT_NAME}_server SRCS_MOCS ${SRCS_BASE_H})
set(SRCS_BASE ${SRCS_BASE} ${SRCS_MOCS} plugin_base.cpp plugin.cpp)
#QT4_AUTOMOC(${SRCS_BASE} ${Shared_SRCS})

#server
file(GLOB_RECURSE SRCS_SERVER services_server/*.cpp)
file(GLOB_RECURSE SRCS_SERVER_H services_server/*.h)
set(SRCS_SERVER_H ${SRCS_SERVER_H} plugin.h plugin_server.h ../../server/executeservice.h ../../server/executeplugin.h)
QT_WRAP_CPP(${PROJECT_NAME}_server SRCS_MOCS ${SRCS_SERVER_H})
set(SRCS_SERVER ${SRCS_SERVER} ${SRCS_MOCS} plugin.cpp plugin_server.cpp ../../server/executeservice.cpp)
#QT4_AUTOMOC(${SRCS_SERVER}} ${Shared_SRCS})

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR} ../ ../../server)

ADD_DEFINITIONS(-D_GNU_SOURCE -Wall -W -DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII -DQT_USE_FAST_OPERATOR_PLUS -DQT_USE_FAST_CONCATENATION ${QT_DEFINITIONS} ${QDBUS_DEFINITIONS})

add_library(${PROJECT_NAME} SHARED ${SRCS_BASE} ${Shared_SRCS})
target_link_libraries(${PROJECT_NAME} ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARIES} ${QT_QTDBUS_LIBRARIES})
INSTALL(TARGETS ${PROJECT_NAME} DESTINATION lib/roomcontrol/plugins CONFIGURATIONS Debug Release RelWithDebInfo)

add_library(${PROJECT_NAME}_server SHARED ${SRCS_SERVER} ${Shared_SRCS})
target_link_libraries(${PROJECT_NAME}_server ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARIES} ${QT_QTDBUS_LIBRARIES})
INSTALL(TARGETS ${PROJECT_NAME}_server DESTINATION lib/roomcontrol/plugins/server CONFIGURATIONS Debug Release RelWithDebInfo)

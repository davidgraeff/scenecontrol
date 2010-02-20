project(roomcontrolClient)
cmake_minimum_required(VERSION 2.6)
find_package(Qt4 REQUIRED)

# search KDE4 and use apply default (but optional) settings
find_package(KDE4 REQUIRED)
include(KDE4Defaults)

include_directories(${QT_INCLUDES} ${QT_QTNETWORK_INCLUDE_DIR} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} 
${CMAKE_CURRENT_BINARY_DIR} framework framework/network)

add_definitions(${QT_DEFINITIONS} ${QDBUS_DEFINITIONS} ${KDE4_DEFINITIONS})

ADD_SUBDIRECTORY(framework)
GET_DIRECTORY_PROPERTY(roomcontrolClient_SRCS DIRECTORY framework DEFINITION roomcontrolClient_SRCS)
GET_DIRECTORY_PROPERTY(roomcontrolClient_SRCS_H DIRECTORY framework DEFINITION roomcontrolClient_SRCS_H)
#MESSAGE(STATUS "my sources: ${roomcontrolClient_SRCS}")
#INCLUDE(${PROJECT_SOURCE_DIR}/FindHDF.cmake)
ADD_SUBDIRECTORY(desktopgui)
ADD_SUBDIRECTORY(touchgui)

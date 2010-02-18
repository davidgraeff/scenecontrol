project(roomcontrolClient)
cmake_minimum_required(VERSION 2.6)
find_package(Qt4 REQUIRED)

# search KDE4 and use apply default (but optional) settings
find_package(KDE4 REQUIRED)
include(KDE4Defaults)

include_directories(${QT_INCLUDES} ${QT_QTNETWORK_INCLUDE_DIR} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} 
${CMAKE_CURRENT_BINARY_DIR})

add_definitions(${QT_DEFINITIONS} ${QDBUS_DEFINITIONS} ${KDE4_DEFINITIONS})

install( FILES roomclient.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )

ADD_SUBDIRECTORY(src)

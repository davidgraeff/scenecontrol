cmake_minimum_required(VERSION 2.8)
get_filename_component(targetname ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(room${targetname})

find_package(Qt4 REQUIRED)
find_package(KDE4 QUIET)
include(KDE4Defaults)
include(MacroLibrary)

MACRO (TODAY RESULT)
    IF (WIN32)
        EXECUTE_PROCESS(COMMAND "date" "/T" OUTPUT_VARIABLE ${RESULT})
        string(REGEX REPLACE "(..)/(..)/..(..).*" "\\3\\2\\1"
${RESULT} ${${RESULT}})
    ELSEIF(UNIX)
        EXECUTE_PROCESS(COMMAND "date" "+%d/%m/%Y" OUTPUT_VARIABLE ${RESULT})
        string(REGEX REPLACE "(..)/(..)/..(..).*" "\\3\\2\\1"
${RESULT} ${${RESULT}})
    ELSE (WIN32)
        MESSAGE(SEND_ERROR "date not implemented")
        SET(${RESULT} 000000)
    ENDIF (WIN32)
ENDMACRO (TODAY)

set (ROOTDIR "${CMAKE_CURRENT_SOURCE_DIR}/../..")
set (SHAREDDIR "${ROOTDIR}/shared")
set (SHAREDCLIENTDIR "${ROOTDIR}/shared/client")

include_directories(${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} ${ROOTDIR} ${SHAREDCLIENTDIR})

ADD_DEFINITIONS(-D_GNU_SOURCE -Wall -W -DQT_USE_FAST_OPERATOR_PLUS -DQT_USE_FAST_CONCATENATION ${QT_DEFINITIONS} ${QDBUS_DEFINITIONS})
#-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII 

today(ROOM_VERSION)
SET(ROOM_VERSION "2.0 ${ROOM_VERSION}")
set(SERVERNAME "RoomControlServer")
SET(ROOM_SYSTEM_CLIENTPLUGINS "lib/${SERVERNAME}/plugins/client" CACHE STRING "Set the directory where client plugins are located")
SET(ROOM_NETWORK_MIN_APIVERSION "3.0" CACHE STRING "Minimal compatible version")
SET(ROOM_NETWORK_MAX_APIVERSION "3.0" CACHE STRING "Maximal compatible version")
configure_file(${SHAREDCLIENTDIR}/config.h.in config.h @ONLY)

#Files
file(GLOB_RECURSE SRCS_H *.h)
file(GLOB_RECURSE Shared_SRCS_H "${SHAREDDIR}/*.h" "${SHAREDCLIENTDIR}/models/*.h"
"${SHAREDCLIENTDIR}/loginwidget/*.h"
"${SHAREDDIR}/qjson/*.h" "${SHAREDDIR}/categorize/*.h")
file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE Shared_SRCS "${SHAREDDIR}/*.cpp" "${SHAREDCLIENTDIR}/models/*.cpp"
"${SHAREDCLIENTDIR}/loginwidget/*.cpp"
"${SHAREDDIR}/qjson/*.cpp" "${SHAREDDIR}/qjson/*.cc" "${SHAREDDIR}/categorize/*.cpp")

#Special files
file(GLOB_RECURSE UI_FILES *.ui "${SHAREDCLIENTDIR}/loginwidget/*.ui")
qt4_wrap_ui(UI_H ${UI_FILES})
file(GLOB_RECURSE GUI_RC *.qrc)
QT4_ADD_RESOURCES(RC_SRC ${GUI_RC})

set(SRCS_H ${SRCS_H} ${SHAREDCLIENTDIR}/clientplugin.h ${SHAREDCLIENTDIR}/networkcontroller.h ${UI_H} ${Shared_SRCS_H})
set(SRCS ${SRCS} ${SHAREDCLIENTDIR}/networkcontroller.cpp ${Shared_SRCS} ${RC_SRC})

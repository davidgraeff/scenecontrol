cmake_minimum_required(VERSION 2.8)
get_filename_component(targetname ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${targetname})
message(STATUS "Configure client: ${targetname}")

find_package(Qt4 4.7.0 REQUIRED)
find_package(KDE4 4.4 QUIET)

set (ROOTDIR "${CMAKE_CURRENT_SOURCE_DIR}/../..")
set (SHAREDDIR "${ROOTDIR}/shared")
set (SHAREDCLIENTDIR "${ROOTDIR}/shared/client")

IF (KDE4_FOUND)
	include(KDE4Defaults)
	include(MacroLibrary)
	include_directories("${KDE4_INCLUDES}" "${KDE4_INCLUDE_DIR}")
	ADD_DEFINITIONS(-DWITHKDE)
	message("Activate kde4 extensions for client ${targetname}")
ELSE()
	include_directories("${SHAREDDIR}/nokde")
ENDIF()

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} ${ROOTDIR} ${SHAREDCLIENTDIR})

ADD_DEFINITIONS(-D_GNU_SOURCE -Wall -W -DQT_NO_CAST_FROM_ASCII -DQT_USE_FAST_OPERATOR_PLUS -DQT_USE_FAST_CONCATENATION -DQT_NO_CAST_TO_ASCII ${QT_DEFINITIONS} ${QDBUS_DEFINITIONS})

SET(ROOM_VERSION "${CPACK_PACKAGE_VERSION}")
set(SERVERNAME "RoomControlServer")
SET(ROOM_SYSTEM_CLIENTPLUGINS "lib/${SERVERNAME}/plugins/client" CACHE STRING "Set the directory where client plugins are located")
SET(ROOM_NETWORK_MIN_APIVERSION "3.0" CACHE STRING "Minimal compatible version")
SET(ROOM_NETWORK_MAX_APIVERSION "3.0" CACHE STRING "Maximal compatible version")
configure_file(${SHAREDCLIENTDIR}/config.h.in config.h @ONLY)

#Files
UNSET(SharedKDE_SRCS_H)
UNSET(SharedKDE_SRCS_SRCS)
UNSET(UIKDE_FILES)
file(GLOB_RECURSE SRCS_H *.h)
file(GLOB_RECURSE SRCS *.cpp)
file(GLOB Shared_SRCS_H "${SHAREDDIR}/*.h" "${SHAREDCLIENTDIR}/models/*.h"
"${SHAREDDIR}/qjson/*.h" "${SHAREDDIR}/categorize/*.h")
file(GLOB Shared_SRCS "${SHAREDDIR}/*.cpp" "${SHAREDCLIENTDIR}/models/*.cpp"
"${SHAREDDIR}/qjson/*.cpp" "${SHAREDDIR}/qjson/*.cc" "${SHAREDDIR}/categorize/*.cpp")
IF (NOT DEFINED WITHOUT_KDEGUI)
	file(GLOB_RECURSE SharedKDE_SRCS_H "${SHAREDCLIENTDIR}/loginwidget/*.h")
	file(GLOB_RECURSE SharedKDE_SRCS_SRCS "${SHAREDCLIENTDIR}/loginwidget/*.cpp")
	file(GLOB_RECURSE UIKDE_FILES "${SHAREDCLIENTDIR}/loginwidget/*.ui")
ENDIF()

# Under Windows, we also include a resource file to the build
if(WIN32 AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/res.rc")
    # Make sure that the resource file is seen as an RC file to be compiled with a resource compiler, not a C++ compiler
    #set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/res.rc LANGUAGE RC)
    # Add the resource file to the list of sources
    list(APPEND SRCS ${CMAKE_CURRENT_SOURCE_DIR}/res.rc)
    # For MinGW, we have to change the compile flags
      IF(MINGW)
          # resource compilation for MinGW
          ADD_CUSTOM_COMMAND( OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/icon.o
			COMMAND windres -I${CMAKE_CURRENT_SOURCE_DIR} -i${CMAKE_CURRENT_SOURCE_DIR}/res.rc -o ${CMAKE_CURRENT_BINARY_DIR}/icon.o )
		  list(APPEND SRCS ${CMAKE_CURRENT_BINARY_DIR}/icon.o)
          SET(LINK_FLAGS -Wl,-subsystem,windows)
      ENDIF(MINGW)
  
      IF(WIN32)
          SET( GUI_TYPE WIN32 )
      ENDIF( WIN32 )
 endif(WIN32 AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/res.rc")

#Special files
file(GLOB_RECURSE UI_FILES *.ui)
file(GLOB_RECURSE GUI_RC *.qrc)
qt4_wrap_ui(UI_H ${UI_FILES} ${UIKDE_FILES})
QT4_ADD_RESOURCES(RC_SRC ${GUI_RC})

list(APPEND SRCS_H ${SHAREDCLIENTDIR}/clientplugin.h ${SHAREDCLIENTDIR}/networkcontroller.h ${SHAREDCLIENTDIR}/servicestorage.h ${SHAREDCLIENTDIR}/modelstorage.h ${UI_H} ${Shared_SRCS_H} ${SharedKDE_SRCS_H})
list(APPEND SRCS ${SHAREDCLIENTDIR}/clientplugin.cpp ${SHAREDCLIENTDIR}/networkcontroller.cpp ${SHAREDCLIENTDIR}/servicestorage.cpp ${SHAREDCLIENTDIR}/modelstorage.cpp ${Shared_SRCS} ${RC_SRC} ${SharedKDE_SRCS_SRCS})

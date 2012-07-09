cmake_minimum_required(VERSION 2.8)
#Only allow this file to be included by the root cmake file
IF (NOT DEFINED PRODUCTID)
	RETURN()
ENDIF()

# project name
get_filename_component(targetname ${CMAKE_CURRENT_SOURCE_DIR} NAME)
message(STATUS "Plugin: ${targetname}")
project(${targetname}_plugin)

LIST(APPEND Shared_SRCS_H "${SHAREDPLUGINDIR}/abstractplugin.h" ${SRCS_JSON_H})
LIST(APPEND Shared_SRCS "${SHAREDPLUGINDIR}/plugineventmap.cpp" "${SHAREDPLUGINDIR}/abstractplugin.cpp"
${SRCS_JSON} "${CMAKE_SOURCE_DIR}/libdatabase/servicedata.cpp")

FILE(GLOB SRCS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
FILE(GLOB SRCS_H "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}
${ROOTDIR} ${PLUGINSDIR})

# rs232
if (DEFINED USE_SERIALPORT)
	LIST(APPEND SRCS_H ${SHAREDPLUGINDIR}/qxtserialdevice/qxtserialdevice.h ${SHAREDPLUGINDIR}/qxtserialdevice/qxtserialdevice_p.h)
	LIST(APPEND SRCS ${SHAREDPLUGINDIR}/qxtserialdevice/qxtserialdevice.cpp)
	IF (WIN32)
		#LIST(APPEND SRCS ${SHAREDPLUGINDIR}/qxtserialdevice/win_qxtserialdevice.cpp)
	ELSE()
		LIST(APPEND SRCS ${SHAREDPLUGINDIR}/qxtserialdevice/qxtserialdevice_unix.cpp)
	ENDIF()
endif()

#-DQT_NO_CAST_FROM_ASCII
ADD_DEFINITIONS(-DPLUGIN_ID="${targetname}" -D_GNU_SOURCE -Wall -W -DQT_NO_CAST_TO_ASCII -DQT_USE_FAST_OPERATOR_PLUS -DQT_USE_FAST_CONCATENATION ${QT_DEFINITIONS})


macro(build_server_lib)
	QT4_WRAP_CPP(SRCS_MOCS_SERVER ${SRCS_H} ${Shared_SRCS_H} ${ADD_H})
	add_executable(${PROJECT_NAME} ${SRCS} ${Shared_SRCS} ${SRCS_MOCS_SERVER} ${ADD_CPP})
	GET_TARGET_PROPERTY(BINARY_NAME ${PROJECT_NAME} OUTPUT_NAME)
endmacro(build_server_lib)

macro(install_server_lib)
	INSTALL(DIRECTORY "data/" DESTINATION ${CONFIG_SERVER_DATABASEIMPORTPATH}/${targetname} COMPONENT ServerPlugins)
	INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${LIBPATH} COMPONENT ServerPlugins)
	SET(CPACK_PACKAGE_EXECUTABLES ${CPACK_PACKAGE_EXECUTABLES} ${PROJECT_NAME} CACHE INTERNAL "")
endmacro()


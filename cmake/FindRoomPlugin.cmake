cmake_minimum_required(VERSION 2.8.8)
#Only allow this file to be included by the root cmake file
IF (NOT DEFINED PRODUCTID)
	RETURN()
ENDIF()

# project name
get_filename_component(targetname ${CMAKE_CURRENT_SOURCE_DIR} NAME)
message(STATUS "Plugin: ${targetname}")
project(${targetname}_plugin)

# abort if dependencies are not found. Message is defined by PLUGIN_DEP_MESSAGE.
IF (NOT PLUGIN_DEPENDENCIES_FOUND)
	MESSAGE("Not building: ${targetname}: ${PLUGIN_DEP_MESSAGE}")
	RETURN()
ENDIF()

# add an cmake gui option for selecting this plugin
OPTION(PLUGIN_${PROJECT_NAME} "Plugin ${PROJECT_NAME}" ON)

IF (NOT PLUGIN_${PROJECT_NAME})
	RETURN()
ENDIF()

# Install macro
macro(install_lib)
	GET_TARGET_PROPERTY(BINARY_NAME ${PROJECT_NAME} OUTPUT_NAME)
	INSTALL(DIRECTORY "data/" DESTINATION ${CONFIG_SERVER_DATABASEIMPORTPATH}/${targetname} COMPONENT ServerPlugins)
	INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${LIBPATH} COMPONENT ServerPlugins)
	SET(CPACK_PACKAGE_EXECUTABLES ${CPACK_PACKAGE_EXECUTABLES} ${PROJECT_NAME} CACHE INTERNAL "")
endmacro()

# definitions
ADD_DEFINITIONS(-DPLUGIN_ID="${targetname}" -D_GNU_SOURCE -Wall -W -DQT_NO_CAST_TO_ASCII -DQT_USE_FAST_OPERATOR_PLUS -DQT_USE_FAST_CONCATENATION ${QT_DEFINITIONS})

# add include directories
include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}
${ROOTDIR} ${PLUGINSDIR})

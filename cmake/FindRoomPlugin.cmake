cmake_minimum_required(VERSION 2.8.8)
#Only allow this file to be included by the root cmake file
IF (NOT DEFINED PRODUCTID)
	RETURN()
ENDIF()

# project name
get_filename_component(targetname ${CMAKE_CURRENT_SOURCE_DIR} NAME)
message(STATUS "Service: ${targetname}")
project(${targetname}_plugin)

# abort if dependencies are not found. Message is defined by PLUGIN_DEP_MESSAGE.
IF (NOT PLUGIN_DEPENDENCIES_FOUND)
	MESSAGE("Not building: ${targetname}: ${PLUGIN_DEP_MESSAGE}")
	RETURN()
ENDIF()

# add an cmake gui option for selecting this plugin
OPTION(PLUGIN_${PROJECT_NAME} "Service ${PROJECT_NAME}" ON)

IF (NOT PLUGIN_${PROJECT_NAME})
	RETURN()
ENDIF()

# Install macro
function(install_schemas)
	file(GLOB_RECURSE SCHEMAFILES "schemas/*.json")
	file(GLOB_RECURSE DESCFILES "desc/*.json")
	INSTALL(FILES ${SCHEMAFILES} ${DESCFILES} DESTINATION ${CONFIG_SERVER_DATABASEIMPORTPATH}/${targetname} COMPONENT ServerPlugins)
endfunction()

function(install_exampleconfig)
	file(GLOB_RECURSE CONFFILES "exampleconfig/*.json")
	INSTALL(FILES ${CONFFILES} DESTINATION ${CONFIG_SERVER_DATABASEIMPORTPATH}/${targetname} COMPONENT ServerPlugins)
endfunction()

macro(install_lib)
	SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${targetname})
	install_exampleconfig()
	INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${LIBPATH} COMPONENT ServerPlugins)
	SET(CPACK_PACKAGE_EXECUTABLES ${CPACK_PACKAGE_EXECUTABLES} ${PROJECT_NAME} CACHE INTERNAL "")
endmacro()

macro(install_nodelib)
	install_exampleconfig()
	#INSTALL(FILES ${CONFFILES} DESTINATION ${LIBPATH}/${targetname} COMPONENT ServerPlugins)
	# Install js files
	INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} DESTINATION "${LIBPATH}" COMPONENT ServerPlugins
	USE_SOURCE_PERMISSIONS
	PATTERN "certificates" EXCLUDE
	PATTERN "node_modules" EXCLUDE
	PATTERN "test" EXCLUDE
	PATTERN "config.js.in" EXCLUDE
	PATTERN "CMakeLists.txt" EXCLUDE
	)
	# execute npm to install node js app dependencies
	INSTALL(CODE "MESSAGE(Execute in \"${CMAKE_INSTALL_PREFIX}/${LIBPATH}/${targetname}\")")

	INSTALL(CODE "execute_process(COMMAND npm install
	WORKING_DIRECTORY \"${CMAKE_INSTALL_PREFIX}/${LIBPATH}/${targetname}\"
	OUTPUT_VARIABLE DIST OUTPUT_STRIP_TRAILING_WHITESPACE)")
endmacro()

# definitions
ADD_DEFINITIONS(-DPLUGIN_ID="${targetname}" -D_GNU_SOURCE -Wall -W -DQT_NO_CAST_TO_ASCII -DQT_USE_FAST_OPERATOR_PLUS -DQT_USE_FAST_CONCATENATION ${QT_DEFINITIONS})

# add include directories
include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}
${ROOTDIR} ${PLUGINSDIR})

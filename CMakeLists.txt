cmake_minimum_required(VERSION 2.8.8)
PROJECT(scenecontrol_suite)

SET(PRODUCTID ${PROJECT_NAME}) # define PRODUCTID otherwise all other CMakeFiles will fail
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
INCLUDE(cmake/InstallMacros.cmake)
INCLUDE(cmake/basevariables.cmake)
INCLUDE(cmake/configfile.cmake)

# add directories
add_subdirectory(core)
add_subdirectory(plugins)
add_subdirectory(htmleditor)
add_subdirectory(tools/startonboot)
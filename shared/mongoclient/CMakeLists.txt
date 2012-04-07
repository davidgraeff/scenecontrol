cmake_minimum_required(VERSION 2.8.4)

#First try to use system libmongoclient
SET(MONGOCLIENT_INCLUDE_DIRECTORIES /usr/include PARENT_SCOPE)
SET(MONGOCLIENT_LIBRARY_DIRECTORIES /usr/lib PARENT_SCOPE)
SET(MONGOCLIENT_LIBRARIES "mongoclient" ${Boost_LIBRARIES} PARENT_SCOPE)

#not found. now try to checkout and build by ourself
# include(ExternalProject)
# find_package(Git)
# message("GIT_EXECUTABLE=${GIT_EXECUTABLE}")
# 
# ExternalProject_Add(mongoclientExtern
#   GIT_REPOSITORY git://github.com/mongodb/mongo.git
#   #GIT_TAG r1.8.1
#   PREFIX "repo"
#   DOWNLOAD_DIR "${CMAKE_CURRENT_BINARY_DIR}"
#   UPDATE_COMMAND ""
#   PATCH_COMMAND ${GIT_EXECUTABLE} am -3 ${CMAKE_CURRENT_SOURCE_DIR}/0001-Squashed-commit-for-CMakeLists.txt.patch
#   CMAKE_GENERATOR "${CMAKE_GENERATOR}"
#   CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
#   #BUILD_COMMAND "cmake --build" # customize this if necessary
#   INSTALL_COMMAND ""
# )
# SET(MONGOCLIENT_INCLUDE_DIRECTORIES
# 	${CMAKE_CURRENT_BINARY_DIR}/repo/src/mongoclientExtern/src
# 	${CMAKE_CURRENT_BINARY_DIR}/repo/src/mongoclientExtern/src/mongo
# 	CACHE INTERNAL "")
# 
# SET(MONGOCLIENT_LIBRARY_DIRECTORIES ${CMAKE_CURRENT_BINARY_DIR}/repo/src/mongoclientExtern-build/ CACHE INTERNAL "")
# 
# # Disable warning about unused parameters
# file(GLOB MONGOH ${CMAKE_CURRENT_BINARY_DIR}/repo/src/mongoclientExtern/src/mongo/client/*.h)
# set_property(SOURCE ${MONGOH} PROPERTY COMPILE_DEFINITIONS "-Wno-unused-parameter" )
# 
# SET(MONGOCLIENT_LIBRARIES "mongoclient" ${Boost_LIBRARIES} CACHE INTERNAL "")
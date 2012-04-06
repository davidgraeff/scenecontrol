cmake_minimum_required(VERSION 2.8.4)
include(ExternalProject)
find_package(Git)
message("GIT_EXECUTABLE=${GIT_EXECUTABLE}")

ExternalProject_Add(mongoclientExtern
  GIT_REPOSITORY git://github.com/mongodb/mongo.git
  #GIT_TAG r1.8.1
  PREFIX "repo"
  DOWNLOAD_DIR "${CMAKE_CURRENT_BINARY_DIR}"
  UPDATE_COMMAND ""
  PATCH_COMMAND ${GIT_EXECUTABLE} am -3 ${CMAKE_CURRENT_SOURCE_DIR}/0001-Squashed-commit-for-CMakeLists.txt.patch
  CMAKE_GENERATOR "${CMAKE_GENERATOR}"
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
  #BUILD_COMMAND "cmake --build" # customize this if necessary
  INSTALL_COMMAND ""
)
SET(MONGOCLIENT_INCLUDE_DIRECTORIES
	${CMAKE_CURRENT_BINARY_DIR}/repo/src/mongoclient/src
	${CMAKE_CURRENT_BINARY_DIR}/repo/src/mongoclient/src/mongo
	CACHE INTERNAL "")

SET(MONGOCLIENT_LIBRARY_DIRECTORIES ${CMAKE_CURRENT_BINARY_DIR}/repo/src/mongoclient-build/ CACHE INTERNAL "")

SET(MONGOCLIENT_LIBRARIES "mongoclient" ${Boost_LIBRARIES} CACHE INTERNAL "")

#set_property(TARGET mongoclient PROPERTY
#                IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/repo/src/mongoclient-build/libmongoclient.a)
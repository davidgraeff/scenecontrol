cmake_minimum_required(VERSION 2.8.4)

find_package(Boost COMPONENTS system thread signals filesystem QUIET) 
IF (NOT ${Boost_FOUND})
	message(FATAL_ERROR "Boost libraries (system thread signals) not intalled! ${Boost_LIBRARIES}  ${Boost_ERROR_REASON}")
	set(MONGOCLIENT_FOUND FALSE)
	return()
ENDIF()

link_directories ( ${Boost_LIBRARY_DIRS} )
include_directories ( ${Boost_INCLUDE_DIRS} )

if (NOT WIN32)
    FIND_PATH(MONGOCLIENT_INCLUDE_DIR dbclient.h
      HINTS
      /usr/include/mongo/client
      )

    FIND_LIBRARY(MONGOCLIENT_LIBRARY NAMES libmongoclient mongoclient
      HINTS
      /usr/lib
      )

    if (MONGOCLIENT_INCLUDE_DIR AND MONGOCLIENT_LIBRARY)
      set(MONGOCLIENT_FOUND TRUE)
    else (MONGOCLIENT_INCLUDE_DIR AND MONGOCLIENT_LIBRARY)
      set(MONGOCLIENT_FOUND FALSE)
    endif (MONGOCLIENT_INCLUDE_DIR AND MONGOCLIENT_LIBRARY)

    if (MONGOCLIENT_FOUND)
      if (NOT MONGOCLIENT_FIND_QUIETLY)
	  message(STATUS "Found Mongoclient: ${MONGOCLIENT_LIBRARY}")
      endif (NOT MONGOCLIENT_FIND_QUIETLY)
    else (MONGOCLIENT_FOUND)
      message(STATUS "Could NOT find Udev")
    endif (MONGOCLIENT_FOUND)

    mark_as_advanced(MONGOCLIENT_INCLUDE_DIR MONGOCLIENT_LIBRARY)
else(NOT WIN32)
    include(ExternalProject)
    find_package(Git REQUIRED)
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
    SET(MONGOCLIENT_INCLUDE_DIR
	    ${CMAKE_CURRENT_BINARY_DIR}/repo/src/mongoclientExtern/src
	    ${CMAKE_CURRENT_BINARY_DIR}/repo/src/mongoclientExtern/src/mongo
	    CACHE INTERNAL "")

    link_directories ( ${CMAKE_CURRENT_BINARY_DIR}/repo/src/mongoclientExtern-build/  )
    
    # Disable warning about unused parameters
    file(GLOB MONGOH ${CMAKE_CURRENT_BINARY_DIR}/repo/src/mongoclientExtern/src/mongo/client/*.h)
    set_property(SOURCE ${MONGOH} PROPERTY COMPILE_DEFINITIONS "-Wno-unused-parameter" )

    SET(MONGOCLIENT_LIBRARY "mongoclient" CACHE INTERNAL "")
    set(MONGOCLIENT_FOUND TRUE)
endif(NOT WIN32)
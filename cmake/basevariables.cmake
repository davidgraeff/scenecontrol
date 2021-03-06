cmake_minimum_required(VERSION 2.8.8)
#Only allow this file to be included by the root cmake file
IF (NOT DEFINED PRODUCTID)
	RETURN()
ENDIF()

# Enable c++11 standard
add_definitions(-std=c++0x)
add_definitions(-std=gnu++0x)

# install path of libs
IF(WIN32)
	IF (CMAKE_BUILD_TYPE EQUAL "DEBUG" OR CMAKE_BUILD_TYPE EQUAL "Debug")
		SET(LIBPATH "lib/debug")
	ELSE()
		SET(LIBPATH "lib")
	ENDIF()
	SET(DATAPATH ".")
	set(LASTCOMMITDATE "")
	set(COMMITHASH "")
ELSE()
	IF (CMAKE_BUILD_TYPE EQUAL "DEBUG" OR CMAKE_BUILD_TYPE EQUAL "Debug")
		SET(LIBPATH "lib/debug/${PRODUCTID}")
	ELSE()
		SET(LIBPATH "lib/${PRODUCTID}")
	ENDIF()
	SET(DATAPATH "share/${PRODUCTID}")
	execute_process(COMMAND /usr/bin/git log --pretty=format:%ad -n 1 --date=iso
					WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
					OUTPUT_VARIABLE LASTCOMMITDATE)
	execute_process(COMMAND /usr/bin/git log --pretty=format:%H -n 1
					WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
					OUTPUT_VARIABLE COMMITHASH)
	MESSAGE(STATUS "Exact version: ${LASTCOMMITDATE} ${COMMITHASH}")
ENDIF()

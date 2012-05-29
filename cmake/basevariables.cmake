cmake_minimum_required(VERSION 2.8)

set (SHAREDPLUGINDIR "${CMAKE_SOURCE_DIR}/plugins/_sharedsrc")

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

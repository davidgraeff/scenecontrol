cmake_minimum_required(VERSION 2.8)
IF(UNIX)
	# try to guess the distribution
	execute_process(COMMAND /usr/bin/lsb_release -i -s
					WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
					OUTPUT_VARIABLE DIST OUTPUT_STRIP_TRAILING_WHITESPACE)

	#ubuntu: use upstart script
	MESSAGE(STATUS "OS: ${DIST}")
	IF ("Ubuntu" STREQUAL ${DIST})
		MESSAGE(STATUS "Bootscript: Upstart")
		add_subdirectory(ubuntulinux)
	ELSEIF ("LinuxMint" STREQUAL ${DIST})
		MESSAGE(STATUS "Bootscript: Upstart")
		add_subdirectory(ubuntulinux)
	ELSE()
		MESSAGE(STATUS "Bootscript: None available")
	ENDIF()
ENDIF(UNIX)
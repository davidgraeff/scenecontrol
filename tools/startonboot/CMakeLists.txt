cmake_minimum_required(VERSION 2.8.8)
IF(UNIX)
	# try to guess the distribution
	execute_process(COMMAND /usr/bin/lsb_release -i -s
					WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
					OUTPUT_VARIABLE DIST OUTPUT_STRIP_TRAILING_WHITESPACE)
	
	SET(NODEEXEC "node")
	SET(CONFIG_SERVEREXECUTABLE "${NODEEXEC} ${CMAKE_INSTALL_PREFIX}/${LIBPATH}/core/index.js")
	configure_file(sceneserversession.in sceneserversession @ONLY IMMEDIATE)
	INSTALL(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/sceneserversession DESTINATION bin COMPONENT Server)
	
	#ubuntu: use upstart script
	MESSAGE(STATUS "OS: ${DIST}")
	IF ("Ubuntu" STREQUAL ${DIST})
		MESSAGE(STATUS "Bootscript: Upstart")
		add_subdirectory(ubuntulinux)
	ELSEIF ("LinuxMint" STREQUAL ${DIST})
		MESSAGE(STATUS "Bootscript: Upstart")
		add_subdirectory(ubuntulinux)
	ELSEIF ("Linaro" STREQUAL ${DIST})
		MESSAGE(STATUS "Bootscript: Upstart")
		add_subdirectory(ubuntulinux)
	ELSE()
		MESSAGE(STATUS "Bootscript: None available")
	ENDIF()
ENDIF(UNIX)
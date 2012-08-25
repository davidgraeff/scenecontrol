
IF (NOT DEFINED TOOLS_ALREADY_BUILD)
	# some variables
	SET(CMAKE_TRY_COMPILE_CONFIGURATION "Release") # try_run command should use release mode
	SET(CMAKE_CFG_INTDIR "") # do not use VisualStudio Release/Debug subdirectories
	
	# Build helper tool
	FUNCTION (Build_tool NAME DIR SOURCE)
		# Define name for tool
		SET(TOOL_${NAME} "${CMAKE_BINARY_DIR}/tool_${NAME}.exe" CACHE INTERNAL "" FORCE)
		
		# Set compiler definitions
		SET (TOOL_COMPILE_DEFINITIONS "${CMAKE_CXX_FLAGS_RELEASE}")
		
		try_compile(TOOLISVALID "${CMAKE_BINARY_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/${DIR}/${SOURCE}"
					COMPILE_DEFINITIONS ${TOOL_COMPILE_DEFINITIONS}
					OUTPUT_VARIABLE ERROUT
					COPY_FILE "${TOOL_${NAME}}")
		
		# Check for successful build of tool
		IF (NOT TOOLISVALID)
			MESSAGE(FATAL_ERROR "${TOOL_${NAME}} failed to build. No C/C++ Compiler found or C/C++ Compiler failed: ${ERROUT}")
		ELSE(NOT TOOLISVALID)
			MESSAGE(STATUS "Build tool ${TOOL_${NAME}} ready")
		ENDIF(NOT TOOLISVALID)
	ENDFUNCTION()
	
	# build tool isQt
	# Identify moc'able header files
	Build_tool (prepareReadme prepareReadme prepareReadme.c)
	
	# SET TOOLS_ALREADY_BUILD to true
	SET (TOOLS_ALREADY_BUILD TRUE CACHE INTERNAL "" FORCE)
ENDIF (NOT DEFINED TOOLS_ALREADY_BUILD)

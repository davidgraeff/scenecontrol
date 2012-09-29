cmake_minimum_required(VERSION 2.8.8)

#Only allow this file to be included by the root cmake file
IF (NOT DEFINED PRODUCTID)
	RETURN()
ENDIF()

project(editor_web)

OPTION(INSTALL_EDITOR "Install Editor" ON)
IF (NOT INSTALL_EDITOR)
  RETURN()
ENDIF()

file(GLOB SRCS *.css *.js *.html)

INSTALL(FILES ${SRCS} DESTINATION ${CONFIG_SERVER_WEBAPP} COMPONENT Server)

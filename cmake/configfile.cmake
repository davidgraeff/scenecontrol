cmake_minimum_required(VERSION 2.8.8)
#Only allow this file to be included by the root cmake file
IF (NOT DEFINED PRODUCTID)
	RETURN()
ENDIF()

# prepare configuration file
SET(CONFIG_SERVER_BASEPATH "${CMAKE_INSTALL_PREFIX}")
SET(CONFIG_SERVER_DATABASEIMPORTPATH "${DATAPATH}/dbimport")
SET(CONFIG_SERVER_CERTPATH "${DATAPATH}/cert")
SET(CONFIG_CLIENT_CERTPATH "${DATAPATH}/cert/clients")
SET(CONFIG_SERVER_LIBPATH "${LIBPATH}")
SET(CONFIG_SERVER_VERSION "${CPACK_PACKAGE_VERSION}")
SET(CONFIG_SERVER_PATHNAME "${PRODUCTID}")
SET(CONFIG_SERVER_LISTENPORT "3101" CACHE INT "Server Listenport")
SET(CONFIG_WEBSOCKETPROXY_LISTENPORT "3102" CACHE INT "WebsocketProxy Listenport")
SET(CONFIG_SERVER_LOGTOCONSOLE TRUE CACHE BOOL "Server Log to Console")
IF (CONFIG_SERVER_LOGTOCONSOLE)
	SET(CONFIG_SERVER_LOGTOCONSOLE 1)
ELSE()
	SET(CONFIG_SERVER_LOGTOCONSOLE 0)
ENDIF()
SET(CONFIG_SERVER_LOGFILE "roomcontrolserver.log" CACHE STRING "Server Logfile. Set to empty for no file logging")
configure_file(config.h.in config.h @ONLY)

cmake_minimum_required(VERSION 2.6)
find_package(RoomPlugins REQUIRED)

SET(CONFIG_${PROJECT_NAME}_SERVER "192.168.1.18:2703" CACHE STRING "UDP IO Server")
SET(CONFIG_SERVER ${CONFIG_${PROJECT_NAME}_SERVER})
configure_file(configplugin.h.in configplugin.h @ONLY)

SET(DEBFOUND TRUE)
IF (DEBFOUND)
	OPTION(PLUGIN_${PROJECT_NAME} "Plugin  ${PROJECT_NAME} Server" ON)

	IF (PLUGIN_${PROJECT_NAME})
		build_server_lib()
		target_link_libraries(${PROJECT_NAME}_server ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARIES})
		install_server_lib()
	ENDIF()
ELSE()
	MESSAGE("Not building: ${PROJECT_NAME}")
ENDIF()

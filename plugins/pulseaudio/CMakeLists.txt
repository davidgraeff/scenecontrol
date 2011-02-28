cmake_minimum_required(VERSION 2.6)
find_package(RoomPlugins REQUIRED)
find_package(PulseAudio QUIET)

SET(DEBFOUND ${PULSEAUDIO_FOUND})
IF (DEBFOUND)
	OPTION(PLUGIN_${PROJECT_NAME}_SERVER "Plugin ${PROJECT_NAME} Server" ON)

	IF (PLUGIN_${PROJECT_NAME}_SERVER)
		build_server_lib()
		target_link_libraries(${PROJECT_NAME}_server ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARIES})
		install_server_lib()
	ENDIF()
ENDIF()
add_subdirectory(roompulsehelper)


cmake_minimum_required(VERSION 2.8.8)
LIST(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
find_package(PulseAudio QUIET)
find_package(GLIB2 QUIET)

SET(PLUGIN_DEPENDENCIES_FOUND PULSEAUDIO_FOUND AND GLIB2_FOUND)
find_package(RoomPlugin REQUIRED)

IF (PULSEAUDIO_FOUND AND GLIB2_FOUND)
	include_directories(${PULSEAUDIO_INCLUDE_DIR} ${GLIB2_INCLUDE_DIRS})
	FILE(GLOB SRCS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
	FILE(GLOB SRCS_H "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
	QT4_WRAP_CPP(SRCS_MOCS ${SRCS_H})
	add_executable(${PROJECT_NAME} ${SRCS} ${SRCS_MOCS} $<TARGET_OBJECTS:PluginLib>)
	target_link_libraries(${PROJECT_NAME} ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARIES} ${GLIB2_LIBRARIES} ${PULSEAUDIO_LIBRARY} ${PULSEAUDIO_MAINLOOP_LIBRARY})

	install_schemas()
install_lib()
ENDIF()

project(roomcontrolserver)
cmake_minimum_required(VERSION 2.6)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
find_package(Qt4 4.7.0 COMPONENTS QtCore REQUIRED)
find_package(PAM QUIET)
#find_package(QJSON REQUIRED)

IF (PAM_FOUND OR WIN32)
	OPTION(BUILD_SERVER "Build server" ON)
ENDIF()

IF (BUILD_SERVER)
	IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	  SET(CMAKE_INSTALL_PREFIX
	    "/usr" CACHE PATH "RoomControlServer install prefix" FORCE
	    )
	ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

	file(GLOB_RECURSE SRCS_H *.h)
	file(GLOB Shared_SRCS_H "../shared/*.h" "../shared/server/*.h" "../shared/categorize/*.h" "../shared/qjson/*.h")

	file(GLOB_RECURSE SRCS *.cpp)
	file(GLOB Shared_SRCS "../shared/*.cpp" "../shared/server/*.cpp" "../shared/categorize/*.cpp" "../shared/qjson/*.cpp" "../shared/qjson/*.cc")

	IF (PAM_FOUND)
		include_directories(${PAM_INCLUDE_DIR})
	ENDIF()
	
	include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/..
	${CMAKE_CURRENT_BINARY_DIR}	${CMAKE_CURRENT_SOURCE_DIR}/../plugins/)

	ADD_DEFINITIONS(-D_IS_ROOMSERVER -D_GNU_SOURCE -Wall -W -DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII -DQT_USE_FAST_OPERATOR_PLUS -DQT_USE_FAST_CONCATENATION ${QT_DEFINITIONS} ${QDBUS_DEFINITIONS})

	# prepare configuration file
	SET(ROOM_VERSION "${CPACK_PACKAGE_VERSION}")
	SET(ROOM_SYSTEM_CLIENTPLUGINS "lib/${PROJECT_NAME}/plugins/client" CACHE STRING "Set the directory where client plugins are located")
	SET(ROOM_SYSTEM_SERVERPLUGINS "lib/${PROJECT_NAME}/plugins/server" CACHE STRING "Set the directory where server plugins are located")
	SET(ROOM_SYSTEM_CERTIFICATES "lib/${PROJECT_NAME}/certificates" CACHE STRING "Set the directory where ssl certificates are located")
	#SET(ROOM_USER_DATA_DIR ".config/${PROJECT_NAME}" CACHE STRING "Set the directory where user profiles and services are stored")
	configure_file(config.h.in config.h @ONLY)

	QT_WRAP_CPP(${PROJECT_NAME} SRCS_MOCS ${Shared_SRCS_H} ${SRCS_H}
	../plugins/coreplugin/services_server/systemACServer.h ../plugins/coreplugin/services/systemAC.h
	../plugins/coreplugin/services_server/systemEVServer.h ../plugins/coreplugin/services/systemEV.h
	../plugins/coreplugin/services_server/profileACServer.h ../plugins/coreplugin/services/profileAC.h )
	#QT4_ADD_RESOURCES(RoomControlServer_RC ../res.qrc)

	add_executable(${PROJECT_NAME} ${SRCS} ${SRCS_MOCS} ${Shared_SRCS}
	../plugins/coreplugin/services_server/systemACServer.cpp ../plugins/coreplugin/services/systemAC.cpp
	../plugins/coreplugin/services_server/systemEVServer.cpp ../plugins/coreplugin/services/systemEV.cpp
	../plugins/coreplugin/services_server/profileACServer.cpp ../plugins/coreplugin/services/profileAC.cpp )

	target_link_libraries(${PROJECT_NAME}
	${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARIES}
	${QT_QTTEST_LIBRARIES} ${QT_QTDBUS_LIBRARIES} ${PAM_LIBRARIES})

	INSTALL(TARGETS ${PROJECT_NAME} DESTINATION bin)
	FILE(GLOB files "../shared/certificates/*")
	install( FILES ${files} DESTINATION ${ROOM_SYSTEM_CERTIFICATES} )
ELSE(BUILD_SERVER)
	MESSAGE("Not building: Server")
ENDIF(BUILD_SERVER)

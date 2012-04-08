cmake_minimum_required(VERSION 2.8)

#Only allow this file to be included by the root cmake file
IF (NOT DEFINED PRODUCTID)
	RETURN()
ENDIF()

LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/" "/usr/share/apps/cmake/modules/")

find_package(OpenSSL QUIET)

project(roomcontrolserver)

IF (OPENSSL_FOUND)
	OPTION(BUILD_SERVER "Build server" ON)
ELSE()
	MESSAGE("Not building: Server (OpenSSL)")
ENDIF()

IF (BUILD_SERVER)
	include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_BINARY_DIR}
	${MONGOCLIENT_INCLUDE_DIRECTORIES} ${ROOTDIR} )
	#message("mongo include: ${MONGOCLIENT_INCLUDE_DIRECTORIES}")
	#LWS_OPENSSL_SUPPORT: for libwebsocket to support wss (ws secured with ssl)
	#LWS_OPENSSL_CLIENT_CERTS: for libwebsocket; client certificate directory (=empty for server)
	ADD_DEFINITIONS(-D_GNU_SOURCE -Wall -W -DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII -DQT_USE_FAST_OPERATOR_PLUS -DQT_USE_FAST_CONCATENATION ${QT_DEFINITIONS})

	file(GLOB SRCS_Hm *.h)
	file(GLOB SRCSm *.cpp)
	file(GLOB SRCS_He execute/*.h)
	file(GLOB SRCSe execute/*.cpp)
	file(GLOB SRCS_Hp plugins/*.h)
	file(GLOB SRCSp plugins/*.cpp)
	SET(SRCS_H ${SRCS_Hm} ${SRCS_Hp} ${SRCS_He} ${SRCS_DATABASE_H})
	SET(SRCS ${SRCSm} ${SRCSp} ${SRCSe} ${SRCS_DATABASE})

	QT_WRAP_CPP(${PROJECT_NAME} SRCS_MOCS ${SRCS_H})
	#QT4_ADD_RESOURCES(RoomControlServer_RC ../res.qrc)

	link_directories(${MONGOCLIENT_LIBRARY_DIRECTORIES})
	add_executable(${PROJECT_NAME} ${SRCS} ${SRCS_MOCS})

	target_link_libraries(${PROJECT_NAME}
	${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARIES}
	${QT_QTTEST_LIBRARIES} ${OPENSSL_LIBRARIES} ${MONGOCLIENT_LIBRARIES})

	add_dependencies(${PROJECT_NAME} mongoclient)
	
	INSTALL(TARGETS ${PROJECT_NAME} DESTINATION bin COMPONENT Server)
	GET_TARGET_PROPERTY(BINARY_NAME ${PROJECT_NAME} OUTPUT_NAME)
	SET(CPACK_PACKAGE_EXECUTABLES ${CPACK_PACKAGE_EXECUTABLES} ${PROJECT_NAME} CACHE INTERNAL "")
	
	INSTALL(DIRECTORY "certificates/" DESTINATION ${CONFIG_SERVER_CERTPATH} COMPONENT Server)
ENDIF(BUILD_SERVER)

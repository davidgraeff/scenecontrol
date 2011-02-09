cmake_minimum_required(VERSION 2.6)
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
find_package(Qt4 4.7.0 COMPONENTS QtCore REQUIRED)
find_package(PAM QUIET)
#find_package(QJSON REQUIRED)

project(roomcontrolserver)
message(STATUS "Configure server")

IF (PAM_FOUND OR WIN32)
	OPTION(BUILD_SERVER "Build server" ON)
ENDIF()

IF (BUILD_SERVER)
	file(GLOB_RECURSE SRCS_H *.h)
	file(GLOB Shared_SRCS_H "../shared/*.h" "../shared/server/*.h" "../shared/categorize/*.h" "../shared/qjson/*.h")

	file(GLOB_RECURSE SRCS *.cpp)
	file(GLOB Shared_SRCS "../shared/*.cpp" "../shared/server/*.cpp" "../shared/categorize/*.cpp" "../shared/qjson/*.cpp" "../shared/qjson/*.cc")

	IF (PAM_FOUND)
		include_directories(${PAM_INCLUDE_DIR})
	ENDIF()
	
	include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_BINARY_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/../plugins/")

	ADD_DEFINITIONS(-D_IS_ROOMSERVER -D_GNU_SOURCE -Wall -W -DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII -DQT_USE_FAST_OPERATOR_PLUS -DQT_USE_FAST_CONCATENATION ${QT_DEFINITIONS} ${QDBUS_DEFINITIONS})

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

	INSTALL(TARGETS ${PROJECT_NAME} DESTINATION bin COMPONENT Server)
	GET_TARGET_PROPERTY(BINARY_NAME ${PROJECT_NAME} OUTPUT_NAME)
	SET(CPACK_PACKAGE_EXECUTABLES "${CPACK_PACKAGE_EXECUTABLES} ${BINARY_NAME}" CACHE INTERNAL "")
	
	FILE(GLOB files "../shared/certificates/*")
	install( FILES ${files} DESTINATION ${CERTPATH} COMPONENT Server)
ELSE(BUILD_SERVER)
	MESSAGE("Not building: Server")
ENDIF(BUILD_SERVER)

project(RoomControlServer)
cmake_minimum_required(VERSION 2.6)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
find_package(4.7.0 COMPONENTS QtCore QtGui REQUIRED)
find_package(PAM REQUIRED)
#find_package(QJSON REQUIRED)

MACRO (TODAY RESULT)
    IF (WIN32)
        EXECUTE_PROCESS(COMMAND "date" "/T" OUTPUT_VARIABLE ${RESULT})
        string(REGEX REPLACE "(..)/(..)/..(..).*" "\\3\\2\\1"
${RESULT} ${${RESULT}})
    ELSEIF(UNIX)
        EXECUTE_PROCESS(COMMAND "date" "+%d/%m/%Y" OUTPUT_VARIABLE ${RESULT})
        string(REGEX REPLACE "(..)/(..)/..(..).*" "\\3\\2\\1"
${RESULT} ${${RESULT}})
    ELSE (WIN32)
        MESSAGE(SEND_ERROR "date not implemented")
        SET(${RESULT} 000000)
    ENDIF (WIN32)
ENDMACRO (TODAY)

IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  SET(CMAKE_INSTALL_PREFIX
    "/usr" CACHE PATH "RoomControlServer install prefix" FORCE
    )
ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

# set default build type
IF (NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Build Type")
  SET(CMAKE_CONFIGURATION_TYPES RelWithDebInfo Release Debug CACHE STRING "")
ENDIF (NOT CMAKE_BUILD_TYPE)

file(GLOB_RECURSE SRCS_H *.h)
file(GLOB Shared_SRCS_H "../shared/*.h" "../shared/server/*.h" "../shared/categorize/*.h" "../shared/qjson/*.h")

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB Shared_SRCS "../shared/*.cpp" "../shared/server/*.cpp" "../shared/categorize/*.cpp" "../shared/qjson/*.cpp" "../shared/qjson/*.cc")

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/..
${CMAKE_CURRENT_BINARY_DIR} ${PAM_INCLUDE_DIR}
${CMAKE_CURRENT_SOURCE_DIR}/../plugins/)

ADD_DEFINITIONS(-D_IS_ROOMSERVER -D_GNU_SOURCE -Wall -W -DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII -DQT_USE_FAST_OPERATOR_PLUS -DQT_USE_FAST_CONCATENATION ${QT_DEFINITIONS} ${QDBUS_DEFINITIONS})

# prepare configuration file
today(ROOM_VERSION)
SET(ROOM_VERSION "2.0 ${ROOM_VERSION}")
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

INSTALL(TARGETS ${PROJECT_NAME} DESTINATION bin CONFIGURATIONS Debug Release RelWithDebInfo)
FILE(GLOB files "../shared/certificates/*")
install( FILES ${files} DESTINATION ${ROOM_SYSTEM_CERTIFICATES} )

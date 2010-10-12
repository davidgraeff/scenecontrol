project(RoomControlServer)
cmake_minimum_required(VERSION 2.6)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
find_package(Qt4 REQUIRED)
find_package(PAM REQUIRED)
#find_package(QJSON REQUIRED)

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

include_directories(. ../plugins/shared)

file(GLOB_RECURSE RoomControlServer_SRCS_H *.h *.hh)
file(GLOB_RECURSE Shared_SRCS_H "../plugins/shared/*.h" "../plugins/shared/qjson/*.h" "../plugins/coreplugin/*.h")

file(GLOB_RECURSE RoomControlServer_SRCS *.cpp *.cc)
file(GLOB_RECURSE Shared_SRCS "../plugins/shared/*.cpp" "../plugins/shared/qjson/*.cpp" "../plugins/shared/qjson/*.cc" "../plugins/coreplugin/*.cpp")

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} ${PAM_INCLUDE_DIR} ../plugins)

ADD_DEFINITIONS(-D_GNU_SOURCE -Wall -W -DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII -DQT_USE_FAST_OPERATOR_PLUS -DQT_USE_FAST_CONCATENATION ${QT_DEFINITIONS} ${QDBUS_DEFINITIONS})

QT_WRAP_CPP(RoomControlServer RoomControlServer_SRCS_MOCS ${Shared_SRCS_H} ${RoomControlServer_SRCS_H})
QT4_AUTOMOC(${RoomControlServer_SRCS} ${Shared_SRCS})
#QT4_ADD_RESOURCES(RoomControlServer_RC ../res.qrc)

add_executable(RoomControlServer ${RoomControlServer_SRCS}
${RoomControlServer_SRCS_MOCS} ${Shared_SRCS})

target_link_libraries(RoomControlServer
${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARIES}
${QT_QTTEST_LIBRARIES} ${QT_QTDBUS_LIBRARIES} ${PAM_LIBRARIES})

INSTALL(TARGETS RoomControlServer DESTINATION bin CONFIGURATIONS Debug Release RelWithDebInfo)

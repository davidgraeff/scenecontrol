include_directories(. qjson)

file(GLOB_RECURSE RoomControlServer_SRCS_H *.h *.hh)

file(GLOB_RECURSE RoomControlServer_SRCS *.cpp *.cc)

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})

ADD_DEFINITIONS(-D_GNU_SOURCE -Wall -W -DWITH_EXECUTION -DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII -DQT_USE_FAST_OPERATOR_PLUS -DQT_USE_FAST_CONCATENATION ${QT_DEFINITIONS} ${QDBUS_DEFINITIONS})

QT_WRAP_CPP(RoomControlServer RoomControlServer_SRCS_MOCS
${RoomControlServer_SRCS_H})
QT4_AUTOMOC(${RoomControlServer_SRCS})
QT4_ADD_RESOURCES(RoomControlServer_RC ../res.qrc)

add_executable(RoomControlServer ${RoomControlServer_SRCS}
${RoomControlServer_SRCS_MOCS} ${RoomControlServer_RC} ${RoomControlServer_UIS_H})

target_link_libraries(RoomControlServer
${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARIES}
${QT_QTTEST_LIBRARIES} ${QT_QTDBUS_LIBRARIES} ${USBLL} phonon)

INSTALL(TARGETS RoomControlServer DESTINATION bin CONFIGURATIONS Debug Release RelWithDebInfo)

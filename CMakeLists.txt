project(RoomControlServer)
cmake_minimum_required(VERSION 2.6)
find_package(Qt4 REQUIRED)


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

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} src/ src/Transferable/ src/widgets/ src/modewidgets/)

set(RoomControlServer_SRCS_H
src/Transferables/transferablefactory.h src/Transferables/transferablecontroller.h
src/Transferables/executableaudio.h  
src/Transferables/timeablerepeating.h src/Transferables/timeableabsolute.h
src/Transferables/timeable.h src/Transferables/transferable.h 

src/dbus/dbus_adapter.h
src/RoomControlServer.h
src/networkcontroller.h
src/ecmdcontroller.h
src/flickcharm.h

src/widgets/abstractcontrolbutton.h
src/widgets/channelbutton.h
src/widgets/menubutton.h
src/widgets/musicplaybutton.h
src/widgets/musictrackbutton.h
src/widgets/alarmboxwidget.h
src/widgets/pinbutton.h
src/widgets/svgbutton.h
src/widgets/touchwindow.h
src/widgets/jumpbarwidget.h

src/media/videocontroller.h
src/media/playlist.h
src/media/playlistcontroller.h
src/media/eventcontroller.h

src/modewidgets/modeleds.h
src/modewidgets/modelights.h
src/modewidgets/modemusic.h
src/modewidgets/modealarm.h
src/modewidgets/modeprofiles.h
src/modewidgets/modeperiodic.h
src/modewidgets/abstractmodewidget.h
)

set(RoomControlServer_SRCS
src/Transferables/transferablefactory.cpp src/Transferables/transferablecontroller.cpp
src/Transferables/executableaudio.cpp  src/Transferables/executablechannel.cpp 
src/Transferables/executablepin.cpp src/Transferables/executablempris.cpp
src/Transferables/executableprojector.cpp src/Transferables/executablecurtain.cpp
src/Transferables/executable.cpp src/Transferables/executablecollection.cpp
src/Transferables/timeablerepeating.cpp src/Transferables/timeableabsolute.cpp
src/Transferables/timeable.cpp src/Transferables/transferable.cpp src/main.cpp

src/RoomControlServer.cpp
src/networkcontroller.cpp
src/dbus/dbus_adapter.cpp
src/ecmdcontroller.cpp 
src/flickcharm.cpp

src/widgets/abstractcontrolbutton.cpp
src/widgets/menubutton.cpp
src/widgets/musicplaybutton.cpp
src/widgets/musictrackbutton.cpp
src/widgets/alarmboxwidget.cpp
src/widgets/channelbutton.cpp
src/widgets/pinbutton.cpp
src/widgets/svgbutton.cpp
src/widgets/touchwindow.cpp
src/widgets/jumpbarwidget.cpp

src/media/videocontroller.cpp
src/media/playlist.cpp
src/media/playlistcontroller.cpp
src/media/eventcontroller.cpp

src/modewidgets/modeleds.cpp
src/modewidgets/modelights.cpp
src/modewidgets/modemusic.cpp
src/modewidgets/modealarm.cpp
src/modewidgets/modeprofiles.cpp
src/modewidgets/modeperiodic.cpp
src/modewidgets/abstractmodewidget.cpp
)

SET(RoomControlServer_UIS
)

QT4_WRAP_UI(RoomControlServer_UIS_H ${RoomControlServer_UIS})

QT_WRAP_CPP(RoomControlServer RoomControlServer_SRCS_MOCS
${RoomControlServer_SRCS_H})
QT4_AUTOMOC(${RoomControlServer_SRCS})
QT4_ADD_RESOURCES(RoomControlServer_RC res.qrc)

add_executable(RoomControlServer ${RoomControlServer_SRCS}
${RoomControlServer_SRCS_MOCS} ${RoomControlServer_RC} ${RoomControlServer_UIS_H})

ADD_DEFINITIONS(-D_GNU_SOURCE -Wall -W -DWITH_EXECUTION ${QT_DEFINITIONS} ${QDBUS_DEFINITIONS})

target_link_libraries(RoomControlServer
${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTNETWORK_LIBRARIES}
${QT_QTTEST_LIBRARIES} ${QT_QTDBUS_LIBRARIES} ${USBLL}
${QT_QTSVG_LIBRARY} phonon)

INSTALL(TARGETS RoomControlServer DESTINATION bin CONFIGURATIONS Debug Release)

INSTALL(FILES 9dd72914-730d-4dc0-b37a-3d37e5c2c59c.desktop
  DESTINATION share/liri/profiles/applications)


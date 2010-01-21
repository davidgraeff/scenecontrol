project(roomcontrolClient)
cmake_minimum_required(VERSION 2.6)
find_package(Qt4 REQUIRED)

# search KDE4 and use apply default (but optional) settings
find_package(KDE4 REQUIRED)
include(KDE4Defaults)

include_directories(${QT_INCLUDES} ${QT_QTNETWORK_INCLUDE_DIR} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} 
${CMAKE_CURRENT_BINARY_DIR} ../RoomControlServer/src/ ../RoomControlServer/src/Transferable/ .)

set(roomcontrolClient_SRCS
    ../RoomControlServer/src/Transferables/executableaudio.cpp
    ../RoomControlServer/src/Transferables/executablechannel.cpp 
    ../RoomControlServer/src/Transferables/executablepin.cpp
    ../RoomControlServer/src/Transferables/executableplaylist.cpp
    ../RoomControlServer/src/Transferables/executablecurtain.cpp
    ../RoomControlServer/src/Transferables/executableprojector.cpp
    ../RoomControlServer/src/Transferables/executablempris.cpp
    ../RoomControlServer/src/Transferables/executable.cpp
    ../RoomControlServer/src/Transferables/executablecollection.cpp
    ../RoomControlServer/src/Transferables/timeablerepeating.cpp
    ../RoomControlServer/src/Transferables/timeableabsolute.cpp
    ../RoomControlServer/src/Transferables/timeable.cpp
    ../RoomControlServer/src/Transferables/transferablefactory.cpp
    ../RoomControlServer/src/Transferables/transferable.cpp
    main.cpp 
    mainwindow.cpp
    settingsdialog.cpp
    models/channelsmodeldelegate.cpp
    models/itemsmodel.cpp
    models/transferablemodel.cpp
    models/profilesmodel.cpp
    models/alarmmodel.cpp
    models/filesmodel.cpp
    models/playlistmodel.cpp
    roomclient.cpp 
    clientwidget.cpp
    tabs/alarmswidget.cpp  
    tabs/profileswidget.cpp
    tabs/pinwidget.cpp
    tabs/channelwidget.cpp
    tabs/actionswidget.cpp
    tabs/playlistwidget.cpp
    tabs/musicwidget.cpp
    widgets/qprogressbarjump.cpp
)

set(roomcontrolClient_H
    ../RoomControlServer/src/Transferables/executableaudio.h
    ../RoomControlServer/src/Transferables/executableplaylist.h
    ../RoomControlServer/src/Transferables/timeablerepeating.h
    ../RoomControlServer/src/Transferables/timeableabsolute.h
    ../RoomControlServer/src/Transferables/timeable.h
    ../RoomControlServer/src/Transferables/transferable.h
    mainwindow.h
    settingsdialog.h
    models/channelsmodeldelegate.h
    models/itemsmodel.h
    models/transferablemodel.h
    models/profilesmodel.h
    models/alarmmodel.h
    models/filesmodel.h
    models/playlistmodel.h
    roomclient.h
    clientwidget.h
    tabs/profileswidget.h
    tabs/pinwidget.h
    tabs/channelwidget.h
    tabs/alarmswidget.h
    tabs/actionswidget.h
    tabs/playlistwidget.h
    tabs/musicwidget.h
    widgets/qprogressbarjump.h
)

QT_WRAP_CPP(roomcontrolClient roomcontrolClient_SRCS_MOCS ${roomcontrolClient_H})

SET(roomcontrolClient_UIS
    mainwindow.ui
    settings.ui
    tabs/alarmsTab.ui
    tabs/pinsTab.ui
    tabs/channelsTab.ui
    tabs/profilesTab.ui
    tabs/actionsTab.ui
    tabs/playlistTab.ui
    tabs/musicTab.ui
)

QT4_WRAP_UI(roomcontrolClient_UIS_H ${roomcontrolClient_UIS})

QT4_ADD_RESOURCES(roomcontrolClient_RC res.qrc)

#MESSAGE(STATUS "my sources: ${roomcontrolClient_UIS_H} ${roomcontrolClient_SRCS_MOCS}")

add_definitions(${QT_DEFINITIONS} ${QDBUS_DEFINITIONS} ${KDE4_DEFINITIONS})
add_executable(roomclient ${roomcontrolClient_SRCS} ${roomcontrolClient_UIS_H} ${roomcontrolClient_SRCS_MOCS} ${roomcontrolClient_RC})
target_link_libraries(roomclient ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTNETWORK_LIBRARIES} ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS})

INSTALL(TARGETS roomclient DESTINATION bin CONFIGURATIONS Debug Release RelWithDebInfo)
install( FILES roomclient.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
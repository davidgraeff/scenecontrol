cmake_minimum_required(VERSION 2.6)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/")
find_package(RoomServerPlugins REQUIRED)

build_lib()
target_link_libraries(${PROJECT_NAME}_client ${QT_QTCORE_LIBRARY})
target_link_libraries(${PROJECT_NAME}_server ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARIES} phonon)
install_lib()

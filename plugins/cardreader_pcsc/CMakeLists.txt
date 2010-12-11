cmake_minimum_required(VERSION 2.6)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/" "${CMAKE_CURRENT_SOURCE_DIR}")
find_package(RoomServerPlugins REQUIRED)
find_package(PCSC REQUIRED)

include_directories(${PCSC_INCLUDE_DIRS})

build_lib()
target_link_libraries(${PROJECT_NAME}_client ${QT_QTCORE_LIBRARY})
target_link_libraries(${PROJECT_NAME}_server ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARIES} ${PCSC_LIBRARIES})
install_lib()

project(statetracker_gen)
cmake_minimum_required(VERSION 2.6)
find_package(Qt4 REQUIRED)

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})

set(statetracker_gen_SRCS main.cpp)
qt4_automoc(${statetracker_gen_SRCS})
add_executable(statetracker_gen ${statetracker_gen_SRCS})
target_link_libraries(statetracker_gen ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})

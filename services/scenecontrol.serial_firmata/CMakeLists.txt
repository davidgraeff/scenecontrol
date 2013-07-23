cmake_minimum_required(VERSION 2.8.8)
SET(PLUGIN_DEPENDENCIES_FOUND TRUE)
find_package(RoomPlugin REQUIRED)

install_schemas()
install_nodelib()
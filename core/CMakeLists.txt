cmake_minimum_required(VERSION 2.8.8)
PROJECT(scenecontrol_core)

# Install server certificate
file(GLOB GENERICCERTS "${CMAKE_CURRENT_SOURCE_DIR}/certificates/*.key" "${CMAKE_CURRENT_SOURCE_DIR}/certificates/*.crt")
INSTALL(FILES ${GENERICCERTS} DESTINATION ${CONFIG_SERVER_CERTPATH} COMPONENT ServerPlugins)
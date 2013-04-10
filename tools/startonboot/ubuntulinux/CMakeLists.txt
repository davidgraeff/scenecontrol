cmake_minimum_required(VERSION 2.8.8)

INSTALL(FILES sceneserver.conf DESTINATION /etc/init COMPONENT Server)
INSTALL(FILES sceneserversession DESTINATION bin COMPONENT Server)
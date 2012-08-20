cmake_minimum_required(VERSION 2.8.8)

INSTALL(FILES roomcontrol.conf DESTINATION /etc/init COMPONENT Server)
INSTALL(PROGRAMS roomcontrolserversession DESTINATION bin COMPONENT Server)
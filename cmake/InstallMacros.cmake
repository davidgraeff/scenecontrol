cmake_minimum_required(VERSION 2.8)

# set default build type
IF (NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE release)
  SET(CMAKE_CONFIGURATION_TYPES RelWithDebInfo release Debug)
ENDIF (NOT CMAKE_BUILD_TYPE)


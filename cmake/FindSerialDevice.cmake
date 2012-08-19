cmake_minimum_required(VERSION 2.8)

# rs232
SET(SRCS_SERIAL_H ${CMAKE_SOURCE_DIR}/shared/serialdevice/qxtserialdevice.h ${CMAKE_SOURCE_DIR}/shared/serialdevice/qxtserialdevice_p.h PARENT_SCOPE)
SET(SRCS_SERIAL ${CMAKE_SOURCE_DIR}/shared/serialdevice/qxtserialdevice.cpp PARENT_SCOPE)
IF (WIN32)
	#LIST(APPEND SRCS ${CMAKE_SOURCE_DIR}/shared/serialdevice/win_qxtserialdevice.cpp)
ELSE()
	SET(SRCS_SERIAL ${SRCS_SERIAL} ${CMAKE_SOURCE_DIR}/shared/serialdevice/qxtserialdevice_unix.cpp PARENT_SCOPE)
ENDIF()
QT4_WRAP_CPP(SRCS_SERIAL_MOCS ${SRCS_SERIAL_H})

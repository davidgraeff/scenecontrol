# Once done this will define:
#
#  UDEV_FOUND - system has the Udev library
#  UDEV_INCLUDE_DIR - the Udev include directory
#  UDEV_LIBRARY - the libraries needed to use Udev
#
# Copyright (c) 2008, Matthias Kretz, <kr...@kde.org>
# Copyright (c) 2009, Marcus Hufgard, <marcus.hufg...@hufgard.de>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (NOT UDEV_MINIMUM_VERSION)
  set(UDEV_MINIMUM_VERSION "0.11.0")
endif (NOT UDEV_MINIMUM_VERSION)

# if (UDEV_INCLUDE_DIR AND UDEV_LIBRARY)
#    # Already in cache, be silent
#    set(UDEV_FIND_QUIETLY TRUE)
# endif (UDEV_INCLUDE_DIR AND UDEV_LIBRARY)

if (NOT WIN32)
   include(FindPkgConfig)
   pkg_check_modules(PC_UDEV libudev>=${UDEV_MINIMUM_VERSION})
endif (NOT WIN32)

FIND_PATH(UDEV_INCLUDE_DIR libudev.h
   HINTS
   ${PC_UDEV_INCLUDEDIR}
   ${PC_UDEV_INCLUDE_DIRS}
   )

FIND_LIBRARY(UDEV_LIBRARY NAMES udev libudev
   HINTS
   ${PC_UDEV_LIBDIR}
   ${PC_UDEV_LIBRARY_DIRS}
   )

if (UDEV_INCLUDE_DIR AND UDEV_LIBRARY)
   set(UDEV_FOUND TRUE)
else (UDEV_INCLUDE_DIR AND UDEV_LIBRARY)
   set(UDEV_FOUND FALSE)
endif (UDEV_INCLUDE_DIR AND UDEV_LIBRARY)

if (UDEV_FOUND)
   if (NOT UDEV_FIND_QUIETLY)
      message(STATUS "Found Udev: ${UDEV_LIBRARY} ${UDEV_INCLUDE_DIR}")
   endif (NOT UDEV_FIND_QUIETLY)
else (UDEV_FOUND)
   message(STATUS "Could NOT find Udev")
endif (UDEV_FOUND)

mark_as_advanced(UDEV_INCLUDE_DIR UDEV_LIBRARY)

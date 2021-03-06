set(SWIPL_ABS_INSTALL_ARCH_LIB "${CMAKE_INSTALL_PREFIX}/${SWIPL_INSTALL_ARCH_LIB}")

set(CMAKE_SKIP_BUILD_RPATH  FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH "${SWIPL_ABS_INSTALL_ARCH_LIB}")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES
     "${SWIPL_ABS_INSTALL_ARCH_LIB}" isSystemDir)
IF("${isSystemDir}" STREQUAL "-1")
   SET(CMAKE_INSTALL_RPATH "${SWIPL_ABS_INSTALL_ARCH_LIB}")
ENDIF("${isSystemDir}" STREQUAL "-1")

if(BUILD_MACOS_BUNDLE)
  set(CMAKE_INSTALL_RPATH "@executable_path/../Frameworks")
endif()

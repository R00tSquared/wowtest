#
# Copyright (C) 2012 HellGround <http://www.hellground.pl/>
#
# Copyright (C) 2005-2012 MaNGOS project <http://getmangos.com/>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#

project(MoonwellCore)
set(SERVER_VERSION 2.4.3)

# CMake policies
cmake_minimum_required(VERSION 3.22)

set(CMAKE_MODULE_PATH
  ${CMAKE_MODULE_PATH}
  ${CMAKE_SOURCE_DIR}/cmake
)

# Force out-of-source build
string(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}" BUILDING_IN_SOURCE)
if(BUILDING_IN_SOURCE)
  message(FATAL_ERROR
    "This project requires an out of source build. Remove the file 'CMakeCache.txt' found in this directory before continuing, create a separate build directory and run 'cmake <srcs> [options]' from there."
  )
endif()

if(WIN32 AND NOT MSVC)
  message(FATAL_ERROR
    "Under Windows other compiler than Microsoft Visual Studio are not supported."
  )
endif()

find_package(Platform REQUIRED)

# VS100 uses MSBuild.exe instead of devenv.com, so force it to use devenv.com
if(WIN32 AND MSVC_VERSION MATCHES 1600)
  find_package(VisualStudio2010)
endif()

# if(NOT PLATFORM MATCHES X86 AND NOT PLATFORM MATCHES X64)
#   message(FATAL_ERROR
#     "An unknown Architecture was selected. Only the values X86 and X64 for PLATFORM are supported."
#   )
# endif()

if(NOT WIN32)
    execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
    if (GCC_VERSION VERSION_LESS 4.7)
        message(FATAL_ERROR "Your GCC have to be >= 4.7")
    endif()
endif()

# Output description of this script
message(
  "\nThis script builds the HellGround project server.
  Options that can be used in order to configure the process:
    PREFIX: Path where the server should be installed to
    PCH   : Use precompiled headers
    DEBUG : Debug mode
	NOT_USE_ELUNA_HOOKS: Do not use Elunas HookMgr
    CLI   : Build with CLI (default)
    TBB_USE_EXTERNAL: Use external TBB library instead of built in (default)
    ACE_USE_EXTERNAL: Use external ACE library instead of built in (default)
    USE_STD_MALLOC  : Use standard malloc instead of TBB
    LARGE_CELL      : Use large cell size
    ADD_COMPILE_F   : Add additional compile flags (default)
    ADD_OPTI_F      : Add additional compile optimization flags
    ADD_MATH_F      : Add additional compile math flags
    ADD_GPROF_F     : Add additional compile gprof flag
    MAP_UPDATE_DIFF_INFO: Used for gathering info about execution time for specific parts of Map::Update

  To set an option simply type -D<OPTION>=<VALUE> after 'cmake <srcs>'.
  For example: cmake .. -DDEBUG=1 -DPREFIX=/opt/mangos\n"
) # TODO: PLATFORM: Sets the architecture for compile (X86,X64)

# Override configuration-types - we don't use anything else than debug and release
if(CMAKE_CONFIGURATION_TYPES)
  if(WIN32)
	set(CMAKE_CONFIGURATION_TYPES Debug)
  else()
	set(CMAKE_CONFIGURATION_TYPES Release Debug)
  endif()
  
  set(CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES}" CACHE STRING
    "Reset the configurations to what we need"
    FORCE)
endif()

# Find out what system we use to include the needed libs
if(WIN32)
  if(PLATFORM MATCHES X86) # 32-bit
    set(DEP_ARCH win32)
  else() # 64-bit
    set(DEP_ARCH x64)
  endif()
endif()

# if(WIN32)
#   if(PLATFORM MATCHES X86)
#     set(ARCH_FLAGS "/MACHINE:X86")
#   else()
#     set(ARCH_FLAGS "/MACHINE:X64")
#   endif()
# elseif(UNIX)
#   if(PLATFORM MATCHES X86)
#     set(ARCH_FLAGS "-m32")
#   else()
#     set(ARCH_FLAGS "-m64")
#   endif()
# endif()

option(DEBUG "Debug mode" 0)
option(CLI "With CLI" 1)

option(USE_STD_MALLOC "Use standard malloc instead of TBB" 0)

option(LARGE_CELL "Use large cell size" 0)
option(NOT_USE_ELUNA_HOOKS "Do not use Elunas HookMgr" 1)
option(ADD_COMPILE_F "Add additional compile flags" 1)
option(ADD_MATH_F "Add additional compile math flags" 0)
option(ADD_GPROF_F "Add additional compile gprof flag" 0)
option(MAP_UPDATE_DIFF_INFO "Used for gathering info about execution time for specific parts of Map::Update" 0)

if(WIN32)
option(TBB_USE_EXTERNAL "Use external TBB" 0)
option(ACE_USE_EXTERNAL "Use external ACE" 0)
else()
option(TBB_USE_EXTERNAL "Use external TBB" 1)
option(ACE_USE_EXTERNAL "Use external ACE" 1)
endif()

find_package(PCHSupport)

# Add options for compile of HG
if(PCHSupport_FOUND)
  if(WIN32)
    option(PCH "Use precompiled headers" 1)
  else()
    option(PCH "Use precompiled headers" 0)
  endif()
endif()

# FIXME: options that should be checked
# option(SQL "Copy SQL files" 0)
# option(TOOLS "Build tools" 0)

# Set up the install-prefix
if(CMAKE_INSTALL_PREFIX STREQUAL "/usr/local")
  get_filename_component(PREFIX_ABSOLUTE "../hellground-server" ABSOLUTE)
  set(CMAKE_INSTALL_PREFIX ${PREFIX_ABSOLUTE} CACHE PATH "Install path prefix." FORCE)
endif()
if(PREFIX)
  if(!WIN32)
    string(REGEX REPLACE "^~" "$ENV{HOME}" PREFIX ${PREFIX})
  endif()
  get_filename_component(PREFIX_ABSOLUTE ${PREFIX} ABSOLUTE)
  set(CMAKE_INSTALL_PREFIX ${PREFIX} CACHE PATH "Install path prefix." FORCE)
else()
  set(PREFIX ${CMAKE_INSTALL_PREFIX} CACHE PATH "Install path prefix.")
endif()

# If win32 put it in the bin dir not lib
if(WIN32)
  set(BIN_DIR ${CMAKE_INSTALL_PREFIX})
  set(CONF_DIR ${CMAKE_INSTALL_PREFIX})  
  set(LIBS_DIR ${CMAKE_INSTALL_PREFIX}/lib)
else()
  set(BIN_DIR ${CMAKE_INSTALL_PREFIX}/bin)
  set(CONF_DIR ${CMAKE_INSTALL_PREFIX}/etc)
  set(LIBS_DIR ${CMAKE_INSTALL_PREFIX}/lib)
endif()

# For Unix systems set the rpath so that libraries are found
set(CMAKE_INSTALL_RPATH ${LIBS_DIR})
set(CMAKE_INSTALL_NAME_DIR ${LIBS_DIR})
# Run out of build tree
set(CMAKE_BUILD_WITH_INSTALL_RPATH OFF)

# Find needed packages and if necessery abort if something important is missing
unset(ACE_INCLUDE_DIR CACHE)
unset(ACE_LIBRARIES CACHE)
unset(ACE_LIBRARIES_DIR CACHE)
unset(ACE_INCLUDE_DIR)
unset(ACE_LIBRARIES)
unset(ACE_LIBRARIES_DIR)
if(ACE_USE_EXTERNAL)
  find_package(ACE)
  if(NOT ACE_FOUND)
    message(FATAL_ERROR
      "This project requires ACE installed when ACE_USE_EXTERNAL is set. Please download the ACE Micro Release Kit from http://download.dre.vanderbilt.edu/ and install it. If this script didn't find ACE and it was correctly installed please set ACE_ROOT to the correct path."
    )
  endif()
  if(EXISTS ${ACE_INCLUDE_DIR}/ace/Stack_Trace.h)
    set(HAVE_ACE_STACK_TRACE_H ON) # config.h.cmake
  endif()
else()
  include(cmake/ImportACE.cmake)
endif()

if (NOT USE_STD_MALLOC)
  unset(TBB_INCLUDE_DIR CACHE)
  unset(TBB_LIBRARIES CACHE)
  unset(TBB_LIBRARIES_DIR CACHE)
  unset(TBB_INCLUDE_DIR)
  unset(TBB_LIBRARIES)
  unset(TBB_LIBRARIES_DIR)
  if(TBB_USE_EXTERNAL)
    find_package(TBB)
    if(NOT TBB_FOUND)
      message(FATAL_ERROR
        "This project requires TBB installed when TBB_USE_EXTERNAL is set. Please download the TBB Stable Release from http://www.threadingbuildingblocks.org/ and install it. If this script didn't find TBB and it was correctly installed please set TBB_ROOT to the correct path."
      )
    endif()
  else()
    include(cmake/ImportTBB.cmake)
  endif()
endif()

# Win32 delifered packages
if(WIN32)
  set(MYSQL_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/dep/include/mysql)
  set(MYSQL_LIBRARY ${CMAKE_SOURCE_DIR}/dep/lib/${DEP_ARCH}_release/libmySQL.lib)
  set(MYSQL_DEBUG_LIBRARY ${CMAKE_SOURCE_DIR}/dep/lib/${DEP_ARCH}_debug/libmySQL.lib)
  # zlib is build
  set(OPENSSL_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/dep/include/openssl)
  set(OPENSSL_LIBRARIES ${CMAKE_SOURCE_DIR}/dep/lib/${DEP_ARCH}_release/libeay32.lib)
  set(OPENSSL_DEBUG_LIBRARIES ${CMAKE_SOURCE_DIR}/dep/lib/${DEP_ARCH}_debug/libeay32.lib)
endif()

# *nix-specific packages
if(UNIX)
  find_package(Readline REQUIRED)
  find_package(MySQL REQUIRED)
  find_package(ZLIB REQUIRED)
  find_package(OpenSSL REQUIRED)
endif()

# Add uninstall script and target
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY
)

add_custom_target(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
)

message("")
message("Revision              : ${HellGround_HG_ID}")
message("Last Changed Author   : ${HellGround_HG_AUTHOR}")
message("Last Changed Rev      : ${HellGround_HG_CHANGESET}")
message("Last Changed Date     : ${HellGround_HG_DATE}")
message("Install server to     : ${CMAKE_INSTALL_PREFIX}")
message("")

if(CLI)
  message("Build with CLI        : Yes (default)")
  add_definitions(-DENABLE_CLI)
else()
  message("Build with CLI        : No")
endif()

# if(RA)
#   message("* Build with RA         : Yes")
#   add_definitions(-DENABLE_RA)
# else(RA)
#   message("* Build with RA         : No  (default)")
# endif(RA)

if(PCH AND NOT PCHSupport_FOUND)
  set(PCH 0 CACHE BOOL
    "Use precompiled headers"
    FORCE)
  message(
    "No PCH for your system possible but PCH was set to 1. Resetting it."
  )
endif()
if(PCH)
  message("Use PCH               : Yes")
else()
  message("Use PCH               : No")
endif()

if(DEBUG)
  message("Build in debug-mode   : Yes")
  set(CMAKE_BUILD_TYPE Debug)
else()
  set(CMAKE_BUILD_TYPE Release)
  message("Build in debug-mode   : No  (default)")
endif()

if(LARGE_CELL)
  message("Build with cell size  : Large")
  add_definitions(-DLARGE_CELL)
else(LARGE_CELL)
  message("Build with cell size  : Small (default)")
endif(LARGE_CELL)

message("")

# Set warning levels for different builds
if(UNIX)
  if(PLATFORM MATCHES X86)
      return("WTF x32?")
  else()
	# -O2 removed to write <optimized out> variables in GDB
	  set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
      set(ADDITIONAL_COMPILE_FLAGS "-std=c++0x -ggdb -msse3 -fno-delete-null-pointer-checks -ftracer -fno-strict-aliasing -funroll-loops -m64 -pipe -DFD_SETSIZE=4096 -Wno-deprecated-declarations -Wno-stringop-overflow")
  endif()
	  set(ADDITIONAL_OPTIMIZE_FLAGS "-fomit-frame-pointer -frename-registers")
	  set(ADDITIONAL_MATH_FLAGS " -ffast-math -fno-math-errno -funsafe-math-optimizations")
	  set(ADDITIONAL_GPROF_FLAGS " -pg")
	  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -Wfatal-errors")
elseif(WIN32)
  # Disable warnings in Visual Studio 8 and above and add /MP
  #  AND NOT CMAKE_GENERATOR MATCHES "Visual Studio 7"
  if(MSVC)
	set(GLOBAL_WIN_FLAGS "/Zi /wd4996 /wd4355 /wd4244 /wd4267 /MP /Zm169") # /Gy /Oi /Ot /Qpar /MDd 
	
    #set(CMAKE_CXX_FLAGS_RELEASE "/O2 /Ob2 /DNDEBUG ${GLOBAL_WIN_FLAGS}")
    set(CMAKE_CXX_FLAGS_DEBUG "/Od /Ob2 /DNDEBUG ${GLOBAL_WIN_FLAGS}")
	
	string(REPLACE "/RTC1" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
  endif()
endif()

# if(SQL)
#   message("Install SQL-files     : Yes")
# else()
#   message("Install SQL-files     : No  (default)")
# endif()

# if(TOOLS)
#   message("Build map/vmap tools  : Yes")
# else()
#   message("Build map/vmap tools  : No  (default)")
# endif()

message("")

# Some small tweaks for Visual Studio 7 and above.
if(MSVC)
  # Mark 32 bit executables large address aware so they can use > 2GB address space
  if(PLATFORM MATCHES X86)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LARGEADDRESSAWARE")
  endif()
endif()

set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH ${LIBS_DIR})
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

if(WIN32)
  # install(
    # FILES
      # ${CMAKE_SOURCE_DIR}/dep/lib/${DEP_ARCH}_release/libmySQL.dll
    # DESTINATION ${LIBS_DIR}
    # CONFIGURATIONS Release
  # )
  install(
    FILES
      ${CMAKE_SOURCE_DIR}/dep/lib/${DEP_ARCH}_debug/libmySQL.dll
    DESTINATION ${LIBS_DIR}
    CONFIGURATIONS Debug
  )
  if(PLATFORM MATCHES X86)
    # Copy dll's Windows needs
    # install(
      # FILES
        # ${CMAKE_SOURCE_DIR}/dep/lib/win32_release/dbghelp.dll
      # DESTINATION ${LIBS_DIR}
      # CONFIGURATIONS Release
    # )
    install(
      FILES
        ${CMAKE_SOURCE_DIR}/dep/lib/win32_debug/dbghelp.dll
      DESTINATION ${LIBS_DIR}
      CONFIGURATIONS Debug
    )
  endif()
endif()

# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ARCH_FLAGS}")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ARCH_FLAGS}")
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${ARCH_FLAGS}")

if(ADD_COMPILE_F)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ADDITIONAL_COMPILE_FLAGS}")
endif()

if(ADD_OPTI_F)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ADDITIONAL_OPTIMIZE_FLAGS}")
endif()

if(ADD_MATH_F)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ADDITIONAL_MATH_FLAGS}")
endif()

if (ADD_GPROF_F)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ADDITIONAL_GPROF_FLAGS}")
endif()

if(XCODE)
  if(PLATFORM MATCHES X86)
    set(CMAKE_OSX_ARCHITECTURES i386)
  else()
    set(CMAKE_OSX_ARCHITECTURES x86_64)
  endif()
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

add_subdirectory(dep) # TODO: add vmap extractor build support

# Add definitions for all build types
# Don't place this above 'dep' subdirectory! Because of defines build will crash.
set(DEFINITIONS
  DO_MYSQL
  HAVE_CONFIG_H
  VERSION="${SERVER_VERSION}"
  SYSCONFDIR="${CONF_DIR}/"
)
set(DEFINITIONS_DEBUG _DEBUG MANGOS_DEBUG)
if(WIN32)
  set(DEFINITIONS ${DEFINITIONS} WIN32 _WIN32)
  set(DEFINITIONS_RELEASE ${DEFINITIONS_RELEASE} _CRT_SECURE_NO_WARNINGS)
endif()
if(USE_STD_MALLOC)
	set(DEFINITIONS ${DEFINITIONS} USE_STANDARD_MALLOC)
endif()

if(NOT_USE_ELUNA_HOOKS)
	set(DEFINITIONS ${DEFINITIONS} NOT_USE_ELUNA_HOOKS)
endif()

set_directory_properties(PROPERTIES COMPILE_DEFINITIONS "${DEFINITIONS}")
set_directory_properties(PROPERTIES COMPILE_DEFINITIONS_RELEASE "${DEFINITIONS_RELEASE}")
set_directory_properties(PROPERTIES COMPILE_DEFINITIONS_DEBUG "${DEFINITIONS_DEBUG}")

message("CXX flags                      : ${CMAKE_CXX_FLAGS}")
message("CXX Debug flags                : ${CMAKE_CXX_FLAGS_DEBUG}")
message("CXX Release flags              : ${CMAKE_CXX_FLAGS_RELEASE}")

message("")

message("Compile definitions            : ${DEFINITIONS}")
message("Compile debug definitions      : ${DEFINITIONS_DEBUG}")
message("Compile release definitions    : ${DEFINITIONS_RELEASE}")

message("")

if(DEBUG)
    message("DEBUG option is enabled")
else()
    message("DEBUG option is disabled")
endif()

message("")

add_subdirectory(src)
# if(SQL)
#   add_subdirectory(sql)
# endif()
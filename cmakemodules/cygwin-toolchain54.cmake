#
# Copyright (C) Alneos, s.a.r.l. (contact@alneos.fr)
# Released under the GNU General Public License
#
# A Default toolchain for compiling Vega under Cygwin
# It uses the mingw-w64 compilers tools
# Syntax:
#   cmake -DCMAKE_TOOLCHAIN_FILE=path/to/this/file
#

# Where you have installed Cygwin
set(CYGWIN_MOUNT "/cygdrive/c/cygwin64_new")

# Looking for what we need
set(CMAKE_FIND_ROOT_PATH "${CYGWIN_MOUNT}")
set(COMPILER_PREFIX "x86_64-w64-mingw32")
find_program(CMAKE_C_COMPILER NAMES ${COMPILER_PREFIX}-gcc)
find_program(CMAKE_CXX_COMPILER NAMES ${COMPILER_PREFIX}-g++)
find_program(CMAKE_AR NAMES ${COMPILER_PREFIX}-ar)
find_program(CMAKE_RANLIB NAMES ${COMPILER_PREFIX}-ranlib)

include_directories(SYSTEM "/usr/x86_64-w64-mingw32/sys-root/mingw/include")
include_directories(SYSTEM "/cygdrive/c/cygwin64_new/usr/include")
include_directories("/usr/include")

SET(CYGWIN TRUE)

#
# Copyright 2012-2013 The Iris Project Developers. See the
# COPYRIGHT file at the top-level directory of this distribution
# and at http://www.softwareradiosystems.com/iris/copyright.html.
#
# This file is part of the Iris Project.
#
# Iris is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation, either version 3 of
# the License, or (at your option) any later version.
#
# Iris is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# A copy of the GNU Lesser General Public License can be found in
# the LICENSE file in the top-level directory of this distribution
# and at http://www.gnu.org/licenses/.
#

# - Try to find iris - the software radio framework
# Once done this will define
#  IRIS_FOUND - System has iris
#  IRIS_INCLUDE_DIRS - The iris include directories
#  IRIS_DEFINITIONS - Compiler switches required for using iris

find_package(PkgConfig)
pkg_check_modules(PC_IRIS QUIET iris_core)
set(IRIS_DEFINITIONS ${PC_IRIS_CFLAGS_OTHER})

find_path(IRIS_INCLUDE_DIR 
            NAMES irisapi/Version.h
            HINTS ${PC_IRIS_INCLUDEDIR} ${PC_IRIS_INCLUDE_DIRS} $ENV{IRIS_DIR}/include
            PATHS /usr/local/include 
                  /usr/include )

set(IRIS_INCLUDE_DIRS ${IRIS_INCLUDE_DIR} )

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set IRIS_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(IRIS DEFAULT_MSG IRIS_INCLUDE_DIR)

mark_as_advanced(IRIS_INCLUDE_DIR )

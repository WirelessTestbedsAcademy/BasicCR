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

MESSAGE(STATUS "  Processing udpsocketrx.")

########################################################################
# Add includes and dependencies
########################################################################
SET(Boost_ADDITIONAL_VERSIONS "1.42.0" "1.42" "1.43.0" "1.43" "1.44.0" "1.44" "1.45.0" "1.45" "1.46.0" "1.46" "1.47.0" "1.47")
FIND_PACKAGE(Boost 1.36)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})

########################################################################
# Build the library from source files
########################################################################
SET(sources
	UdpSocketRxComponent.cpp
)

IF (Boost_FOUND)
  ADD_LIBRARY(comp_gpp_phy_udpsocketrx SHARED ${sources})
  TARGET_LINK_LIBRARIES(comp_gpp_phy_udpsocketrx)
  SET_TARGET_PROPERTIES(comp_gpp_phy_udpsocketrx PROPERTIES OUTPUT_NAME "udpsocketrx")
  IRIS_INSTALL(comp_gpp_phy_udpsocketrx)
  IRIS_APPEND_INSTALL_LIST(udpsocketrx)
ELSE (Boost_FOUND)
  IRIS_APPEND_NOINSTALL_LIST(udpsocketrx)
ENDIF (Boost_FOUND)

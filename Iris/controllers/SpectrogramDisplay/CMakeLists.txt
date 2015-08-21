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

MESSAGE(STATUS "  Processing spectrogramdisplay.")

########################################################################
# Add includes and dependencies
########################################################################

########################################################################
# Build the library from source files
########################################################################
SET(sources
	SpectrogramDisplayController.cpp
)

IF(IRIS_HAVE_REALPLOT)
    # Targets must be globally unique for cmake
    ADD_LIBRARY(controller_spectrogramdisplay SHARED ${sources})
    TARGET_LINK_LIBRARIES(controller_spectrogramdisplay realplot)
    SET_TARGET_PROPERTIES(controller_spectrogramdisplay PROPERTIES OUTPUT_NAME "spectrogramdisplay")
    IRIS_INSTALL(controller_spectrogramdisplay)
    IRIS_APPEND_INSTALL_LIST("spectrogramdisplay")
ELSE(IRIS_HAVE_REALPLOT)
    IRIS_APPEND_NOINSTALL_LIST("spectrogramdisplay")
ENDIF(IRIS_HAVE_REALPLOT)

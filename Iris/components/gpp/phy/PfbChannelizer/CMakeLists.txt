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

MESSAGE(STATUS "  Processing pfbchannelizer.")

########################################################################
# Add includes and dependencies
########################################################################
FIND_PACKAGE(LIQUIDDSP REQUIRED)

########################################################################
# Build the library from source files
########################################################################
SET(sources
	PfbChannelizerComponent.cpp
)

IF(LIQUIDDSP_FOUND)
    INCLUDE_DIRECTORIES(${LIQUIDDSP_INCLUDE_DIRS})

    # Static library to be used in tests
    ADD_LIBRARY(comp_gpp_phy_pfbchannelizer_static STATIC ${sources})
    TARGET_LINK_LIBRARIES(comp_gpp_phy_pfbchannelizer_static ${LIQUIDDSP_LIBRARIES})

    # Shared library to be used in radios
    ADD_LIBRARY(comp_gpp_phy_pfbchannelizer SHARED ${sources})
    TARGET_LINK_LIBRARIES(comp_gpp_phy_pfbchannelizer ${LIQUIDDSP_LIBRARIES})
    SET_TARGET_PROPERTIES(comp_gpp_phy_pfbchannelizer PROPERTIES OUTPUT_NAME "pfbchannelizer")
    IRIS_INSTALL(comp_gpp_phy_pfbchannelizer)
    IRIS_APPEND_INSTALL_LIST(pfbchannelizer)

    # Add the test and benchmark directories
    ADD_SUBDIRECTORY(test)
    #ADD_SUBDIRECTORY(benchmark)
ELSE(LIQUIDDSP_FOUND)
    IRIS_APPEND_NOINSTALL_LIST(pfbchannelizer)
ENDIF(LIQUIDDSP_FOUND)



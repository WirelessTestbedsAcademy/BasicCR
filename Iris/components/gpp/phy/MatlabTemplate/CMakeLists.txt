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

MESSAGE(STATUS "  Processing matlabtemplate.")

########################################################################
# Add includes and dependencies
########################################################################
FIND_PACKAGE( MATLAB )

########################################################################
# Build the library from source files
########################################################################
SET(sources
	MatlabTemplateComponent.cpp
)

IF (MATLAB_FOUND)
  INCLUDE_DIRECTORIES(${MATLAB_INCLUDE_DIR})
  ADD_LIBRARY(comp_gpp_phy_matlabtemplate SHARED ${sources})
  TARGET_LINK_LIBRARIES(comp_gpp_phy_matlabtemplate ${MATLAB_LIBRARIES})
  SET_TARGET_PROPERTIES(comp_gpp_phy_matlabtemplate PROPERTIES OUTPUT_NAME "matlabtemplate")
  
  # We add an rpath to the library so matlab dependencies will be found
  # while avoiding clashes with system libraries
  SET_TARGET_PROPERTIES(comp_gpp_phy_matlabtemplate PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
  SET_TARGET_PROPERTIES(comp_gpp_phy_matlabtemplate PROPERTIES INSTALL_RPATH ${MATLAB_LIB_DIR})
  IRIS_INSTALL(comp_gpp_phy_matlabtemplate)
  IRIS_APPEND_INSTALL_LIST(matlabtemplate)
ELSE (MATLAB_FOUND)
  IRIS_APPEND_NOINSTALL_LIST(matlabtemplate)
ENDIF (MATLAB_FOUND)

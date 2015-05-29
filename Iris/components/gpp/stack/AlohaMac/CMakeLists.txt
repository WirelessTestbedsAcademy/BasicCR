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

MESSAGE(STATUS "  Processing alohamac.")

########################################################################
# Add includes and dependencies
########################################################################
FIND_PACKAGE( Protobuf )

########################################################################
# Build the library from source files
########################################################################
SET(sources
  AlohaMacComponent.cpp
  alohamac.pb.cc
)

IF (PROTOBUF_FOUND)
    # Targets must be globally unique for cmake 
    ADD_LIBRARY(comp_gpp_stack_alohamac SHARED ${sources})
    TARGET_LINK_LIBRARIES(comp_gpp_stack_alohamac ${PROTOBUF_LIBRARIES})
    INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
    PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS alohamac.proto)
    SET_TARGET_PROPERTIES(comp_gpp_stack_alohamac PROPERTIES OUTPUT_NAME "alohamac")
    IRIS_INSTALL(comp_gpp_stack_alohamac)
    IRIS_APPEND_INSTALL_LIST(alohamac)
ELSE (PROTOBUF_FOUND)
    IRIS_APPEND_NOINSTALL_LIST(alohamac)
ENDIF (PROTOBUF_FOUND)

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

########################################################################
# unit test suite
########################################################################


########################################################################
# Add includes and dependencies
########################################################################
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR})

########################################################################
# Build executables, register as tests, and install
########################################################################
# Build each test and link to libraries
SET(test_sources
	
)

#turn each test cpp file into an executable with an int main() function
ADD_DEFINITIONS(-DBOOST_TEST_DYN_LINK -DBOOST_TEST_MAIN)

#for each source: build an executable, register it as a test, and install
FOREACH(test_source ${test_sources})
    GET_FILENAME_COMPONENT(test_name ${test_source} NAME_WE)
    ADD_EXECUTABLE(${test_name} ${test_source})
    ADD_TEST(${test_name} ${test_name})
    INSTALL(TARGETS ${test_name} RUNTIME DESTINATION ${DATA_DIR}/tests/controllers)
ENDFOREACH(test_source)


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
# Build header-only tests
########################################################################
# Build each test and link to libraries
SET(test_sources
    Crc_test.cpp
    OfdmIndexGenerator_test.cpp
    OfdmPreambleDetector_test.cpp
    QamDemodulator_test.cpp
    QamModulator_test.cpp
    ToneGenerator_test.cpp
    Whitener_test.cpp
)

#turn each test cpp file into an executable with an int main() function
ADD_DEFINITIONS(-DBOOST_TEST_DYN_LINK -DBOOST_TEST_MAIN)

#for each source: build an executable, register it as a test, and install
INCLUDE_DIRECTORIES(..)
FOREACH(test_source ${test_sources})
    GET_FILENAME_COMPONENT(test_name ${test_source} NAME_WE)
    ADD_EXECUTABLE(${test_name} ${test_source})
    TARGET_LINK_LIBRARIES(${test_name} ${Boost_LIBRARIES})
    ADD_TEST(${test_name} ${test_name})
ENDFOREACH(test_source)

########################################################################
# Build any lib-dependent tests
########################################################################
FIND_PACKAGE( FFTW3F )

IF (FFTW3F_FOUND)
    INCLUDE_DIRECTORIES(${FFTW3F_INCLUDE_DIRS})
    ADD_EXECUTABLE(OfdmPreambleGenerator_test OfdmPreambleGenerator_test.cpp)
    TARGET_LINK_LIBRARIES(OfdmPreambleGenerator_test ${Boost_LIBRARIES} ${FFTW3F_LIBRARIES})
    ADD_TEST(OfdmPreambleGenerator_test OfdmPreambleGenerator_test)
ENDIF (FFTW3F_FOUND)

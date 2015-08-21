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
# Add includes and dependencies
########################################################################
INCLUDE_DIRECTORIES(..)

ADD_DEFINITIONS(-DBOOST_TEST_DYN_LINK -DBOOST_TEST_MAIN)
ADD_EXECUTABLE(udpsocket_test UdpSocket_test.cpp)
TARGET_LINK_LIBRARIES(udpsocket_test ${Boost_LIBRARIES})
ADD_TEST(udpsocket_test udpsocket_test)

IF (IRIS_HAVE_MATLABPLOTTER)
    ADD_DEFINITIONS(-DBOOST_TEST_DYN_LINK -DBOOST_TEST_MAIN)
    ADD_EXECUTABLE(matlabplotter_test MatlabPlotter_test.cpp)
    TARGET_LINK_LIBRARIES(matlabplotter_test ${Boost_LIBRARIES} matlabplotter)
    ADD_TEST(matlabplotter_test matlabplotter_test)
ENDIF (IRIS_HAVE_MATLABPLOTTER)

IF (IRIS_HAVE_PYTHONPLOTTER)
    ADD_DEFINITIONS(-DBOOST_TEST_DYN_LINK -DBOOST_TEST_MAIN)
    ADD_EXECUTABLE(pythonplotter_test PythonPlotter_test.cpp)
    TARGET_LINK_LIBRARIES(pythonplotter_test ${Boost_LIBRARIES} pythonplotter)
    ADD_TEST(pythonplotter_test pythonplotter_test)
ENDIF (IRIS_HAVE_PYTHONPLOTTER)

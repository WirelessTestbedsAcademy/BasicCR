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
# Custom target to ensure headers get picked up by IDEs
########################################################################
SET(headers
    DataBufferTrivial.h
    EndianConversion.h
    FileUtility.h
    FirFilter.h
    Matlab.h
    RawFileUtility.h
    StackHelper.h
    UdpSocketReceiver.h
    UdpSocketTransmitter.h
)
ADD_CUSTOM_TARGET(libgenericutilityheaders SOURCES ${headers})

########################################################################
# Build the MatlabPlotter lib
# We build this as a shared lib so that we can add the rpath here
# and don't have to worry about it elsewhere.
########################################################################
MESSAGE(STATUS "Building MatlabPlotter utility.")
FIND_PACKAGE( MATLAB )

IF (MATLAB_FOUND)
    INCLUDE_DIRECTORIES(${MATLAB_INCLUDE_DIR})
    ADD_LIBRARY(matlabplotter STATIC MatlabPlotter.cpp MatlabPlotter.h)
    TARGET_LINK_LIBRARIES(matlabplotter ${Boost_LIBRARIES} ${MATLAB_LIBRARIES})
    # We add an rpath to the library so matlab dependencies will be found
    # while avoiding clashes with system libraries
    SET_TARGET_PROPERTIES(matlabplotter PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
    SET_TARGET_PROPERTIES(matlabplotter PROPERTIES INSTALL_RPATH ${MATLAB_LIB_DIR})
    SET(IRIS_HAVE_MATLABPLOTTER ON CACHE INTERNAL "Has MatlabPlotter been built?")
    IRIS_SET_PIC(matlabplotter)
ELSE (MATLAB_FOUND)
    SET(IRIS_HAVE_MATLABPLOTTER OFF CACHE INTERNAL "Has MatlabPlotter been built?")
ENDIF (MATLAB_FOUND)

########################################################################
# Build the PythonPlotter lib
########################################################################
MESSAGE(STATUS "Building PythonPlotter utility.")
FIND_PACKAGE( PythonLibs )

IF (PYTHONLIBS_FOUND)
    INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_DIRS})
    ADD_LIBRARY(pythonplotter STATIC PythonPlotter.cpp PythonPlotter.h)
    TARGET_LINK_LIBRARIES(pythonplotter ${Boost_LIBRARIES} ${PYTHON_LIBRARIES})
    SET(IRIS_HAVE_PYTHONPLOTTER ON CACHE INTERNAL "Has PythonPlotter been built?")
    IRIS_SET_PIC(pythonplotter)
ELSE (PYTHONLIBS_FOUND)
    SET(IRIS_HAVE_PYTHONPLOTTER OFF CACHE INTERNAL "Has PythonPlotter been built?")
ENDIF (PYTHONLIBS_FOUND)

########################################################################
# Add the subdirectories
########################################################################
# Recurse into subdirectories. This does not actually cause another cmake 
# executable to run. The same process will walk through the project's 
# entire directory structure.
ADD_SUBDIRECTORY(test)

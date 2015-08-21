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

# - Try to find matlab
# Once done this will define
#  MATLAB_FOUND - System has matlab
#  MATLAB_LIBRARIES - The matlab libraries
#  MATLAB_INCLUDE_DIRS - The matlab include directories
#  MATLAB_LIB_DIRS - The matlab library directories
MESSAGE(STATUS "Checking for Matlab")

SET(MATLAB_FOUND 0)
IF(WIN32)
  SET(MATLAB_ROOT 
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\MathWorks\\MATLAB\\7.4;MATLABROOT]/extern/lib/win32/microsoft"
    "$ENV{MATLAB_DIR}/extern/lib/win32/microsoft"
    "$ENV{MATLAB_LIBS_DIR}"
    )
  FIND_LIBRARY(MATLAB_MEX_LIBRARY
    NAMES libmex
    PATHS ${MATLAB_ROOT}
    )
  FIND_LIBRARY(MATLAB_MX_LIBRARY
    NAMES libmx
    PATHS ${MATLAB_ROOT}
    )
  FIND_LIBRARY(MATLAB_ENG_LIBRARY
    NAMES libeng
    PATHS ${MATLAB_ROOT}
    )
  FIND_PATH(MATLAB_LIB_DIR
    NAMES libmex.dll
    PATHS ${MATLAB_ROOT}
    )
  FIND_PATH(MATLAB_INCLUDE_DIR
    "mex.h"
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\MathWorks\\MATLAB\\7.4;MATLABROOT]/extern/include"
    "$ENV{MATLAB_DIR}/extern/include"
    )
ELSEIF(APPLE)
  SET(MATLAB_ROOT 
    /Applications/MATLAB_R2013a.app/bin/maci64/
    /Applications/MATLAB_R2012b.app/bin/maci64/
    /Applications/MATLAB_R2012a.app/bin/maci64
    )
  FIND_LIBRARY(MATLAB_MEX_LIBRARY
    NAMES mex
    PATHS ${MATLAB_ROOT}
    )
  FIND_LIBRARY(MATLAB_MX_LIBRARY
    NAMES mx
    PATHS ${MATLAB_ROOT}
    NO_DEFAULT_PATH
    )
  FIND_LIBRARY(MATLAB_ENG_LIBRARY
    NAMES eng
    PATHS ${MATLAB_ROOT}
    )
  FIND_PATH(MATLAB_LIB_DIR
    NAMES libmex.dylib
    PATHS ${MATLAB_ROOT}
    )
  FIND_PATH(MATLAB_INCLUDE_DIR
    NAMES mex.h
    PATHS /Applications/MATLAB_R2013a.app/extern/include/
          /Applications/MATLAB_R2012b.app/extern/include/
          /Applications/MATLAB_R2012a.app/extern/include/    
    )
ELSE( WIN32 )
  IF(CMAKE_SIZEOF_VOID_P EQUAL 4)
    # Regular x86
    SET(MATLAB_ROOT
      /usr/local/matlab74/bin/glnx86/
      /opt/matlab74/bin/glnx86/
      $ENV{HOME}/matlab74/bin/glnx86/
      $ENV{HOME}/redhat-matlab/bin/glnx86/
      $ENV{MATLAB_DIR}/bin/glnx86/
      )
  ELSE(CMAKE_SIZEOF_VOID_P EQUAL 4)
    # AMD64:
    SET(MATLAB_ROOT
      /usr/local/matlab74/bin/glnxa64/
      /opt/matlab74/bin/glnxa64/
      $ENV{HOME}/matlab7_64/bin/glnxa64/
      $ENV{HOME}/matlab74/bin/glnxa64/
      $ENV{HOME}/redhat-matlab/bin/glnxa64/
      $ENV{MATLAB_DIR}/bin/glnxa64/
      )
  ENDIF(CMAKE_SIZEOF_VOID_P EQUAL 4)
  FIND_LIBRARY(MATLAB_MEX_LIBRARY
    NAMES mex
    PATHS ${MATLAB_ROOT}
    )
  FIND_LIBRARY(MATLAB_MX_LIBRARY
    NAMES mx
    PATHS ${MATLAB_ROOT}
    )
  FIND_LIBRARY(MATLAB_ENG_LIBRARY
    NAMES eng
    PATHS ${MATLAB_ROOT}
    )
  FIND_PATH(MATLAB_LIB_DIR
    NAMES libmex.so
    PATHS ${MATLAB_ROOT}
    )
  FIND_PATH(MATLAB_INCLUDE_DIR
    NAMES mex.h
    PATHS /usr/local/matlab74/extern/include/
    /opt/matlab74/extern/include/
    $ENV{HOME}/matlab74/extern/include/
    $ENV{HOME}/redhat-matlab/extern/include/
    $ENV{MATLAB_DIR}/extern/include
    )

ENDIF(WIN32)

# This is common to UNIX and Win32:
SET(MATLAB_LIBRARIES
  ${MATLAB_MEX_LIBRARY}
  ${MATLAB_MX_LIBRARY}
  ${MATLAB_ENG_LIBRARY}
)
SET(MATLAB_INCLUDE_DIRS ${MATLAB_INCLUDE_DIR} )
SET(MATLAB_LIB_DIRS ${MATLAB_LIB_DIR} )

IF(MATLAB_INCLUDE_DIRS AND MATLAB_LIBRARIES)
  SET(MATLAB_FOUND 1)
  MESSAGE(STATUS "Found Matlab")
  MESSAGE(STATUS "MATLAB_LIB_DIRS - ${MATLAB_LIB_DIRS}")
  MESSAGE(STATUS "MATLAB_INCLUDE_DIRS - ${MATLAB_INCLUDE_DIRS}")
  MESSAGE(STATUS "MATLAB_LIBRARIES - ${MATLAB_LIBRARIES}")
ELSE(MATLAB_INCLUDE_DIRS AND MATLAB_LIBRARIES)
  MESSAGE(STATUS "Matlab not found")
ENDIF(MATLAB_INCLUDE_DIRS AND MATLAB_LIBRARIES)

MARK_AS_ADVANCED(
  MATLAB_MEX_LIBRARY
  MATLAB_MX_LIBRARY
  MATLAB_ENG_LIBRARY
  MATLAB_INCLUDE_DIR
  MATLAB_LIB_DIR
  MATLAB_FOUND
  MATLAB_ROOT
)


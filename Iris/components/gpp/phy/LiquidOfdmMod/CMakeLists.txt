MESSAGE(STATUS "  Processing liquidofdmmod.")

########################################################################
# Add includes and dependencies
########################################################################

FIND_PACKAGE(LIQUIDDSP REQUIRED)
FIND_PACKAGE(FFTW3F)

########################################################################
# Build the library from source files
########################################################################
SET(sources
  LiquidOfdmModComponent.cpp
)

IF (LIQUIDDSP_FOUND AND FFTW3F_FOUND)
  # Static library to be used in tests
  ADD_LIBRARY(comp_gpp_phy_liquidofdmmod_static STATIC ${sources})
  TARGET_LINK_LIBRARIES(comp_gpp_phy_liquidofdmmod_static ${LIQUIDDSP_LIBRARIES} ${FFTW3F_LIBRARY})

  ADD_LIBRARY(comp_gpp_phy_liquidofdmmod SHARED ${sources})
  TARGET_LINK_LIBRARIES(comp_gpp_phy_liquidofdmmod ${LIQUIDDSP_LIBRARIES} ${FFTW3F_LIBRARY})
  SET_TARGET_PROPERTIES(comp_gpp_phy_liquidofdmmod PROPERTIES OUTPUT_NAME "liquidofdmmod")
  IRIS_INSTALL(comp_gpp_phy_liquidofdmmod)
  IRIS_APPEND_INSTALL_LIST(liquidofdmmod)
  # Add the test directory
  ADD_SUBDIRECTORY(test)
ELSE (LIQUIDDSP_FOUND AND FFTW3F_FOUND)
  IRIS_APPEND_NOINSTALL_LIST(liquidofdmmod)
ENDIF (LIQUIDDSP_FOUND AND FFTW3F_FOUND)

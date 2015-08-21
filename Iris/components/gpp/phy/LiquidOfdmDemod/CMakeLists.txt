MESSAGE(STATUS "  Processing liquidofdmdemod.")

########################################################################
# Add includes and dependencies
########################################################################

FIND_PACKAGE(LIQUIDDSP REQUIRED)
FIND_PACKAGE(FFTW3F)

########################################################################
# Build the library from source files
########################################################################
SET(sources
    LiquidOfdmDemodComponent.cpp
)

IF (LIQUIDDSP_FOUND AND FFTW3F_FOUND)
  # Static library to be used in tests
  ADD_LIBRARY(comp_gpp_phy_liquidofdmdemod_static STATIC ${sources})
  TARGET_LINK_LIBRARIES(comp_gpp_phy_liquidofdmdemod_static ${LIQUIDDSP_LIBRARIES} ${FFTW3F_LIBRARY})

  ADD_LIBRARY(comp_gpp_phy_liquidofdmdemod SHARED ${sources})
  TARGET_LINK_LIBRARIES(comp_gpp_phy_liquidofdmdemod ${LIQUIDDSP_LIBRARIES} ${FFTW3F_LIBRARY})
  SET_TARGET_PROPERTIES(comp_gpp_phy_liquidofdmdemod PROPERTIES OUTPUT_NAME "liquidofdmdemod")
  ADD_DEPENDENCIES(comp_gpp_phy_liquidofdmdemod liquid-dsp)
  IRIS_INSTALL(comp_gpp_phy_liquidofdmdemod)
  IRIS_APPEND_INSTALL_LIST(liquidofdmdemod)
  # Add the test directory
  ADD_SUBDIRECTORY(test)
ELSE (LIQUIDDSP_FOUND AND FFTW3F_FOUND)
  IRIS_APPEND_NOINSTALL_LIST(liquidofdmdemod)
ENDIF (LIQUIDDSP_FOUND AND FFTW3F_FOUND)

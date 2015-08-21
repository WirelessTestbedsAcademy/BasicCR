MESSAGE(STATUS "Building kissfft.")

########################################################################
# Build the library from source files
########################################################################
SET(sources
	kiss_fft.c
)

ADD_LIBRARY(kissfft STATIC ${sources})
IRIS_SET_PIC(kissfft)

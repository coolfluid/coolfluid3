find_path(MPFR_INCLUDE_DIRS NAMES mpfr.h)
find_library(MPFR_LIBRARIES NAMES mpfr libmpfr-4 libmpfr-1)

if(MPFR_LIBRARIES)
  set(MPFR_FOUND TRUE)
endif()
  
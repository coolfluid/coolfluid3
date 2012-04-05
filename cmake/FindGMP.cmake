find_path(GMP_INCLUDE_DIRS NAMES gmp.h)
find_library(GMP_LIBRARIES NAMES gmp libgmp-10)

if(GMP_LIBRARIES)
  set(GMP_FOUND TRUE)
endif()
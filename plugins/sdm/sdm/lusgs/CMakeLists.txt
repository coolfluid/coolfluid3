list( APPEND coolfluid_sdm_lusgs_files 
  LUSGS.hpp
  LUSGS.cpp
  LibLUSGS.hpp
  LibLUSGS.cpp
)
list( APPEND coolfluid_sdm_lusgs_cflibs coolfluid_sdm)

coolfluid_add_library( coolfluid_sdm_lusgs )

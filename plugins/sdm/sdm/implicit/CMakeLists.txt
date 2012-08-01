list( APPEND coolfluid_sdm_implicit_files 
  BackwardEuler.hpp
  BackwardEuler.cpp
  BDF2.hpp
  BDF2.cpp
  LibImplicit.hpp
  LibImplicit.cpp
)
list( APPEND coolfluid_sdm_implicit_cflibs coolfluid_sdm)

coolfluid_add_library( coolfluid_sdm_implicit )

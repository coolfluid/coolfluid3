list( APPEND coolfluid_rdm_schemes_files
  LibSchemes.cpp
  LibSchemes.hpp
  # steady RD schemes
  LDA.hpp
  LDA.cpp
  LF.hpp
  LF.cpp
  N.hpp
  N.cpp
  B.hpp
  B.cpp
  SUPG.hpp
  SUPG.cpp
  # standalone diffusion term
  CDiffGalerkin.hpp
  CDiffGalerkin.cpp
  # unsteady RKRD schemes
  RKLDA.hpp
  RKLDA.cpp
)

list( APPEND coolfluid_rdm_schemes_cflibs coolfluid_rdm )

coolfluid_add_library( coolfluid_rdm_schemes )

list( APPEND ns_implementation_files_no_debug
  NavierStokes.cpp
  NavierStokesExplicit.cpp
  NavierStokesExplicitHexasPressureRHS.cpp
  NavierStokesExplicitHexasPressureMatrix.cpp
  NavierStokesExplicitHexasPressureGradient.cpp
  NavierStokesExplicitHexasVelocity.cpp
  NavierStokesExplicitHexasVelocityImplicit.cpp
  NavierStokesExplicitQuadsPressureRHS.cpp
  NavierStokesExplicitQuadsPressureMatrix.cpp
  NavierStokesExplicitQuadsPressureGradient.cpp
  NavierStokesExplicitQuadsVelocity.cpp
  NavierStokesExplicitQuadsVelocityImplicit.cpp
  NavierStokesExplicitTetrasPressureRHS.cpp
  NavierStokesExplicitTetrasPressureMatrix.cpp
  NavierStokesExplicitTetrasPressureGradient.cpp
  NavierStokesExplicitTetrasVelocity.cpp
  NavierStokesExplicitTetrasVelocityImplicit.cpp
  NavierStokesExplicitTriagsPressureRHS.cpp
  
  NavierStokesExplicitTriagsPressureGradient.cpp
  NavierStokesExplicitTriagsVelocity.cpp
  NavierStokesExplicitTriagsVelocityImplicit.cpp
  NavierStokesHexas.cpp
  NavierStokesQuads.cpp
  NavierStokesTetras.cpp
  NavierStokesTriags.cpp
)
list( APPEND ns_implementation_files_with_debug
NavierStokesExplicitTriagsPressureMatrix.cpp
)

list( APPEND ns_implementation_files ${ns_implementation_files_no_debug} ${ns_implementation_files_with_debug})

add_library(cf3_ufem_ns_implementation OBJECT ${ns_implementation_files})

coolfluid_mark_not_orphan(${ns_implementation_files})

set_source_files_properties(${ns_implementation_files_with_debug} PROPERTIES COMPILE_FLAGS "-fPIC")
set_source_files_properties(${ns_implementation_files_no_debug} PROPERTIES COMPILE_FLAGS "-g0 -fPIC")

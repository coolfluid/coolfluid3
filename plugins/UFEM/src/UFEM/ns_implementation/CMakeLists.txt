list( APPEND ns_implementation_files
  NavierStokes.cpp
  NavierStokesExplicit.cpp
  NavierStokesExplicitHexasPressure.cpp
  NavierStokesExplicitHexasPressureGradient.cpp
  NavierStokesExplicitHexasVelocity.cpp
  NavierStokesExplicitHexasVelocityImplicit.cpp
  NavierStokesExplicitQuadsPressure.cpp
  NavierStokesExplicitQuadsPressureGradient.cpp
  NavierStokesExplicitQuadsVelocity.cpp
  NavierStokesExplicitQuadsVelocityImplicit.cpp
  NavierStokesExplicitTetrasPressure.cpp
  NavierStokesExplicitTetrasPressureGradient.cpp
  NavierStokesExplicitTetrasVelocity.cpp
  NavierStokesExplicitTetrasVelocityImplicit.cpp
  NavierStokesExplicitTriagsPressure.cpp
  NavierStokesExplicitTriagsPressureGradient.cpp
  NavierStokesExplicitTriagsVelocity.cpp
  NavierStokesExplicitTriagsVelocityImplicit.cpp
  NavierStokesHexas.cpp
  NavierStokesQuads.cpp
  NavierStokesTetras.cpp
  NavierStokesTriags.cpp
)

add_library(cf3_ufem_ns_implementation OBJECT ${ns_implementation_files})

coolfluid_mark_not_orphan(${ns_implementation_files})

set_source_files_properties(${ns_implementation_files} PROPERTIES COMPILE_FLAGS "-g0 -fPIC")
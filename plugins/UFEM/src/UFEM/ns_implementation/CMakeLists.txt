list( APPEND ns_implementation_files
  NavierStokes.cpp
  NavierStokesExplicit.cpp
  NavierStokesExplicitQuadsPressure.cpp
  NavierStokesExplicitQuadsPressureGradient.cpp
  NavierStokesExplicitQuadsVelocity.cpp
  NavierStokesExplicitQuadsVelocityImplicit.cpp
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

set_source_files_properties(${ns_implementation_files} PROPERTIES COMPILE_FLAGS "-g0")
list(APPEND coolfluid_field_generation_files
  LibFieldGeneration.cpp
  LibFieldGeneration.hpp
  FieldGeneration.cpp
  FieldGeneration.hpp
)

list( APPEND coolfluid_field_generation_cflibs coolfluid_mesh coolfluid_solver_actions)

if( CF_ENABLE_PROFILING AND coolfluid_googleperftools_builds )
  list( APPEND coolfluid_field_generation_cflibs coolfluid_googleperftools )
endif()

set( coolfluid_field_generation_condition ${CF_ENABLE_PROTO} )

coolfluid_add_library( coolfluid_field_generation )



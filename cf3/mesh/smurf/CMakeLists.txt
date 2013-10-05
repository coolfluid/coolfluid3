list( APPEND coolfluid_mesh_smurf_files
  Writer.hpp
  Writer.cpp
  Reader.hpp
  Reader.cpp
  LibSmurf.cpp
  LibSmurf.hpp
  Shared.cpp
  Shared.hpp
  
  smurf/smurf.cpp
  smurf/smurf.h
)

coolfluid3_add_library( TARGET  coolfluid_mesh_smurf
                        KERNEL
                        SOURCES ${coolfluid_mesh_smurf_files} 
                        LIBS    coolfluid_mesh )

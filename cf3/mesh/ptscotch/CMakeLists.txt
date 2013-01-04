list( APPEND coolfluid_mesh_ptscotch_files
      Partitioner.hpp
      Partitioner.cpp
      LibPTScotch.cpp
      LibPTScotch.hpp
    )

coolfluid3_add_library( TARGET    coolfluid_mesh_ptscotch
                        KERNEL
                        SOURCES   ${coolfluid_mesh_ptscotch_files}
                        LIBS      coolfluid_mesh
                                  ${PTSCOTCH_LIBRARIES}
                        INCLUDES  ${PTSCOTCH_INCLUDE_DIRS} 
                        CONDITION ${CF3_HAVE_PTSCOTCH} )


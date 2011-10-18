
coolfluid_add_test( UTEST      utest-mesh-generation
                    CPP        utest-mesh-generation.cpp
                    ARGUMENTS  ${CMAKE_CURRENT_SOURCE_DIR}/meshes
                    LIBS       coolfluid_mesh_generation coolfluid_mesh_lagrangep1 coolfluid_mesh
                    CONDITION  coolfluid_mesh_generation_builds )


coolfluid_add_test( UTEST utest-tools-growl
                    CPP   utest-tools-growl.cpp
                    LIBS  coolfluid_tools_growl )

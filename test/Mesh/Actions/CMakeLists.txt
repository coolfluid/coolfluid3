################################################################################

list( APPEND utest-mesh-actions-facebuilder_cflibs coolfluid_mesh_actions )
list( APPEND utest-mesh-actions-facebuilder_files  utest-mesh-actions-facebuilder.cpp )

coolfluid_add_unittest( utest-mesh-actions-facebuilder )

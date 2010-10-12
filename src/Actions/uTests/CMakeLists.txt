
list( APPEND uTestActions_cflibs Actions SF Gmsh Neu)
list( APPEND uTestActions_files
  uTestActions.cpp )

coolfluid_add_unittest( uTestActions )

# Custom post-build rule (must be defined after adding target):
# Copy the file rotation.neu from source dir to binary dir.
# this must happen after building, as some IDE like Xcode define their
# build types internally, in which the binary dir exists
add_custom_command(TARGET uTestActions
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/rotation.neu ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
                  )

################################################################################

list( APPEND uTestProtoOperators_cflibs Mesh Actions SF BlockMesh Testing MeshGeneration )
list( APPEND uTestProtoOperators_files
  uTestProtoOperators.cpp )
set( uTestProtoOperators_profile ON )
coolfluid_add_unittest( uTestProtoOperators )

################################################################################
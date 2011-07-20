coolfluid_find_orphan_files()

list( APPEND sandbox-lss-trilinos_files
  lss-trilinos.cpp
  lss-trilinos.hpp
  lss-interface.hpp
  blockaccumulator.hpp
  test_matrix.hpp
)

list( APPEND sandbox-lss-trilinos_includedirs ${TRILINOS_INCLUDE_DIR} )
list( APPEND sandbox-lss-trilinos_libs ${TRILINOS_LIBRARIES})
list( APPEND sandbox-lss-trilinos_cflibs coolfluid_common)

set( sandbox-lss-trilinos_sandbox_app TRUE )
#set( sandbox-lss-trilinos_condition ${CF_HAVE_ZOLTAN} )

coolfluid_add_application( sandbox-lss-trilinos )



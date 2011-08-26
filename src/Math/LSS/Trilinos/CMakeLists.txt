coolfluid_find_orphan_files()

list( APPEND coolfluid_lss_trilinos_files
)

list( APPEND coolfluid_lss_trilinos_libs coolfluid_common_libs ${CF_BOOST_LIBRARIES} ${CF_TRILINOS_LIBRARIES})

set( coolfluid_lss_trilinos_kernellib TRUE )

coolfluid_add_library( coolfluid_lss_trilinos )
set_target_properties( coolfluid_lss_trilinos PROPERTIES LINKER_LANGUAGE CXX )


coolfluid_find_orphan_files()

list( APPEND coolfluid_lssinterface_trilinos_files
)

list( APPEND coolfluid_lssinterface_trilinos_libs coolfluid_common_libs ${CF_BOOST_LIBRARIES} ${CF_TRILINOS_LIBRARIES})

set( coolfluid_lssinterface_trilinos_kernellib TRUE )

coolfluid_add_library( coolfluid_lssinterface_trilinos )


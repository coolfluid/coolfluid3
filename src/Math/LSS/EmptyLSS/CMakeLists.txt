coolfluid_find_orphan_files()

list( APPEND coolfluid_lss_emptylss_files
  EmptyLSSVector.hpp
  EmptyLSSMatrix.hpp
)

list( APPEND coolfluid_lss_emptylss_libs coolfluid_common_libs ${CF_BOOST_LIBRARIES} ${CF_emptylss_LIBRARIES})

set( coolfluid_lss_emptylss_kernellib TRUE )

coolfluid_add_library( coolfluid_lss_emptylss )
set_target_properties( coolfluid_lss_emptylss PROPERTIES LINKER_LANGUAGE CXX )


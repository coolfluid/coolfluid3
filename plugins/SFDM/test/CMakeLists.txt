list( APPEND utest-sfdm-spaces_cflibs coolfluid_sfdm_core coolfluid_sfdm_sf coolfluid_mesh )
list( APPEND utest-sfdm-spaces_files  utest-sfdm-spaces.cpp )

coolfluid_add_unit_test( utest-sfdm-spaces )


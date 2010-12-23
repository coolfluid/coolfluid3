list( APPEND utest-rdm-steady_cflibs coolfluid_rdm coolfluid_neutral )
list( APPEND utest-rdm-steady_files  utest-rdm-steady.cpp )

coolfluid_add_unittest( utest-rdm-steady )

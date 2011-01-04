list( APPEND utest-fvm-steady_cflibs coolfluid_fvm coolfluid_mesh_neutral )
list( APPEND utest-fvm-steady_files  utest-fvm-steady.cpp )

coolfluid_add_unittest( utest-fvm-steady )

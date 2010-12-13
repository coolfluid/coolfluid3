
list( APPEND utest-cphysical-model_cflibs coolfluid_solver )
list( APPEND utest-cphysical-model_files
  uTestCPhysicalModel.cpp )

coolfluid_add_unittest( utest-cphysical-model )

#########################################################################

list( APPEND utest-forwardeuler_cflibs coolfluid_solver )
list( APPEND utest-forwardeuler_files
  utest-forwardeuler.cpp )

coolfluid_add_unittest( utest-forwardeuler )
